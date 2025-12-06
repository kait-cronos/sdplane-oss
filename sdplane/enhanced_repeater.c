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

#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>

#include <sdplane/debug_log.h>
#include "debug_sdplane.h"

#include "l2fwd_export.h"
#include "sdplane.h"
#include "tap_handler.h"

#include "rib_manager.h"
#include "thread_info.h"

#include "packet_hdr.h"
#include "rte_override.h"

#include "dhcp_server.h"

static __thread unsigned lcore_id;
static __thread struct rib *rib = NULL;

extern struct rte_eth_dev_tx_buffer
    *tx_buffer_per_q[RTE_MAX_ETHPORTS][RTE_MAX_LCORE];

/* _thread_tx_flush() flushes the queue'ed packets
   in tx_buffer_per_q[] onto the NIC. */
static inline __attribute__ ((always_inline)) void
_thread_tx_flush ()
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
            DEBUG_NEW (ENHANCED_REPEATER,
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

static inline __attribute__ ((always_inline)) int
_send_ring (struct rte_mbuf *m,
            unsigned rx_portid, unsigned rx_queueid,
            struct rte_ring *ring)
{
  struct rte_mbuf *c;
  uint32_t pkt_len;
  uint16_t data_len;
  int ret;
  pkt_len = rte_pktmbuf_pkt_len (m);
  data_len = rte_pktmbuf_data_len (m);

  DEBUG_NEW (ENHANCED_REPEATER,
             "m: %p port %d queue %d to ring: %s (%p)",
             m, rx_portid, rx_queueid, ring->name, ring);

  c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
  if (! c)
    return -1;

  ret = rte_ring_enqueue (ring, c);
  if (ret)
    {
      /* enqueue failed */
      DEBUG_NEW (ENHANCED_REPEATER,
          "lcore[%d]: m: %p port %d queue %d to ring %s: %s: %d",
          lcore_id, m, rx_portid, rx_queueid,
          ring->name, (ret == -ENOBUFS ? "ENOBUFS" : "failed"), ret);

      rte_pktmbuf_free (c);
      return -1;
    }

  return 0;
}

static inline __attribute__ ((always_inline)) void
_send_link (struct rte_mbuf *m, unsigned rx_portid, unsigned rx_queueid,
            unsigned tx_portid, unsigned tx_queueid,
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
      DEBUG_NEW (ENHANCED_REPEATER,
                 "lcore[%d]: m: %p router_if -> port %d "
                 "rte_pktmbuf_copy() failed.",
                 lcore_id, m, tx_portid);
      return;
    }

  struct rte_ether_hdr *eth;
  uint16_t eth_type;
  struct rte_vlan_hdr *vlan = NULL;
  uint16_t vlan_id;
  uint16_t old_vlan_tci, new_vlan_tci;

  eth = rte_pktmbuf_mtod (c, struct rte_ether_hdr *);
  eth_type = rte_be_to_cpu_16 (eth->ether_type);

  /* operate on the VLAN header */
  if (eth_type == RTE_ETHER_TYPE_VLAN)
    {
      vlan = (struct rte_vlan_hdr *) (eth + 1);
      vlan_id = RTE_VLAN_TCI_ID (rte_be_to_cpu_16 (vlan->vlan_tci));

      if (vswitch_link->tag_id != 0 && vswitch_link->tag_id != vlan_id)
        {
          /* vlan_id translation: modify vlan_id on tx */
          old_vlan_tci = rte_be_to_cpu_16 (vlan->vlan_tci);
          new_vlan_tci =
              ((old_vlan_tci & 0xf000) | (vswitch_link->tag_id & 0x0fff));
          DEBUG_NEW (ENHANCED_REPEATER,
                     "m: %p port[%d]: vlan_id modification: %u -> %u",
                     m, vswitch_link->port_id, vlan_id, vswitch_link->tag_id);
          vlan->vlan_tci = rte_cpu_to_be_16 (new_vlan_tci);
        }
      else if (vswitch_link->tag_id == 0)
        {
          /* remove vlan_hdr */
          rte_vlan_strip (c);
          DEBUG_NEW (ENHANCED_REPEATER,
                     "m: %p port[%d]: vlan_id strip: %u -> %u",
                     m, vswitch_link->port_id, vlan_id, vswitch_link->tag_id);
        }
    }
  else
    {
      if (vswitch_link->tag_id != 0)
        {
          /* insert vlan_hdr */
          rte_vlan_insert (&c);
          eth = rte_pktmbuf_mtod (c, struct rte_ether_hdr *);
          eth_type = rte_be_to_cpu_16 (eth->ether_type);
          assert (eth_type == RTE_ETHER_TYPE_VLAN);
          vlan = (struct rte_vlan_hdr *) (eth + 1);
          old_vlan_tci = rte_be_to_cpu_16 (vlan->vlan_tci);
          new_vlan_tci =
              ((old_vlan_tci & 0xf000) | (vswitch_link->tag_id & 0x0fff));
          DEBUG_NEW (ENHANCED_REPEATER,
                     "m: %p port[%d]: add vlan_id: %u",
                     m, vswitch_link->port_id, vswitch_link->tag_id);
          vlan->vlan_tci = rte_cpu_to_be_16 (new_vlan_tci);
        }
    }

  sent = rte_eth_tx_buffer (tx_portid, tx_queueid, buffer, c);
  if (sent)
    port_statistics[tx_portid].tx += sent;
}

/* imaginary port_id/queue_id to avoid
   split-horizon check in transmission. */
#define ROUTER_IF_TX_SELF_PORT_ID  UINT16_MAX
#define ROUTER_IF_TX_SELF_QUEUE_ID 0

static inline __attribute__ ((always_inline)) void
_switching (struct rte_mbuf *m, struct vswitch_conf *vswitch,
            struct rte_ether_hdr *eth, struct rte_vlan_hdr *vlan,
            uint16_t eth_type)
{
}

static inline __attribute__ ((always_inline)) void
_process_rx_packet (struct rte_mbuf *m, unsigned rx_portid,
                    unsigned rx_queueid)
{
  struct port_conf *port_conf;
  uint16_t vswitch_link_id;
  struct vswitch_conf *vswitch = NULL;
  struct vswitch_link *vswitch_link = NULL;
  int i;
  int ret;

  struct rte_ether_hdr *eth;
  uint16_t eth_type;
  struct rte_vlan_hdr *vlan = NULL;

  eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  eth_type = rte_be_to_cpu_16 (eth->ether_type);

  DEBUG_NEW (ENHANCED_REPEATER, "m: %p received on port: %d queue: %d",
             m, rx_portid, rx_queueid);

  /* 1. search and select rx_vswitch */
  port_conf = &rib->rib_info->port[rx_portid];
  if (eth_type == RTE_ETHER_TYPE_VLAN)
    {
      uint16_t vlan_id;
      vlan = (struct rte_vlan_hdr *) (eth + 1);
      vlan_id = RTE_VLAN_TCI_ID (rte_be_to_cpu_16 (vlan->vlan_tci));
      DEBUG_NEW (ENHANCED_REPEATER, "m: %p tagged: vlan: %u", m, vlan_id);

      for (i = 0; i < port_conf->vlan_size; i++)
        {
          struct vswitch_link *vs_link;
          vswitch_link_id = port_conf->vswitch_link_id_of_vlan[i];
          vs_link = &rib->rib_info->vswitch_link[vswitch_link_id];
          DEBUG_NEW (ENHANCED_REPEATER,
                     "m: %p check: vswitch: %u, vlan: %u",
                     m, vs_link->vswitch_id, vs_link->tag_id);
          if (vs_link->tag_id == vlan_id)
            {
              vswitch_link = vs_link;
              DEBUG_NEW (ENHANCED_REPEATER,
                         "m: %p tagged: vswitch: %u, vlan: %u",
                         m, vswitch_link->vswitch_id, vlan_id);
              break;
            }
        }
    }
  else
    {
      /* in case of untagged port, direct the packet to the default vlan
         for the port. */
      vswitch_link_id = port_conf->vswitch_link_id_of_native_vlan;
      if (0 <= vswitch_link_id &&
          vswitch_link_id < rib->rib_info->vswitch_link_size)
        {
          vswitch_link = &rib->rib_info->vswitch_link[vswitch_link_id];
          DEBUG_NEW (ENHANCED_REPEATER,
                     "m: %p untag: vswitch: %u",
                     m, vswitch_link->vswitch_id);
        }
    }

  if (! vswitch_link)
    {
      DEBUG_NEW (ENHANCED_REPEATER, "m: %p cannot find the vswitch link", m);
      return;
    }

  if (vswitch_link->vswitch_id < 0 ||
      rib->rib_info->vswitch_size > MAX_VSWITCH)
    {
      DEBUG_NEW (ENHANCED_REPEATER,
                 "m: %p a broken vswitch link: vswitch: %d vswitch_size: %d",
                 m, vswitch_link->vswitch_id, rib->rib_info->vswitch_size);
      return;
    }

  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      if (vswitch_link->vswitch_id == rib->rib_info->vswitch[i].vswitch_id)
        {
          vswitch = &rib->rib_info->vswitch[i];
          break;
        }
    }

  if (! vswitch)
    {
      DEBUG_NEW (ENHANCED_REPEATER, "m: %p cannot find the vswitch", m);
      return;
    }

  DEBUG_NEW (ENHANCED_INFO,
      "m: %p received: port: %d queue: %d tag: %d "
      "vswitch[%d]: (vlan: %d) rx-vlink[%d]: (tag: %d port: %d)",
      m, rx_portid, rx_queueid, vlan_id,
      vswitch->vswitch_id, vswitch->vlan_id,
      vswitch_link_id, vswitch_link->tag_id, vswitch_link->port_id);

  unsigned tx_queueid = lcore_id;
  for (i = 0; i < vswitch->vswitch_port_size; i++)
    {
      uint16_t vswitch_link_id = vswitch->vswitch_link_id[i];
      struct vswitch_link *link =
          &rib->rib_info->vswitch_link[vswitch_link_id];
      unsigned tx_portid = link->port_id;

      DEBUG_NEW (ENHANCED_REPEATER,
          "m: %p vswitch[%d] vswport[%d/%d]: vswitch_link_id: %u "
          "port: %d vlan: %d tag: %d (vswitch%u[%d])",
          m, link->vswitch_id, i, vswitch->vswitch_port_size,
          vswitch_link_id, link->port_id, link->vlan_id, link->tag_id,
          link->vswitch_id, link->vswitch_port);

      DEBUG_NEW (ENHANCED_INFO,
          "m: %p vswitch[%d] tx-vlink[%d/%d]: vswitch_link_id: %u "
          "port: %d vlan: %d tag: %d (vswitch%u[%d])",
          m, vswitch->vswitch_id, i, vswitch->vswitch_port_size,
          link->vswitch_link_id,
	  link->port_id, link->vlan_id, link->tag_id);

      /* skip received link port. split-horizon */
      if (link == vswitch_link)
        {
          DEBUG_NEW (ENHANCED_INFO, "m: %p split horizon.", m);
          continue;
        }

      /* skip if the tx_port is stopped. */
      if (unlikely (rib->rib_info->port[tx_portid].is_stopped))
        continue;

      /* forward to dpdk ports, accoding to the vswitch_link. */
      _send_link (m, rx_portid, rx_queueid, tx_portid, tx_queueid, link);
    }

  struct router_if *rif;

  /* router-if */
  rif = &vswitch->router_if;
  if (rif->sockfd >= 0 && rif->ring_up)
    {
      struct rte_mbuf *c;
      c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
      if (c)
        {
          if (rif->vlan_id)
            {
              if (is_rte_vlan_hdr (c))
                {
                  DEBUG_NEW (ENHANCED_INFO,
                             "m: %p router-if: vlan modify: %d.",
                             m, rif->vlan_id);
                  rte_vlan_hdr_set (c, rif->vlan_id);
                }
              else
                {
                  DEBUG_NEW (ENHANCED_INFO,
                             "m: %p router-if: vlan insert: %d.",
                             m, rif->vlan_id);
                  rte_vlan_insert (&c);
                  rte_vlan_hdr_set (c, rif->vlan_id);
                }
            }
          else
            {
              if (is_rte_vlan_hdr (c))
                {
                  DEBUG_NEW (ENHANCED_INFO,
                             "m: %p router-if: vlan strip", m);
                  rte_vlan_strip (c);
                }
            }

          _send_ring (c, rx_portid, rx_queueid, rif->ring_up);
          rte_pktmbuf_free (c);
        }
    }

  /* 2. send to capture_if */
  struct capture_if *cif = &vswitch->capture_if;
  if (cif->sockfd >= 0 && cif->ring_up)
    {
      DEBUG_NEW (ENHANCED_INFO,
                 "m: %p capture-if.", m);
      _send_ring (m, rx_portid, rx_queueid, cif->ring_up);
    }

  /* application */
  for (i = 0; i < rib->rib_info->application_slot_size; i++)
    {
      struct application_slot_entry *app;
      app = &rib->rib_info->application_slot[i];
      if (app->ring && (*app->is_packet_match) (m))
        {
          DEBUG_NEW (ENHANCED_REPEATER, "call appli_slot[%d]", i);
          _send_ring (m, rx_portid, rx_queueid, app->ring);
        }
    }
}

static inline __attribute__ ((always_inline)) void
_thread_rx_burst ()
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

      /* skip if the rx_port is stopped. */
      if (unlikely (rib->rib_info->port[portid].is_stopped))
        continue;

      nb_rx = rte_eth_rx_burst (portid, queueid, pkts_burst, MAX_PKT_BURST);
      // nb_rx_burst++;

      if (unlikely (nb_rx == 0))
        continue;

      port_statistics[portid].rx += nb_rx;

      for (j = 0; j < nb_rx; j++)
        {
          m = pkts_burst[j];
          rte_prefetch0 (rte_pktmbuf_mtod (m, void *));

          _process_rx_packet (m, portid, queueid);

          rte_pktmbuf_free (m);
        }
    }
}

/* tx function of packets from router_if to physical port. */
static inline __attribute__ ((always_inline)) void
_thread_tx_burst ()
{
  struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
  struct rte_mbuf *m;
  unsigned i, nb_rx;

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

          unsigned j;
          struct vswitch_link *vswitch_link;
          uint16_t vswitch_link_id;
          uint16_t tx_queueid;
          tx_queueid = lcore_id;
          for (j = 0; j < vswitch->vswitch_port_size; j++)
            {
              vswitch_link_id = vswitch->vswitch_link_id[j];
              if (vswitch_link_id >= rib->rib_info->vswitch_link_size)
                continue;

              vswitch_link = &rib->rib_info->vswitch_link[vswitch_link_id];

              if (vswitch_link->vswitch_id != vswitch->vswitch_id)
                continue;

              _send_link (m,
                  ROUTER_IF_TX_SELF_PORT_ID, ROUTER_IF_TX_SELF_QUEUE_ID,
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
  const uint64_t drain_tsc =
      (rte_get_tsc_hz () + US_PER_S - 1) / US_PER_S * BURST_TX_DRAIN_US;

  /* the tx_buffer_per_q is initialized in rib_manager. */

  prev_tsc = 0;
  lcore_id = rte_lcore_id ();

  int thread_id;
  thread_id = thread_lookup_by_lcore (enhanced_repeater, lcore_id);
  thread_register_loop_counter (thread_id, &loop_counter);

  DEBUG_NEW (ENHANCED_REPEATER, "entering main loop on lcore %u", lcore_id);

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

      /* better to add processing here only when absolutely necessary
         or cannot be postponed. */

      diff_tsc = cur_tsc - prev_tsc;
      if (unlikely (diff_tsc > drain_tsc))
        {
          _thread_tx_flush ();
          prev_tsc = cur_tsc;
        }

      _thread_rx_burst ();
      _thread_tx_burst ();

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_unlock ();
      urcu_qsbr_quiescent_state ();
#endif /*HAVE_LIBURCU_QSBR*/

      loop_counter++;
    }

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_unregister_thread ();
#endif /*HAVE_LIBURCU_QSBR*/

  return 0;
}
