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

#include "fib.h"
#include "log_packet.h"
#include "tap_handler.h"

#include "rte_override.h"

static __thread unsigned lcore_id;
static __thread struct rib *rib = NULL;

extern struct rte_eth_dev_tx_buffer
    *tx_buffer_per_q[RTE_MAX_ETHPORTS][RTE_MAX_LCORE];

/* router_tx_flush() flushes the queue'ed packets
   in tx_buffer_per_q[] onto the NIC. */
static inline __attribute__ ((always_inline)) void
_router_tx_flush ()
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
            DEBUG_SDPLANE_LOG (ROUTER,
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
      DEBUG_SDPLANE_LOG (ROUTER,
                         "lcore[%d]: m: %p L2 forward -> port %d "
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

  if (eth_type == RTE_ETHER_TYPE_VLAN)
    vlan = (struct rte_vlan_hdr *) (eth + 1);

  /* operate VLAN header */
  if (eth_type == RTE_ETHER_TYPE_VLAN)
    {
      vlan_id = RTE_VLAN_TCI_ID (rte_be_to_cpu_16 (vlan->vlan_tci));

      if (vswitch_link->tag_id != 0 && vswitch_link->tag_id != vlan_id)
        {
          /* vlan_id translation: modify vlan_id on tx */
          old_vlan_tci = rte_be_to_cpu_16 (vlan->vlan_tci);
          new_vlan_tci =
              ((old_vlan_tci & 0xf000) | (vswitch_link->tag_id & 0x0fff));
          DEBUG_SDPLANE_LOG (
              ROUTER, "m: %p port[%d]: vlan_id modification: %u -> %u", m,
              vswitch_link->port_id, vlan_id, vswitch_link->tag_id);
          vlan->vlan_tci = rte_cpu_to_be_16 (new_vlan_tci);
        }
      else if (vswitch_link->tag_id == 0)
        {
          /* remove vlan_hdr */
          rte_vlan_strip (c);
          DEBUG_SDPLANE_LOG (ROUTER, "m: %p port[%d]: vlan_id strip: %u -> %u",
                             m, vswitch_link->port_id, vlan_id,
                             vswitch_link->tag_id);
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
          DEBUG_SDPLANE_LOG (ROUTER, "m: %p port[%d]: add vlan_id: %u", m,
                             vswitch_link->port_id, vswitch_link->tag_id);
          vlan->vlan_tci = rte_cpu_to_be_16 (new_vlan_tci);
        }
    }

  sent = rte_eth_tx_buffer (tx_portid, tx_queueid, buffer, c);
  if (sent)
    port_statistics[tx_portid].tx += sent;
}

static inline __attribute__ ((always_inline)) int
_verify_packet (struct rte_mbuf *m, struct rte_ipv4_hdr *ipv4,
                struct rte_ipv6_hdr *ipv6)
{
  if (ipv4)
    {
      /* check IPv4 header. see RFC1814 section 5.2.2. */
      if (rte_pktmbuf_data_len (m) < sizeof (struct rte_ipv4_hdr))
        {
          DEBUG_SDPLANE_LOG (ROUTER,
                             "error: m:%p %d bytes is smaller than 20 bytes",
                             m, rte_pktmbuf_data_len (m));
          return -1;
        }
      if (rte_ipv4_cksum (ipv4))
        {
          DEBUG_SDPLANE_LOG (ROUTER, "error: IPv4 checksum");
          return -1;
        }
      if (ipv4->version != IPVERSION)
        {
          DEBUG_SDPLANE_LOG (ROUTER, "error: version is %d", ipv4->version);
          return -1;
        }
      if (rte_ipv4_hdr_len (ipv4) < sizeof (struct rte_ipv4_hdr))
        {
          DEBUG_SDPLANE_LOG (ROUTER,
                             "error: %d bytes is smaller than 20 bytes",
                             rte_ipv4_hdr_len (ipv4));
          return -1;
        }
      if (rte_be_to_cpu_16 (ipv4->total_length) < sizeof (struct rte_ipv4_hdr))
        {
          DEBUG_SDPLANE_LOG (ROUTER,
                             "error: %d bytes is smaller than 20 bytes",
                             rte_be_to_cpu_16 (ipv4->total_length));
          return -1;
        }

      if (ipv4->time_to_live <= 1)
        {
          DEBUG_SDPLANE_LOG (ROUTER, "error: TTL is %d", ipv4->time_to_live);
          /* TODO: send ICMP TIME EXCEEDED */
          return -1;
        }
    }
  else if (ipv6)
    {
      if (rte_pktmbuf_data_len (m) < sizeof (struct rte_ipv6_hdr))
        {
          DEBUG_SDPLANE_LOG (ROUTER,
                             "error: m:%p %d bytes is smaller than 40 bytes",
                             m, rte_pktmbuf_data_len (m));
          return -1;
        }
      uint8_t version = ((const uint8_t *) ipv6)[0];
      if ((version & 0xf0) != 0x60)
        {
          DEBUG_SDPLANE_LOG (ROUTER, "error: version is %d", version);
          return -1;
        }

      if (ipv6->hop_limits <= 1)
        {
          DEBUG_SDPLANE_LOG (ROUTER, "error: Hop Limit is %d",
                             ipv6->hop_limits);
          /* TODO: send ICMPv6 TIME EXCEEDED */
          return -1;
        }
    }
  return 0;
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
  /* for ARP replies from tap, we need L2 switching based on dst MAC */
  DEBUG_SDPLANE_LOG (ROUTER,
                     "m: %p non-IP packet from router_if (eth_type: 0x%04x), "
                     "L2 switching",
                     m, eth_type);

  /* check if destination MAC is multicast or broadcast */
  if (rte_is_multicast_ether_addr (&eth->dst_addr) ||
      rte_is_broadcast_ether_addr (&eth->dst_addr))
    {
      unsigned tx_queueid = lcore_id;
      DEBUG_SDPLANE_LOG (
          ROUTER,
          "m: %p multicast/broadcast from router_if, flooding to VLAN %u", m,
          vswitch->vswitch_id);

      /* flood to all ports in the same vswitch */
      for (int i = 0; i < vswitch->vswitch_port_size; i++)
        {
          uint16_t link_id = vswitch->vswitch_link_id[i];
          struct vswitch_link *link = &rib->rib_info->vswitch_link[link_id];
          unsigned tx_portid = link->port_id;

          /* skip if the tx_port is stopped */
          if (unlikely (rib->rib_info->port[tx_portid].is_stopped))
            continue;

          /* send to DPDK ports according to the vswitch_link */
          _send_link (m, ROUTER_IF_TX_SELF_PORT_ID, ROUTER_IF_TX_SELF_QUEUE_ID,
                      tx_portid, tx_queueid, link);
        }
      return;
    }

  /* L2 switching: lookup dst MAC in FDB */
  int dst_port = -1;

  /* search for the destination MAC in FDB */
  for (int i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vs = &rib->rib_info->vswitch[i];
      if (vs->is_deleted || vs != vswitch)
        continue;

      for (int j = 0; j < vs->vswitch_port_size; j++)
        {
          uint16_t link_id = vs->vswitch_link_id[j];
          struct vswitch_link *link = &rib->rib_info->vswitch_link[link_id];

          dst_port =
              fdb_lookup_entry (rib->rib_info, &eth->dst_addr, link->tag_id);
          if (dst_port >= 0 && link->port_id == dst_port)
            {
              unsigned tx_queueid = lcore_id;
              DEBUG_SDPLANE_LOG (ROUTER,
                                 "m: %p L2 switching to port %d via link %d",
                                 m, dst_port, link_id);
              _send_link (m, ROUTER_IF_TX_SELF_PORT_ID,
                          ROUTER_IF_TX_SELF_QUEUE_ID, dst_port, tx_queueid,
                          link);
              return;
            }
        }
    }

  DEBUG_SDPLANE_LOG (ROUTER,
                     "m: %p L2 switching: dst MAC not found in FDB, drop", m);
  return;
}

static inline __attribute__ ((always_inline)) void
_forwarding (struct rte_mbuf *m, unsigned rx_portid, unsigned rx_queueid,
             struct rte_ether_hdr *eth, struct rte_ipv4_hdr *ipv4,
             struct rte_ipv6_hdr *ipv6, struct vswitch_conf *vswitch,
             struct vswitch_link *vswitch_link)
{
  struct route_entry *route_entry;
  struct neigh_entry *neigh_entry;
  struct rte_ether_addr *dst_mac;
  struct fib_node *fib_node;
  uint8_t dst_ip[16] = { 0 };
  int i, j;
  struct vswitch_link *tx_link = NULL;
  int dst_port = -1;

  if (ipv4)
    memcpy (dst_ip, &ipv4->dst_addr, sizeof (ipv4->dst_addr));
  else if (ipv6)
    memcpy (dst_ip, IPV6_ADDR_BYTES (ipv6->dst_addr), RTE_IPV6_ADDR_SIZE);

  /* FIB lookup */
  int tree_idx = (ipv4) ? 0 : 1; // IPv4=0, IPv6=1
  fib_node = fib_route_lookup (rib->rib_info->fib_tree[tree_idx], dst_ip);
  if (! fib_node || fib_node->num_routes == 0)
    {
      DEBUG_SDPLANE_LOG (ROUTER, "m: %p FIB lookup failed, send to router_if",
                         m);
      return;
    }
  route_entry =
      &rib->rib_info
           ->route_table[fib_node->route_idx[0]]; // TODO: ECMP support

  char nexthop_str[INET6_ADDRSTRLEN];
  inet_ntop (route_entry->family, route_entry->nexthop, nexthop_str,
             sizeof (nexthop_str));
  DEBUG_SDPLANE_LOG (ROUTER, "m: %p route found: nexthop=%s family=%d", m,
                     nexthop_str, route_entry->family);

  /* determine the actual nexthop IP to lookup in neighbor table */
  uint8_t lookup_ip[16];
  memcpy (lookup_ip, route_entry->nexthop, sizeof (lookup_ip));

  /* for directly connected routes (nexthop = 0.0.0.0), use destination IP */
  bool is_zero_nexthop = true;
  for (i = 0; i < 16; i++)
    {
      if (route_entry->nexthop[i] != 0)
        {
          is_zero_nexthop = false;
          break;
        }
    }

  if (is_zero_nexthop)
    {
      memcpy (lookup_ip, dst_ip, sizeof (lookup_ip));
      char lookup_str[INET6_ADDRSTRLEN];
      inet_ntop (route_entry->family, lookup_ip, lookup_str,
                 sizeof (lookup_str));
      DEBUG_SDPLANE_LOG (
          ROUTER,
          "m: %p directly connected route, using dst IP for ARP lookup:%s", m,
          lookup_str);
    }

  /* neighbor table lookup */
  if (route_entry->family == AF_INET)
    {
      neigh_manager_lookup (&rib->rib_info->neigh_tables[NEIGH_ARP_TABLE],
                            NEIGH_ARP_TABLE, lookup_ip, &neigh_entry);
      if (! neigh_entry)
        {
          DEBUG_SDPLANE_LOG (ROUTER,
                             "m: %p ARP lookup failed, send to router_if", m);
          /* send to router_if for ARP resolution */
          struct router_if *rif = &vswitch->router_if;
          if (rif->sockfd >= 0 && rif->ring_up)
            _send_ring (m, rx_portid, rx_queueid, rif->ring_up);
          return;
        }
    }
  else
    {
      neigh_manager_lookup (&rib->rib_info->neigh_tables[NEIGH_ND_TABLE],
                            NEIGH_ND_TABLE, lookup_ip, &neigh_entry);
      if (! neigh_entry)
        {
          DEBUG_SDPLANE_LOG (ROUTER,
                             "m: %p ND lookup failed, send to router_if", m);
          /* send to router_if for ND resolution */
          struct router_if *rif = &vswitch->router_if;
          if (rif->sockfd >= 0 && rif->ring_up)
            _send_ring (m, rx_portid, rx_queueid, rif->ring_up);
          return;
        }
    }
  dst_mac = &neigh_entry->mac_addr;

  char neigh_mac_str[RTE_ETHER_ADDR_FMT_SIZE];
  rte_ether_format_addr (neigh_mac_str, sizeof (neigh_mac_str), dst_mac);
  DEBUG_SDPLANE_LOG (ROUTER, "m: %p neighbor lookup succeeded: MAC=%s",
                     m, neigh_mac_str);

  /* search tx vswitch_link */
  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vs = &rib->rib_info->vswitch[i];

      if (vs->is_deleted)
        continue;

      for (j = 0; j < vs->vswitch_port_size; j++)
        {
          uint16_t link_id = vs->vswitch_link_id[j];
          struct vswitch_link *link = &rib->rib_info->vswitch_link[link_id];

          /* FDB lookup */
          dst_port = fdb_lookup_entry (rib->rib_info, dst_mac, link->tag_id);
          if (dst_port >= 0 && link->port_id == dst_port)
            {
              /* split-horizon check: don't send back to the same link */
              if (vswitch_link && link == vswitch_link)
                continue;

              tx_link = link;
              break;
            }
        }

      if (tx_link)
        break;
    }

  if (! tx_link)
    {
      DEBUG_SDPLANE_LOG (ROUTER, "m: %p FDB lookup failed, send to router_if",
                         m);
      return;
    }

  /* check if tx_port is stopped */
  if (unlikely (rib->rib_info->port[dst_port].is_stopped))
    {
      DEBUG_SDPLANE_LOG (ROUTER, "m: %p tx_port %d is stopped", m, dst_port);
      return;
    }

  /* rewrite L2/L3 header */
  if (rx_portid != ROUTER_IF_TX_SELF_PORT_ID)
    {
      if (ipv4)
        {
          ipv4->time_to_live--;
          ipv4->hdr_checksum = 0;
          ipv4->hdr_checksum = rte_ipv4_cksum (ipv4);
        }
      else if (ipv6)
        ipv6->hop_limits--;

      struct rte_ether_addr src_mac;
      rte_eth_macaddr_get (dst_port, &src_mac);
      rte_ether_addr_copy (&src_mac, &eth->src_addr);
      rte_ether_addr_copy (dst_mac, &eth->dst_addr);
    }

  DEBUG_SDPLANE_LOG (ROUTER, "m: %p forwarding to port %d via link %d",
                     m, dst_port, tx_link->vswitch_link_id);

  unsigned tx_queueid = lcore_id;
  _send_link (m, rx_portid, rx_queueid, dst_port, tx_queueid, tx_link);
}

static inline __attribute__ ((always_inline)) void
_process_rx_packet (struct rte_mbuf *m, unsigned rx_portid,
                    unsigned rx_queueid)
{
  struct vswitch_conf *vswitch = NULL;
  struct vswitch_link *vswitch_link = NULL;
  int i;

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

  DEBUG_SDPLANE_LOG (ROUTER, "m: %p received on port: %d queue: %d", m,
                     rx_portid, rx_queueid);

  /* 1. search and select rx_vswitch */
  struct port_conf *port_conf = &rib->rib_info->port[rx_portid];
  uint16_t vswitch_link_id;
  if (eth_type == RTE_ETHER_TYPE_VLAN)
    {
      uint16_t vlan_id;
      vlan_id = RTE_VLAN_TCI_ID (rte_be_to_cpu_16 (vlan->vlan_tci));
      DEBUG_SDPLANE_LOG (ROUTER, "m: %p tagged: vlan: %u", m, vlan_id);

      for (i = 0; i < port_conf->vlan_size; i++)
        {
          struct vswitch_link *vs_link;
          vswitch_link_id = port_conf->vswitch_link_id_of_vlan[i];
          vs_link = &rib->rib_info->vswitch_link[vswitch_link_id];
          if (vs_link->tag_id == vlan_id)
            {
              vswitch_link = vs_link;
              DEBUG_SDPLANE_LOG (ROUTER, "m: %p tagged: vswitch: %u, vlan: %u",
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
          DEBUG_SDPLANE_LOG (ROUTER, "m: %p untag: vswitch: %u", m,
                             vswitch_link->vswitch_id);
        }
    }

  if (! vswitch_link)
    {
      DEBUG_SDPLANE_LOG (ROUTER, "m: %p cannot find the vswitch link", m);
      return;
    }

  if (vswitch_link->vswitch_id < 0 ||
      rib->rib_info->vswitch_size > MAX_VSWITCH)
    {
      DEBUG_SDPLANE_LOG (ROUTER,
                         "m: %p a broken vswitch link: "
                         "vswitch: %d vswitch_size: %d",
                         m, vswitch_link->vswitch_id,
                         rib->rib_info->vswitch_size);
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
      DEBUG_SDPLANE_LOG (ROUTER, "m: %p cannot find the vswitch", m);
      return;
    }

  /* 2. send to capture_if */
  struct capture_if *cif = &vswitch->capture_if;
  if (cif->sockfd >= 0 && cif->ring_up)
    _send_ring (m, rx_portid, rx_queueid, cif->ring_up);

  /* 3. check if control packet (ARP, ICMP, etc.) */
  struct router_if *rif = &vswitch->router_if;
  if (rif->sockfd >= 0 && rif->ring_up)
    {
      /* simple filter */
      bool is_control = false;
      /* ARP target_ip is me */
      uint16_t inner_eth_type = eth_type;
      if (eth_type == RTE_ETHER_TYPE_VLAN && vlan)
        inner_eth_type = rte_be_to_cpu_16 (vlan->eth_proto);

      if (inner_eth_type == RTE_ETHER_TYPE_ARP)
        {
          struct rte_arp_hdr *arp;
          if (vlan)
            arp = (struct rte_arp_hdr *) (vlan + 1);
          else
            arp = (struct rte_arp_hdr *) (eth + 1);
          if (memcmp (&arp->arp_data.arp_tip, &vswitch->router_if.ipv4_addr,
                      sizeof (struct in_addr)) == 0)
            is_control = true;
        }
        /* ND target_ip is me */
#define ICMPV6_NS 135
#define ICMPV6_NA 136
      else if (ipv6 && icmp &&
               (icmp->icmp_type == ICMPV6_NS || icmp->icmp_type == ICMPV6_NA))
        {
          /* NS/NA: ICMPv6 header (8 bytes) + Target Address (16 bytes) */
          struct in6_addr *target_addr =
              (struct in6_addr *) ((uint8_t *) icmp + 8);
          if (memcmp (target_addr, &vswitch->router_if.ipv6_addr,
                      sizeof (struct in6_addr)) == 0)
            is_control = true;
        }
      /* dst_ip is me */
      else if (ipv4 && memcmp (&ipv4->dst_addr, &vswitch->router_if.ipv4_addr,
                               sizeof (struct in_addr)) == 0)
        is_control = true;
      else if (ipv6 && memcmp (IPV6_ADDR_BYTES (ipv6->dst_addr),
               &vswitch->router_if.ipv6_addr, sizeof (struct in6_addr)) == 0)
        is_control = true;
#define IPPROTO_OSPF 89
      else if (ipv4 && ipv4->next_proto_id == IPPROTO_OSPF)
        is_control = true;
      else if (ipv6 && ipv6->proto == IPPROTO_OSPF)
        is_control = true;

      if (is_control)
        {
          DEBUG_SDPLANE_LOG (
              ROUTER,
              "m: %p control packet (eth_type:0x%04x), send to router_if", m,
              eth_type);
          _send_ring (m, rx_portid, rx_queueid, rif->ring_up);
          return;
        }
    }

  /* 4. check if l3_forwarding */
  if (! rte_is_same_ether_addr (&eth->dst_addr, &vswitch->router_if.mac_addr))
    {
      DEBUG_SDPLANE_LOG (ROUTER, "L2 switching");
      _switching (m, vswitch, eth, vlan, eth_type);
      return;
    }

  /* 5. verify IP packet */
  if (_verify_packet (m, ipv4, ipv6) != 0)
    {
      DEBUG_SDPLANE_LOG (ROUTER, "m: %p failed verify", m);
      return;
    }

  /* 6. forwarding */
  _forwarding (m, rx_portid, rx_queueid, eth, ipv4, ipv6, vswitch,
               vswitch_link);

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

  DEBUG_SDPLANE_LOG (ROUTER, "m: %p received on port: %d queue: %d", m,
                     ROUTER_IF_TX_SELF_PORT_ID, ROUTER_IF_TX_SELF_QUEUE_ID);

  /* check if this is a non-IP packet or multicast or broadcast */
  if ((! ipv4 && ! ipv6) || 
      rte_is_multicast_ether_addr (&eth->dst_addr) ||
      rte_is_broadcast_ether_addr (&eth->dst_addr))
    {
      _switching (m, vswitch, eth, vlan, eth_type);
      return;
    }

  _forwarding (m, ROUTER_IF_TX_SELF_PORT_ID, ROUTER_IF_TX_SELF_QUEUE_ID, eth,
               ipv4, ipv6, vswitch, NULL);
}

static inline __attribute__ ((always_inline)) void
_router_rx_burst ()
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
_router_tx_burst ()
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

  DEBUG_SDPLANE_LOG (ROUTER, "entering main loop on lcore %u", lcore_id);

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
          _router_tx_flush ();
          prev_tsc = cur_tsc;
        }

      _router_rx_burst ();
      _router_tx_burst ();

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
