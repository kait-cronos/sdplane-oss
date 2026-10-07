#ifndef __SRV6_H__
#define __SRV6_H__

#include "log_packet.h"
#include <rte_version.h>
#if RTE_VERSION < RTE_VERSION_NUM(24, 0, 0, 0)
  #include <rte_ip.h>
#else
  #include <rte_ip6.h>
#endif
#include "rte_override.h"
#include "debug_sdplane.h"
#include "nexthop.h"
#include <linux/seg6_local.h>

static inline __attribute__ ((always_inline)) bool
_process_srv6_packet (struct rte_mbuf *m,
                      struct rte_ipv6_hdr *ipv6,
                      struct in6_addr *local_end_sid)
{
  bool modified = false;

  /* If not routing-header, quit processing. */
  if (ipv6->proto != IPPROTO_ROUTING)
  {
    DEBUG_NEW (ROUTER, "m: %p: ipv6->proto != ROUTING", m);
    return modified;
  }

  /* If the packet is too short. */
  uint32_t pkt_len = rte_pktmbuf_pkt_len (m);
  if (unlikely (pkt_len < sizeof (struct rte_ipv6_hdr) +
                          sizeof (struct rte_ipv6_routing_ext)))
  {
    DEBUG_NEW (ROUTER, "m: %p: pkt_len short: %d", m, pkt_len);
    return modified;
  }

  struct rte_ipv6_routing_ext *srh;
  srh = (struct rte_ipv6_routing_ext *)(ipv6 + 1);
  if (srh->type != RTE_IPV6_SRCRT_TYPE_4)
  {
    DEBUG_NEW (ROUTER, "m: %p: SRH->type != SRCRT_TYPE_4", m);
    return modified;
  }

  int max_last_entry = (srh->hdr_len / 2) - 1;
  if (srh->last_entry > max_last_entry ||
      (srh->segments_left > srh->last_entry + 1))
  {
    DEBUG_NEW (ROUTER, "m: %p: max_entry or segments_left invalid", m);
    return modified;
  }

  if (srh->segments_left == 0)
  {
    DEBUG_NEW (ROUTER, "m: %p: zero segments_left", m);
    return modified;
  }

  void *dst_addr;
#if RTE_VERSION < RTE_VERSION_NUM(24, 0, 0, 0)
  dst_addr = (void *) &ipv6->dst_addr[0];
#else
  dst_addr = (void *) &ipv6->dst_addr.a;
#endif

  /* if the IPv6 destination is not our local End SID, return */
  if (memcmp (dst_addr, local_end_sid, 16))
  {
    DEBUG_NEW (ROUTER, "m: %p: local_end_sid mismatch", m);
    return modified;
  }

  srh->segments_left --;

  struct in6_addr* sid_list = (struct in6_addr *)(srh + 1);
  rte_memcpy (dst_addr, &sid_list[srh->segments_left], 16);

  DEBUG_NEW (ROUTER, "m: %p: reload dst_addr from sid_list[%d]",
             m, srh->segments_left);
  modified = true;
  return modified;
}

static inline __attribute__ ((always_inline)) bool
_process_srv6_packet_multi_sids (struct rte_mbuf *m,
                      struct rte_ipv6_hdr *ipv6,
                      struct srv6_local_sid_entry *srv6_local_sid,
                      int num_sids)
{
  bool modified = false;

  /* If not routing-header, quit processing. */
  if (ipv6->proto != IPPROTO_ROUTING)
  {
    DEBUG_NEW (ROUTER, "m: %p: ipv6->proto != ROUTING", m);
    return modified;
  }

  /* If the packet is too short. */
  uint32_t pkt_len = rte_pktmbuf_pkt_len (m);
  if (unlikely (pkt_len < sizeof (struct rte_ipv6_hdr) +
                          sizeof (struct rte_ipv6_routing_ext)))
  {
    DEBUG_NEW (ROUTER, "m: %p: pkt_len short: %d", m, pkt_len);
    return modified;
  }

  struct rte_ipv6_routing_ext *srh;
  srh = (struct rte_ipv6_routing_ext *)(ipv6 + 1);
  if (srh->type != RTE_IPV6_SRCRT_TYPE_4)
  {
    DEBUG_NEW (ROUTER, "m: %p: SRH->type != SRCRT_TYPE_4", m);
    return modified;
  }

  int max_last_entry = (srh->hdr_len / 2) - 1;
  if (srh->last_entry > max_last_entry ||
      (srh->segments_left > srh->last_entry + 1))
  {
    DEBUG_NEW (ROUTER, "m: %p: max_entry or segments_left invalid", m);
    return modified;
  }

  if (srh->segments_left == 0)
  {
    DEBUG_NEW (ROUTER, "m: %p: zero segments_left", m);
    return modified;
  }

  void *dst_addr;
#if RTE_VERSION < RTE_VERSION_NUM(24, 0, 0, 0)
  dst_addr = (void *) &ipv6->dst_addr[0];
#else
  dst_addr = (void *) &ipv6->dst_addr.a;
#endif

  /* if the IPv6 destination is not our local End SID, return */
  int i;
  int match = 0;
  for (i = 0; i < num_sids; i++)
    {
      if (! memcmp (dst_addr, &srv6_local_sid[i].sid, 16))
        match++;
    }
  if (match == 0)
    {
      DEBUG_NEW (ROUTER, "m: %p: local_end_sid mismatch with any sids", m);
      return modified;
    }

  srh->segments_left --;

  struct in6_addr* sid_list = (struct in6_addr *)(srh + 1);
  rte_memcpy (dst_addr, &sid_list[srh->segments_left], 16);

  DEBUG_NEW (ROUTER, "m: %p: reload dst_addr from sid_list[%d]",
             m, srh->segments_left);
  modified = true;
  return modified;
}

/* SRv6 Endpoint (RFC 8986 End) processing for the nexthop-based model.
   The SID match has already been done by the FIB lookup, so unlike
   _process_srv6_packet_multi_sids() this does not compare the destination
   against a local SID table.  Behaviors other than plain End are not
   implemented yet and are rejected here rather than silently treated as
   End. */
static inline __attribute__ ((always_inline)) bool
_process_srv6_end_packet (struct rte_mbuf *m, struct rte_ipv6_hdr *ipv6,
                          struct srv6_local_sid_entry *srv6_local_sid)
{
  bool modified = false;

  if (srv6_local_sid->action != SEG6_LOCAL_ACTION_END &&
      srv6_local_sid->action != SEG6_LOCAL_ACTION_END_X)
    {
      DEBUG_NEW (ROUTER, "m: %p: unsupported seg6local action: %u", m,
                 srv6_local_sid->action);
      return modified;
    }

  /* If not routing-header, quit processing. */
  if (ipv6->proto != IPPROTO_ROUTING)
    {
      DEBUG_NEW (ROUTER, "m: %p: ipv6->proto != ROUTING", m);
      return modified;
    }

  /* Bound every access below by the contiguous data of the first segment.
     Deriving the end from the ipv6 pointer keeps this correct whatever L2
     header sits in front (untagged 14, VLAN 18, ...), which is why
     comparing rte_pktmbuf_pkt_len() against L3-only sizes is not enough. */
  const uint8_t *pkt_end =
      rte_pktmbuf_mtod (m, const uint8_t *) + rte_pktmbuf_data_len (m);

  struct rte_ipv6_routing_ext *srh;
  srh = (struct rte_ipv6_routing_ext *) (ipv6 + 1);
  if (unlikely ((const uint8_t *) (srh + 1) > pkt_end))
    {
      DEBUG_NEW (ROUTER, "m: %p: SRH does not fit in the buffer", m);
      return modified;
    }

  if (srh->type != RTE_IPV6_SRCRT_TYPE_4)
    {
      DEBUG_NEW (ROUTER, "m: %p: SRH->type != SRCRT_TYPE_4", m);
      return modified;
    }

  /* hdr_len counts 8-octet units excluding the first 8 octets.  Confirm the
     whole SRH is present before trusting it: the SID list is addressed
     through hdr_len, so an over-declared hdr_len would index past the
     buffer even when the declared IPv6 payload length looks consistent. */
  uint32_t srh_len = ((uint32_t) srh->hdr_len + 1) * 8;
  if (unlikely ((const uint8_t *) srh + srh_len > pkt_end))
    {
      DEBUG_NEW (ROUTER, "m: %p: SRH hdr_len %u exceeds the buffer", m,
                 srh->hdr_len);
      return modified;
    }

  /* With the SRH bounded above, last_entry <= max_last_entry keeps
     sid_list[last_entry] inside it: the last SID ends exactly at
     srh + (hdr_len + 1) * 8. */
  int max_last_entry = (srh->hdr_len / 2) - 1;
  if (srh->last_entry > max_last_entry ||
      (srh->segments_left > srh->last_entry + 1))
    {
      DEBUG_NEW (ROUTER, "m: %p: max_entry or segments_left invalid", m);
      return modified;
    }

  if (srh->segments_left == 0)
    {
      DEBUG_NEW (ROUTER, "m: %p: zero segments_left", m);
      return modified;
    }

  void *dst_addr;
#if RTE_VERSION < RTE_VERSION_NUM(24, 0, 0, 0)
  dst_addr = (void *) &ipv6->dst_addr[0];
#else
  dst_addr = (void *) &ipv6->dst_addr.a;
#endif

  srh->segments_left--;

  struct in6_addr *sid_list = (struct in6_addr *) (srh + 1);
  rte_memcpy (dst_addr, &sid_list[srh->segments_left], 16);

  DEBUG_NEW (ROUTER, "m: %p: reload dst_addr from sid_list[%d]", m,
             srh->segments_left);
  modified = true;
  return modified;
}

static inline __attribute__ ((always_inline)) bool
_process_srv6_encap (struct rte_mbuf *m,
                                  struct rte_ether_hdr **original_ether_hdr,
                                  struct rte_ipv4_hdr **original_ipv4_hdr,
                                  struct rte_ipv6_hdr **original_ipv6_hdr,
                                  struct srv6_encap_sid_entry *srv6_encap_sid,
                                  struct in6_addr *src_addr, int rx_portid,
                                  int rx_queueid)
{
  /* encapsulate with SRv6 */
  uint32_t outer_header_size =
      sizeof (struct rte_ipv6_hdr) + sizeof (struct rte_ipv6_routing_ext) +
      srv6_encap_sid->segment_list_size * sizeof (struct in6_addr);

  DEBUG_NEW (ROUTER,
             "m: %p: encapsulating with SRv6, original pkt len"
             " %d, outer header size %d, segment list size %d,"
             " original %s hdr packet len %d",
             m, rte_pktmbuf_pkt_len (m), outer_header_size,
             srv6_encap_sid->segment_list_size,
             ((*original_ipv4_hdr) ? "IPv4" : "IPv6"),
             ((*original_ipv4_hdr)
                  ? rte_be_to_cpu_16 ((*original_ipv4_hdr)->total_length)
                  : ((*original_ipv6_hdr)
                         ? rte_be_to_cpu_16 ((*original_ipv6_hdr)->payload_len)
                         : 0)));
  // because the original may have some headers between ether_hdr and ipv6_hdr,
  // we need to prepend the same size of header space for new_ether_hdr,
  // and copy the header content before ipv6_hdr to the new_ether_hdr.
  size_t original_pre_ip_hdr_size;
  if (*original_ipv4_hdr)
    {
      // RFC8986 and RFC2473, decrement TTL and recalculate checksum
      if ((*original_ipv4_hdr)->time_to_live <= 1)
        {
          DEBUG_NEW (ROUTER, "m: %p: TTL is %d, cannot encapsulate", m,
                     (*original_ipv4_hdr)->time_to_live);
          return false;
        }
      (*original_ipv4_hdr)->time_to_live--;
      (*original_ipv4_hdr)->hdr_checksum = 0;
      (*original_ipv4_hdr)->hdr_checksum = rte_ipv4_cksum (*original_ipv4_hdr);
      original_pre_ip_hdr_size =
          (uint8_t *) *original_ipv4_hdr - (uint8_t *) *original_ether_hdr;
    }
  else if (*original_ipv6_hdr)
    {
      // RFC8986 and RFC2473, decrement hop_limits
      if ((*original_ipv6_hdr)->hop_limits <= 1)
        {
          DEBUG_NEW (ROUTER, "m: %p: hop_limits is %d, cannot encapsulate", m,
                     (*original_ipv6_hdr)->hop_limits);
          return false;
        }
      (*original_ipv6_hdr)->hop_limits--;
      original_pre_ip_hdr_size =
          (uint8_t *) *original_ipv6_hdr - (uint8_t *) *original_ether_hdr;
    }
  else
    {
      DEBUG_NEW (ROUTER, "m: %p: no ipv4 or ipv6 header found for encapsulate",
                 m);
      return false;
    }
  struct rte_ether_hdr *new_ether_hdr =
      (struct rte_ether_hdr *) rte_pktmbuf_prepend (m, outer_header_size);
  if (! new_ether_hdr)
    {
      DEBUG_NEW (ROUTER, "m: %p: failed to prepend for encapsulate", m);
      return false;
    }
  memmove (new_ether_hdr, *original_ether_hdr, original_pre_ip_hdr_size);
  // the L3 protocol type is held in the VLAN header if the packet is
  // tagged.  overwriting ether_type would break the VLAN tag, and the
  // tx side (_send_link) would no longer recognize the packet as tagged.
  if (new_ether_hdr->ether_type == rte_cpu_to_be_16 (RTE_ETHER_TYPE_VLAN))
    {
      struct rte_vlan_hdr *new_vlan_hdr =
          (struct rte_vlan_hdr *) (new_ether_hdr + 1);
      new_vlan_hdr->eth_proto = rte_cpu_to_be_16 (RTE_ETHER_TYPE_IPV6);
    }
  else
    new_ether_hdr->ether_type = rte_cpu_to_be_16 (RTE_ETHER_TYPE_IPV6);
  struct rte_ipv6_hdr *new_ipv6_hdr =
      (struct rte_ipv6_hdr *) ((uint8_t *) new_ether_hdr +
                               original_pre_ip_hdr_size);
  // version 6, traffic class and flow label are set to 0
  new_ipv6_hdr->vtc_flow = rte_cpu_to_be_32 (6 << 28);
  new_ipv6_hdr->payload_len =
      rte_cpu_to_be_16 (rte_pktmbuf_pkt_len (m) - original_pre_ip_hdr_size -
                        sizeof (struct rte_ipv6_hdr));
  new_ipv6_hdr->proto = IPPROTO_ROUTING;
  new_ipv6_hdr->hop_limits = 64; // TODO: configurable hop limit
  rte_memcpy (&new_ipv6_hdr->src_addr, src_addr, 16);
  struct rte_ipv6_routing_ext *new_srh =
      (struct rte_ipv6_routing_ext *) (new_ipv6_hdr + 1);
  new_srh->type = RTE_IPV6_SRCRT_TYPE_4;
  new_srh->hdr_len = srv6_encap_sid->segment_list_size * 2;
  new_srh->segments_left = srv6_encap_sid->segment_list_size - 1;
  new_srh->last_entry = srv6_encap_sid->segment_list_size - 1;
  new_srh->next_hdr = ((*original_ipv4_hdr) ? 4 : 41); // IPv4: 4, IPv6: 41
  new_srh->tag = 0;                                  // TODO: configurable tag
  new_srh->flag = 0;                                 // TODO: configurable flag
  rte_memcpy (&new_ipv6_hdr->dst_addr,
              &srv6_encap_sid->sids[srv6_encap_sid->segment_list_size - 1],
              16);
  rte_memcpy (new_srh + 1, srv6_encap_sid->sids,
              srv6_encap_sid->segment_list_size * 16);
  *original_ether_hdr = new_ether_hdr;
  *original_ipv6_hdr = new_ipv6_hdr;
  *original_ipv4_hdr = NULL; // Clear the IPv4 header pointer
  if (IS_DEBUG (ROUTER))
    {
      DEBUG_NEW (ROUTER,
                 "m: %p: encapsulated with new outer IPv6 and "
                 "SRH encaped packet len %d",
                 m, rte_pktmbuf_pkt_len (m));
      log_packet (m, rx_portid, rx_queueid);
    }

  return true;
}

static inline __attribute__ ((always_inline)) bool
_process_srv6_end_dx_packet (struct rte_mbuf *m, struct rte_ether_hdr **eth,
                             struct rte_ipv4_hdr **ipv4,
                             struct rte_ipv6_hdr **ipv6, uint8_t inner_proto)
{
  /* Bound every access by the contiguous data of the first segment, derived
     from the ipv6 pointer so that whatever L2 header sits in front (untagged
     14, VLAN 18, ...) needs no arithmetic here. */
  const uint8_t *pkt_end =
      rte_pktmbuf_mtod (m, const uint8_t *) + rte_pktmbuf_data_len (m);

  /* Walk the extension header chain to the encapsulated packet rather than
     assuming the SRH is the only thing in the way: RFC 8986 4.4 S02 says
     "remove the outer IPv6 header with all its extension headers", and Linux
     does the same with ipv6_find_hdr() + pskb_pull(off).  Stopping at the SRH
     would drop a packet that also carries Hop-by-Hop or Destination
     Options. */
  // TODO: use this chain read for End and End.X
  int next = (*ipv6)->proto;
  const uint8_t *p = (const uint8_t *) (*ipv6 + 1);

  while (next != inner_proto)
    {
      size_t ext_len;
      int upper;

      /* rte_ipv6_get_next_ext() reads the first two bytes of the header it is
         given (next_hdr and the length), so they have to be there first. */
      if (unlikely (p + 2 > pkt_end))
        {
          DEBUG_NEW (ROUTER, "m: %p: ext header %d does not fit in the buffer",
                     m, next);
          return false;
        }

      /* A fragment is not something this can decapsulate: there is no
         reassembly here, and walking past the Fragment header would treat a
         fragment's payload as a whole inner packet.  For a non-first fragment
         that payload is arbitrary mid-packet bytes that merely sit where a
         header is expected -- the checks downstream would reject most of
         them, which is not the same as rejecting them.  Linux refuses the
         same case in ipv6_find_hdr() (-ENOENT once the fragment offset is
         non-zero); a first fragment is no safer here, since what follows it
         is a truncated inner packet, so both are dropped. */
      if (unlikely (next == IPPROTO_FRAGMENT))
        {
          DEBUG_NEW (ROUTER,
                     "m: %p: fragmented packet, not reassembling, drop.", m);
          return false;
        }

      /* An SRH is allowed but not required.  If there is one, this SID must be
         the last segment: RFC 8986 4.4 S02 makes segments_left != 0 an error
         (it asks for an ICMP Parameter Problem, which is not implemented, so
         the packet is dropped instead). */
      if (next == IPPROTO_ROUTING)
        {
          const struct rte_ipv6_routing_ext *srh =
              (const struct rte_ipv6_routing_ext *) p;

          if (unlikely ((const uint8_t *) (srh + 1) > pkt_end))
            {
              DEBUG_NEW (ROUTER, "m: %p: SRH does not fit in the buffer", m);
              return false;
            }
          if (srh->type != RTE_IPV6_SRCRT_TYPE_4)
            {
              DEBUG_NEW (ROUTER, "m: %p: SRH->type != SRCRT_TYPE_4", m);
              return false;
            }
          if (srh->segments_left != 0)
            {
              DEBUG_NEW (ROUTER,
                         "m: %p: End.DX with segments_left %u, "
                         "this SID is not the last segment",
                         m, srh->segments_left);
              return false;
            }
        }

      upper = rte_ipv6_get_next_ext (p, next, &ext_len);
      if (unlikely (upper < 0))
        {
          /* not an extension header, so this IS the upper-layer header and it
             is not the one this SID decapsulates.  RFC 8986 4.4/4.5 S05 sends
             that to 4.1.1 (local delivery or ICMP), which is not implemented
             -- drop rather than guess. */
          DEBUG_NEW (ROUTER, "m: %p: upper-layer header is %d, not %d", m,
                     next, inner_proto);
          return false;
        }
      /* ext_len is at least 8 for every header rte_ipv6_get_next_ext()
         accepts, so p advances and the walk terminates; check it anyway so
         that stays true here rather than in another library's internals. */
      if (unlikely (ext_len == 0 || p + ext_len > pkt_end))
        {
          DEBUG_NEW (ROUTER, "m: %p: ext header %d length %zu is out of range",
                     m, next, ext_len);
          return false;
        }
      p += ext_len;
      next = upper;
    }

  bool inner_is_v6 = (inner_proto == IPPROTO_IPV6);
  size_t inner_hdr_size = inner_is_v6 ? sizeof (struct rte_ipv6_hdr)
                                      : sizeof (struct rte_ipv4_hdr);
  if (unlikely (p + inner_hdr_size > pkt_end))
    {
      DEBUG_NEW (ROUTER, "m: %p: inner header does not fit in the buffer", m);
      return false;
    }

  /* Slide the L2 headers forward over the outer L3 and give the room back at
     the front.  The mirror of _process_srv6_encap()'s prepend + memmove.
     Both offsets have to be taken before ipv6 is repointed below. */
  size_t pre_ip_hdr_size = (const uint8_t *) *ipv6 - (const uint8_t *) *eth;
  size_t outer_len = p - (const uint8_t *) *ipv6;

  memmove ((uint8_t *) *eth + outer_len, *eth, pre_ip_hdr_size);
  if (unlikely (! rte_pktmbuf_adj (m, outer_len)))
    {
      DEBUG_NEW (ROUTER, "m: %p: failed to adjust %zu bytes for decapsulate", m,
                 outer_len);
      return false;
    }

  *eth = (struct rte_ether_hdr *) ((uint8_t *) *eth + outer_len);

  /* The L2 header now has to announce the family of the packet it precedes.
     For End.DX6 that is what it already said; for End.DX4 it is not, and
     leaving it would make the tx side offer an IPv4 packet as IPv6.  The type
     lives in the VLAN header when there is one -- overwriting ether_type there
     would destroy the tag, which is the mistake _process_srv6_encap() carries
     the same guard against. */
  uint16_t eth_type = inner_is_v6 ? RTE_ETHER_TYPE_IPV6 : RTE_ETHER_TYPE_IPV4;
  if ((*eth)->ether_type == rte_cpu_to_be_16 (RTE_ETHER_TYPE_VLAN))
    {
      struct rte_vlan_hdr *vlan = (struct rte_vlan_hdr *) (*eth + 1);
      vlan->eth_proto = rte_cpu_to_be_16 (eth_type);
    }
  else
    (*eth)->ether_type = rte_cpu_to_be_16 (eth_type);

  if (inner_is_v6)
    {
      *ipv6 = (struct rte_ipv6_hdr *) ((uint8_t *) *eth + pre_ip_hdr_size);
      *ipv4 = NULL;
    }
  else
    {
      *ipv4 = (struct rte_ipv4_hdr *) ((uint8_t *) *eth + pre_ip_hdr_size);
      *ipv6 = NULL;
    }

  /* The RX checksum results describe the OUTER header, which is the only one
     the NIC ever looked at.  They say nothing about what has just been
     exposed, and carrying RTE_MBUF_F_RX_IP_CKSUM_GOOD onto an inner header
     would let a later check skip a checksum nobody computed.  Linux clears the
     same class of state when it pulls a tunnel header
     (iptunnel_pull_offloads(), net/ipv4/ip_tunnel_core.c). */
  m->ol_flags &= ~(RTE_MBUF_F_RX_IP_CKSUM_MASK | RTE_MBUF_F_RX_L4_CKSUM_MASK);

  DEBUG_NEW (ROUTER,
             "m: %p: End.DX%d removed %zu bytes of outer header, exposed %s, "
             "inner pkt len %d",
             m,inner_is_v6 ? 6:4 , outer_len, (inner_is_v6 ? "IPv6" : "IPv4"),
             rte_pktmbuf_pkt_len (m));
  return true;
}

#endif /*__SRV6_H__*/
