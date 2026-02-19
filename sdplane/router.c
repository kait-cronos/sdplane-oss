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

#include <linux/rtnetlink.h>

#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

#include <sdplane/debug_log.h>
#include "debug_sdplane.h"

#include "l2fwd_export.h"
#include "sdplane.h"
#include "tap_handler.h"

#include "rib_manager.h"
#include "thread_info.h"

#include "packet_hdr.h"
#include "packet_gen.h"
#include "rte_override.h"

#include "fib.h"
#include "log_packet.h"
#include "tap_handler.h"

static __thread unsigned lcore_id;
static __thread struct rib *rib = NULL;

#include "net_subr.h"

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
  struct rte_ipv4_hdr *ipv4 = NULL;
  struct rte_ipv6_hdr *ipv6 = NULL;
  struct rte_icmp_hdr *icmp = NULL;

  eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  eth_type = rte_be_to_cpu_16 (eth->ether_type);

  if (eth_type == RTE_ETHER_TYPE_VLAN)
    {
      uint16_t eth_proto;
      vlan = (struct rte_vlan_hdr *) (eth + 1);
      eth_proto = rte_be_to_cpu_16 (vlan->eth_proto);
      if (eth_proto == RTE_ETHER_TYPE_IPV4)
        ipv4 = (struct rte_ipv4_hdr *) (vlan + 1);
      else if (eth_proto == RTE_ETHER_TYPE_IPV6)
        ipv6 = (struct rte_ipv6_hdr *) (vlan + 1);
    }
  else
    {
      if (eth_type == RTE_ETHER_TYPE_IPV4)
        ipv4 = (struct rte_ipv4_hdr *) (eth + 1);
      else if (eth_type == RTE_ETHER_TYPE_IPV6)
        ipv6 = (struct rte_ipv6_hdr *) (eth + 1);
    }

  uint8_t ip_proto;
  if (ipv4)
    {
      ip_proto = ipv4->next_proto_id;
      if (ip_proto == IPPROTO_ICMP)
        icmp = (struct rte_icmp_hdr *) (ipv4 + 1);
    }
  else if (ipv6)
    {
      ip_proto = ipv6->proto;
      if (ip_proto == IPPROTO_ICMPV6)
        icmp = (struct rte_icmp_hdr *) (ipv6 + 1);
    }

  DEBUG_NEW (ROUTER, "m: %p received on port: %d queue: %d",
             m, rx_portid, rx_queueid);

  /* 1. search and select rx_vswitch */
  port_conf = &rib->rib_info->port[rx_portid];
  if (eth_type == RTE_ETHER_TYPE_VLAN)
    {
      uint16_t vlan_id;
      vlan_id = RTE_VLAN_TCI_ID (rte_be_to_cpu_16 (vlan->vlan_tci));
      DEBUG_NEW (ROUTER, "m: %p tagged: vlan: %u", m, vlan_id);

      for (i = 0; i < port_conf->vlan_size; i++)
        {
          struct vswitch_link *vs_link;
          vswitch_link_id = port_conf->vswitch_link_id_of_vlan[i];
          vs_link = &rib->rib_info->vswitch_link[vswitch_link_id];
          if (vs_link->tag_id == vlan_id)
            {
              vswitch_link = vs_link;
              DEBUG_NEW (ROUTER,
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
          DEBUG_NEW (ROUTER,
                     "m: %p untag: vswitch: %u",
                     m, vswitch_link->vswitch_id);
        }
    }

  if (! vswitch_link)
    {
      DEBUG_NEW (ROUTER, "m: %p cannot find the vswitch link", m);
      return;
    }

  if (vswitch_link->vswitch_id < 0 ||
      rib->rib_info->vswitch_size > MAX_VSWITCH)
    {
      DEBUG_NEW (ROUTER,
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
      DEBUG_NEW (ROUTER, "m: %p cannot find the vswitch", m);
      return;
    }

  if (IS_DEBUG (ROUTER) || IS_DEBUG (PACKET))
    log_packet (m, rx_portid, rx_queueid);

  /* 2. send to capture_if */
  struct capture_if *cif = &vswitch->capture_if;
  if (cif->sockfd >= 0 && cif->ring_up)
    _send_ring (m, rx_portid, rx_queueid, cif->ring_up);

  struct router_if *rif = &vswitch->router_if;

  /* check if destination MAC is ours */
  if (rte_is_same_ether_addr (&eth->dst_addr, &rif->mac_addr))
    {
      if (ipv4 && ipv4->dst_addr == rif->ipv4_addr.s_addr)
        {
          DEBUG_NEW (ROUTER, "m: %p mac_addr+ipv4 match: send to router_if", m,
                     eth_type);
          _send_router_if_ring (m, rx_portid, rx_queueid, rif);
          return;
        }
      else if (ipv6 && memcmp (IPV6_ADDR_BYTES (ipv6->dst_addr),
                               &rif->ipv6_addr, sizeof (struct in6_addr)) == 0)
        {
          DEBUG_NEW (ROUTER, "m: %p mac_addr+ipv6 match: send to router_if", m,
                     eth_type);
          _send_router_if_ring (m, rx_portid, rx_queueid, rif);
          return;
        }
    }

  /* check if destination MAC is multicast or broadcast */
  bool is_mcast_bcast = false;
  if (rte_is_multicast_ether_addr (&eth->dst_addr) ||
      rte_is_broadcast_ether_addr (&eth->dst_addr))
    {
      is_mcast_bcast = true;
      DEBUG_NEW (ROUTER, "m: %p mac_addr bcast/mcast: send to router_if",
                 m, eth_type);
      _send_router_if_ring (m, rx_portid, rx_queueid, rif);
      /* fall through */
    }

  /* 3. check if control packet (ARP, ICMP, etc.) */
  bool is_control = false;
  if (rif->sockfd >= 0 && rif->ring_up)
    {
      is_control = _check_control_packet (m, vswitch, eth_type, eth,
                                          vlan, ipv4, ipv6, icmp);
      if (is_control)
        {
          if (! is_mcast_bcast) //unicast
            {
              DEBUG_NEW (ROUTER,
                  "m: %p control packet (eth_type:0x%04x), send to router_if", m,
                  eth_type);
              _send_router_if_ring (m, rx_portid, rx_queueid, rif);
            }

          return;
        }
    }

  /* 4. check if l3_forwarding */
  if (! rte_is_same_ether_addr (&eth->dst_addr, &rif->mac_addr))
    {
      DEBUG_NEW (ROUTER, "L2 switching");
      _switching (m, rx_portid, rx_queueid,
                  vswitch_link, vswitch, eth);
      return;
    }

  /* 5. verify IP packet */
  if (_verify_packet (m, ipv4, ipv6) != 0)
    {
      DEBUG_NEW (ROUTER, "m: %p failed verify", m);
      return;
    }

  /* 6. forwarding */
  _forwarding (m, rx_portid, rx_queueid,
               eth, ipv4, ipv6,
               vswitch, vswitch_link);

  return;
}

static inline __attribute__ ((always_inline)) void
_process_tx_packet (struct rte_mbuf *m, struct vswitch_conf *vswitch)
{
  struct rte_ether_hdr *eth;
  uint16_t eth_type;
  struct rte_vlan_hdr *vlan = NULL;
  struct rte_ipv4_hdr *ipv4 = NULL;
  struct rte_ipv6_hdr *ipv6 = NULL;

  eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  eth_type = rte_be_to_cpu_16 (eth->ether_type);
  if (eth_type == RTE_ETHER_TYPE_VLAN)
    {
      uint16_t eth_proto;
      vlan = (struct rte_vlan_hdr *) (eth + 1);
      eth_proto = rte_be_to_cpu_16 (vlan->eth_proto);
      if (eth_proto == RTE_ETHER_TYPE_IPV4)
        ipv4 = (struct rte_ipv4_hdr *) (vlan + 1);
      else if (eth_proto == RTE_ETHER_TYPE_IPV6)
        ipv6 = (struct rte_ipv6_hdr *) (vlan + 1);
    }
  else
    {
      if (eth_type == RTE_ETHER_TYPE_IPV4)
        ipv4 = (struct rte_ipv4_hdr *) (eth + 1);
      else if (eth_type == RTE_ETHER_TYPE_IPV6)
        ipv6 = (struct rte_ipv6_hdr *) (eth + 1);
    }

  DEBUG_NEW (ROUTER,
             "m: %p received on port: %d queue: %d",
             m, ROUTER_IF_RX_SELF_PORT_ID, ROUTER_IF_RX_SELF_QUEUE_ID);

#if 0
  /* check if this is a non-IP packet or multicast or broadcast */
  if ((! ipv4 && ! ipv6) ||
      rte_is_multicast_ether_addr (&eth->dst_addr) ||
      rte_is_broadcast_ether_addr (&eth->dst_addr))
    {
      _switching (m, ROUTER_IF_RX_SELF_PORT_ID, ROUTER_IF_RX_SELF_QUEUE_ID,
                  NULL, vswitch, eth);
      return;
    }

  _forwarding (m,
               ROUTER_IF_RX_SELF_PORT_ID, ROUTER_IF_RX_SELF_QUEUE_ID,
               eth, ipv4, ipv6,
               vswitch, NULL);
#else
  /* This is a packet sent from the Linux tap thru
     the packet_down(ring_dn).  I think it is safe to send it
     without any condition, and I think indeed it should be sent,
     for the proper L3-switch behavior. */
  _switching (m, ROUTER_IF_RX_SELF_PORT_ID, ROUTER_IF_RX_SELF_QUEUE_ID,
              NULL, vswitch, eth);
#endif
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

      //DEBUG_NEW (ROUTER, "process vswitch[%d]", vswitch_id);

      if (vswitch->router_if.sockfd < 0 || ! vswitch->router_if.ring_dn)
        continue;

      nb_rx =
          rte_ring_dequeue_burst (vswitch->router_if.ring_dn,
                                  (void **) pkts_burst, MAX_PKT_BURST, NULL);

      if (unlikely (nb_rx == 0))
        continue;

      DEBUG_NEW (ROUTER, "process vswitch[%d]: nb_rx: %d", vswitch_id, nb_rx);

      for (i = 0; i < nb_rx; i++)
        {
          m = pkts_burst[i];
          if (! m)
            continue;

          rte_prefetch0 (rte_pktmbuf_mtod (m, void *));

          DEBUG_NEW (ROUTER, "m: %p vswitch[%d]: packet[%d]",
                     m, vswitch_id, i);
          _process_tx_packet (m, vswitch);

          rte_pktmbuf_free (m);
        }
    }
}

static __thread uint64_t loop_counter = 0;

int
router (__rte_unused void *dummy)
{
  uint64_t prev_tsc, diff_tsc, cur_tsc;
  const uint64_t drain_tsc =
      (rte_get_tsc_hz () + US_PER_S - 1) / US_PER_S * BURST_TX_DRAIN_US;

  /* the tx_buffer_per_q is initialized in rib_manager. */

  prev_tsc = 0;
  lcore_id = rte_lcore_id ();

  int thread_id;
  thread_id = thread_lookup_by_lcore (router, lcore_id);
  thread_register_loop_counter (thread_id, &loop_counter);

  DEBUG_NEW (ROUTER, "entering main loop on lcore %u", lcore_id);

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
