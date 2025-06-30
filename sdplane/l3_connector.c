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

/* l3_connector_tx_flush() flushes the queue'ed packets
   in tx_buffer_per_q[] onto the NIC. */
static inline __attribute__ ((always_inline)) void
l3_connector_tx_flush ()
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
            DEBUG_SDPLANE_LOG (L3_CONNECTOR,
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

#if 0
/* vlan_switch_tx_burst() derived probably from l2_repeater
   and changed its name systematically, I mean, without solid design.
   it does not work as "vlan_switch_tx". It is somekind of
   obsoleted code. */
/* vlan_switch_tx_burst() reads from ring_dn[] and
   tx_bursts it directly to the NIC. */
static inline __attribute__ ((always_inline)) void
vlan_switch_tx_burst ()
{
  struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
  struct rte_mbuf *m;
  unsigned i, nb_rx;
  uint16_t portid, queueid;
  uint16_t tx_queueid;

  tx_queueid = lcore_id;

  if (unlikely (! rib || ! rib->rib_info))
    return;

  struct lcore_qconf *lcore_qconf;
  lcore_qconf = &rib->rib_info->lcore_qconf[lcore_id];
  for (i = 0; i < lcore_qconf->nrxq; i++)
    {
      portid = lcore_qconf->rx_queue_list[i].port_id;
      queueid = lcore_qconf->rx_queue_list[i].queue_id;

      nb_rx = rte_ring_dequeue_burst (
          ring_dn[portid][queueid], (void **) pkts_burst, MAX_PKT_BURST, NULL);

      if (unlikely (nb_rx == 0))
        continue;

      rte_eth_tx_burst (portid, tx_queueid, pkts_burst, nb_rx);
      DEBUG_SDPLANE_LOG (VLAN_SWITCH,
                         "lcore[%d]: tx_burst: port: %d queue: %d pkts: %d",
                         lcore_id, portid, tx_queueid, nb_rx);
    }
}
#endif

static inline __attribute__ ((always_inline)) void
l3_connector_router_if_send (struct rte_mbuf *m, unsigned rx_portid,
                             unsigned rx_queueid, struct router_if *rif)
{
  /* Caller already verified sockfd > 0 && ring_up */

  struct rte_mbuf *c;
  uint32_t pkt_len;
  uint16_t data_len;
  int ret;
  pkt_len = rte_pktmbuf_pkt_len (m);
  data_len = rte_pktmbuf_data_len (m);

  c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
  if (! c)
    {
      DEBUG_SDPLANE_LOG (L3_CONNECTOR,
                         "m: %p failed to copy mbuf for router_if", m);
      return;
    }
  ret = rte_ring_enqueue (rif->ring_up, c);
  if (ret)
    {
      if (ret == -ENOBUFS)
        DEBUG_SDPLANE_LOG (
            L3_CONNECTOR,
            "lcore[%d]: m: %p port %d queue %d to router_if ring: "
            "ENOBUFS: %d",
            lcore_id, m, rx_portid, rx_queueid, ret);
      else
        DEBUG_SDPLANE_LOG (
            L3_CONNECTOR,
            "lcore[%d]: m: %p port %d queue %d to router_if ring: "
            "failed: %d",
            lcore_id, m, rx_portid, rx_queueid, ret);
      rte_pktmbuf_free (c);
    }
}

static inline __attribute__ ((always_inline)) void
l3_connector_capture_if_send (struct rte_mbuf *m, unsigned rx_portid,
                              unsigned rx_queueid, struct capture_if *cif)
{
  /* Caller already verified sockfd > 0 && ring_up */

  struct rte_mbuf *c;
  uint32_t pkt_len;
  uint16_t data_len;
  int ret;
  pkt_len = rte_pktmbuf_pkt_len (m);
  data_len = rte_pktmbuf_data_len (m);

  c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
  if (! c)
    {
      DEBUG_SDPLANE_LOG (L3_CONNECTOR,
                         "m: %p failed to copy mbuf for capture_if", m);
      return;
    }
  ret = rte_ring_enqueue (cif->ring_up, c);
  if (ret)
    {
      if (ret == -ENOBUFS)
        DEBUG_SDPLANE_LOG (
            L3_CONNECTOR,
            "lcore[%d]: m: %p port %d queue %d to capture_if ring: "
            "ENOBUFS: %d",
            lcore_id, m, rx_portid, rx_queueid, ret);
      else
        DEBUG_SDPLANE_LOG (
            L3_CONNECTOR,
            "lcore[%d]: m: %p port %d queue %d to capture_if ring: "
            "failed: %d",
            lcore_id, m, rx_portid, rx_queueid, ret);
      rte_pktmbuf_free (c);
    }
}

static inline __attribute__ ((always_inline)) void
l3_connector_select (struct rte_mbuf *m, unsigned rx_portid,
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

  DEBUG_SDPLANE_LOG (L3_CONNECTOR, "m: %p received on port: %d queue: %d", m,
                     rx_portid, rx_queueid);

  /* Safety checks for RIB access */
  /* 🤖 生成AI (CLAUDE) */
  if (unlikely (! rib || ! rib->rib_info))
    {
      DEBUG_SDPLANE_LOG (L3_CONNECTOR,
                         "m: %p rib not available, dropping packet", m);
      return;
    }

  /* 🤖 生成AI (CLAUDE) */
  if (unlikely (rx_portid >= rib->rib_info->port_size))
    {
      DEBUG_SDPLANE_LOG (L3_CONNECTOR,
                         "m: %p invalid port_id %d, dropping packet", m,
                         rx_portid);
      return;
    }

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
      DEBUG_SDPLANE_LOG (L3_CONNECTOR, "m: %p tagged: vlan: %u", m, vlan_id);

      for (i = 0; i < port_config->vlan_size; i++)
        {
          struct vswitch_link *link;
          vswitch_link_id = port_config->vswitch_link_id_of_vlan[i];

          link = &rib->rib_info->vswitch_link[vswitch_link_id];
          if (link->tag_id == vlan_id)
            {
              vswitch_link = link;
              DEBUG_SDPLANE_LOG (L3_CONNECTOR,
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
          DEBUG_SDPLANE_LOG (L3_CONNECTOR, "m: %p untag: vswitch: %u", m,
                             vswitch_link->vswitch_id);
        }
    }

  /* 🤖 生成AI (CLAUDE) */
  if (! vswitch_link)
    {
      DEBUG_SDPLANE_LOG (
          L3_CONNECTOR,
          "m: %p cannot find vswitch link: port %d, vlan_size %d, native_vlan_id %d",
          m, rx_portid, port_config->vlan_size,
          port_config->vswitch_link_id_of_native_vlan);
      return;
    }

  if (vswitch_link->vswitch_id < 0 ||
      rib->rib_info->vswitch_size <= vswitch_link->vswitch_id)
    {
      DEBUG_SDPLANE_LOG (L3_CONNECTOR,
                         "m: %p a broken vswitch link: "
                         "vswitch: %d vswitch_size: %d",
                         m, vswitch_link->vswitch_id,
                         rib->rib_info->vswitch_size);
      return;
    }

  struct vswitch_conf *vswitch;
  vswitch = &rib->rib_info->vswitch[vswitch_link->vswitch_id];

  unsigned tx_queueid = lcore_id;

  rif = &vswitch->router_if;
  cif = &vswitch->capture_if;

  /* TODO: check dst ip address */

  if (rif->sockfd > 0 && rif->ring_up)
    {
      l3_connector_router_if_send (m, rx_portid, rx_queueid, rif);
    }

  /* TODO: add fib lookup and forwarding process */

  if (cif->sockfd > 0 && cif->ring_up)
    {
      l3_connector_capture_if_send (m, rx_portid, rx_queueid, cif);
    }

  return;
}

//__thread uint32_t nb_rx_burst = 0;

static inline __attribute__ ((always_inline)) void
l3_connector_rx_burst ()
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
          l3_connector_select (m, portid, queueid);
#else
          vlan_switch_run (m, portid, queueid);
#endif

          rte_pktmbuf_free (m);
        }
    }
}

static inline __attribute__ ((always_inline)) void
l3_connector_vswitch_send_link (struct rte_mbuf *m, unsigned rx_portid,
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

  /* Skip if same port (for router_if, rx_portid is UINT16_MAX) */
  if (rx_portid != UINT16_MAX && rx_portid == tx_portid)
    return;

  if (! rib->rib_info->port[tx_portid].link.link_status)
    {
      DEBUG_SDPLANE_LOG (L3_CONNECTOR, "m: %p port[%d] link down, skipping", m,
                         tx_portid);
      return;
    }

  buffer = tx_buffer_per_q[tx_portid][tx_queueid];
  if (! buffer)
    {
      DEBUG_SDPLANE_LOG (L3_CONNECTOR, "m: %p port[%d] no tx_buffer, skipping",
                         m, tx_portid);
      return;
    }

  /* copy the packet */
  c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
  if (! c)
    {
      DEBUG_SDPLANE_LOG (L3_CONNECTOR,
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
          DEBUG_SDPLANE_LOG (
              L3_CONNECTOR, "m: %p port[%d]: vlan_id modification: %u -> %u",
              m, vswitch_link->port_id, vlan_id, vswitch_link->tag_id);
          vlan_hdr->vlan_tci = rte_cpu_to_be_16 (new_vlan_tci);
        }
      else if (vswitch_link->tag_id == 0)
        {
          /* remove vlan_hdr */
          rte_vlan_strip (c);
          DEBUG_SDPLANE_LOG (
              L3_CONNECTOR, "m: %p port[%d]: vlan_id strip: %u -> %u", m,
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
          DEBUG_SDPLANE_LOG (L3_CONNECTOR, "m: %p port[%d]: add vlan_id: %u",
                             m, vswitch_link->port_id, vswitch_link->tag_id);
          vlan_hdr->vlan_tci = rte_cpu_to_be_16 (new_vlan_tci);
        }
    }

  sent = rte_eth_tx_buffer (tx_portid, tx_queueid, buffer, c);
  if (sent)
    {
      port_statistics[tx_portid].tx += sent;
    }
}

static inline __attribute__ ((always_inline)) void
l3_connector_router_if_forward (struct rte_mbuf *m,
                                struct vswitch_conf *vswitch,
                                unsigned tx_queueid)
{
  int i;
  struct vswitch_link *vswitch_link;
  uint16_t vswitch_link_id;

  for (i = 0; i < vswitch->vswitch_port_size; i++)
    {
      vswitch_link_id = vswitch->vswitch_link_id[i];
      if (vswitch_link_id >= rib->rib_info->vswitch_link_size)
        continue;

      vswitch_link = &rib->rib_info->vswitch_link[vswitch_link_id];

      if (vswitch_link->vswitch_id != vswitch->vswitch_id)
        continue;

      l3_connector_vswitch_send_link (m, UINT16_MAX, 0, vswitch_link->port_id,
                                      tx_queueid, vswitch_link);
    }
}

static inline __attribute__ ((always_inline)) void
l3_connector_tx_burst ()
{
  struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
  struct rte_mbuf *m;
  unsigned i, nb_rx;
  uint16_t tx_queueid;

  tx_queueid = lcore_id;

  if (unlikely (! rib || ! rib->rib_info))
    return;

  struct vswitch_conf *vswitch;
  int vswitch_id;

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

          l3_connector_router_if_forward (m, vswitch, tx_queueid);

          rte_pktmbuf_free (m);
        }
    }
}

static __thread uint64_t loop_counter = 0;

int
l3_connector (__rte_unused void *dummy)
{
  uint64_t prev_tsc, diff_tsc, cur_tsc;
  struct lcore_queue_conf *qconf;
  const uint64_t drain_tsc =
      (rte_get_tsc_hz () + US_PER_S - 1) / US_PER_S * BURST_TX_DRAIN_US;

  uint16_t nb_ports;

  /* the tx_buffer_per_q is initialized in rib_manager. */

  prev_tsc = 0;
  lcore_id = rte_lcore_id ();
  qconf = &lcore_queue_conf[lcore_id];

  int thread_id;
  thread_id = thread_lookup_by_lcore (l3_connector, lcore_id);
  thread_register_loop_counter (thread_id, &loop_counter);

  DEBUG_SDPLANE_LOG (L3_CONNECTOR, "entering main loop on lcore %u", lcore_id);

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_register_thread ();
#endif /*HAVE_LIBURCU_QSBR*/

  /* Wait for RIB to be initialized */
  int rib_wait_count = 0;
  while (! rcu_dereference (rcu_global_ptr_rib) && rib_wait_count < 100)
    {
      rte_delay_ms (10);
      rib_wait_count++;
    }

  if (! rcu_dereference (rcu_global_ptr_rib))
    {
      DEBUG_SDPLANE_LOG (L3_CONNECTOR,
                         "RIB not available after wait, exiting");
      return -1;
    }

  while (! force_quit && ! force_stop[lcore_id])
    {
      cur_tsc = rte_rdtsc ();

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_lock ();
      rib = (struct rib *) rcu_dereference (rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

      diff_tsc = cur_tsc - prev_tsc;
      if (unlikely (diff_tsc > drain_tsc))
        {
          l3_connector_tx_flush ();
          prev_tsc = cur_tsc;
        }

      l3_connector_rx_burst ();
      l3_connector_tx_burst ();

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
