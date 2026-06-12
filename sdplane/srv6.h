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

static inline __attribute__ ((always_inline)) bool
_process_srv6_end_packet (struct rte_mbuf *m,
                      struct rte_ipv6_hdr *ipv6,
                      struct srv6_local_sid_entry *srv6_local_sid)
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

  srh->segments_left --;

  struct in6_addr* sid_list = (struct in6_addr *)(srh + 1);
  rte_memcpy (dst_addr, &sid_list[srh->segments_left], 16);

  DEBUG_NEW (ROUTER, "m: %p: reload dst_addr from sid_list[%d]",
             m, srh->segments_left);
  modified = true;
  return modified;
}

static inline __attribute__ ((always_inline)) bool
_process_encapsulate_srv6_packet (struct rte_mbuf *m,
                      struct rte_ether_hdr** original_ether_hdr,
                      struct rte_ipv4_hdr** original_ipv4_hdr,
                      struct rte_ipv6_hdr **original_ipv6_hdr,
                      struct srv6_encap_sid_entry *srv6_encap_sid,
                      struct in6_addr* src_addr,int rx_portid,int rx_queueid)
{
  /* encapsulate with SRv6 */
  uint32_t outer_header_size = sizeof (struct rte_ipv6_hdr) +
                              sizeof (struct rte_ipv6_routing_ext) +
                              srv6_encap_sid->segment_list_size *
                              sizeof(struct in6_addr);

  DEBUG_NEW (ROUTER, "m: %p: encapsulating with SRv6, original pkt len"
             " %d, outer header size %d, segment list size %d,"
             " original ipv6 hdr packet len %d",
             m, rte_pktmbuf_pkt_len (m), outer_header_size,
             srv6_encap_sid->segment_list_size,
       (*original_ipv4_hdr) ? (*original_ipv4_hdr)->total_length :
       (*original_ipv6_hdr) ? (*original_ipv6_hdr)->payload_len : 0);
  // because the original may have some headers between ether_hdr and ipv6_hdr,
  // we need to prepend the same size of header space for new_ether_hdr,
  // and copy the header content before ipv6_hdr to the new_ether_hdr.
  // TODO: support ipv4
  size_t original_pre_ip_hdr_size;
  if (*original_ipv4_hdr)
    {
      original_pre_ip_hdr_size = (uint8_t *) *original_ipv4_hdr -
                                  (uint8_t *) *original_ether_hdr;
    }
  else if (*original_ipv6_hdr)
    {
      original_pre_ip_hdr_size =
      (uint8_t *) *original_ipv6_hdr -
                                  (uint8_t *) *original_ether_hdr;
    }
  else
    {
      DEBUG_NEW (ROUTER,
                 "m: %p: no ipv4 or ipv6 header found for encapsulate", m);
      return false;
    }
  struct rte_ether_hdr* new_ether_hdr =
    rte_pktmbuf_prepend (m, outer_header_size);
  if (!new_ether_hdr)
    {
      DEBUG_NEW (ROUTER, "m: %p: failed to prepend for encapsulate", m);
      return false;
    }
  memmove (new_ether_hdr, *original_ether_hdr, original_pre_ip_hdr_size);
  struct rte_ipv6_hdr* new_ipv6_hdr =
          (struct rte_ipv6_hdr *) ((uint8_t *) new_ether_hdr +
                                   original_pre_ip_hdr_size);
  // version 6, traffic class and flow label are set to 0
  new_ipv6_hdr->vtc_flow = rte_cpu_to_be_32(6 << 28);
  new_ipv6_hdr->payload_len = rte_cpu_to_be_16(rte_pktmbuf_pkt_len(m) -
                            original_pre_ip_hdr_size -
                            sizeof(struct rte_ipv6_hdr));
  new_ipv6_hdr->proto = IPPROTO_ROUTING;
  new_ipv6_hdr->hop_limits = 64; // TODO: configurable hop limit
  rte_memcpy (&new_ipv6_hdr->src_addr, src_addr, 16);
  struct rte_ipv6_routing_ext* new_srh =
    (struct rte_ipv6_routing_ext *) (new_ipv6_hdr + 1);
  new_srh->type = RTE_IPV6_SRCRT_TYPE_4;
  new_srh->hdr_len = srv6_encap_sid->segment_list_size * 2;
  new_srh->segments_left = srv6_encap_sid->segment_list_size - 1;
  new_srh->last_entry = srv6_encap_sid->segment_list_size - 1;
  new_srh->next_hdr = IPPROTO_IPV6;
  new_srh->tag = 0; // TODO: configurable tag
  new_srh->flag = 0; // TODO: configurable flag
  rte_memcpy (&new_ipv6_hdr->dst_addr,
          &srv6_encap_sid->sids[srv6_encap_sid->segment_list_size - 1], 16);
  rte_memcpy (new_srh + 1, srv6_encap_sid->sids,
                srv6_encap_sid->segment_list_size * 16);
  *original_ether_hdr = new_ether_hdr;
  *original_ipv6_hdr = new_ipv6_hdr;
  *original_ipv4_hdr = NULL; // Clear the IPv4 header pointer
  if (IS_DEBUG (ROUTER))
    {
      DEBUG_NEW (ROUTER,
        "m: %p: encapsulated with new outer IPv6 and SRH encaped packet len %d",
         m, rte_pktmbuf_pkt_len(m));
      log_packet (m, rx_portid, rx_queueid);
    }

  return true;
}

extern bool enable_srv6_encap_mode;

#endif /*__SRV6_H__*/
