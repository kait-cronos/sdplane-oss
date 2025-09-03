/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#include "include.h"

#include <rte_common.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>

#ifndef RTE_VLAN_TCI_ID
#define RTE_VLAN_TCI_ID(vlan_tci) ((vlan_tci) & 0x0fff)
#endif

#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

#include <sdplane/command.h>

#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "l2fwd_export.h"
#include "sdplane.h"
#include "tap_handler.h"

#include "rib_manager.h"
#include "thread_info.h"

static __thread unsigned lcore_id;
static __thread struct rib *rib = NULL;

extern struct rte_eth_dev_tx_buffer
    *tx_buffer_per_q[RTE_MAX_ETHPORTS][RTE_MAX_LCORE];

/* enhanced_repeater_tx_flush() flushes the queue'ed packets
   in tx_buffer_per_q[] onto the NIC. */
static inline __attribute__ ((always_inline)) void
enhanced_repeater_tx_flush ()
{
  uint16_t nb_ports;
  int tx_portid;
  struct rte_eth_dev_tx_buffer *buffer;
  int sent;
  uint16_t tx_queueid;

  tx_queueid = lcore_id;

  nb_ports = rte_eth_dev_count_avail ();
  for (tx_portid = 0; tx_portid < nb_ports; tx_portid++)
    {
      buffer = tx_buffer_per_q[tx_portid][tx_queueid];
      sent = 0;
      if (buffer)
        {
          sent = rte_eth_tx_buffer_flush (tx_portid, tx_queueid, buffer);

          if (sent || buffer->length)
            DEBUG_SDPLANE_LOG (ENHANCED_REPEATER,
                               "lcore[%d]: port %d queue %d flush: "
                               "sent: %d buffer->length: %d",
                               lcore_id, tx_portid, tx_queueid, sent,
                               buffer->length);
        }
      if (sent)
        {
          port_statistics[tx_portid].tx += sent;
        }
    }
}

static inline __attribute__ ((always_inline)) void
enhanced_repeater_send_router_if (struct rte_mbuf *m, unsigned rx_portid,
                                  unsigned rx_queueid, struct router_if *rif)
{
  struct rte_mbuf *c;
  uint32_t pkt_len;
  uint16_t data_len;
  int ret;
  pkt_len = rte_pktmbuf_pkt_len (m);
  data_len = rte_pktmbuf_data_len (m);

  DEBUG_SDPLANE_LOG (ENHANCED_REPEATER,
                     "m: %p port %d queue %d to ring_up: %p", m, rx_portid,
                     rx_queueid, rif->ring_up);

  c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
  ret = rte_ring_enqueue (rif->ring_up, c);
  if (ret)
    {
      if (ret == -ENOBUFS)
        DEBUG_SDPLANE_LOG (
            ENHANCED_REPEATER,
            "lcore[%d]: m: %p port %d queue %d to router_if ring: "
            "ENOBUFS: %d",
            lcore_id, m, rx_portid, rx_queueid, ret);
      else
        DEBUG_SDPLANE_LOG (
            ENHANCED_REPEATER,
            "lcore[%d]: m: %p port %d queue %d to router_if ring: "
            "failed: %d",
            lcore_id, m, rx_portid, rx_queueid, ret);
      rte_pktmbuf_free (c);
    }
}

static inline __attribute__ ((always_inline)) void
enhanced_repeater_send_capture_if (struct rte_mbuf *m, unsigned rx_portid,
                                   unsigned rx_queueid, struct capture_if *cif)
{
  struct rte_mbuf *c;
  uint32_t pkt_len;
  uint16_t data_len;
  int ret;
  pkt_len = rte_pktmbuf_pkt_len (m);
  data_len = rte_pktmbuf_data_len (m);

  DEBUG_SDPLANE_LOG (ENHANCED_REPEATER,
                     "m: %p port %d queue %d to ring_up: %p", m, rx_portid,
                     rx_queueid, cif->ring_up);

  c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
  ret = rte_ring_enqueue (cif->ring_up, c);
  if (ret)
    {
      if (ret == -ENOBUFS)
        DEBUG_SDPLANE_LOG (
            ENHANCED_REPEATER,
            "lcore[%d]: m: %p port %d queue %d to capture_if ring: "
            "ENOBUFS: %d",
            lcore_id, m, rx_portid, rx_queueid, ret);
      else
        DEBUG_SDPLANE_LOG (
            ENHANCED_REPEATER,
            "lcore[%d]: m: %p port %d queue %d to capture_if ring: "
            "failed: %d",
            lcore_id, m, rx_portid, rx_queueid, ret);
      rte_pktmbuf_free (c);
    }
}

static inline __attribute__ ((always_inline)) void
enhanced_repeater_send_link (struct rte_mbuf *m, unsigned rx_portid,
                             unsigned rx_queueid, unsigned tx_portid,
                             unsigned tx_queueid,
                             struct vswitch_link *vswitch_link)
{
  struct rte_eth_dev_tx_buffer *buffer;
  uint16_t nb_ports;
  int sent;
  struct rte_mbuf *c;

  nb_ports = rte_eth_dev_count_avail ();

  if (tx_portid >= nb_ports)
    return;

  if (rx_portid == tx_portid)
    return;

  if (! rib->rib_info->port[tx_portid].link.link_status)
    return;

  buffer = tx_buffer_per_q[tx_portid][tx_queueid];
  if (! buffer)
    return;

  /* copy the packet */
  c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
  if (! c)
    {
      DEBUG_SDPLANE_LOG (ENHANCED_REPEATER,
                         "lcore[%d]: m: %p router_if -> port %d "
                         "rte_pktmbuf_copy() failed.",
                         lcore_id, m, tx_portid);
      return;
    }

  struct rte_ether_hdr *eth_hdr;
  uint16_t eth_type;
  uint16_t vlan_id;
  struct rte_vlan_hdr *vlan_hdr;
  uint16_t old_vlan_tci, new_vlan_tci;

  eth_hdr = rte_pktmbuf_mtod (c, struct rte_ether_hdr *);
  eth_type = rte_be_to_cpu_16 (eth_hdr->ether_type);
  if (eth_type == RTE_ETHER_TYPE_VLAN)
    {
      vlan_hdr = (struct rte_vlan_hdr *) (eth_hdr + 1);
      vlan_id = RTE_VLAN_TCI_ID (rte_be_to_cpu_16 (vlan_hdr->vlan_tci));

      if (vswitch_link->tag_id != 0 && vswitch_link->tag_id != vlan_id)
        {
          /* vlan_id translation: modify vlan_id on tx */
          old_vlan_tci = rte_be_to_cpu_16 (vlan_hdr->vlan_tci);
          new_vlan_tci =
              ((old_vlan_tci & 0xf000) | (vswitch_link->tag_id & 0x0fff));
          DEBUG_SDPLANE_LOG (ENHANCED_REPEATER,
                             "m: %p port[%d]: vlan_id modification: %u -> %u",
                             m, vswitch_link->port_id, vlan_id,
                             vswitch_link->tag_id);
          vlan_hdr->vlan_tci = rte_cpu_to_be_16 (new_vlan_tci);
        }
      else if (vswitch_link->tag_id == 0)
        {
          /* remove vlan_hdr */
          rte_vlan_strip (c);
          DEBUG_SDPLANE_LOG (
              ENHANCED_REPEATER, "m: %p port[%d]: vlan_id strip: %u -> %u", m,
              vswitch_link->port_id, vlan_id, vswitch_link->tag_id);
        }
    }
  else
    {
      if (vswitch_link->tag_id != 0)
        {
          /* insert vlan_hdr */
          rte_vlan_insert (&c);
          eth_hdr = rte_pktmbuf_mtod (c, struct rte_ether_hdr *);
          eth_type = rte_be_to_cpu_16 (eth_hdr->ether_type);
          assert (eth_type == RTE_ETHER_TYPE_VLAN);
          vlan_hdr = (struct rte_vlan_hdr *) (eth_hdr + 1);
          old_vlan_tci = rte_be_to_cpu_16 (vlan_hdr->vlan_tci);
          new_vlan_tci =
              ((old_vlan_tci & 0xf000) | (vswitch_link->tag_id & 0x0fff));
          DEBUG_SDPLANE_LOG (ENHANCED_REPEATER,
                             "m: %p port[%d]: add vlan_id: %u", m,
                             vswitch_link->port_id, vswitch_link->tag_id);
          vlan_hdr->vlan_tci = rte_cpu_to_be_16 (new_vlan_tci);
        }
    }

  sent = rte_eth_tx_buffer (tx_portid, tx_queueid, buffer, c);
  if (sent)
    port_statistics[tx_portid].tx += sent;
}

static inline __attribute__ ((always_inline)) void
enhanced_repeater_select (struct rte_mbuf *m, unsigned rx_portid,
                          unsigned rx_queueid)
{
  struct rte_ether_hdr *eth_hdr;
  struct port_conf *port_config;
  uint16_t eth_type;
  uint16_t vswitch_link_id;
  struct vswitch_link *vswitch_link = NULL;
  struct router_if *rif;
  struct capture_if *cif;
  int i;

  DEBUG_SDPLANE_LOG (ENHANCED_REPEATER, "m: %p received on port: %d queue: %d",
                     m, rx_portid, rx_queueid);

  port_config = &rib->rib_info->port[rx_portid];
  eth_hdr = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  eth_type = rte_be_to_cpu_16 (eth_hdr->ether_type);
  if (eth_type == RTE_ETHER_TYPE_VLAN)
    {
      uint16_t vlan_id;
      struct rte_vlan_hdr *vlan_hdr;
      vlan_hdr = (struct rte_vlan_hdr *) (eth_hdr + 1);
#ifndef RTE_VLAN_TCI_ID
#define RTE_VLAN_TCI_ID(vlan_tci) ((vlan_tci) & 0x0fff)
#endif
      vlan_id = RTE_VLAN_TCI_ID (rte_be_to_cpu_16 (vlan_hdr->vlan_tci));
      DEBUG_SDPLANE_LOG (ENHANCED_REPEATER, "m: %p tagged: vlan: %u", m,
                         vlan_id);

      for (i = 0; i < port_config->vlan_size; i++)
        {
          struct vswitch_link *link;
          vswitch_link_id = port_config->vswitch_link_id_of_vlan[i];
          link = &rib->rib_info->vswitch_link[vswitch_link_id];
          if (link->tag_id == vlan_id)
            {
              vswitch_link = link;
              DEBUG_SDPLANE_LOG (ENHANCED_REPEATER,
                                 "m: %p tagged: vswitch: %u, vlan: %u", m,
                                 vswitch_link->vswitch_id, vlan_id);
              break;
            }
        }
    }
  else
    {
      /* in case of untagged port, direct the packet to the default vlan
         for the port. */
      vswitch_link_id = port_config->vswitch_link_id_of_native_vlan;
      if (0 <= vswitch_link_id &&
          vswitch_link_id < rib->rib_info->vswitch_link_size)
        {
          vswitch_link = &rib->rib_info->vswitch_link[vswitch_link_id];
          DEBUG_SDPLANE_LOG (ENHANCED_REPEATER, "m: %p untag: vswitch: %u", m,
                             vswitch_link->vswitch_id);
        }
    }

  if (! vswitch_link)
    {
      DEBUG_SDPLANE_LOG (ENHANCED_REPEATER,
                         "m: %p cannot find the vswitch link", m);
      return;
    }

  if (vswitch_link->vswitch_id < 0 ||
      rib->rib_info->vswitch_size >= MAX_VSWITCH)
    {
      DEBUG_SDPLANE_LOG (ENHANCED_REPEATER,
                         "m: %p a broken vswitch link: "
                         "vswitch: %d vswitch_size: %d",
                         m, vswitch_link->vswitch_id,
                         rib->rib_info->vswitch_size);
      return;
    }

  struct vswitch_conf *vswitch;
  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      if (vswitch_link->vswitch_id == rib->rib_info->vswitch[i].vswitch_id)
        {
          vswitch = &rib->rib_info->vswitch[i];
        };
    };

  unsigned tx_queueid = lcore_id;

  for (i = 0; i < vswitch->vswitch_port_size; i++)
    {
      uint16_t vswitch_link_id = vswitch->vswitch_link_id[i];
      struct vswitch_link *link =
          &rib->rib_info->vswitch_link[vswitch_link_id];
      unsigned tx_portid = link->port_id;

      DEBUG_SDPLANE_LOG (
          ENHANCED_REPEATER,
          "m: %p vswitch[%d]vswport[%d/%d]: vswitch_link_id: %u "
          "port: %d vlan: %d tag: %d (vswitch%u[%d])",
          m, vswitch_link->vswitch_id, i, vswitch->vswitch_port_size,
          vswitch_link_id, link->port_id, link->vlan_id, link->tag_id,
          link->vswitch_id, link->vswitch_port);

      /* skip received link port. split-horizon */
      if (link == vswitch_link || tx_portid == rx_portid)
        {
          DEBUG_SDPLANE_LOG (ENHANCED_REPEATER, "m: %p split horizon.", m);
          continue;
        }

      /* forward to dpdk ports, accoding to the vswitch_link. */
      enhanced_repeater_send_link (m, rx_portid, rx_queueid, tx_portid,
                                   tx_queueid, link);
    }

  rif = &vswitch->router_if;
  cif = &vswitch->capture_if;

  if (rif->sockfd >= 0 && rif->ring_up)
    {
      enhanced_repeater_send_router_if (m, rx_portid, rx_queueid, rif);
    }

  if (cif->sockfd >= 0 && cif->ring_up)
    {
      enhanced_repeater_send_capture_if (m, rx_portid, rx_queueid, cif);
    }

  return;
}

//__thread uint32_t nb_rx_burst = 0;

static inline __attribute__ ((always_inline)) void
enhanced_repeater_rx_burst ()
{
  struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
  struct rte_mbuf *m;
  unsigned i, j, nb_rx;
  uint16_t portid, queueid;

  if (unlikely (! rib || ! rib->rib_info))
    return;

  struct lcore_qconf *lcore_qconf;
  lcore_qconf = &rib->rib_info->lcore_qconf[lcore_id];
  for (i = 0; i < lcore_qconf->nrxq; i++)
    {
      portid = lcore_qconf->rx_queue_list[i].port_id;
      queueid = lcore_qconf->rx_queue_list[i].queue_id;

      nb_rx = rte_eth_rx_burst (portid, queueid, pkts_burst, MAX_PKT_BURST);
      // nb_rx_burst++;

      if (unlikely (nb_rx == 0))
        continue;

      port_statistics[portid].rx += nb_rx;

      for (j = 0; j < nb_rx; j++)
        {
          m = pkts_burst[j];
          rte_prefetch0 (rte_pktmbuf_mtod (m, void *));

#if 1
          enhanced_repeater_select (m, portid, queueid);
#else
          vlan_switch_run (m, portid, queueid);
#endif

          rte_pktmbuf_free (m);
        }
    }
}

#define ROUTER_IF_PORT_ID  UINT16_MAX // for marker to unused split-horizon
#define ROUTER_IF_QUEUE_ID 0          // for debug

static inline __attribute__ ((always_inline)) void
enhanced_repeater_tx_burst ()
{
  struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
  struct rte_mbuf *m;
  unsigned i, j, nb_rx;
  uint16_t tx_queueid;

  tx_queueid = lcore_id;

  if (unlikely (! rib || ! rib->rib_info))
    return;

  struct vswitch_conf *vswitch;
  int vswitch_id;
  struct vswitch_link *vswitch_link;
  uint16_t vswitch_link_id;

  for (vswitch_id = 0; vswitch_id < rib->rib_info->vswitch_size; vswitch_id++)
    {
      vswitch = &rib->rib_info->vswitch[vswitch_id];

      if (vswitch->router_if.sockfd < 0 || ! vswitch->router_if.ring_dn)
        continue;

      nb_rx =
          rte_ring_dequeue_burst (vswitch->router_if.ring_dn,
                                  (void **) pkts_burst, MAX_PKT_BURST, NULL);

      if (unlikely (nb_rx == 0))
        continue;

      for (i = 0; i < nb_rx; i++)
        {
          m = pkts_burst[i];
          if (! m)
            continue;

          rte_prefetch0 (rte_pktmbuf_mtod (m, void *));

          for (j = 0; j < vswitch->vswitch_port_size; j++)
            {
              vswitch_link_id = vswitch->vswitch_link_id[j];
              if (vswitch_link_id >= rib->rib_info->vswitch_link_size)
                continue;

              vswitch_link = &rib->rib_info->vswitch_link[vswitch_link_id];

              if (vswitch_link->vswitch_id != vswitch->vswitch_id)
                continue;

              enhanced_repeater_send_link (
                  m, ROUTER_IF_PORT_ID, ROUTER_IF_QUEUE_ID,
                  vswitch_link->port_id, tx_queueid, vswitch_link);
            }

          rte_pktmbuf_free (m);
        }
    }
}

static __thread uint64_t loop_counter = 0;

int
enhanced_repeater (__rte_unused void *dummy)
{
  uint64_t prev_tsc, diff_tsc, cur_tsc;
  struct lcore_queue_conf *qconf;
  const uint64_t drain_tsc =
      (rte_get_tsc_hz () + US_PER_S - 1) / US_PER_S * BURST_TX_DRAIN_US;

  uint16_t nb_ports;
  bool ready;

  /* the tx_buffer_per_q is initialized in rib_manager. */

  prev_tsc = 0;
  lcore_id = rte_lcore_id ();
  qconf = &lcore_queue_conf[lcore_id];

  int thread_id;
  thread_id = thread_lookup_by_lcore (enhanced_repeater, lcore_id);
  thread_register_loop_counter (thread_id, &loop_counter);

  DEBUG_SDPLANE_LOG (ENHANCED_REPEATER, "entering main loop on lcore %u",
                     lcore_id);

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_register_thread ();
#endif /*HAVE_LIBURCU_QSBR*/

  while (! force_quit && ! force_stop[lcore_id])
    {
      cur_tsc = rte_rdtsc ();

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_lock ();
      rib = (struct rib *) rcu_dereference (rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

      /* check port ready */
      ready = true;
      nb_ports = rte_eth_dev_count_avail ();
      for (int i = 0; i < nb_ports; i++)
        {
          if (unlikely (rib->rib_info->port[i].is_stopped))
            {
              ready = false;
              DEBUG_SDPLANE_LOG (ENHANCED_REPEATER,
                                 "port %d link is down, waiting...", i);
              break;
            }
        }
      if (unlikely (! ready))
        continue;

      diff_tsc = cur_tsc - prev_tsc;
      if (unlikely (diff_tsc > drain_tsc))
        {
          enhanced_repeater_tx_flush ();
          prev_tsc = cur_tsc;
        }

      enhanced_repeater_rx_burst ();
      enhanced_repeater_tx_burst ();

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_unlock ();
      urcu_qsbr_quiescent_state ();
#endif /*HAVE_LIBURCU_QSBR*/

      loop_counter++;
    }

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_unregister_thread ();
#endif /*HAVE_LIBURCU_QSBR*/
}
