#include "include.h"

#include <lthread.h>

#include <rte_common.h>
#include <rte_launch.h>
#include <rte_ether.h>
#include <rte_malloc.h>

#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>
#include <zcmdsh/command_shell.h>

#include <zcmdsh/debug.h>
#include <zcmdsh/debug_cmd.h>
#include <zcmdsh/debug_log.h>
#include <zcmdsh/debug_category.h>
#include <zcmdsh/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "rib_manager.h"
#include "sdplane.h"
#include "thread_info.h"

#include "l2fwd_export.h"

#include "internal_message.h"

#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

extern int lthread_core;
extern volatile bool force_stop[RTE_MAX_LCORE];

extern struct rte_eth_dev_tx_buffer *tx_buffer_per_q[RTE_MAX_ETHPORTS][RTE_MAX_LCORE];

struct rte_ring *msg_queue_nettlp;

static __thread  unsigned lcore_id;
static __thread uint64_t loop_counter = 0;
static __thread struct rib *rib;

#include "nettlp.h"

int tx_portid;
struct rte_ether_addr local_ether;
struct rte_ether_addr remote_ether;
struct in_addr local_addr;
struct in_addr remote_addr;
uint16_t src_port;
uint16_t dst_port;
uint16_t bus_number;
uint16_t dev_number;
uint8_t pci_tag; /* PCI tag value */
uintptr_t memory_addr; /* DMA memory address */
int size;
int max_payload_size; /*Max Payload Size */
char payload_string[4096];

int tlp_calculate_lstdw(uintptr_t addr, size_t count)
{
        uintptr_t end, end_start, start;

        start = (addr >> 2) << 2;
        end = addr + count;
        if ((end & 0x3) == 0)
                end_start = end - 4;
        else
                end_start = (end >> 2) << 2;

        /* corner case. count is smaller than 8 */
        if (end_start <= start)
                end_start = addr + 4;
        if (end < end_start)
                return 0;

        return ~(0xF << (end - end_start)) & 0xF;
}

int tlp_calculate_fstdw(uintptr_t addr, size_t count)
{
        uint8_t be = 0xF;

        if (count < 4)
                be = ~(0xF << count) & 0xF;

        return (be << (addr & 0x3)) & 0xF;
}

int tlp_calculate_length(uintptr_t addr, size_t count)
{
        size_t len = 0;
        uintptr_t start, end;

        start = addr & 0xFFFFFFFFFFFFFFFc;
        end = addr + count;

        len = (end - start) >> 2;

        if ((end - start) & 0x3)
                len++;

        return len;
}

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

  m = rte_pktmbuf_alloc (l2fwd_pktmbuf_pool);

  length = sizeof (struct nettlp_hdr) + sizeof (struct tlp_mr_hdr);
  rte_pktmbuf_append (m, length);
  nh = rte_pktmbuf_mtod (m, struct nettlp_hdr *);
  mh = (struct tlp_mr_hdr *) (nh + 1);

  uint16_t requester;
  requester = (bus_number << 8 | dev_number);
  mh->requester = rte_cpu_to_be_16 (requester);
  mh->tag = pci_tag;
  mh->lstdw = tlp_calculate_lstdw(memory_addr, size);
  mh->fstdw = tlp_calculate_fstdw(memory_addr, size);
  tlp_set_length(mh->tlp.falen, tlp_calculate_length(memory_addr, size));

  uint32_t *dst_addr32;
  uint64_t *dst_addr64;
  uint8_t *memory_addrp;

  tlp_set_type(mh->tlp.fmt_type, TLP_TYPE_MWr);
  if (memory_addr < UINT32_MAX)
    {
      tlp_set_fmt(mh->tlp.fmt_type, TLP_FMT_3DW, TLP_FMT_W_DATA);
      length = sizeof (uint32_t);
      rte_pktmbuf_append (m, length);
      dst_addr32 = (uint32_t *) (mh + 1);
      *dst_addr32 = rte_cpu_to_be_32 (memory_addr & 0xFFFFFFFC);
      memory_addrp = (uint8_t *) dst_addr32;
    }
  else
    {
      tlp_set_fmt(mh->tlp.fmt_type, TLP_FMT_4DW, TLP_FMT_W_DATA);
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


  if (! tx_buffer_per_q[tx_portid][tx_queueid])
    {
      tx_buffer_per_q[tx_portid][tx_queueid] =
        rte_zmalloc_socket ("tx_buffer",
                        RTE_ETH_TX_BUFFER_SIZE (MAX_PKT_BURST), 0,
                        rte_eth_dev_socket_id (tx_portid));
      rte_eth_tx_buffer_init (tx_buffer_per_q[tx_portid][tx_queueid],
                              MAX_PKT_BURST);
    }

  buffer = tx_buffer_per_q[tx_portid][lcore_id];

  if (buffer)
    {
      rte_eth_tx_buffer (tx_portid, tx_queueid, buffer, m);
      rte_eth_tx_buffer_flush (tx_portid, tx_queueid, buffer);
      DEBUG_SDPLANE_LOG (NETTLP, "sent.");
    }
  else
    DEBUG_SDPLANE_LOG (NETTLP, "no tx buffer.");
}

int
nettlp_thread (void *arg)
{
  int ret;
  void *msgp;

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
      //lthread_sleep (100); // yield.
      //DEBUG_SDPLANE_LOG (NETTLP, "%s: schedule.", __func__);

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_lock ();
      rib = (struct rib *) rcu_dereference (rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

      msgp = internal_msg_recv (msg_queue_nettlp);
      if (msgp)
        {
          nettlp_send_dma_write ();
          internal_msg_delete (msgp);
        }

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

CLI_COMMAND2 (nettlp_send_dma_write,
              "nettlp-send dma-write",
              "NetTLP send command\n",
              "DMA write packet\n"
              )
{
  struct shell *shell = (struct shell *) context;

  void *msgp;
  struct internal_msg_qconf *msg_qconf;

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_QCONF, thread_qconf,
                              sizeof (thread_qconf));
  internal_msg_send_to (msg_queue_nettlp, msgp, shell);
}

CLI_COMMAND2 (show_nettlp,
              "show nettlp",
              SHOW_HELP,
              "NetTLP information.\n")
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

  fprintf (shell->terminal, "bus_number: %hx:%hx.%s",
           bus_number, dev_number, shell->NL);
  fprintf (shell->terminal, "pci-tag: %d.%s", pci_tag, shell->NL);
  fprintf (shell->terminal, "memory-addr: %p.%s",
           (void *) memory_addr, shell->NL);
  fprintf (shell->terminal, "size: %d.%s", size, shell->NL);
  fprintf (shell->terminal, "MaxPayloadSize: %d.%s",
           max_payload_size, shell->NL);
  fprintf (shell->terminal, "payload-string: %s.%s",
           payload_string, shell->NL);
}

CLI_COMMAND2 (set_nettlp_ether_local_remote,
              "set nettlp ether (local-addr|remote-addr) <WORD>",
              SET_HELP,
              "NetTLP information.\n",
              "local ethernet source address.\n",
              "remote ethernet destination address.\n",
              "Specify ethernet address.\n"
              )
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
      fprintf (shell->terminal, "invalid address format: %s.%s",
               argv[4], shell->NL);
      return;
    }

  fprintf (shell->terminal, "set %s: %s%s",
           argv[3], argv[4], shell->NL);
}


CLI_COMMAND2 (set_nettlp_ipv4_local_remote,
              "set nettlp ipv4 (local-addr|remote-addr) A.B.C.D",
              SET_HELP,
              "NetTLP information.\n",
              "IPv information.\n",
              "local IPv4 source address.\n",
              "remote IPv4 destination address.\n",
              "Specify IPv4 address.\n"
              )
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
      fprintf (shell->terminal, "invalid address format: %s.%s",
               argv[4], shell->NL);
      return;
    }
  if (ret < 0)
    {
      fprintf (shell->terminal, "inet_pton() failed: %s.%s",
               strerror (errno), shell->NL);
      return;
    }

  fprintf (shell->terminal, "set %s: %s%s",
           argv[3], argv[4], shell->NL);
}

CLI_COMMAND2 (set_nettlp_bus_number,
              "set nettlp bus-number <0-65535> device-number <0-65535>",
              SET_HELP,
              "NetTLP information.\n",
              "Set bus-number.\n",
              "Specify bus-number (i.e. 0x1234 in Hex).\n",
              "Set device-number.\n",
              "Specify device-number (i.e. 0x1234 in Hex).\n"
              )
{
  struct shell *shell = (struct shell *) context;
  int ret;

  bus_number = strtol (argv[3], NULL, 0);
  dev_number = strtol (argv[5], NULL, 0);

  fprintf (shell->terminal, "bus_number %hx:%hx%s",
           bus_number, dev_number, shell->NL);
}

CLI_COMMAND2 (set_nettlp_pci_tag,
              "set nettlp pci-tag <0-255>",
              SET_HELP,
              "NetTLP information.\n",
              "Set PCI tag.\n",
              "Specify PCI tag.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  pci_tag = strtol (argv[3], NULL, 0);
  fprintf (shell->terminal, "pci-tag: %d%s", pci_tag, shell->NL);
}

CLI_COMMAND2 (set_nettlp_txportid,
              "set nettlp tx-portid <0-128>",
              SET_HELP,
              "NetTLP information.\n",
              "Set transmission port-id.\n",
              "Specify transmission port-id.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  tx_portid = strtol (argv[3], NULL, 0);
  fprintf (shell->terminal, "tx-portid: %d%s", tx_portid, shell->NL);
}

CLI_COMMAND2 (set_nettlp_udp_port,
              "set nettlp udp (src-port|dst-port) <0-65535>",
              SET_HELP,
              "NetTLP information.\n",
              "UDP information.\n",
              "Set UDP src port.\n",
              "Set UDP dst port.\n",
              "Specify UDP port.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  uint16_t *dst;

  if (! strcmp (argv[3], "src-port"))
    dst = &src_port;
  else if (! strcmp (argv[3], "dst-port"))
    dst = &dst_port;

  *dst = strtol (argv[4], NULL, 0);
  fprintf (shell->terminal, "UDP %s: %hu%s", argv[3], *dst, shell->NL);
}

CLI_COMMAND2 (set_nettlp_memory_addr,
              "set nettlp memory-address <WORD>",
              SET_HELP,
              "NetTLP information.\n",
              "Set core memory address.\n",
              "Specify core memory address.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  memory_addr = strtoul (argv[3], NULL, 0);
  fprintf (shell->terminal, "memory-addr: %p%s",
           (void *) memory_addr, shell->NL);
}

CLI_COMMAND2 (set_nettlp_size,
              "set nettlp size <0-4096>",
              SET_HELP,
              "NetTLP information.\n",
              "Set size.\n",
              "Specify size.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  size = strtoul (argv[3], NULL, 0);
  fprintf (shell->terminal, "size: %d%s", size, shell->NL);
}

CLI_COMMAND2 (set_nettlp_max_payload_size,
              "set nettlp max-payload-size <0-4096>",
              SET_HELP,
              "NetTLP information.\n",
              "Set Max Payload Size (mps).\n",
              "Specify Max Payload Size.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  max_payload_size = strtoul (argv[3], NULL, 0);
  fprintf (shell->terminal, "MaxPayloadSize: %d%s",
           max_payload_size, shell->NL);
}

CLI_COMMAND2 (set_nettlp_payload_string,
              "set nettlp payload-string <LINE>",
              SET_HELP,
              "NetTLP information.\n",
              "Set payload string.\n",
              "Specify payload string.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  snprintf (payload_string, sizeof (payload_string),
            "%s", argv[3]);
  size = strlen (payload_string);
  fprintf (shell->terminal, "payload-string: %s%s",
           payload_string, shell->NL);
}

void
nettlp_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, nettlp_send_dma_write);
  INSTALL_COMMAND2 (cmdset, show_nettlp);
  INSTALL_COMMAND2 (cmdset, set_nettlp_ether_local_remote);
  INSTALL_COMMAND2 (cmdset, set_nettlp_ipv4_local_remote);
  INSTALL_COMMAND2 (cmdset, set_nettlp_bus_number);
  INSTALL_COMMAND2 (cmdset, set_nettlp_pci_tag);
  INSTALL_COMMAND2 (cmdset, set_nettlp_txportid);
  INSTALL_COMMAND2 (cmdset, set_nettlp_udp_port);
  INSTALL_COMMAND2 (cmdset, set_nettlp_memory_addr);
  INSTALL_COMMAND2 (cmdset, set_nettlp_size);
  INSTALL_COMMAND2 (cmdset, set_nettlp_max_payload_size);
  INSTALL_COMMAND2 (cmdset, set_nettlp_payload_string);
}

