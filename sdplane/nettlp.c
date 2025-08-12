/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#include "include.h"

#include <lthread.h>

#include <rte_common.h>
#include <rte_launch.h>
#include <rte_ether.h>
#include <rte_malloc.h>

#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>

#include <sdplane/debug.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "rib_manager.h"
#include "sdplane.h"
#include "thread_info.h"

#include "l2fwd_export.h"

#include "internal_message.h"

#include "log_packet.h"

#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

extern int lthread_core;
extern volatile bool force_stop[RTE_MAX_LCORE];

extern struct rte_eth_dev_tx_buffer
    *tx_buffer_per_q[RTE_MAX_ETHPORTS][RTE_MAX_LCORE];

struct rte_ring *msg_queue_nettlp;

static __thread unsigned lcore_id;
static __thread uint64_t loop_counter = 0;
static __thread struct rib *rib = NULL;

static inline __attribute__ ((always_inline)) void
nettlp_send_packet_tap_up (struct rte_mbuf *m, unsigned portid,
                           unsigned queueid)
{
  struct rte_mbuf *c;
  uint32_t pkt_len;
  uint16_t data_len;
  int ret = 0;
  pkt_len = rte_pktmbuf_pkt_len (m);
  data_len = rte_pktmbuf_data_len (m);

  DEBUG_SDPLANE_LOG (NETTLP,
                     "lcore[%d]: m: %p port %d queue %d to ring_up: %p",
                     lcore_id, m, portid, queueid, ring_up[portid][queueid]);
  c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
  if (ring_up[portid][queueid])
    ret = rte_ring_enqueue (ring_up[portid][queueid], c);
  if (ret)
    {
      DEBUG_SDPLANE_LOG (NETTLP,
                         "lcore[%d]: m: %p port %d queue %d to ring: %d",
                         lcore_id, m, portid, queueid, ret);
      rte_pktmbuf_free (c);
    }
}

#include "nettlp.h"
#include "nettlp_support.h"

int rx_portid = -1;
int rx_queueid = -1;
int tx_portid;
struct rte_ether_addr local_ether;
struct rte_ether_addr remote_ether;
struct in_addr local_addr;
struct in_addr remote_addr;
uint16_t src_port;
uint16_t dst_port;
uint16_t bus_number;
uint16_t dev_number;
uint8_t pci_tag;       /* PCI tag value */
uintptr_t memory_addr; /* DMA memory address */
int payload_size;
int max_payload_size; /*Max Payload Size */
char payload_string[4096];
int read_payload_size = 4;
uint16_t requester;

uintptr_t psmem_addr = 0; /* Pseudo memory base address (BAR4) */
char *psmem_memory = NULL;
// uint32_t psmem_size = 256 * 1024 * 1024; //256MiB.
uint32_t psmem_size = 8 * 1024; // 8KiB.

void
nettlp_send_dma_write ()
{
  uint16_t tx_queueid;
  struct rte_mbuf *m;
  uint16_t udp_length;
  uint16_t length;
  struct rte_ether_hdr *eth;
  struct rte_ipv4_hdr *ipv4;
  struct rte_udp_hdr *udp;
  struct rte_eth_dev_tx_buffer *buffer;

  struct nettlp_hdr *nh;
  struct tlp_mr_hdr *mh;

  tx_queueid = lcore_id;
  DEBUG_SDPLANE_LOG (NETTLP, "send DMA write: to port: %d queue %d.",
                     tx_portid, tx_queueid);

  if (! rib || ! rib->rib_info)
    {
      DEBUG_SDPLANE_LOG (NETTLP, "rib not yet.");
      return;
    }

  if (tx_portid >= rib->rib_info->port_size ||
      tx_queueid >= rib->rib_info->port[tx_portid].dev_info.nb_tx_queues)
    {
      DEBUG_SDPLANE_LOG (NETTLP, "port %d queue %d not yet.", tx_portid,
                         tx_queueid);
      return;
    }

  m = rte_pktmbuf_alloc (l2fwd_pktmbuf_pool);

  length = sizeof (struct nettlp_hdr) + sizeof (struct tlp_mr_hdr);
  rte_pktmbuf_append (m, length);
  nh = rte_pktmbuf_mtod (m, struct nettlp_hdr *);
  mh = (struct tlp_mr_hdr *) (nh + 1);

  mh->requester = rte_cpu_to_be_16 (requester);
  mh->tag = pci_tag;
  mh->lstdw = tlp_calculate_lstdw (memory_addr, payload_size);
  mh->fstdw = tlp_calculate_fstdw (memory_addr, payload_size);
  tlp_set_length (mh->tlp.falen,
                  tlp_calculate_length (memory_addr, payload_size));

  uint32_t *dst_addr32;
  uint64_t *dst_addr64;
  uint8_t *memory_addrp;

  tlp_set_type (mh->tlp.fmt_type, TLP_TYPE_MWr);
  if (memory_addr < UINT32_MAX)
    {
      tlp_set_fmt (mh->tlp.fmt_type, TLP_FMT_3DW, TLP_FMT_W_DATA);
      length = sizeof (uint32_t);
      rte_pktmbuf_append (m, length);
      dst_addr32 = (uint32_t *) (mh + 1);
      *dst_addr32 = rte_cpu_to_be_32 (memory_addr & 0xFFFFFFFC);
      memory_addrp = (uint8_t *) dst_addr32;
    }
  else
    {
      tlp_set_fmt (mh->tlp.fmt_type, TLP_FMT_4DW, TLP_FMT_W_DATA);
      length = sizeof (uint64_t);
      rte_pktmbuf_append (m, length);
      dst_addr64 = (uint64_t *) (mh + 1);
      *dst_addr64 = rte_cpu_to_be_64 (memory_addr & 0xFFFFFFFFFFFFFFFC);
      memory_addrp = (uint8_t *) dst_addr64;
    }

  /* XXX:
   *
   * 1st DW BE is used and not 0xF, move the buffer, if 1st DW
   * is xx10, x100, or 1000. It needs padding.
   */
  int n;
  int pad_len;
  uint8_t *pad;

  pad_len = 0;
  pad = (uint8_t *) (memory_addrp + length);
  if (mh->fstdw && mh->fstdw != 0xF)
    {
      for (n = 0; n < 3; n++)
        {
          if ((mh->fstdw & (0x1 << n)) == 0)
            {
              /* this byte is not used. padding! */
              pad_len++;
            }
        }
    }
  if (pad_len)
    {
      DEBUG_SDPLANE_LOG (NETTLP, "pad: fstdw: len %d.", pad_len);
      rte_pktmbuf_append (m, pad_len);
      memset (pad, 0, pad_len);
    }

  uint8_t *data;
  rte_pktmbuf_append (m, payload_size);
  data = (uint8_t *) (pad + pad_len);
  memcpy (data, payload_string, payload_size);

  pad_len = 0;
  pad = (uint8_t *) (data + payload_size);
  if (mh->lstdw && mh->lstdw != 0xF)
    {
      for (n = 0; n < 3; n++)
        {
          if ((mh->lstdw & (0x8 >> n)) == 0)
            {
              /* this byte is not used, padding! */
              pad_len++;
            }
        }
    }
  if (pad_len)
    {
      DEBUG_SDPLANE_LOG (NETTLP, "pad: lstdw: len %d.", pad_len);
      rte_pktmbuf_append (m, pad_len);
      memset (pad, 0, pad_len);
    }

  rte_pktmbuf_prepend (m, sizeof (struct rte_udp_hdr));
  udp = rte_pktmbuf_mtod (m, struct rte_udp_hdr *);
  udp_length = rte_pktmbuf_pkt_len (m);

  udp->src_port = rte_cpu_to_be_16 (src_port);
  udp->dst_port = rte_cpu_to_be_16 (dst_port);
  udp->dgram_len = rte_cpu_to_be_16 (udp_length);

  rte_pktmbuf_prepend (m, sizeof (struct rte_ipv4_hdr));
  ipv4 = rte_pktmbuf_mtod (m, struct rte_ipv4_hdr *);
  length = rte_pktmbuf_pkt_len (m);

  ipv4->version_ihl = 0x45;
  ipv4->total_length = rte_cpu_to_be_16 (length);
  ipv4->packet_id = random ();
  ipv4->src_addr = local_addr.s_addr;
  ipv4->dst_addr = remote_addr.s_addr;
  ipv4->next_proto_id = IPPROTO_UDP;
  ipv4->hdr_checksum = rte_ipv4_cksum (ipv4);

  rte_pktmbuf_prepend (m, sizeof (struct rte_ether_hdr));
  eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);

  eth->ether_type = rte_cpu_to_be_16 (RTE_ETHER_TYPE_IPV4);
  rte_ether_addr_copy (&local_ether, &eth->src_addr);
  rte_ether_addr_copy (&remote_ether, &eth->dst_addr);

  if (rx_portid >= 0 && rx_queueid >= 0)
    nettlp_send_packet_tap_up (m, rx_portid, rx_queueid);

  if (! tx_buffer_per_q[tx_portid][tx_queueid])
    {
      tx_buffer_per_q[tx_portid][tx_queueid] = rte_zmalloc_socket (
          "tx_buffer", RTE_ETH_TX_BUFFER_SIZE (MAX_PKT_BURST), 0,
          rte_eth_dev_socket_id (tx_portid));
      rte_eth_tx_buffer_init (tx_buffer_per_q[tx_portid][tx_queueid],
                              MAX_PKT_BURST);
    }

  buffer = tx_buffer_per_q[tx_portid][tx_queueid];

  if (buffer)
    {
      rte_eth_tx_buffer (tx_portid, tx_queueid, buffer, m);
      rte_eth_tx_buffer_flush (tx_portid, tx_queueid, buffer);
      DEBUG_SDPLANE_LOG (NETTLP, "sent.");
    }
  else
    DEBUG_SDPLANE_LOG (NETTLP, "no tx buffer.");
}

void
nettlp_send_dma_read ()
{
  uint16_t tx_queueid;
  struct rte_mbuf *m;
  uint16_t udp_length;
  uint16_t length;
  struct rte_ether_hdr *eth;
  struct rte_ipv4_hdr *ipv4;
  struct rte_udp_hdr *udp;
  struct rte_eth_dev_tx_buffer *buffer;

  struct nettlp_hdr *nh;
  struct tlp_mr_hdr *mh;

  tx_queueid = lcore_id;
  DEBUG_SDPLANE_LOG (NETTLP, "send DMA read: to port: %d queue %d.", tx_portid,
                     tx_queueid);

  if (! rib || ! rib->rib_info)
    {
      DEBUG_SDPLANE_LOG (NETTLP, "rib not yet.");
      return;
    }

  if (tx_portid >= rib->rib_info->port_size ||
      tx_queueid >= rib->rib_info->port[tx_portid].dev_info.nb_tx_queues)
    {
      DEBUG_SDPLANE_LOG (NETTLP, "port %d queue %d not yet.", tx_portid,
                         tx_queueid);
      return;
    }

  m = rte_pktmbuf_alloc (l2fwd_pktmbuf_pool);

  length = sizeof (struct nettlp_hdr) + sizeof (struct tlp_mr_hdr);
  rte_pktmbuf_append (m, length);
  nh = rte_pktmbuf_mtod (m, struct nettlp_hdr *);
  mh = (struct tlp_mr_hdr *) (nh + 1);

  mh->requester = rte_cpu_to_be_16 (requester);
  mh->tag = pci_tag;
  mh->lstdw = tlp_calculate_lstdw (memory_addr, read_payload_size);
  mh->fstdw = tlp_calculate_fstdw (memory_addr, read_payload_size);
  tlp_set_length (mh->tlp.falen,
                  tlp_calculate_length (memory_addr, read_payload_size));

  uint32_t *dst_addr32;
  uint64_t *dst_addr64;
  uint8_t *memory_addrp;

  tlp_set_type (mh->tlp.fmt_type, TLP_TYPE_MRd);
  if (memory_addr < UINT32_MAX)
    {
      tlp_set_fmt (mh->tlp.fmt_type, TLP_FMT_3DW, TLP_FMT_WO_DATA);
      length = sizeof (uint32_t);
      rte_pktmbuf_append (m, length);
      dst_addr32 = (uint32_t *) (mh + 1);
      *dst_addr32 = rte_cpu_to_be_32 (memory_addr & 0xFFFFFFFC);
      memory_addrp = (uint8_t *) dst_addr32;
    }
  else
    {
      tlp_set_fmt (mh->tlp.fmt_type, TLP_FMT_4DW, TLP_FMT_WO_DATA);
      length = sizeof (uint64_t);
      rte_pktmbuf_append (m, length);
      dst_addr64 = (uint64_t *) (mh + 1);
      *dst_addr64 = rte_cpu_to_be_64 (memory_addr & 0xFFFFFFFFFFFFFFFC);
      memory_addrp = (uint8_t *) dst_addr64;
    }

#if 0
  /* XXX:
   *
   * 1st DW BE is used and not 0xF, move the buffer, if 1st DW
   * is xx10, x100, or 1000. It needs padding.
   */
  int n;
  int pad_len;
  uint8_t *pad;

  pad_len = 0;
  pad = (uint8_t *) (memory_addrp + length);
  if (mh->fstdw && mh->fstdw != 0xF)
    {
      for (n = 0; n < 3; n++)
        {
          if ((mh->fstdw & (0x1 << n)) == 0)
            {
              /* this byte is not used. padding! */
              pad_len++;
            }
        }
    }
  if (pad_len)
    {
      rte_pktmbuf_append (m, pad_len);
      memset (pad, 0, pad_len);
    }

  uint8_t *data;
  rte_pktmbuf_append (m, size);
  data = (uint8_t *) (pad + pad_len);
  memcpy (data, payload_string, size);

  pad_len = 0;
  pad = (uint8_t *) (data + size);
  if (mh->lstdw && mh->lstdw != 0xF)
    {
      for (n = 0; n < 3; n++)
        {
          if ((mh->lstdw & (0x8 >> n)) == 0)
            {
              /* this byte is not used, padding! */
              pad_len++;
            }
        }
    }
  if (pad_len)
    {
      rte_pktmbuf_append (m, pad_len);
      memset (pad, 0, pad_len);
    }
#endif

  rte_pktmbuf_prepend (m, sizeof (struct rte_udp_hdr));
  udp = rte_pktmbuf_mtod (m, struct rte_udp_hdr *);
  udp_length = rte_pktmbuf_pkt_len (m);

  udp->src_port = rte_cpu_to_be_16 (src_port);
  udp->dst_port = rte_cpu_to_be_16 (dst_port);
  udp->dgram_len = rte_cpu_to_be_16 (udp_length);

  rte_pktmbuf_prepend (m, sizeof (struct rte_ipv4_hdr));
  ipv4 = rte_pktmbuf_mtod (m, struct rte_ipv4_hdr *);
  length = rte_pktmbuf_pkt_len (m);

  ipv4->version_ihl = 0x45;
  ipv4->total_length = rte_cpu_to_be_16 (length);
  ipv4->src_addr = local_addr.s_addr;
  ipv4->dst_addr = remote_addr.s_addr;
  ipv4->next_proto_id = IPPROTO_UDP;
  ipv4->hdr_checksum = rte_ipv4_cksum (ipv4);

  rte_pktmbuf_prepend (m, sizeof (struct rte_ether_hdr));
  eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);

  eth->ether_type = rte_cpu_to_be_16 (RTE_ETHER_TYPE_IPV4);
  rte_ether_addr_copy (&local_ether, &eth->src_addr);
  rte_ether_addr_copy (&remote_ether, &eth->dst_addr);

  if (rx_portid >= 0 && rx_queueid >= 0)
    nettlp_send_packet_tap_up (m, rx_portid, rx_queueid);

  if (! tx_buffer_per_q[tx_portid][tx_queueid])
    {
      tx_buffer_per_q[tx_portid][tx_queueid] = rte_zmalloc_socket (
          "tx_buffer", RTE_ETH_TX_BUFFER_SIZE (MAX_PKT_BURST), 0,
          rte_eth_dev_socket_id (tx_portid));
      rte_eth_tx_buffer_init (tx_buffer_per_q[tx_portid][tx_queueid],
                              MAX_PKT_BURST);
    }

  buffer = tx_buffer_per_q[tx_portid][tx_queueid];

  if (buffer)
    {
      rte_eth_tx_buffer (tx_portid, tx_queueid, buffer, m);
      rte_eth_tx_buffer_flush (tx_portid, tx_queueid, buffer);
      DEBUG_SDPLANE_LOG (NETTLP, "sent.");
    }
  else
    DEBUG_SDPLANE_LOG (NETTLP, "no tx buffer.");
}

static inline __attribute__ ((always_inline)) void
nettlp_internal_msg_recv ()
{
  void *msgp;

  msgp = internal_msg_recv (msg_queue_nettlp);
  if (msgp)
    {
      struct internal_msg_header *imsghdr;
      imsghdr = (struct internal_msg_header *) msgp;

      switch (imsghdr->type)
        {
        case INTERNAL_MSG_TYPE_NETTLP_SEND_DMA_WRITE:
          nettlp_send_dma_write ();
          break;

        case INTERNAL_MSG_TYPE_NETTLP_SEND_DMA_READ:
          nettlp_send_dma_read ();
          break;

        default:
          break;
        }

      internal_msg_delete (msgp);
    }
}

static inline __attribute__ ((always_inline)) void
nettlp_udp_send (struct rte_mbuf *m)
{
  struct rte_udp_hdr *udp;
  uint16_t udp_length;
  struct rte_ipv4_hdr *ipv4;
  uint16_t length;
  struct rte_ether_hdr *eth;
  struct rte_eth_dev_tx_buffer *buffer;

  uint16_t tx_queueid = lcore_id;
  uint16_t tx_portid = rx_portid;

  rte_pktmbuf_prepend (m, sizeof (struct rte_udp_hdr));
  udp = rte_pktmbuf_mtod (m, struct rte_udp_hdr *);
  udp_length = rte_pktmbuf_pkt_len (m);

  udp->src_port = rte_cpu_to_be_16 (src_port);
  udp->dst_port = rte_cpu_to_be_16 (dst_port);
  udp->dgram_len = rte_cpu_to_be_16 (udp_length);

  rte_pktmbuf_prepend (m, sizeof (struct rte_ipv4_hdr));
  ipv4 = rte_pktmbuf_mtod (m, struct rte_ipv4_hdr *);
  length = rte_pktmbuf_pkt_len (m);

  ipv4->version_ihl = 0x45;
  ipv4->total_length = rte_cpu_to_be_16 (length);
  ipv4->src_addr = local_addr.s_addr;
  ipv4->dst_addr = remote_addr.s_addr;
  ipv4->next_proto_id = IPPROTO_UDP;
  ipv4->hdr_checksum = rte_ipv4_cksum (ipv4);

  rte_pktmbuf_prepend (m, sizeof (struct rte_ether_hdr));
  eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);

  eth->ether_type = rte_cpu_to_be_16 (RTE_ETHER_TYPE_IPV4);
  rte_ether_addr_copy (&local_ether, &eth->src_addr);
  rte_ether_addr_copy (&remote_ether, &eth->dst_addr);

  if (rx_portid >= 0 && rx_queueid >= 0)
    nettlp_send_packet_tap_up (m, rx_portid, rx_queueid);

  buffer = tx_buffer_per_q[tx_portid][tx_queueid];
  if (buffer)
    {
      rte_eth_tx_buffer (tx_portid, tx_queueid, buffer, m);
      rte_eth_tx_buffer_flush (tx_portid, tx_queueid, buffer);
      DEBUG_SDPLANE_LOG (NETTLP, "sent.");
    }
  else
    DEBUG_SDPLANE_LOG (NETTLP, "no tx buffer.");
}

static inline __attribute__ ((always_inline)) void
nettlp_psmem_mrd (struct nettlp_hdr *nh_recv, struct tlp_mr_hdr *mh)
{
  ssize_t len = tlp_length (mh->tlp.falen) << 2;
  ssize_t data_len = tlp_mr_data_length (mh);
  uintptr_t addr = tlp_mr_addr (mh);

  DEBUG_SDPLANE_LOG (NETTLP, "MRd to 0x%lx, tag 0x%02x %lu byte",
                     (addr >> 2) << 2, mh->tag, data_len);

  if (addr < psmem_addr || addr + len > psmem_addr + psmem_size)
    {
      DEBUG_SDPLANE_LOG (NETTLP,
                         "MRd request to 0x%lx which goes "
                         "out of the pseudo memory region: %lx -- %lx.",
                         addr, psmem_addr, psmem_addr + psmem_size);
      return;
    }

  struct rte_mbuf *m;
  struct nettlp_hdr *nh;
  struct tlp_cpl_hdr *ch;
  void *data;
  void *from;

#define MAXPAYLOADSIZE 256
  do
    {
      ssize_t send_len;
      send_len = len < MAXPAYLOADSIZE ? len : MAXPAYLOADSIZE;

      m = rte_pktmbuf_alloc (l2fwd_pktmbuf_pool);
      rte_pktmbuf_append (m, sizeof (struct nettlp_hdr));
      rte_pktmbuf_append (m, sizeof (struct tlp_cpl_hdr));
      nh = rte_pktmbuf_mtod (m, struct nettlp_hdr *);
      ch = (struct tlp_cpl_hdr *) (nh + 1);

      memset (nh, 0, sizeof (struct nettlp_hdr));
      memset (ch, 0, sizeof (struct tlp_cpl_hdr));

      /* XXX: copy flag and attribute. should handle properly */
      memcpy (&ch->tlp, &mh->tlp, sizeof (struct tlp_cpl_hdr));

      /* Build CplD header */
      tlp_set_fmt (ch->tlp.fmt_type, TLP_FMT_3DW, TLP_FMT_W_DATA);
      tlp_set_type (ch->tlp.fmt_type, TLP_TYPE_Cpl);
      tlp_set_length (ch->tlp.falen, send_len >> 2);
      tlp_set_cpl_status (ch->stcnt, TLP_CPL_STATUS_SC);
      tlp_set_cpl_bcnt (ch->stcnt, data_len);
      ch->completer = rte_cpu_to_be_16 (requester);
      ch->requester = mh->requester;
      ch->tag = mh->tag;
      ch->lowaddr = addr & 0x7F;

      /* memory to be sent */
      rte_pktmbuf_append (m, send_len);
      data = (void *) (ch + 1);
      from = psmem_memory + ((addr >> 2) << 2) - psmem_addr;
      memcpy (data, from, send_len);

      nettlp_udp_send (m);

      len -= send_len;
      data_len -= send_len - (addr & 0x3);
      addr = ((addr >> 2) << 2) + send_len;
    }
  while (data_len > 0);
}

static inline __attribute__ ((always_inline)) void
nettlp_psmem_mwr (struct nettlp_hdr *nh, struct tlp_mr_hdr *mh)
{
  uintptr_t addr = tlp_mr_addr (mh);
  void *ptr = tlp_mwr_data (mh);
  int len = tlp_mr_data_length (mh);

  DEBUG_SDPLANE_LOG (NETTLP, "MWr to 0x%lx, tag 0x%02x, %lu byte", addr,
                     mh->tag, len);

  if (addr < psmem_addr || addr + len > psmem_addr + psmem_size)
    {
      DEBUG_SDPLANE_LOG (NETTLP,
                         "MWr request to 0x%lx which goes "
                         "out of the pseudo memory region: %lx -- %lx.",
                         addr, psmem_addr, psmem_addr + psmem_size);
      return;
    }

  memcpy (psmem_memory + (addr - psmem_addr), ptr, len);
}

static inline __attribute__ ((always_inline)) void
nettlp_psmem_cpl (struct nettlp_hdr *nh, struct tlp_cpl_hdr *ch)
{
}

static inline __attribute__ ((always_inline)) void
nettlp_psmem_cpld (struct nettlp_hdr *nh, struct tlp_cpl_hdr *ch)
{
}

static inline __attribute__ ((always_inline)) void
nettlp_psmem_receive (struct rte_mbuf *m)
{
  struct rte_ether_hdr *eth;
  struct rte_ipv4_hdr *ipv4;
  struct rte_udp_hdr *udp;
  struct nettlp_hdr *nh;
  struct tlp_hdr *th;
  struct tlp_mr_hdr *mh;
  struct tlp_cpl_hdr *ch;

  eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  if (memcmp (&eth->dst_addr, &local_ether, RTE_ETHER_ADDR_LEN))
    {
      char eth_dst[32];
      rte_ether_format_addr (eth_dst, sizeof (eth_dst), &eth->dst_addr);
      DEBUG_SDPLANE_LOG (NETTLP, "different ether dst: %s", eth_dst);
    }
  if (rte_be_to_cpu_16 (eth->ether_type) != 0x0800)
    {
      DEBUG_SDPLANE_LOG (NETTLP, "different ether proto: 0x%04hx",
                         rte_be_to_cpu_16 (eth->ether_type));
      return;
    }

  ipv4 = (struct rte_ipv4_hdr *) (eth + 1);
  if (memcmp (&ipv4->dst_addr, &local_addr, sizeof (struct in_addr)))
    {
      char ip_dst[16];
      inet_ntop (AF_INET, &ipv4->dst_addr, ip_dst, sizeof (ip_dst));
      DEBUG_SDPLANE_LOG (NETTLP, "different ipv4 dst: %s", ip_dst);
    }
  if (ipv4->next_proto_id != IPPROTO_UDP)
    {
      DEBUG_SDPLANE_LOG (NETTLP, "non-UDP packet: proto: %d",
                         ipv4->next_proto_id);
      return;
    }

  udp = (struct rte_udp_hdr *) (ipv4 + 1);

  nh = (struct nettlp_hdr *) (udp + 1);
  th = (struct tlp_hdr *) (nh + 1);
  mh = (struct tlp_mr_hdr *) th;
  ch = (struct tlp_cpl_hdr *) th;

  if (tlp_is_mrd (th->fmt_type))
    {
      DEBUG_SDPLANE_LOG (NETTLP, "MRd: addr: %p len: %d", tlp_mr_addr (mh),
                         tlp_mr_data_length (mh));
      nettlp_psmem_mrd (nh, mh);
    }
  else if (tlp_is_mwr (th->fmt_type))
    {
      DEBUG_SDPLANE_LOG (NETTLP, "MWr: data: %p len: %d", tlp_mwr_data (mh),
                         tlp_mr_data_length (mh));
      nettlp_psmem_mwr (nh, mh);
    }
  else if (tlp_is_cpl (th->fmt_type) && tlp_is_wo_data (th->fmt_type))
    {
      DEBUG_SDPLANE_LOG (NETTLP, "Cpl without data");
      nettlp_psmem_cpl (nh, ch);
    }
  else if (tlp_is_cpl (th->fmt_type) && tlp_is_w_data (th->fmt_type))
    {
      DEBUG_SDPLANE_LOG (NETTLP, "Cpl with data: %p len: %d",
                         tlp_cpld_data (ch), tlp_cpld_data_length (ch));
      nettlp_psmem_cpld (nh, ch);
    }
  else
    {
      DEBUG_SDPLANE_LOG (NETTLP, "unknown TLP: fmt_type: %#x", th->fmt_type);
    }

  rte_pktmbuf_free (m);
}

static inline __attribute__ ((always_inline)) void
nettlp_rx_burst ()
{
  struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
  struct rte_mbuf *m;
  unsigned i, j, nb_rx;
  uint16_t portid, queueid;

  if (unlikely (! rib || ! rib->rib_info))
    return;

  struct lcore_qconf *lcore_qconf;
  lcore_qconf = &rib->rib_info->lcore_qconf[lcore_id];

  if (lcore_qconf->nrxq == 0)
    {
      rx_portid = -1;
      rx_queueid = -1;
      return;
    }

  for (i = 0; i < lcore_qconf->nrxq; i++)
    {
      portid = lcore_qconf->rx_queue_list[i].port_id;
      queueid = lcore_qconf->rx_queue_list[i].queue_id;
      rx_portid = portid;
      rx_queueid = queueid;

      nb_rx = 0;
      if (queueid < rib->rib_info->port[portid].dev_info.nb_rx_queues)
        nb_rx = rte_eth_rx_burst (portid, queueid, pkts_burst, MAX_PKT_BURST);
      if (unlikely (nb_rx == 0))
        continue;

      for (j = 0; j < nb_rx; j++)
        {
          m = pkts_burst[j];
          rte_prefetch0 (rte_pktmbuf_mtod (m, void *));

          if (rx_portid >= 0 && rx_queueid >= 0)
            nettlp_send_packet_tap_up (m, rx_portid, rx_queueid);

          // log_packet (m, rx_portid, rx_queueid);
          nettlp_psmem_receive (m);
        }
    }
}

int
nettlp_thread (void *arg)
{
  int ret;

  lcore_id = rte_lcore_id ();

  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);
  DEBUG_SDPLANE_LOG (NETTLP, "%s: started.", __func__);

  /* initialize */
  msg_queue_nettlp =
      rte_ring_create ("msg_queue_nettlp", 32, SOCKET_ID_ANY, RING_F_SC_DEQ);

  int thread_id;
  thread_id = thread_lookup (nettlp_thread);
  thread_register_loop_counter (thread_id, &loop_counter);

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_register_thread ();
#endif /*HAVE_LIBURCU_QSBR*/

  while (! force_quit && ! force_stop[lthread_core])
    {
      // lthread_sleep (100); // yield.
      // DEBUG_SDPLANE_LOG (NETTLP, "%s: schedule.", __func__);

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_lock ();
      rib = (struct rib *) rcu_dereference (rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

      nettlp_internal_msg_recv ();
      nettlp_rx_burst ();

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_unlock ();
      urcu_qsbr_quiescent_state ();
#endif /*HAVE_LIBURCU_QSBR*/

      loop_counter++;
    }

  rte_ring_free (msg_queue_nettlp);

  DEBUG_SDPLANE_LOG (NETTLP, "%s: terminating.", __func__);
  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_unregister_thread ();
#endif /*HAVE_LIBURCU_QSBR*/
}

CLI_COMMAND2 (nettlp_send_dma_write_read, "nettlp-send (dma-write|dma-read)",
              "NetTLP send command\n", "DMA write packet\n",
              "DMA read packet\n")
{
  struct shell *shell = (struct shell *) context;
  void *msgp;
  uint16_t type;
  char buf[8] = { 0 };

  if (! strcmp (argv[1], "dma-write"))
    type = INTERNAL_MSG_TYPE_NETTLP_SEND_DMA_WRITE;
  else if (! strcmp (argv[1], "dma-read"))
    type = INTERNAL_MSG_TYPE_NETTLP_SEND_DMA_READ;

  msgp = internal_msg_create (type, buf, sizeof (buf));
  internal_msg_send_to (msg_queue_nettlp, msgp, shell);
  return 0;
}

CLI_COMMAND2 (show_nettlp, "show nettlp", SHOW_HELP, "NetTLP information.\n")
{
  struct shell *shell = (struct shell *) context;
  char local[32], remote[32];

  fprintf (shell->terminal, "TX port-id: %d.%s", tx_portid, shell->NL);

  rte_ether_format_addr (local, sizeof (local), &local_ether);
  rte_ether_format_addr (remote, sizeof (remote), &remote_ether);
  fprintf (shell->terminal, "local-ether: %s%s", local, shell->NL);
  fprintf (shell->terminal, "remote-ether: %s%s", remote, shell->NL);

  inet_ntop (AF_INET, &local_addr, local, sizeof (local));
  inet_ntop (AF_INET, &remote_addr, remote, sizeof (remote));
  fprintf (shell->terminal, "local-addr: %s%s", local, shell->NL);
  fprintf (shell->terminal, "remote-addr: %s%s", remote, shell->NL);

  fprintf (shell->terminal, "UDP src-port: %hu.%s", src_port, shell->NL);
  fprintf (shell->terminal, "UDP dst-port: %hu.%s", dst_port, shell->NL);

  fprintf (shell->terminal, "bus_number: %hx:%hx.%s", bus_number, dev_number,
           shell->NL);
  fprintf (shell->terminal, "pci-tag: %d.%s", pci_tag, shell->NL);
  fprintf (shell->terminal, "memory-addr: %p.%s", (void *) memory_addr,
           shell->NL);
  fprintf (shell->terminal, "payload-size: %d.%s", payload_size, shell->NL);
  fprintf (shell->terminal, "MaxPayloadSize: %d.%s", max_payload_size,
           shell->NL);
  fprintf (shell->terminal, "payload-string: %s.%s", payload_string,
           shell->NL);
  return 0;
}

CLI_COMMAND2 (set_nettlp_ether_local_remote,
              "set nettlp ether (local-addr|remote-addr) <WORD>", SET_HELP,
              "NetTLP information.\n", "local ethernet source address.\n",
              "remote ethernet destination address.\n",
              "Specify ethernet address.\n")
{
  struct shell *shell = (struct shell *) context;
  struct rte_ether_addr *dst;
  int ret;

  if (! strcmp (argv[3], "local-addr"))
    dst = &local_ether;
  else if (! strcmp (argv[3], "remote-addr"))
    dst = &remote_ether;

  ret = rte_ether_unformat_addr (argv[4], dst);
  if (ret < 0)
    {
      fprintf (shell->terminal, "invalid address format: %s.%s", argv[4],
               shell->NL);
      return -1;
    }

  fprintf (shell->terminal, "set %s: %s%s", argv[3], argv[4], shell->NL);
  return 0;
}


CLI_COMMAND2 (set_nettlp_ipv4_local_remote,
              "set nettlp ipv4 (local-addr|remote-addr) A.B.C.D", SET_HELP,
              "NetTLP information.\n", "IPv information.\n",
              "local IPv4 source address.\n",
              "remote IPv4 destination address.\n", "Specify IPv4 address.\n")
{
  struct shell *shell = (struct shell *) context;
  struct in_addr *dst;
  int ret;

  if (! strcmp (argv[3], "local-addr"))
    dst = &local_addr;
  else if (! strcmp (argv[3], "remote-addr"))
    dst = &remote_addr;

  ret = inet_pton (AF_INET, argv[4], dst);
  if (ret == 0)
    {
      fprintf (shell->terminal, "invalid address format: %s.%s", argv[4],
               shell->NL);
      return -1;
    }
  if (ret < 0)
    {
      fprintf (shell->terminal, "inet_pton() failed: %s.%s", strerror (errno),
               shell->NL);
      return -1;
    }

  fprintf (shell->terminal, "set %s: %s%s", argv[3], argv[4], shell->NL);
  return 0;
}

CLI_COMMAND2 (set_nettlp_bus_number,
              "set nettlp bus-number <0-65535> device-number <0-65535>",
              SET_HELP, "NetTLP information.\n", "Set bus-number.\n",
              "Specify bus-number (i.e. 0x1234 in Hex).\n",
              "Set device-number.\n",
              "Specify device-number (i.e. 0x1234 in Hex).\n")
{
  struct shell *shell = (struct shell *) context;
  int ret;

  bus_number = strtol (argv[3], NULL, 0);
  dev_number = strtol (argv[5], NULL, 0);
  requester = (bus_number << 8 | dev_number);

  fprintf (shell->terminal, "bus_number %hx:%hx%s", bus_number, dev_number,
           shell->NL);
  return 0;
}

CLI_COMMAND2 (set_nettlp_pci_tag, "set nettlp pci-tag <0-255>", SET_HELP,
              "NetTLP information.\n", "Set PCI tag.\n", "Specify PCI tag.\n")
{
  struct shell *shell = (struct shell *) context;
  pci_tag = strtol (argv[3], NULL, 0);
  fprintf (shell->terminal, "pci-tag: %d%s", pci_tag, shell->NL);
  return 0;
}

CLI_COMMAND2 (set_nettlp_txportid, "set nettlp tx-portid <0-128>", SET_HELP,
              "NetTLP information.\n", "Set transmission port-id.\n",
              "Specify transmission port-id.\n")
{
  struct shell *shell = (struct shell *) context;
  tx_portid = strtol (argv[3], NULL, 0);
  fprintf (shell->terminal, "tx-portid: %d%s", tx_portid, shell->NL);
  return 0;
}

CLI_COMMAND2 (set_nettlp_udp_port,
              "set nettlp udp (src-port|dst-port) <0-65535>", SET_HELP,
              "NetTLP information.\n", "UDP information.\n",
              "Set UDP src port.\n", "Set UDP dst port.\n",
              "Specify UDP port.\n")
{
  struct shell *shell = (struct shell *) context;
  uint16_t *dst;

  if (! strcmp (argv[3], "src-port"))
    dst = &src_port;
  else if (! strcmp (argv[3], "dst-port"))
    dst = &dst_port;

  *dst = strtol (argv[4], NULL, 0);
  fprintf (shell->terminal, "UDP %s: %hu%s", argv[3], *dst, shell->NL);
  return 0;
}

CLI_COMMAND2 (set_nettlp_memory_addr, "set nettlp memory-address <WORD>",
              SET_HELP, "NetTLP information.\n", "Set core memory address.\n",
              "Specify core memory address.\n")
{
  struct shell *shell = (struct shell *) context;
  memory_addr = strtoul (argv[3], NULL, 0);
  fprintf (shell->terminal, "memory-addr: %p%s", (void *) memory_addr,
           shell->NL);
  return 0;
}

CLI_COMMAND2 (set_nettlp_payload_size, "set nettlp payload-size <0-4096>",
              SET_HELP, "NetTLP information.\n", "Set payload-size.\n",
              "Specify size.\n")
{
  struct shell *shell = (struct shell *) context;
  payload_size = strtoul (argv[3], NULL, 0);
  fprintf (shell->terminal, "payload-size: %d%s", payload_size, shell->NL);
  return 0;
}

CLI_COMMAND2 (set_nettlp_max_payload_size,
              "set nettlp max-payload-size <0-4096>", SET_HELP,
              "NetTLP information.\n", "Set Max Payload Size (mps).\n",
              "Specify Max Payload Size.\n")
{
  struct shell *shell = (struct shell *) context;
  max_payload_size = strtoul (argv[3], NULL, 0);
  fprintf (shell->terminal, "MaxPayloadSize: %d%s", max_payload_size,
           shell->NL);
  return 0;
}

CLI_COMMAND2 (set_nettlp_payload_string, "set nettlp payload-string <LINE>",
              SET_HELP, "NetTLP information.\n", "Set payload string.\n",
              "Specify payload string.\n")
{
  struct shell *shell = (struct shell *) context;
  memset (payload_string, 0, sizeof (payload_string));
  snprintf (payload_string, sizeof (payload_string), "%s", argv[3]);
  payload_size = strlen (payload_string);
  fprintf (shell->terminal, "payload-string: %s (size: %d)%s", payload_string,
           payload_size, shell->NL);
  return 0;
}

CLI_COMMAND2 (set_nettlp_psmem_addr, "set nettlp psmem-address <WORD>",
              SET_HELP, "NetTLP information.\n", "Set psmem address.\n",
              "Specify memory address.\n")
{
  struct shell *shell = (struct shell *) context;
  char *endptr;
  uintptr_t new_addr;

  new_addr = strtoul (argv[3], &endptr, 0);
  if (*endptr != '\0')
    {
      fprintf (shell->terminal, "invalid addr: %s%s", argv[3], shell->NL);
      return -1;
    }

  /* if new_addr is set to 0, it indicates release. */
  if (psmem_memory && (new_addr == 0 || ! psmem_size))
    {
      free (psmem_memory);
      psmem_memory = NULL;
    }
  else if (psmem_memory && psmem_size)
    {
      psmem_memory = realloc (psmem_memory, psmem_size);
      if (! psmem_memory)
        {
          fprintf (shell->terminal, "realloc() failed: %s%s", strerror (errno),
                   shell->NL);
        }
      else
        {
          psmem_addr = new_addr;
        }
    }
  else
    {
      assert (psmem_memory == NULL);
      psmem_memory = malloc (psmem_size);
      if (! psmem_memory)
        {
          fprintf (shell->terminal, "malloc() failed: %s%s", strerror (errno),
                   shell->NL);
          return -1;
        }
      else
        {
          psmem_addr = new_addr;
        }
    }

  fprintf (shell->terminal, "psmem: 0x%lx--0x%lx memory: %p size: %d%s",
           psmem_addr, psmem_addr + psmem_size, psmem_memory, psmem_size,
           shell->NL);
  return 0;
}

CLI_COMMAND2 (set_nettlp_psmem_size, "set nettlp psmem-size (<WORD>|256M)",
              SET_HELP, "NetTLP information.\n", "Set psmem size.\n",
              "Specify memory size.\n"
              "Specify memory size as 256MiB.\n")
{
  struct shell *shell = (struct shell *) context;
  char *endptr;
  uint32_t new_size;

  if (! strcmp (argv[3], "256M"))
    new_size = 256 * 1024 * 1024;
  else
    {
      new_size = strtoul (argv[3], &endptr, 0);
      if (*endptr != '\0')
        {
          fprintf (shell->terminal, "invalid addr: %s%s", argv[3], shell->NL);
          return 0;
        }
    }

  /* if psmem_addr is set to 0, it indicates release. */
  if (psmem_memory && (psmem_addr == 0 || ! new_size))
    {
      free (psmem_memory);
      psmem_memory = NULL;
      psmem_size = new_size;
    }
  else if (psmem_memory && new_size)
    {
      psmem_memory = realloc (psmem_memory, new_size);
      if (! psmem_memory)
        {
          fprintf (shell->terminal, "realloc() failed: %s%s", strerror (errno),
                   shell->NL);
        }
      else
        {
          psmem_size = new_size;
        }
    }
  else
    {
      assert (psmem_memory == NULL);
      psmem_memory = malloc (new_size);
      if (! psmem_memory)
        {
          fprintf (shell->terminal, "malloc() failed: %s%s", strerror (errno),
                   shell->NL);
        }
      else
        {
          psmem_size = new_size;
        }
    }

  fprintf (shell->terminal, "psmem: 0x%lx--0x%lx memory: %p size: %d%s",
           psmem_addr, psmem_addr + psmem_size, psmem_memory, psmem_size,
           shell->NL);
  return 0;
}

CLI_COMMAND2 (show_nettlp_psmem, "show nettlp psmem (|<WORD>)", SHOW_HELP,
              "NetTLP information.\n", "Show psmem.\n",
              "Show from the specified start offset.\n")
{
  struct shell *shell = (struct shell *) context;
  char *endptr;
  uintptr_t start_offset;
  uintptr_t end_offset;
  uint32_t i;
  uint32_t psmem_show_size = 8 * 1024; // 8KiB.
  uint32_t show_size;

  start_offset = 0;
  if (argc > 3)
    {
      start_offset = strtoul (argv[3], &endptr, 0);
      if (*endptr != '\0')
        {
          fprintf (shell->terminal, "invalid addr: %s%s", argv[3], shell->NL);
          return 0;
        }
    }

  if (! psmem_memory || ! psmem_size)
    return 0;

  end_offset = start_offset + psmem_show_size;
  if (psmem_size < end_offset)
    end_offset = psmem_size;

  fprintf (shell->terminal, "psmem: 0x%lx--0x%lx memory: %p size: %d%s",
           psmem_addr, psmem_addr + psmem_size, psmem_memory, psmem_size,
           shell->NL);

  fprintf (shell->terminal, "show: offset: 0x%lx--0x%lx addr: 0x%lx--0x%lx%s",
           start_offset, end_offset, psmem_addr + start_offset,
           psmem_addr + end_offset, shell->NL);

  for (i = start_offset; i < end_offset; i++)
    {
      if (i % 64 == 0)
        fprintf (shell->terminal, "0x%04x  ", i);
      if (isspace (psmem_memory[i]))
        fprintf (shell->terminal, "%c", '_');
      else if (psmem_memory[i] == '\n')
        fprintf (shell->terminal, "%c", 'n');
      else if (isgraph (psmem_memory[i]))
        fprintf (shell->terminal, "%c", psmem_memory[i]);
      else
        fprintf (shell->terminal, "#");
      if (i % 8 == 7)
        fprintf (shell->terminal, " ");
      if (i % 64 == 63)
        fprintf (shell->terminal, "%s", shell->NL);
    }
  return 0;
}

void
nettlp_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, nettlp_send_dma_write_read);
  INSTALL_COMMAND2 (cmdset, show_nettlp);
  INSTALL_COMMAND2 (cmdset, set_nettlp_ether_local_remote);
  INSTALL_COMMAND2 (cmdset, set_nettlp_ipv4_local_remote);
  INSTALL_COMMAND2 (cmdset, set_nettlp_bus_number);
  INSTALL_COMMAND2 (cmdset, set_nettlp_pci_tag);
  INSTALL_COMMAND2 (cmdset, set_nettlp_txportid);
  INSTALL_COMMAND2 (cmdset, set_nettlp_udp_port);
  INSTALL_COMMAND2 (cmdset, set_nettlp_memory_addr);
  INSTALL_COMMAND2 (cmdset, set_nettlp_payload_size);
  INSTALL_COMMAND2 (cmdset, set_nettlp_max_payload_size);
  INSTALL_COMMAND2 (cmdset, set_nettlp_payload_string);

  INSTALL_COMMAND2 (cmdset, set_nettlp_psmem_addr);
  INSTALL_COMMAND2 (cmdset, set_nettlp_psmem_size);
  INSTALL_COMMAND2 (cmdset, show_nettlp_psmem);
}
