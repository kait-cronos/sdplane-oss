#ifndef __SRV6_H__
#define __SRV6_H__

#include <rte_version.h>
#if RTE_VERSION < RTE_VERSION_NUM(24, 0, 0, 0)
  #include <rte_ip.h>
#else
  #include <rte_ip6.h>
#endif
#include "rte_override.h"
#include "debug_sdplane.h"

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
                      struct in6_addr *local_end_sid,
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
      if (! memcmp (dst_addr, &local_end_sid[i], 16))
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

#endif /*__SRV6_H__*/
