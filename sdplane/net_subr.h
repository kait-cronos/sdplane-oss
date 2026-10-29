#ifndef __NET_SUBR_H__
#define __NET_SUBR_H__

#include <linux/seg6_local.h>
#include <stdint.h>
#include <rte_common.h>
#include <rte_ethdev.h>
#include <linux/rtnetlink.h>

#include "fib.h"
#include "internal_message.h"
#include "nexthop.h"
#include "packet_gen.h"
#include "neigh_manager.h"
#include "rib_manager.h"
#include "fdb.h"
#include "packet_hdr.h"
#include "log_packet.h"
#include "control_packet.h"
#include "srv6.h"

#ifndef FLOOD_CONSUME_LAST
#define FLOOD_CONSUME_LAST 1
#endif


#ifndef FLOOD_CLONE_MIN_LEN
#define FLOOD_CLONE_MIN_LEN 512
#endif

extern struct rte_eth_dev_tx_buffer
    *tx_buffer_per_q[RTE_MAX_ETHPORTS][RTE_MAX_LCORE];

static inline __attribute__ ((always_inline)) uint16_t
_nb_ports_cached (void)
{
  static __thread uint16_t nb_ports_cached = 0;

  if (unlikely (! nb_ports_cached))
    nb_ports_cached = rte_eth_dev_count_avail ();

  return nb_ports_cached;
}

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
  int ret;

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
_send_capture_if_ring (struct rte_mbuf *m,
                       unsigned portid, unsigned queueid,
                       struct vswitch_conf *vswitch)
{
  struct capture_if *cif = &vswitch->capture_if;

  if (cif->sockfd >= 0 && cif->ring_up)
    _send_ring (m, portid, queueid, cif->ring_up);
}

extern volatile int router_mode;
static inline __attribute__ ((always_inline)) void
_debug_manual_send (struct rte_mbuf *m)
{
  unsigned tx_portid = 1;
  unsigned tx_queueid;
  struct rte_eth_dev_tx_buffer *buffer;
  struct rte_mbuf *c;
  tx_queueid = rte_lcore_id ();
  buffer = tx_buffer_per_q[tx_portid][tx_queueid];
  c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
  if (c)
    rte_eth_tx_buffer (tx_portid, tx_queueid, buffer, c);
}

/* returns true if _send_link() will modify the VLAN header */
static inline __attribute__ ((always_inline)) bool
_tx_vlan_write_needed (struct rte_mbuf *m, struct vswitch_link *tx_link)
{
  struct rte_ether_hdr *eth;
  uint16_t eth_type;

  eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  eth_type = rte_be_to_cpu_16 (eth->ether_type);

  if (eth_type == RTE_ETHER_TYPE_VLAN)
    {
      struct rte_vlan_hdr *vlan = (struct rte_vlan_hdr *) (eth + 1);
      uint16_t vlan_id = RTE_VLAN_TCI_ID (rte_be_to_cpu_16 (vlan->vlan_tci));
      /* modify (tag_id != 0 && tag_id != vlan_id) or strip (tag_id == 0) */
      return ! (tx_link->tag_id != 0 && tx_link->tag_id == vlan_id);
    }

  /* insert */
  return tx_link->tag_id != 0;
}

static inline __attribute__ ((always_inline)) bool
_send_link (struct rte_mbuf *m,
            unsigned rx_portid, unsigned rx_queueid,
            struct vswitch_link *rx_link,
            unsigned tx_portid, unsigned tx_queueid,
            struct vswitch_link *tx_link,
            struct vswitch_conf *vswitch,
            uint16_t nb_ports,
            bool consume)
{
  struct rte_eth_dev_tx_buffer *buffer;
  int sent;
  struct rte_mbuf *c;

#if 1
  DEBUG_NEW (ROUTER, "m: %p rxport: %d rxqueue: %d txport: %d txqueue: %d",
             m, rx_portid, rx_queueid, tx_portid, tx_queueid);
#endif

  if (tx_portid >= nb_ports)
    return false;

  if (rx_link && rx_link == tx_link)
    {
      DEBUG_NEW (ROUTER, "m: %p port[%d]: split-horizon, drop packet", m,
                 tx_portid);
      return false;
    }

  if (! rib->rib_info->port[tx_portid].link.link_status)
    {
      DEBUG_NEW (ROUTER, "m: %p port[%d]: link down, drop packet", m,
                 tx_portid);
      return false;
    }

  buffer = tx_buffer_per_q[tx_portid][tx_queueid];
  if (! buffer)
    {
      DEBUG_NEW (ROUTER, "m: %p port[%d]: no buffer",
                 m, tx_portid);
      return false;
    }

  /* how to obtain the mbuf to transmit:
     - consume: take over the ownership of m (zero-copy).
       when m is shared (flooding clones in flight) and the VLAN
       header must be modified, fall back to copy and leave m
       to the caller.
     - clone: share the packet data by reference (refcnt) for
       long packets when no VLAN header modification is needed
       for this tx_link.
     - copy: short packets, or the VLAN header must be modified
       on a shared mbuf.
  */
  bool tx_fast_free = port_tx_offload_active (
      &rib->rib_info->port[tx_portid], RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE);

  if (consume &&
      ((RTE_MBUF_DIRECT (m) && rte_mbuf_refcnt_read (m) == 1) ||
       (! tx_fast_free && ! _tx_vlan_write_needed (m, tx_link))))
    {
      c = m;
    }
  else if (! tx_fast_free &&
           rte_pktmbuf_pkt_len (m) >= FLOOD_CLONE_MIN_LEN &&
           ! _tx_vlan_write_needed (m, tx_link))
    {
      c = rte_pktmbuf_clone (m, m->pool);
      if (! c)
        {
          DEBUG_NEW (ROUTER,
                     "lcore[%d]: m: %p L2 forward -> port %d "
                     "rte_pktmbuf_clone() failed.",
                     lcore_id, m, tx_portid);
          return false;
        }
    }
  else
    {
      c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
      if (! c)
        {
          DEBUG_NEW (ROUTER,
                     "lcore[%d]: m: %p L2 forward -> port %d "
                     "rte_pktmbuf_copy() failed.",
                     lcore_id, m, tx_portid);
          return false;
        }
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

  /* send to capture_if */
  _send_capture_if_ring (c, tx_portid, tx_queueid, vswitch);

  sent = rte_eth_tx_buffer (tx_portid, tx_queueid, buffer, c);
  if (sent)
    {
      port_statistics[tx_portid].tx += sent;
      DEBUG_NEW (ROUTER, "m: %p successfully sent (%d packets): port: %d queue: %u",
                 m, tx_portid, tx_queueid);
    }
  else if (sent == 0)
    {
      DEBUG_NEW (ROUTER, "m: %p successfully buffered: port: %d queue: %u",
                 m, tx_portid, tx_queueid);
    }
  else
    {
      DEBUG_NEW (ROUTER, "m: %p port[%d]: send failed", m, tx_portid);
    }

  return (c == m);
}

static inline __attribute__ ((always_inline)) int
_verify_packet (struct rte_mbuf *m, const struct port_conf *rx_port,
                struct rte_ipv4_hdr *ipv4, struct rte_ipv6_hdr *ipv6)
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
      /* Trust the NIC's IPv4-header checksum result when present. */
      const uint64_t ip_ck = m->ol_flags & RTE_MBUF_F_RX_IP_CKSUM_MASK;
      const uint16_t hdr_len = rte_ipv4_hdr_len (ipv4);

      switch (ip_ck)
        {
        case RTE_MBUF_F_RX_IP_CKSUM_BAD:
          DEBUG_NEW (ROUTER, "error: IPv4 checksum (HW)");
          return -1;

        case RTE_MBUF_F_RX_IP_CKSUM_GOOD:
          if (likely (port_rx_offload_active (rx_port,
                                              RTE_ETH_RX_OFFLOAD_IPV4_CKSUM)))
            break;

        case RTE_MBUF_F_RX_IP_CKSUM_NONE:
        case RTE_MBUF_F_RX_IP_CKSUM_UNKNOWN:
        default:
          if (unlikely (rte_raw_cksum (ipv4, hdr_len) != UINT16_MAX))
            {
              DEBUG_NEW (ROUTER, "error: IPv4 checksum (SW)");
              return -1;
            }
          break;
        }
      if (ipv4->version != IPVERSION)
        {
          DEBUG_NEW (ROUTER, "error: version is %d", ipv4->version);
          return -1;
        }
      if (hdr_len < sizeof (struct rte_ipv4_hdr))
        {
          DEBUG_NEW (ROUTER, "error: %d bytes is smaller than 20 bytes",
                     hdr_len);
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

/* returns true if _send_link() would actually try to transmit
   the mbuf to the tx_link */
static inline __attribute__ ((always_inline)) bool
_send_link_ok (struct vswitch_link *rx_link, struct vswitch_link *tx_link,
               unsigned tx_portid, unsigned tx_queueid, uint16_t nb_ports)
{
  if (tx_portid >= nb_ports)
    return false;
  if (rx_link && rx_link == tx_link)
    return false;
  if (! rib->rib_info->port[tx_portid].link.link_status)
    return false;
  if (! tx_buffer_per_q[tx_portid][tx_queueid])
    return false;
  return true;
}

static inline __attribute__ ((always_inline)) bool
_flooding (struct rte_mbuf *m,
           uint16_t rx_portid, uint16_t rx_queueid,
           struct vswitch_link *rx_link,
           struct vswitch_conf *vswitch,
           bool consume)
{
  unsigned tx_queueid = lcore_id;
  uint16_t nb_ports = _nb_ports_cached ();
  struct vswitch_link *last_link = NULL;
  unsigned last_portid = 0;

  DEBUG_NEW (ROUTER, "m: %p flooding to VLAN %u",
             m, vswitch->vswitch_id);

  if (router_mode == 11) /* flooding: the beginning. */
    {
      _debug_manual_send (m);
      return false;
    }

  /* flood to all ports in the same vswitch */
  for (int i = 0; i < vswitch->vswitch_port_size; i++)
    {
      uint16_t link_id = vswitch->vswitch_link_id[i];
      struct vswitch_link *link = &rib->rib_info->vswitch_link[link_id];
      unsigned tx_portid = link->port_id;

      /* skip if the tx_port is stopped */
      if (unlikely (rib->rib_info->port[tx_portid].is_stopped))
        {
          DEBUG_NEW (ROUTER, "m: %p skipping stopped port: %d",
                     m, tx_portid);
          continue;
        }

      DEBUG_NEW (FLOODING, "m: %p send_link(): vswitch[%d].port[%d] "
                 "rx-port[%d:%d](%p) -> tx-port[%d:%d](%p)",
                 m, vswitch->vswitch_id, i,
                 rx_portid, rx_queueid, rx_link,
                 tx_portid, tx_queueid, link);

      if (! _send_link_ok (rx_link, link, tx_portid, tx_queueid, nb_ports))
        {
#if 0
          DEBUG_NEW (FLOODING, "m: %p send to a NG link.", m);
          _send_link (m, rx_portid, rx_queueid, rx_link,
                      tx_portid, tx_queueid, link, vswitch, nb_ports, false);
#endif
          continue;
        }

      /* We would like to postpone the _send_link() of the last_link
         to a later time, because we want the consume-mode to work and
         the consume-mode may delete the packet immediately.
         So basically we want to send here all but the last_link,
         with consume = false. */

      /* if there's already last_link, process _send_link() for the link
         because we're gonna update the last_link to the next one. */
      if (last_link)
        _send_link (m, rx_portid, rx_queueid, rx_link,
                    last_portid, tx_queueid, last_link, vswitch,
                    nb_ports, false);

      /* update the last_link */
      last_link = link;
      last_portid = tx_portid;
    }

  if (router_mode == 12) /* flooding: after sending all ports, without sending router-if. */
    {
      if (last_link)
        _send_link (m, rx_portid, rx_queueid, rx_link,
                    last_portid, tx_queueid, last_link, vswitch,
                    nb_ports, false);
      //_debug_manual_send (m);
      return false;
    }

  /* Flooding is also required for router_if */
  struct router_if *rif = &vswitch->router_if;
  if (rx_portid != ROUTER_IF_RX_SELF_PORT_ID
      && rif->sockfd >= 0 && rif->ring_up)
    _send_router_if_ring (m, rx_portid, rx_queueid, rif);

  if (last_link)
    return _send_link (m, rx_portid, rx_queueid, rx_link,
                       last_portid, tx_queueid, last_link, vswitch,
                       nb_ports, consume && FLOOD_CONSUME_LAST);

  return false;
}

static inline __attribute__ ((always_inline)) bool
_switching (struct rte_mbuf *m,
            uint16_t rx_portid, uint16_t rx_queueid,
            struct vswitch_link *rx_link,
            struct vswitch_conf *vswitch,
            struct rte_ether_hdr *eth,
            bool consume)
{
  /* for ARP replies from tap, we need L2 switching based on dst MAC */

  assert (vswitch);

  if (vswitch->is_deleted)
    {
      DEBUG_NEW (ROUTER, "vswitch[%d]: is deleted.",
                 vswitch->vswitch_id);
      return false;
    }

  /* check if destination MAC is multicast or broadcast */
  if (rte_is_multicast_ether_addr (&eth->dst_addr) ||
      rte_is_broadcast_ether_addr (&eth->dst_addr))
    {
      DEBUG_NEW (ROUTER, "m: %p L2 switching: ether broadcast/multicast, flooding", m);
      return _flooding (m, rx_portid, rx_queueid, rx_link, vswitch, consume);
    }

  /* switching: just before the fdb-lookup */
  if (unlikely (router_mode == 8))
    {
      _debug_manual_send (m);
      return false;
    }

  /* L2 switching: lookup dst MAC in FDB */
  int dst_port = -1;
  dst_port =
      fdb_lookup_entry (rib->rib_info, &eth->dst_addr, vswitch->vlan_id);

  /* switching: right after the fdb-lookup */
  if (unlikely (router_mode == 9))
    {
      _debug_manual_send (m);
      return false;
    }

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
              return _send_link (m, rx_portid, rx_queueid, rx_link,
                                 dst_port, tx_queueid, link, vswitch,
                                 _nb_ports_cached (), consume);
            }
        }
    }

  /* switching: just before the flooding */
  if (unlikely (router_mode == 10))
    {
      _debug_manual_send (m);
      return false;
    }

  DEBUG_NEW (ROUTER, "m: %p L2 switching: dst MAC not learned, flooding", m);
  return _flooding (m, rx_portid, rx_queueid, rx_link, vswitch, consume);
}


/*
 * IP lookup key
 *
 * A 16-byte buffer holding either an IPv6 address or an IPv4 address in
 * its first 4 bytes with the remaining bytes zero-filled.  Both address
 * families can therefore be built, copied and compared as two 64-bit
 * words; the 8-byte alignment lets the compiler fold the memcpy () calls
 * below into plain 64-bit loads and stores.
 */
#define IP16_KEY_SIZE 16
typedef uint8_t ip16_key_t[IP16_KEY_SIZE] __attribute__ ((aligned (8)));

/* stores the destination address of the packet into key.
   exactly one of ipv4/ipv6 must be non-NULL. */
static inline __attribute__ ((always_inline)) void
_ip16_key_set_dst (ip16_key_t key, const struct rte_ipv4_hdr *ipv4,
                   const struct rte_ipv6_hdr *ipv6)
{
  uint64_t lo = 0, hi = 0;

  if (ipv4)
    memcpy (&lo, &ipv4->dst_addr, sizeof (ipv4->dst_addr));
  else
    {
      const uint8_t *addr = (const uint8_t *) IPV6_ADDR_BYTES (ipv6->dst_addr);

      memcpy (&lo, addr, sizeof (lo));
      memcpy (&hi, addr + sizeof (lo), sizeof (hi));
    }

  memcpy (key, &lo, sizeof (lo));
  memcpy (key + sizeof (lo), &hi, sizeof (hi));
}

static inline __attribute__ ((always_inline)) bool
_ip16_key_eq (const ip16_key_t a, const ip16_key_t b)
{
  uint64_t a_lo, a_hi, b_lo, b_hi;

  memcpy (&a_lo, a, sizeof (a_lo));
  memcpy (&a_hi, a + sizeof (a_lo), sizeof (a_hi));
  memcpy (&b_lo, b, sizeof (b_lo));
  memcpy (&b_hi, b + sizeof (b_lo), sizeof (b_hi));

  return ((a_lo ^ b_lo) | (a_hi ^ b_hi)) == 0;
}

struct fwd_lookup_cache
{
  ip16_key_t fib_key;
  struct fib_node *fib_node;
  int fib_tree;
  bool fib_valid;

  ip16_key_t neigh_key;
  struct neigh_entry *neigh_entry;
  int neigh_table;
  bool neigh_valid;

  /* interface: oif ifindex -> tx vswitch */
  struct vswitch_conf *tx_vswitch;
  int oif;
  bool oif_valid;
};

static __thread struct fwd_lookup_cache fwd_cache;

/* drops every cached lookup result. */
static inline __attribute__ ((always_inline)) void
_fwd_cache_invalidate (void)
{
  fwd_cache.fib_valid = false;
  fwd_cache.neigh_valid = false;
  fwd_cache.oif_valid = false;
}

static inline __attribute__ ((always_inline)) struct fib_node *
_fib_lookup_cached (int tree_idx, const ip16_key_t dst_ip)
{
  struct fib_tree *tree;

  if (likely (fwd_cache.fib_valid && fwd_cache.fib_tree == tree_idx &&
              _ip16_key_eq (fwd_cache.fib_key, dst_ip)))
    return fwd_cache.fib_node;

  tree = rib->rib_info->fib_tree[tree_idx];
  if (unlikely (! tree))
    return NULL;

  fwd_cache.fib_node = fib_route_lookup (tree, dst_ip);
  memcpy (fwd_cache.fib_key, dst_ip, sizeof (fwd_cache.fib_key));
  fwd_cache.fib_tree = tree_idx;
  fwd_cache.fib_valid = true;

  return fwd_cache.fib_node;
}

static inline __attribute__ ((always_inline)) struct neigh_entry *
_neigh_lookup_cached (int table_type, const ip16_key_t lookup_ip)
{
  struct neigh_entry *neigh_entry = NULL;

  if (likely (fwd_cache.neigh_valid && fwd_cache.neigh_table == table_type &&
              _ip16_key_eq (fwd_cache.neigh_key, lookup_ip)))
    return fwd_cache.neigh_entry;

  neigh_entry = neigh_lookup (&rib->rib_info->neigh_tables[table_type], table_type,
                              lookup_ip);
  memcpy (fwd_cache.neigh_key, lookup_ip, sizeof (fwd_cache.neigh_key));
  fwd_cache.neigh_table = table_type;
  fwd_cache.neigh_entry = neigh_entry;
  fwd_cache.neigh_valid = true;

  return neigh_entry;
}

static inline __attribute__ ((always_inline)) struct vswitch_conf *
_tx_vswitch_lookup_cached (int oif)
{
  struct vswitch_conf *tx_vswitch = NULL;
  int i;

  if (likely (fwd_cache.oif_valid && fwd_cache.oif == oif))
    return fwd_cache.tx_vswitch;

  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vs = &rib->rib_info->vswitch[i];

      if (vs->is_deleted)
        continue;

      if (vs->router_if.ifindex == oif)
        {
          tx_vswitch = vs;
          break;
        }
    }

  fwd_cache.oif = oif;
  fwd_cache.tx_vswitch = tx_vswitch;
  fwd_cache.oif_valid = true;

  return tx_vswitch;
}

static inline __attribute__ ((always_inline)) bool
_forwarding (struct rte_mbuf *m,
             unsigned rx_portid, unsigned rx_queueid,
             struct rte_ether_hdr *eth, struct rte_ipv4_hdr *ipv4,
             struct rte_ipv6_hdr *ipv6, struct vswitch_conf *rx_vswitch,
             struct vswitch_link *rx_link,
             bool consume)
{
  struct nh_info *nh_info;
  struct neigh_entry *neigh_entry;
  struct rte_ether_addr *dst_mac, *src_mac;
  struct fib_node *fib_node;
  ip16_key_t dst_ip;
  int i;
  struct vswitch_link *tx_link = NULL;
  int dst_port = -1, neigh_table_type;

  if (unlikely (! ipv4 && ! ipv6))
    return false;

  char dest[64];
  dest[0] = '\0';
  if (IS_DEBUG (ROUTING_LOOKUP))
    {
      if (ipv4)
        inet_ntop (AF_INET, &ipv4->dst_addr, dest, sizeof (dest));
      else
        inet_ntop (AF_INET6, IPV6_ADDR_BYTES (ipv6->dst_addr),
                   dest, sizeof (dest));
      //DEBUG_LOG_MSG ("m: %p routing-lookup: dest: %s", m, dest);
    }

  _ip16_key_set_dst (dst_ip, ipv4, ipv6);

  /* forwarding: just before the routing-lookup. */
  if (unlikely (router_mode == 3))
    {
      _debug_manual_send (m);
      return false;
    }

  int tree_idx = (ipv4) ? 0 : 1; // IPv4=0, IPv6=1
  fib_node = _fib_lookup_cached (tree_idx, dst_ip);
  if (! fib_node)
    {
      DEBUG_NEW (ROUTING_LOOKUP,
                 "m: %p routing-lookup: dest: %s FIB not found. drop.", m,
                 dest);
#if 0
      if (IS_DEBUG (ROUTER))
        log_packet (m, rx_portid, rx_queueid);
#endif
      return false;
    }

  /* forwarding: right after the routing-lookup. */
  if (unlikely (router_mode == 4))
    {
      _debug_manual_send (m);
      return false;
    }

  /* determine the actual nexthop IP to lookup in neighbor table */
  ip16_key_t lookup_ip;
  int family, oif;

  nh_info = nexthop_get_info_by_index (rib->rib_info, fib_node->sdplane_nh_id);
  if (unlikely (! nh_info))
    return false;

  if (unlikely (nh_info->type != NEXTHOP_TYPE_CONNECTED &&
                nh_info->type != NEXTHOP_TYPE_GATEWAY))
    {
      DEBUG_NEW (ROUTING_LOOKUP,
                 "m: %p routing-lookup: dest: %s nexthop is not CONNECTED "
                 "or GATEWAY on _forwarding, drop.",
                 m, dest);
      return false;
    }

  family = nh_info->family;
  oif = nh_info->oif;

  if (IS_DEBUG (ROUTING_LOOKUP) || IS_DEBUG (ROUTER))
    {
      char nexthop_str[INET6_ADDRSTRLEN];
      inet_ntop (family, &nh_info->gw, nexthop_str, sizeof (nexthop_str));
      DEBUG_NEW (ROUTING_LOOKUP,
                 "m: %p: routing-lookup: dest: %s nexthop: %s oif: %d", m,
                 dest, nexthop_str, oif);
      DEBUG_NEW (ROUTER, "m: %p route found: nexthop: %s",
                 m, nexthop_str);
    }

  memcpy (lookup_ip, &nh_info->gw, sizeof (lookup_ip));

  if (nh_info->type == NEXTHOP_TYPE_CONNECTED)
    {
      memcpy (lookup_ip, dst_ip, sizeof (lookup_ip));
      if (IS_DEBUG (ROUTER))
        {
          char lookup_ip_str[INET6_ADDRSTRLEN];
          inet_ntop (family, lookup_ip, lookup_ip_str, sizeof (lookup_ip_str));
          DEBUG_NEW (ROUTER,
                     "m: %p directly connected route. "
                     "using dst IP for neighbor lookup: %s",
                     m, lookup_ip_str);
        }
    }

  if (fib_node->table_id == RT_TABLE_LOCAL)
    {
      struct router_if *target_rif = NULL;
      if (rx_vswitch)
        target_rif = &rx_vswitch->router_if;

      if (target_rif && target_rif->sockfd >= 0 && target_rif->ring_up)
        {
          DEBUG_NEW (ROUTER, "m: %p local route (table %u). "
                     "sending to router_if %s (ifindex=%d)",
                     m, fib_node->table_id, target_rif->tap_name,
                     target_rif->ifindex);
          _send_router_if_ring (m, rx_portid, rx_queueid, target_rif);
        }
      else
        {
          DEBUG_NEW (ROUTER, "m: %p local route (table %u). "
                     "but router_if unavailable (ifindex=%d)",
                     m, fib_node->table_id, oif);
        }
      return false;
    }

  /* forwarding: just before the neighbor-lookup. */
  if (unlikely (router_mode == 5))
    {
      _debug_manual_send (m);
      return false;
    }

  neigh_table_type = AF_TO_NEIGH_TABLE (family);
  neigh_entry = _neigh_lookup_cached (neigh_table_type, lookup_ip);

  /* forwarding: right after the neighbor-lookup. */
  if (unlikely (router_mode == 6))
    {
      _debug_manual_send (m);
      return false;
    }

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
                  if (IS_DEBUG (ROUTER))
                    {
                      char dst_ip_str[INET_ADDRSTRLEN];
                      inet_ntop (AF_INET, lookup_ip, dst_ip_str,
                                 sizeof (dst_ip_str));
                      DEBUG_NEW (ROUTER,
                                 "m: %p send ARP request: target_ip: %s,"
                                 " flooding to vswitch[%d]",
                                 arp_pkt, dst_ip_str, vs->vswitch_id);
                    }

                  _flooding (arp_pkt, ROUTER_IF_RX_SELF_PORT_ID,
                             ROUTER_IF_RX_SELF_QUEUE_ID, NULL, vs, false);

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
                  if (IS_DEBUG (ROUTER))
                    {
                      char dst_ip_str[INET6_ADDRSTRLEN];
                      inet_ntop (AF_INET6, lookup_ip, dst_ip_str,
                                 sizeof (dst_ip_str));
                      DEBUG_NEW (ROUTER,
                                 "m: %p send NS: %p, target_ip: %s,"
                                 " flooding to vswitch[%d]",
                                 m, ns_pkt, dst_ip_str, vs->vswitch_id);
                    }
                  _flooding (ns_pkt, ROUTER_IF_RX_SELF_PORT_ID,
                             ROUTER_IF_RX_SELF_QUEUE_ID, NULL, vs, false);

                  rte_pktmbuf_free (ns_pkt);
                }
              else
                DEBUG_NEW (ROUTER, "m: %p failed to generate NS", m);
            }
          return false;
        }
      return false;
    }
  dst_mac = &neigh_entry->mac_addr;

  if (IS_DEBUG (ROUTER))
    {
      char neigh_ip_str[INET6_ADDRSTRLEN];
      char neigh_mac_str[RTE_ETHER_ADDR_FMT_SIZE];
      inet_ntop (family, lookup_ip, neigh_ip_str, sizeof (neigh_ip_str));
      rte_ether_format_addr (neigh_mac_str, sizeof (neigh_mac_str), dst_mac);
      DEBUG_NEW (ROUTER,
                 "m: %p neighbor lookup succeeded: "
                 "neighbour: %s lladdr: %s",
                 m, neigh_ip_str, neigh_mac_str);
    }

  /* search the target vswitch */
  struct vswitch_conf *tx_vswitch = _tx_vswitch_lookup_cached (oif);

  if (unlikely (! tx_vswitch))
    {
      DEBUG_NEW (ROUTER, "m: %p vswitch for oif %d: not found.", m, oif);
      return false;
    }

  /* rewrite L2/L3 header */
  if (rx_portid != ROUTER_IF_RX_SELF_PORT_ID)
    {
      if (ipv4)
        {
          /* Incremental checksum update (RFC 1624) */
          uint32_t sum = ipv4->hdr_checksum;
          sum += rte_cpu_to_be_16 (0x0100);
          ipv4->hdr_checksum = (uint16_t) (sum + (sum >= 0xFFFF));
          ipv4->time_to_live--;
        }
      else if (ipv6)
        ipv6->hop_limits--;

      src_mac = &tx_vswitch->router_if.mac_addr;
      rte_ether_addr_copy (src_mac, &eth->src_addr);
      rte_ether_addr_copy (dst_mac, &eth->dst_addr);
    }

  /* forwarding: just before the switching. */
  if (unlikely (router_mode == 7))
    {
      _debug_manual_send (m);
      return false;
    }

  DEBUG_NEW (ROUTER, "m: %p L2 switching for oif %d in vswitch[%d](vlan%d)",
             m, oif, tx_vswitch->vswitch_id, tx_vswitch->vlan_id);
  return _switching (m, rx_portid, rx_queueid, NULL, tx_vswitch, eth,
                     consume);
}

#define SRV6_MAX_RECURSION 4
static inline void
_srv6_forwarding (struct rte_mbuf *m, unsigned rx_portid, unsigned rx_queueid,
                  struct rte_ether_hdr *eth, struct rte_ipv4_hdr *ipv4,
                  struct rte_ipv6_hdr *ipv6, struct vswitch_conf *rx_vswitch,
                  struct vswitch_link *rx_link, int recursion_count,
                  bool outer_is_fresh)
{
  struct nh_info *nh_info;
  struct neigh_entry *neigh_entry;
  struct rte_ether_addr *dst_mac, *src_mac;
  struct fib_node *fib_node;
  uint8_t dst_ip[16] = { 0 };
  int i;
  struct vswitch_link *tx_link = NULL;
  int dst_port = -1, neigh_table_type;

  char dest[64];
  if (IS_DEBUG (ROUTING_LOOKUP))
    {
      if (ipv4)
        inet_ntop (AF_INET, &ipv4->dst_addr, dest, sizeof (dest));
      else
        inet_ntop (AF_INET6, IPV6_ADDR_BYTES (ipv6->dst_addr), dest,
                   sizeof (dest));
      //DEBUG_LOG_MSG ("m: %p routing-lookup: dest: %s", m, dest);
    }

  if (ipv4)
    memcpy (dst_ip, &ipv4->dst_addr, sizeof (ipv4->dst_addr));
  else if (ipv6)
    memcpy (dst_ip, IPV6_ADDR_BYTES (ipv6->dst_addr), RTE_IPV6_ADDR_SIZE);

  if (router_mode == 3) /* forwarding: just before the routing-lookup. */
    {
      _debug_manual_send (m);
      return;
    }

  /* FIB lookup */
  int tree_idx = ((ipv4) ? 0 : 1); // IPv4=0, IPv6=1
  struct fib_tree *t = rib->rib_info->fib_tree[tree_idx];
  if (! t)
    return;
  fib_node = fib_route_lookup (t, dst_ip);
  if (! fib_node)
    {
      DEBUG_NEW (ROUTING_LOOKUP,
                 "m: %p routing-lookup: dest: %s FIB not found. drop.", m,
                 dest);
#if 0
      if (IS_DEBUG (ROUTER))
        log_packet (m, rx_portid, rx_queueid);
#endif
      return;
    }

  if (router_mode == 4) /* forwarding: right after the routing-lookup. */
    {
      _debug_manual_send (m);
      return;
    }

  char nexthop_str[INET6_ADDRSTRLEN];
  /* determine the actual nexthop IP to lookup in neighbor table */
  uint8_t lookup_ip[16];
  char lookup_ip_str[INET6_ADDRSTRLEN];
  int family, oif;

  nh_info = nexthop_get_info_by_index (rib->rib_info, fib_node->sdplane_nh_id);
  family = nh_info->family;
  oif = nh_info->oif;

  if (nh_info->type == NEXTHOP_TYPE_SRV6_END)
    {
      DEBUG_NEW (ROUTING_LOOKUP, "m: %p SRv6 Endpoint processing for SID: %s",
                 m, dest);
      if (recursion_count >= SRV6_MAX_RECURSION)
        {
          DEBUG_NEW (ROUTER, "m: %p SRv6 recursion limit reached", m);
          return;
        }
      if (! ipv6)
        {
          DEBUG_NEW (ROUTER,
                     "m: %p SRv6 Endpoint SID matched a non-IPv6 "
                     "packet, drop.",
                     m);
          return;
        }

      if (unlikely (nh_info->gw.srv6_local_sid.action !=
                    SEG6_LOCAL_ACTION_END))
        {
          DEBUG_NEW (ROUTER,
                     "m: %p: unsupported seg6local action "
                     "on _srv6_forwrding: %u, drop",
                     m, srv6_local_sid->action);
          return;
        }

      if (! _process_srv6_end_packet (m, ipv6, &nh_info->gw.srv6_local_sid))
        {
          DEBUG_NEW (ROUTER, "m: %p failed to process SRv6 Endpoint", m);
          return;
        }
      rx_link = NULL; // to avoid split-horizon check
      /* outer_is_fresh is passed through: unlike the encap case this outer
         header came off the wire, so it still owes a hop. */
      _srv6_forwarding (m, rx_portid, rx_queueid, eth, ipv4, ipv6, rx_vswitch,
                        rx_link, recursion_count + 1, outer_is_fresh);
      return;
    }

  if (nh_info->type == NEXTHOP_TYPE_SRV6_HEAD)
    {
      DEBUG_NEW (ROUTING_LOOKUP, "m: %p SRv6 Encapsulation for dest: %s", m,
                 dest);
      if (recursion_count >= SRV6_MAX_RECURSION)
        {
          DEBUG_NEW (ROUTER, "m: %p SRv6 recursion limit reached", m);
          return;
        }
      if (! _process_srv6_encap (
              m, &eth, &ipv4, &ipv6, &nh_info->gw.srv6_encap_sid,
              &rib->rib_info->srv6_tunnel_src_addr, rx_portid, rx_queueid))
        {
          DEBUG_NEW (ROUTER, "m: %p failed to process SRv6 encapsulation", m);
          return;
        }
      rx_link = NULL; // to avoid split-horizon check
      _srv6_forwarding (m, rx_portid, rx_queueid, eth, ipv4, ipv6, rx_vswitch,
                        rx_link, recursion_count + 1, true);
      return;
    }

  /* Everything below resolves the nexthop as an L3 adjacency and reads
     nh_info->gw as an address, so reject the types that do not hold one.
     Same allow-list as _forwarding(). */
  if (unlikely (nh_info->type != NEXTHOP_TYPE_CONNECTED &&
                nh_info->type != NEXTHOP_TYPE_GATEWAY))
    {
      DEBUG_NEW (ROUTING_LOOKUP,
                 "m: %p routing-lookup: dest: %s nexthop is not CONNECTED "
                 "or GATEWAY on _srv6_forwarding, drop.",
                 m, dest);
      return;
    }

  if (IS_DEBUG (ROUTING_LOOKUP) || IS_DEBUG (ROUTER))
    inet_ntop (family, &nh_info->gw, nexthop_str, sizeof (nexthop_str));

  DEBUG_NEW (ROUTING_LOOKUP,
             "m: %p: routing-lookup: dest: %s nexthop: %s oif: %d", m, dest,
             nexthop_str, oif);
  DEBUG_NEW (ROUTER, "m: %p route found: nexthop: %s", m, nexthop_str);

  memcpy (lookup_ip, &nh_info->gw, sizeof (lookup_ip));
  if (IS_DEBUG (ROUTER))
    inet_ntop (family, lookup_ip, lookup_ip_str, sizeof (lookup_ip_str));

  if (nh_info->type == NEXTHOP_TYPE_CONNECTED)
    {
      memcpy (lookup_ip, dst_ip, sizeof (lookup_ip));
      if (IS_DEBUG (ROUTER))
        inet_ntop (family, lookup_ip, lookup_ip_str, sizeof (lookup_ip_str));
      DEBUG_NEW (ROUTER,
                 "m: %p directly connected route. "
                 "using dst IP for neighbor lookup: %s",
                 m, lookup_ip_str);
    }

  if (fib_node->table_id == RT_TABLE_LOCAL)
    {
      struct router_if *target_rif;
      target_rif = &rx_vswitch->router_if;

      if (target_rif && target_rif->sockfd >= 0 && target_rif->ring_up)
        {
          DEBUG_NEW (ROUTER,
                     "m: %p local route (table %u). "
                     "sending to router_if %s (ifindex=%d)",
                     m, fib_node->table_id, target_rif->tap_name,
                     target_rif->ifindex);
          _send_router_if_ring (m, rx_portid, rx_queueid, target_rif);
        }
      else
        {
          DEBUG_NEW (ROUTER,
                     "m: %p local route (table %u). "
                     "but router_if unavailable (ifindex=%d)",
                     m, fib_node->table_id, oif);
        }
      return;
    }

  if (router_mode == 5) /* forwarding: just before the neighbor-lookup. */
    {
      _debug_manual_send (m);
      return;
    }

  /* neighbor table lookup */
  neigh_table_type = AF_TO_NEIGH_TABLE (family);
  neigh_entry =
      neigh_lookup (&rib->rib_info->neigh_tables[neigh_table_type],
                    neigh_table_type, lookup_ip);

  if (router_mode == 6) /* forwarding: right after the neighbor-lookup. */
    {
      _debug_manual_send (m);
      return;
    }

  if (! neigh_entry || rte_is_zero_ether_addr (&neigh_entry->mac_addr))
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
                  if (IS_DEBUG (ROUTER))
                    {
                      char dst_ip_str[INET_ADDRSTRLEN];
                      inet_ntop (AF_INET, lookup_ip, dst_ip_str,
                                 sizeof (dst_ip_str));
                      DEBUG_NEW (ROUTER,
                                 "m: %p send ARP request: target_ip: %s,"
                                 " flooding to vswitch[%d]",
                                 arp_pkt, dst_ip_str, vs->vswitch_id);
                    }

                  _flooding (arp_pkt, ROUTER_IF_RX_SELF_PORT_ID,
                             ROUTER_IF_RX_SELF_QUEUE_ID, NULL, vs, false);

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
                  if (IS_DEBUG (ROUTER))
                    {
                      char dst_ip_str[INET6_ADDRSTRLEN];
                      inet_ntop (AF_INET6, lookup_ip, dst_ip_str,
                                 sizeof (dst_ip_str));
                      DEBUG_NEW (ROUTER,
                                 "m: %p send NS: %p, target_ip: %s,"
                                 " flooding to vswitch[%d]",
                                 m, ns_pkt, dst_ip_str, vs->vswitch_id);
                    }
                  _flooding (ns_pkt, ROUTER_IF_RX_SELF_PORT_ID,
                             ROUTER_IF_RX_SELF_QUEUE_ID, NULL, vs, false);

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
  if (IS_DEBUG (ROUTER))
    rte_ether_format_addr (neigh_mac_str, sizeof (neigh_mac_str), dst_mac);
  DEBUG_NEW (ROUTER,
             "m: %p neighbor lookup succeeded: "
             "neighbour: %s lladdr: %s",
             m, lookup_ip_str, neigh_mac_str);

  /* search the target vswitch */
  struct vswitch_conf *tx_vswitch = NULL;
  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vs = &rib->rib_info->vswitch[i];

      if (vs->is_deleted)
        continue;

      if (vs->router_if.ifindex == oif)
        {
          tx_vswitch = vs;
          break;
        }
    }

  if (! tx_vswitch)
    {
      DEBUG_NEW (ROUTER, "m: %p vswitch for oif %d: not found.", m, oif);
      return;
    }

  /* rewrite L2/L3 header */
  if (rx_portid != ROUTER_IF_RX_SELF_PORT_ID)
    {
      /* outer_is_fresh means _process_srv6_encap() built this outer header
         on the way in.  It already decremented the inner header, and RFC 2473
         has the encapsulating node set the outer Hop Limit rather than spend
         one on it, so leave the outer alone. */
      if (! outer_is_fresh)
        {
          if (ipv4)
            {
              ipv4->time_to_live--;
              ipv4->hdr_checksum = 0;
              ipv4->hdr_checksum = rte_ipv4_cksum (ipv4);
            }
          else if (ipv6)
            ipv6->hop_limits--;
        }

      src_mac = &tx_vswitch->router_if.mac_addr;
      rte_ether_addr_copy (src_mac, &eth->src_addr);
      rte_ether_addr_copy (dst_mac, &eth->dst_addr);
    }

  if (router_mode == 7) /* forwarding: just before the switching. */
    {
      _debug_manual_send (m);
      return;
    }

  DEBUG_NEW (ROUTER, "m: %p L2 switching for oif %d in vswitch[%d](vlan%d)", m,
             oif, tx_vswitch->vswitch_id, tx_vswitch->vlan_id);
  _switching (m, rx_portid, rx_queueid, NULL, tx_vswitch, eth, false);
}

static inline void
_srv6_dx_forwarding (struct rte_mbuf *m, unsigned rx_portid, unsigned rx_queueid,
                  struct rte_ether_hdr *eth, struct rte_ipv4_hdr *ipv4,
                  struct rte_ipv6_hdr *ipv6, struct vswitch_conf *rx_vswitch,
                  struct vswitch_link *rx_link, int recursion_count,
                  bool outer_is_fresh)
{
  struct nh_info *nh_info, endx_nh;
  struct neigh_entry *neigh_entry;
  struct rte_ether_addr *dst_mac, *src_mac;
  struct fib_node *fib_node;
  uint8_t dst_ip[16] = { 0 };
  int i;
  struct vswitch_link *tx_link = NULL;
  int dst_port = -1, neigh_table_type;

  char dest[64];
  if (IS_DEBUG (ROUTING_LOOKUP))
    {
      if (ipv4)
        inet_ntop (AF_INET, &ipv4->dst_addr, dest, sizeof (dest));
      else
        inet_ntop (AF_INET6, IPV6_ADDR_BYTES (ipv6->dst_addr), dest,
                   sizeof (dest));
      //DEBUG_LOG_MSG ("m: %p routing-lookup: dest: %s", m, dest);
    }

  if (ipv4)
    memcpy (dst_ip, &ipv4->dst_addr, sizeof (ipv4->dst_addr));
  else if (ipv6)
    memcpy (dst_ip, IPV6_ADDR_BYTES (ipv6->dst_addr), RTE_IPV6_ADDR_SIZE);

  if (router_mode == 3) /* forwarding: just before the routing-lookup. */
    {
      _debug_manual_send (m);
      return;
    }

  /* FIB lookup */
  int tree_idx = ((ipv4) ? 0 : 1); // IPv4=0, IPv6=1
  struct fib_tree *t = rib->rib_info->fib_tree[tree_idx];
  if (! t)
    return;
  fib_node = fib_route_lookup (t, dst_ip);
  if (! fib_node)
    {
      DEBUG_NEW (ROUTING_LOOKUP,
                 "m: %p routing-lookup: dest: %s FIB not found. drop.", m,
                 dest);
#if 0
      if (IS_DEBUG (ROUTER))
        log_packet (m, rx_portid, rx_queueid);
#endif
      return;
    }

  if (router_mode == 4) /* forwarding: right after the routing-lookup. */
    {
      _debug_manual_send (m);
      return;
    }

  char nexthop_str[INET6_ADDRSTRLEN];
  /* determine the actual nexthop IP to lookup in neighbor table */
  uint8_t lookup_ip[16];
  char lookup_ip_str[INET6_ADDRSTRLEN];
  int family, oif;

  nh_info = nexthop_get_info_by_index (rib->rib_info, fib_node->sdplane_nh_id);

  if (nh_info->type == NEXTHOP_TYPE_SRV6_END)
    {
      DEBUG_NEW (ROUTING_LOOKUP, "m: %p SRv6 Endpoint processing for SID: %s",
                 m, dest);
      if (recursion_count >= SRV6_MAX_RECURSION)
        {
          DEBUG_NEW (ROUTER, "m: %p SRv6 recursion limit reached", m);
          return;
        }
      if (! ipv6)
        {
          DEBUG_NEW (ROUTER,
                     "m: %p SRv6 Endpoint SID matched a non-IPv6 "
                     "packet, drop.",
                     m);
          return;
        }

      uint32_t sid_action = nh_info->gw.srv6_local_sid.action;

      // handle End.DX6 or End.DX4
      if (sid_action == SEG6_LOCAL_ACTION_END_DX6 ||
          sid_action == SEG6_LOCAL_ACTION_END_DX4)
        {
          bool dx6 = (sid_action == SEG6_LOCAL_ACTION_END_DX6);
          bool is_unspecified =
              (dx6 ? IN6_IS_ADDR_UNSPECIFIED (
                         &nh_info->gw.srv6_local_sid.param.nh6)
                   : (nh_info->gw.srv6_local_sid.param.nh4.s_addr == 0));
          if (is_unspecified)
            {
              DEBUG_NEW (ROUTER, "m: %p End.DX%d SID has no adjacency, drop.",
                         m, (dx6 ? 6 : 4));
              return;
            }

          if (! _process_srv6_end_dx_packet (
                  m, &eth, &ipv4, &ipv6, (dx6 ? IPPROTO_IPV6 : IPPROTO_IPIP)))
            {
              DEBUG_NEW (ROUTER, "m: %p failed to process SRv6 End.DX%d", m,
                         (dx6 ? 6 : 4));
              return;
            }

          // verify inner packet
          // because hw offload state of `m` is reset by `_process_srv6_end_dx_packet`,
          // rx_port can be NULL
          if (unlikely ( _verify_packet (m, NULL, ipv4, ipv6) != 0))
            {
              DEBUG_NEW (ROUTER,
                         "m: %p decapsulated packet is not forwardable, "
                         "drop.",
                         m);
              return;
            }
        }
      // handle End or End.X
      else
        {
          if (sid_action == SEG6_LOCAL_ACTION_END_X)
            {
              if (IN6_IS_ADDR_UNSPECIFIED (
                      &nh_info->gw.srv6_local_sid.param.nh6))
                {
                  DEBUG_NEW (ROUTER, "m: %p End.X SID has no adjacency, drop.",
                             m);
                  return;
                }
            }
          else if (unlikely (sid_action != SEG6_LOCAL_ACTION_END))
            {
              DEBUG_NEW (ROUTER,
                         "m: %p: unsupported seg6local action "
                         "on _srv6_dx_forwrding: %u, drop",
                         m, srv6_local_sid->action);
              return;
            }

          if (! _process_srv6_end_packet (m, ipv6,
                                          &nh_info->gw.srv6_local_sid))
            {
              DEBUG_NEW (ROUTER, "m: %p failed to process SRv6 Endpoint", m);
              return;
            }
        }

      rx_link = NULL; // to avoid split-horizon check

      if (sid_action == SEG6_LOCAL_ACTION_END)
        {
          /* outer_is_fresh is passed through: unlike the encap case this
             outer header came off the wire, so it still owes a hop. */
          _srv6_dx_forwarding (m, rx_portid, rx_queueid, eth, ipv4, ipv6,
                            rx_vswitch, rx_link, recursion_count + 1,
                            outer_is_fresh);
          return;
        }

      // now common routine of End.X, End.DX4, and End.DX6
      // translate srv6_local_sid.param.nh4/nh6 into NEXTHOP_TYPE_GATEWAY
      // and go through normal neighbor resolution
      memset (&endx_nh, 0, sizeof (endx_nh));
      endx_nh.type = NEXTHOP_TYPE_GATEWAY;
      if (sid_action == SEG6_LOCAL_ACTION_END_DX4)
        {
          endx_nh.family = AF_INET;
          endx_nh.gw.ipv4_addr = nh_info->gw.srv6_local_sid.param.nh4;
        }
      else
        {
          endx_nh.family = AF_INET6;
          endx_nh.gw.ipv6_addr = nh_info->gw.srv6_local_sid.param.nh6;
        }

      endx_nh.oif = nh_info->oif;
      nh_info = &endx_nh;
      /* dest was rendered before the SRH was advanced (End.X) or the outer
         header removed (End.DX6), so it still names the SID.  The End case
         re-enters this function and rebuilds it; these two fall through, so
         refresh it here or the routing-lookup line below reports the wrong
         destination. */
      if (IS_DEBUG (ROUTING_LOOKUP))
        {
          if (ipv4)
            inet_ntop (AF_INET, &ipv4->dst_addr, dest, sizeof (dest));
          else
            inet_ntop (AF_INET6, IPV6_ADDR_BYTES (ipv6->dst_addr), dest,
                       sizeof (dest));
        }

    }
  else if (nh_info->type == NEXTHOP_TYPE_SRV6_HEAD)
    {
      DEBUG_NEW (ROUTING_LOOKUP, "m: %p SRv6 Encapsulation for dest: %s", m,
                 dest);
      if (recursion_count >= SRV6_MAX_RECURSION)
        {
          DEBUG_NEW (ROUTER, "m: %p SRv6 recursion limit reached", m);
          return;
        }
      if (! _process_srv6_encap (
              m, &eth, &ipv4, &ipv6, &nh_info->gw.srv6_encap_sid,
              &rib->rib_info->srv6_tunnel_src_addr, rx_portid, rx_queueid))
        {
          DEBUG_NEW (ROUTER, "m: %p failed to process SRv6 encapsulation", m);
          return;
        }
      rx_link = NULL; // to avoid split-horizon check
      _srv6_dx_forwarding (m, rx_portid, rx_queueid, eth, ipv4, ipv6, rx_vswitch,
                        rx_link, recursion_count + 1, true);
      return;
    }

  family = nh_info->family;
  oif = nh_info->oif;

  /* Everything below resolves the nexthop as an L3 adjacency and reads
     nh_info->gw as an address, so reject the types that do not hold one.
     Same allow-list as _forwarding(). */
  if (unlikely (nh_info->type != NEXTHOP_TYPE_CONNECTED &&
                nh_info->type != NEXTHOP_TYPE_GATEWAY))
    {
      DEBUG_NEW (ROUTING_LOOKUP,
                 "m: %p routing-lookup: dest: %s nexthop is not CONNECTED "
                 "or GATEWAY on _srv6_dx_forwarding, drop.",
                 m, dest);
      return;
    }

  if (IS_DEBUG (ROUTING_LOOKUP) || IS_DEBUG (ROUTER))
    inet_ntop (family, &nh_info->gw, nexthop_str, sizeof (nexthop_str));

  DEBUG_NEW (ROUTING_LOOKUP,
             "m: %p: routing-lookup: dest: %s nexthop: %s oif: %d", m, dest,
             nexthop_str, oif);
  DEBUG_NEW (ROUTER, "m: %p route found: nexthop: %s", m, nexthop_str);

  memcpy (lookup_ip, &nh_info->gw, sizeof (lookup_ip));
  if (IS_DEBUG (ROUTER))
    inet_ntop (family, lookup_ip, lookup_ip_str, sizeof (lookup_ip_str));

  if (nh_info->type == NEXTHOP_TYPE_CONNECTED)
    {
      memcpy (lookup_ip, dst_ip, sizeof (lookup_ip));
      if (IS_DEBUG (ROUTER))
        inet_ntop (family, lookup_ip, lookup_ip_str, sizeof (lookup_ip_str));
      DEBUG_NEW (ROUTER,
                 "m: %p directly connected route. "
                 "using dst IP for neighbor lookup: %s",
                 m, lookup_ip_str);
    }

  if (fib_node->table_id == RT_TABLE_LOCAL)
    {
      struct router_if *target_rif;
      target_rif = &rx_vswitch->router_if;

      if (target_rif && target_rif->sockfd >= 0 && target_rif->ring_up)
        {
          DEBUG_NEW (ROUTER,
                     "m: %p local route (table %u). "
                     "sending to router_if %s (ifindex=%d)",
                     m, fib_node->table_id, target_rif->tap_name,
                     target_rif->ifindex);
          _send_router_if_ring (m, rx_portid, rx_queueid, target_rif);
        }
      else
        {
          DEBUG_NEW (ROUTER,
                     "m: %p local route (table %u). "
                     "but router_if unavailable (ifindex=%d)",
                     m, fib_node->table_id, oif);
        }
      return;
    }

  if (router_mode == 5) /* forwarding: just before the neighbor-lookup. */
    {
      _debug_manual_send (m);
      return;
    }

  /* neighbor table lookup */
  neigh_table_type = AF_TO_NEIGH_TABLE (family);
  neigh_entry =
      neigh_lookup (&rib->rib_info->neigh_tables[neigh_table_type],
                    neigh_table_type, lookup_ip);

  if (router_mode == 6) /* forwarding: right after the neighbor-lookup. */
    {
      _debug_manual_send (m);
      return;
    }

  if (! neigh_entry || rte_is_zero_ether_addr (&neigh_entry->mac_addr))
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
                  if (IS_DEBUG (ROUTER))
                    {
                      char dst_ip_str[INET_ADDRSTRLEN];
                      inet_ntop (AF_INET, lookup_ip, dst_ip_str,
                                 sizeof (dst_ip_str));
                      DEBUG_NEW (ROUTER,
                                 "m: %p send ARP request: target_ip: %s,"
                                 " flooding to vswitch[%d]",
                                 arp_pkt, dst_ip_str, vs->vswitch_id);
                    }

                  _flooding (arp_pkt, ROUTER_IF_RX_SELF_PORT_ID,
                             ROUTER_IF_RX_SELF_QUEUE_ID, NULL, vs, false);

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
                  if (IS_DEBUG (ROUTER))
                    {
                      char dst_ip_str[INET6_ADDRSTRLEN];
                      inet_ntop (AF_INET6, lookup_ip, dst_ip_str,
                                 sizeof (dst_ip_str));
                      DEBUG_NEW (ROUTER,
                                 "m: %p send NS: %p, target_ip: %s,"
                                 " flooding to vswitch[%d]",
                                 m, ns_pkt, dst_ip_str, vs->vswitch_id);
                    }
                  _flooding (ns_pkt, ROUTER_IF_RX_SELF_PORT_ID,
                             ROUTER_IF_RX_SELF_QUEUE_ID, NULL, vs, false);

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
  if (IS_DEBUG (ROUTER))
    rte_ether_format_addr (neigh_mac_str, sizeof (neigh_mac_str), dst_mac);
  DEBUG_NEW (ROUTER,
             "m: %p neighbor lookup succeeded: "
             "neighbour: %s lladdr: %s",
             m, lookup_ip_str, neigh_mac_str);

  /* search the target vswitch */
  struct vswitch_conf *tx_vswitch = NULL;
  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vs = &rib->rib_info->vswitch[i];

      if (vs->is_deleted)
        continue;

      if (vs->router_if.ifindex == oif)
        {
          tx_vswitch = vs;
          break;
        }
    }

  if (! tx_vswitch)
    {
      DEBUG_NEW (ROUTER, "m: %p vswitch for oif %d: not found.", m, oif);
      return;
    }

  /* rewrite L2/L3 header */
  if (rx_portid != ROUTER_IF_RX_SELF_PORT_ID)
    {
      /* outer_is_fresh means _process_srv6_encap() built this outer header
         on the way in.  It already decremented the inner header, and RFC 2473
         has the encapsulating node set the outer Hop Limit rather than spend
         one on it, so leave the outer alone. */
      if (! outer_is_fresh)
        {
          if (ipv4)
            {
              ipv4->time_to_live--;
              ipv4->hdr_checksum = 0;
              ipv4->hdr_checksum = rte_ipv4_cksum (ipv4);
            }
          else if (ipv6)
            ipv6->hop_limits--;
        }

      src_mac = &tx_vswitch->router_if.mac_addr;
      rte_ether_addr_copy (src_mac, &eth->src_addr);
      rte_ether_addr_copy (dst_mac, &eth->dst_addr);
    }

  if (router_mode == 7) /* forwarding: just before the switching. */
    {
      _debug_manual_send (m);
      return;
    }

  DEBUG_NEW (ROUTER, "m: %p L2 switching for oif %d in vswitch[%d](vlan%d)", m,
             oif, tx_vswitch->vswitch_id, tx_vswitch->vlan_id);
  _switching (m, rx_portid, rx_queueid, NULL, tx_vswitch, eth, false);
}

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

// IPv4 multicast (224.0.0.0/4 except 224.0.0.0/24)
static inline bool
_is_ipv4_multicast (struct rte_ipv4_hdr *ipv4)
{
  uint32_t dst_ip = rte_be_to_cpu_32 (ipv4->dst_addr);
  return (dst_ip & 0xf0000000) == 0xe0000000 &&
         (dst_ip & 0xffffff00) != 0xe0000000;
}

// if IPv4 FIB has an entry for SRv6 H.Encaps route
static inline bool
_has_srv6_h_encaps_route (const struct rte_ipv4_hdr *ipv4)
{
  uint8_t key[16] = { 0 };
  struct fib_tree *t = rib->rib_info->fib_tree[0]; // IPv4=0
  struct fib_node *fib_node;
  struct nh_info *nh_info;

  if (! t)
    return false;

  memcpy (key, &ipv4->dst_addr, sizeof (ipv4->dst_addr));
  fib_node = fib_route_lookup (t, key);
  if (! fib_node)
    return false;

  nh_info = nexthop_get_info_by_index (rib->rib_info, fib_node->sdplane_nh_id);
  if (! nh_info)
    return false;

  return nh_info->type == NEXTHOP_TYPE_SRV6_HEAD;
}

#endif /*__NET_SUBR_H__*/
