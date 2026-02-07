#ifndef __NET_SUBR_H__
#define __NET_SUBR_H__

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

  if (! rib || ! rib->rib_info)
    return;

  nb_ports = rte_eth_dev_count_avail ();
  for (tx_portid = 0; tx_portid < nb_ports; tx_portid++)
    {
      buffer = tx_buffer_per_q[tx_portid][tx_queueid];

      /* skip if the tx_port is stopped */
      if (unlikely (rib->rib_info->port[tx_portid].is_stopped))
        continue;

      sent = 0;
      if (buffer)
        {
          sent = rte_eth_tx_buffer_flush (tx_portid, tx_queueid, buffer);

          if (sent || buffer->length)
            DEBUG_NEW (ROUTER,
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

  DEBUG_NEW (ROUTER,
             "m: %p port %d queue %d to ring: %s (%p)",
             m, rx_portid, rx_queueid, ring->name, ring);

  c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
  if (! c)
    return -1;

  ret = rte_ring_enqueue (ring, c);
  if (ret)
    {
      /* enqueue failed */
      DEBUG_NEW (ROUTER,
          "lcore[%d]: m: %p port %d queue %d to ring %s: %s: %d",
          lcore_id, m, rx_portid, rx_queueid,
          ring->name, (ret == -ENOBUFS ? "ENOBUFS" : "failed"), ret);

      rte_pktmbuf_free (c);
      return -1;
    }

  return 0;
}

static inline __attribute__ ((always_inline)) void
_send_router_if_ring (struct rte_mbuf *m,
                      unsigned rx_portid, unsigned rx_queueid,
                      struct router_if *rif)
{
  struct rte_mbuf *c;
  c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
  if (c)
    {
      if (rif->vlan_id)
        {
          if (is_rte_vlan_hdr (c))
            {
              DEBUG_NEW (ROUTER,
                          "m: %p router-if: vlan modify: %d.",
                          m, rif->vlan_id);
              rte_vlan_hdr_set (c, rif->vlan_id);
            }
          else
            {
              DEBUG_NEW (ROUTER,
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
              DEBUG_NEW (ROUTER,
                          "m: %p router-if: vlan strip", m);
              rte_vlan_strip (c);
            }
        }

      if (rif->ring_up)
      _send_ring (c, rx_portid, rx_queueid, rif->ring_up); 
      rte_pktmbuf_free (c);
    }
}


static inline __attribute__ ((always_inline)) void
_send_link (struct rte_mbuf *m,
            unsigned rx_portid, unsigned rx_queueid,
            struct vswitch_link *rx_link,
            unsigned tx_portid, unsigned tx_queueid,
            struct vswitch_link *tx_link)
{
  struct rte_eth_dev_tx_buffer *buffer;
  uint16_t nb_ports;
  int sent;
  struct rte_mbuf *c;

  nb_ports = rte_eth_dev_count_avail ();

  if (tx_portid >= nb_ports)
    return;

  if (rx_link && rx_link == tx_link)
    {
      DEBUG_NEW (ROUTER, "m: %p port[%d]: split-horizon, drop packet", m,
                 tx_portid);
      return;
    }

  if (! rib->rib_info->port[tx_portid].link.link_status)
    {
      DEBUG_NEW (ROUTER, "m: %p port[%d]: link down, drop packet", m,
                 tx_portid);
      return;
    }
  buffer = tx_buffer_per_q[tx_portid][tx_queueid];
  if (! buffer)
    return;

  /* copy the packet */
  c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
  if (! c)
    {
      DEBUG_NEW (ROUTER,
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

  /* operate on the VLAN header */
  if (eth_type == RTE_ETHER_TYPE_VLAN)
    {
      vlan = (struct rte_vlan_hdr *) (eth + 1);
      vlan_id = RTE_VLAN_TCI_ID (rte_be_to_cpu_16 (vlan->vlan_tci));

      if (tx_link->tag_id != 0 && tx_link->tag_id != vlan_id)
        {
          /* vlan_id translation: modify vlan_id on tx */
          old_vlan_tci = rte_be_to_cpu_16 (vlan->vlan_tci);
          new_vlan_tci =
              ((old_vlan_tci & 0xf000) | (tx_link->tag_id & 0x0fff));
          DEBUG_NEW (ROUTER,
                     "m: %p port[%d]: vlan_id modification: %u -> %u",
                     m, tx_link->port_id, vlan_id, tx_link->tag_id);
          vlan->vlan_tci = rte_cpu_to_be_16 (new_vlan_tci);
        }
      else if (tx_link->tag_id == 0)
        {
          /* remove vlan_hdr */
          rte_vlan_strip (c);
          DEBUG_NEW (ROUTER,
                     "m: %p port[%d]: vlan_id strip: %u -> %u",
                     m, tx_link->port_id, vlan_id, tx_link->tag_id);
        }
    }
  else
    {
      if (tx_link->tag_id != 0)
        {
          /* insert vlan_hdr */
          rte_vlan_insert (&c);
          eth = rte_pktmbuf_mtod (c, struct rte_ether_hdr *);
          eth_type = rte_be_to_cpu_16 (eth->ether_type);
          assert (eth_type == RTE_ETHER_TYPE_VLAN);
          vlan = (struct rte_vlan_hdr *) (eth + 1);
          old_vlan_tci = rte_be_to_cpu_16 (vlan->vlan_tci);
          new_vlan_tci =
              ((old_vlan_tci & 0xf000) | (tx_link->tag_id & 0x0fff));
          DEBUG_NEW (ROUTER,
                     "m: %p port[%d]: add vlan_id: %u",
                     m, tx_link->port_id, tx_link->tag_id);
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
          DEBUG_NEW (ROUTER, "error: m:%p %d bytes is smaller than 20 bytes",
                     m, rte_pktmbuf_data_len (m));
          return -1;
        }
      if (rte_ipv4_cksum (ipv4))
        {
          DEBUG_NEW (ROUTER, "error: IPv4 checksum");
          return -1;
        }
      if (ipv4->version != IPVERSION)
        {
          DEBUG_NEW (ROUTER, "error: version is %d", ipv4->version);
          return -1;
        }
      if (rte_ipv4_hdr_len (ipv4) < sizeof (struct rte_ipv4_hdr))
        {
          DEBUG_NEW (ROUTER, "error: %d bytes is smaller than 20 bytes",
                     rte_ipv4_hdr_len (ipv4));
          return -1;
        }
      if (rte_be_to_cpu_16 (ipv4->total_length) < sizeof (struct rte_ipv4_hdr))
        {
          DEBUG_NEW (ROUTER, "error: %d bytes is smaller than 20 bytes",
                     rte_be_to_cpu_16 (ipv4->total_length));
          return -1;
        }

      if (ipv4->time_to_live <= 1)
        {
          DEBUG_NEW (ROUTER, "error: TTL is %d", ipv4->time_to_live);
          /* TODO: send ICMP TIME EXCEEDED */
          return -1;
        }
    }
  else if (ipv6)
    {
      if (rte_pktmbuf_data_len (m) < sizeof (struct rte_ipv6_hdr))
        {
          DEBUG_NEW (ROUTER, "error: m:%p %d bytes is smaller than 40 bytes",
                     m, rte_pktmbuf_data_len (m));
          return -1;
        }
      uint8_t version = ((const uint8_t *) ipv6)[0];
      if ((version & 0xf0) != 0x60)
        {
          DEBUG_NEW (ROUTER, "error: version is %d", version);
          return -1;
        }

      if (ipv6->hop_limits <= 1)
        {
          DEBUG_NEW (ROUTER, "error: Hop Limit is %d", ipv6->hop_limits);
          /* TODO: send ICMPv6 TIME EXCEEDED */
          return -1;
        }
    }
  return 0;
}

/* imaginary port_id/queue_id to avoid
   split-horizon check in transmission. */
#define ROUTER_IF_RX_SELF_PORT_ID  UINT16_MAX
#define ROUTER_IF_RX_SELF_QUEUE_ID 0

static inline __attribute__ ((always_inline)) void
_flooding (struct rte_mbuf *m,
           uint16_t rx_portid, uint16_t rx_queueid,
           struct vswitch_link *rx_link,
           struct vswitch_conf *vswitch)
{
  unsigned tx_queueid = lcore_id;
  DEBUG_NEW (ROUTER,
             "m: %p flooding to VLAN %u",
             m, vswitch->vswitch_id);

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
      _send_link (m, rx_portid, rx_queueid, rx_link,
                  tx_portid, tx_queueid, link);
    }
  
    /* Flooding is also required for router_if */
    struct router_if *rif = &vswitch->router_if;
    if (rx_portid != ROUTER_IF_RX_SELF_PORT_ID 
        && rif->sockfd >= 0 && rif->ring_up)
      _send_router_if_ring (m, rx_portid, rx_queueid, rif);
}

static inline __attribute__ ((always_inline)) void
_switching (struct rte_mbuf *m,
            uint16_t rx_portid, uint16_t rx_queueid,
            struct vswitch_link *rx_link,
            struct vswitch_conf *vswitch,
            struct rte_ether_hdr *eth)
{
  /* for ARP replies from tap, we need L2 switching based on dst MAC */

  assert (vswitch);
  DEBUG_NEW (ROUTER,
             "m: %p packet from rx_port: %d rx_queue: %d "
             "L2 switching: vswitch[%d]",
             m, rx_portid, rx_queueid, vswitch->vswitch_id);

  if (vswitch->is_deleted)
    {
      DEBUG_NEW (ROUTER,
                 "vswitch[%d]: is deleted.",
                 vswitch->vswitch_id);
      return;
    }

  /* check if destination MAC is multicast or broadcast */
  if (rte_is_multicast_ether_addr (&eth->dst_addr) ||
      rte_is_broadcast_ether_addr (&eth->dst_addr))
    {
      _flooding (m, rx_portid, rx_queueid, rx_link, vswitch);
      return;
    }

  /* L2 switching: lookup dst MAC in FDB */
  int dst_port = -1;
  dst_port =
      fdb_lookup_entry (rib->rib_info, &eth->dst_addr, vswitch->vlan_id);

  /* search for the destination MAC in FDB */
  if (dst_port >= 0)
    {
      for (int j = 0; j < vswitch->vswitch_port_size; j++)
        {
          uint16_t link_id = vswitch->vswitch_link_id[j];
          struct vswitch_link *link = &rib->rib_info->vswitch_link[link_id];

          if (link->port_id == dst_port)
            {
              unsigned tx_queueid = lcore_id;
              DEBUG_NEW (ROUTER,
                         "m: %p L2 switching to port %d via link %d",
                         m, dst_port, link_id);
              _send_link (m, rx_portid, rx_queueid, rx_link,
                          dst_port, tx_queueid, link);
              return;
            }
        }
    }

  DEBUG_NEW (ROUTER, "m: %p L2 switching: dst MAC unknown, flooding", m);
  _flooding (m, rx_portid, rx_queueid, rx_link, vswitch);

  return;
}

static inline __attribute__ ((always_inline)) void
_forwarding (struct rte_mbuf *m,
             unsigned rx_portid, unsigned rx_queueid,
             struct rte_ether_hdr *eth, struct rte_ipv4_hdr *ipv4,
             struct rte_ipv6_hdr *ipv6, struct vswitch_conf *vswitch,
             struct vswitch_link *vswitch_link)
{
  struct nh_legacy *nh_legacy;
  struct neigh_entry *neigh_entry;
  struct rte_ether_addr *dst_mac;
  struct fib_node *fib_node;
  uint8_t dst_ip[16] = { 0 };
  int i;
  struct vswitch_link *tx_link = NULL;
  int dst_port = -1, neigh_table_type;

  if (ipv4)
    memcpy (dst_ip, &ipv4->dst_addr, sizeof (ipv4->dst_addr));
  else if (ipv6)
    memcpy (dst_ip, IPV6_ADDR_BYTES (ipv6->dst_addr), RTE_IPV6_ADDR_SIZE);

  /* FIB lookup */
  int tree_idx = (ipv4) ? 0 : 1; // IPv4=0, IPv6=1
  fib_node = fib_route_lookup (rib->rib_info->fib_tree[tree_idx], dst_ip);
  if (! fib_node)
    {
      DEBUG_NEW (ROUTER, "m: %p FIB lookup failed, drop", m);
      if (IS_DEBUG (ROUTER))
        log_packet (m, rx_portid, rx_queueid);
      return;
    }

  /**
   * Currently only the legacy nexthop type is supported,
   * where each FIB entry directly stores nexthops.
   * TODO: add support for ECMP via multipath nexthops and nexthop groups.
   */
  char nexthop_str[INET6_ADDRSTRLEN];
  /* determine the actual nexthop IP to lookup in neighbor table */
  uint8_t lookup_ip[16];
  char lookup_ip_str[INET6_ADDRSTRLEN];
  int family, oif;

  switch (fib_node->nh.nh_type)
    {
    case NH_TYPE_LEGACY:
      nh_legacy =
          &rib->rib_info->nexthop.legacy.object[fib_node->nh.nh_id];
      switch (nh_legacy->type)
        {
        case NH_OBJ_TYPE_OBJECT:
          family = nh_legacy->nh_info.family;
          oif = nh_legacy->nh_info.oif;
          inet_ntop (family,
                     &nh_legacy->nh_info.nh_ip_addr,
                     nexthop_str, sizeof (nexthop_str));
          memcpy (lookup_ip,
                  &nh_legacy->nh_info.nh_ip_addr,
                  sizeof (lookup_ip));
          /* for directly connected routes (nexthop = 0.0.0.0), use destination IP */
          if (nh_legacy->nh_info.type == ROUTE_TYPE_CONNECTED)
            {
              memcpy (lookup_ip, dst_ip, sizeof (lookup_ip));
              inet_ntop (family,
                         lookup_ip, lookup_ip_str,
                         sizeof (lookup_ip_str));
              DEBUG_NEW (
                  ROUTER,
                  "m: %p directly connected route, using dst IP for ARP lookup:%s",
                  m, lookup_ip_str);
            }
          break;

        case NH_OBJ_TYPE_GROUP:
          family = nh_legacy->nh_grp.nh_info_list[0].family;
          oif = nh_legacy->nh_grp.nh_info_list[0].oif;
          /* use the first nexthop in the group for now */
          inet_ntop (family,
                     &nh_legacy->nh_grp.nh_info_list[0].nh_ip_addr,
                     nexthop_str, sizeof (nexthop_str));
          memcpy (lookup_ip,
                  &nh_legacy->nh_grp.nh_info_list[0].nh_ip_addr,
                  sizeof (lookup_ip));
          /* for directly connected routes (nexthop = 0.0.0.0), use destination IP */
          if (nh_legacy->nh_grp.nh_info_list[0].type == ROUTE_TYPE_CONNECTED)
            {
              memcpy (lookup_ip, dst_ip, sizeof (lookup_ip));
              inet_ntop (family,
                         lookup_ip, lookup_ip_str,
                         sizeof (lookup_ip_str));
              DEBUG_NEW (
                  ROUTER,
                  "m: %p directly connected route, using dst IP for ARP lookup:%s",
                  m, lookup_ip_str);
            }
          break;

        default:
          return;
        }
      break;

    case NH_TYPE_OBJECT_CAP:
      DEBUG_NEW (ROUTER, "m: %p unsupported nexthop type: %d",
                 m, fib_node->nh.nh_type);
      break;

    default:
      return;
    }

  DEBUG_NEW (ROUTER, "m: %p route found: nexthop=%s",
             m, nexthop_str);

  /* neighbor table lookup */
  neigh_table_type = AF_TO_NEIGH_TABLE (family);
  neigh_manager_lookup (&rib->rib_info->neigh_tables[neigh_table_type],
                        neigh_table_type, lookup_ip, &neigh_entry);
  if (! neigh_entry || rte_is_zero_ether_addr(&neigh_entry->mac_addr))
    {
      /* ARP/ND resolution */
      for (i = 0; i < rib->rib_info->vswitch_size; i++)
        {
          struct vswitch_conf *vs = &rib->rib_info->vswitch[i];
          struct router_if *rif = &vs->router_if;
          if (rif->ifindex != oif)
            continue;

          if (family == AF_INET)
            {
              struct rte_mbuf *arp_pkt;
              arp_pkt = arp_req_pkt_gen (&rif->mac_addr, &rif->ipv4_addr,
                                         (struct in_addr *) lookup_ip);
              if (arp_pkt)
                {
                  char dst_ip_str[INET_ADDRSTRLEN];
                  inet_ntop (AF_INET, lookup_ip, dst_ip_str, sizeof (dst_ip_str));
                  DEBUG_NEW (ROUTER,
                             "m: %p send ARP request: target_ip: %s, flooding to vswitch[%d]",
                             arp_pkt, dst_ip_str, vs->vswitch_id);

                  _flooding (arp_pkt, ROUTER_IF_RX_SELF_PORT_ID,
                             ROUTER_IF_RX_SELF_QUEUE_ID, NULL, vs);

                  rte_pktmbuf_free (arp_pkt);
                }
              else
                DEBUG_NEW (ROUTER, "m: %p failed to generate ARP request", m);
            }
          else
            {
              struct rte_mbuf *ns_pkt;
              ns_pkt = ns_pkt_gen (&rif->mac_addr, &rif->ll_addr,
                                   (struct in6_addr *) lookup_ip);
              if (ns_pkt)
                {
                  char dst_ip_str[INET6_ADDRSTRLEN];
                  inet_ntop (AF_INET6, lookup_ip, dst_ip_str, sizeof (dst_ip_str));
                  DEBUG_NEW (ROUTER,
                             "m: %p send NS: %p, target_ip: %s, flooding to vswitch[%d]",
                             m, ns_pkt, dst_ip_str, vs->vswitch_id);
                  _flooding (ns_pkt, ROUTER_IF_RX_SELF_PORT_ID,
                             ROUTER_IF_RX_SELF_QUEUE_ID, NULL, vs);

                  rte_pktmbuf_free (ns_pkt);
                }
              else
                DEBUG_NEW (ROUTER, "m: %p failed to generate NS", m);
            }
          return;
        }
      return;
    }
  dst_mac = &neigh_entry->mac_addr;

  char neigh_mac_str[RTE_ETHER_ADDR_FMT_SIZE];
  rte_ether_format_addr (neigh_mac_str, sizeof (neigh_mac_str), dst_mac);
  DEBUG_NEW (ROUTER, "m: %p neighbor lookup succeeded: MAC=%s",
                     m, neigh_mac_str);

  /* search tx vswitch_link */
  struct vswitch_conf *target_vswitch = NULL;
  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vs = &rib->rib_info->vswitch[i];

      if (vs->is_deleted)
        continue;

      if (vs->router_if.ifindex == oif)
        {
          target_vswitch = vs;
          break;
        }
    }

  if (! target_vswitch)
    {
      DEBUG_NEW (ROUTER, "m: %p vswitch not found for oif %d", m, oif);
      return;
    }

  dst_port = fdb_lookup_entry (rib->rib_info, dst_mac, target_vswitch->vlan_id);
  if (dst_port < 0)
    {
      DEBUG_NEW (ROUTER, "m: %p FDB lookup failed, drop", m);
      return;
    }

  for (i = 0; i < target_vswitch->vswitch_port_size; i++)
    {
      uint16_t link_id = target_vswitch->vswitch_link_id[i];
      struct vswitch_link *link = &rib->rib_info->vswitch_link[link_id];

      if (link->port_id == dst_port)
        {
          tx_link = link;
          break;
        }
    }

  if (! tx_link)
    {
      DEBUG_NEW (ROUTER, "m: %p FDB lookup failed, send to router_if",
                         m);
      return;
    }

  /* check if tx_port is stopped */
  if (unlikely (rib->rib_info->port[dst_port].is_stopped))
    {
      DEBUG_NEW (ROUTER, "m: %p tx_port %d is stopped", m, dst_port);
      return;
    }

  /* rewrite L2/L3 header */
  if (rx_portid != ROUTER_IF_RX_SELF_PORT_ID)
    {
      if (ipv4)
        {
          ipv4->time_to_live--;
          ipv4->hdr_checksum = 0;
          ipv4->hdr_checksum = rte_ipv4_cksum (ipv4);
        }
      else if (ipv6)
        ipv6->hop_limits--;

      rte_ether_addr_copy (&target_vswitch->router_if.mac_addr,
                           &eth->src_addr);
      rte_ether_addr_copy (dst_mac, &eth->dst_addr);
    }

  DEBUG_NEW (ROUTER, "m: %p forwarding to port %d via link %d",
                     m, dst_port, tx_link->vswitch_link_id);

  unsigned tx_queueid = lcore_id;
  _send_link (m, rx_portid, rx_queueid, vswitch_link,
              dst_port, tx_queueid, tx_link);
}

#define IPPROTO_OSPF 89

static inline __attribute__ ((always_inline)) bool
_check_control_packet (struct rte_mbuf *m,
                       struct vswitch_conf *vswitch,
                       uint16_t eth_type,
                       struct rte_ether_hdr *eth,
                       struct rte_vlan_hdr *vlan,
                       struct rte_ipv4_hdr *ipv4,
                       struct rte_ipv6_hdr *ipv6,
                       struct rte_icmp_hdr *icmp)
{
  bool is_control = false;
  struct router_if *rif = &vswitch->router_if;

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

      if (memcmp (&arp->arp_data.arp_tip, &rif->ipv4_addr,
                  sizeof (struct in_addr)) == 0)
        {
          is_control = true;

          /* send internal_msg for neighbor resolution */
          if (rte_be_to_cpu_16 (arp->arp_opcode) == RTE_ARP_OP_REPLY)
            {
              void *msgp;
              struct internal_msg_neigh_entry msg_neigh_entry;
              msg_neigh_entry.data.family = AF_INET;
              msg_neigh_entry.data.state = NUD_REACHABLE;
              msg_neigh_entry.data.ifindex = rif->ifindex;
              msg_neigh_entry.type = NEIGH_ARP_TABLE;
              memcpy (&msg_neigh_entry.data.ip_addr.ipv4_addr,
                      &arp->arp_data.arp_sip, sizeof (struct in_addr));
              memcpy (&msg_neigh_entry.data.mac_addr,
                      &arp->arp_data.arp_sha, sizeof (struct rte_ether_addr));
              msgp = internal_msg_create (INTERNAL_MSG_TYPE_NEIGH_ENTRY_ADD,
                                          &msg_neigh_entry, sizeof (msg_neigh_entry));
              if (! msg_queue_neigh)
                DEBUG_NEW (ROUTER,
                           "error: neigh_manager is not started.");
              internal_msg_send_to (msg_queue_neigh, msgp, NULL);
            }
        }
    }
  else if (ipv4)
    {
      if (memcmp (&ipv4->dst_addr, &rif->ipv4_addr,
                  sizeof (struct in_addr)) == 0)
        is_control = true;
      else if (ipv4->next_proto_id == IPPROTO_OSPF)
        is_control = true;
    }
  else if (ipv6)
    {
#if 0
      if (memcmp (IPV6_ADDR_BYTES (ipv6->dst_addr),
          &rif->ipv6_addr, sizeof (struct in6_addr)) == 0)
        is_control = true;
      else if (ipv6->proto == IPPROTO_OSPF)
#endif
      if (ipv6->proto == IPPROTO_OSPF)
        is_control = true;
      else if (icmp)
        {
          if (icmp->icmp_type == ICMPV6_NS)
            {
              /* NS: ICMPv6 header (8 bytes) + Target Address (16 bytes) */
              struct in6_addr *target_addr =
                  (struct in6_addr *) ((uint8_t *) icmp + 8);
              if (memcmp (target_addr, &rif->ipv6_addr,
                          sizeof (struct in6_addr)) == 0)
                is_control = true;
            }
          else if (icmp->icmp_type == ICMPV6_NA)
            {
              if (memcmp (IPV6_ADDR_BYTES (ipv6->dst_addr),
                  &rif->ll_addr, sizeof (struct in6_addr)) == 0)
                {
                  is_control = true;

                  void *msgp;
                  struct internal_msg_neigh_entry msg_neigh_entry;
                  msg_neigh_entry.data.family = AF_INET6;
                  msg_neigh_entry.data.state = NUD_REACHABLE;
                  msg_neigh_entry.data.ifindex = rif->ifindex;
                  msg_neigh_entry.type = NEIGH_ND_TABLE;
                  struct in6_addr *target_ip =
                      (struct in6_addr *) ((uint8_t *) icmp + 8);
                  memcpy (&msg_neigh_entry.data.ip_addr.ipv6_addr,
                          target_ip, sizeof (struct in6_addr));
                  memcpy (&msg_neigh_entry.data.mac_addr,
                          &eth->src_addr, sizeof (struct rte_ether_addr));
                  msgp = internal_msg_create (INTERNAL_MSG_TYPE_NEIGH_ENTRY_ADD,
                                              &msg_neigh_entry, sizeof (msg_neigh_entry));
                  if (! msg_queue_neigh)
                    DEBUG_NEW (ROUTER,
                               "error: neigh_manager is not started.");
                  internal_msg_send_to (msg_queue_neigh, msgp, NULL);
                }
            }
        }
    }

  return is_control;
}

#endif /*__NET_SUBR_H__*/
