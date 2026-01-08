#ifndef __SRV6_H__
#define __SRV6_H__

static inline __attribute__ ((always_inline)) void
_process_srv6_packet (struct rte_mbuf *m,
                      struct rte_ipv6_hdr *ipv6)
{
  if (ipv6->proto != IPPROTO_ROUTING)
    return;

  uint32_t pkt_len = rte_pktmbuf_pkt_len(m);
  if (unlikely(pkt_len < sizeof(struct rte_ipv6_hdr) + sizeof(struct rte_ipv6_routing_ext)))
    return;

  struct rte_ipv6_routing_ext *srh = (struct rte_ipv6_routing_ext *)(ipv6 +1);
  if (srh->type != RTE_IPV6_SRCRT_TYPE_4)
    return;

  int max_last_entry = (srh->hdr_len / 2) - 1;
  if ( srh->last_entry > max_last_entry
    || (srh->segments_left > srh->last_entry + 1))
    return;

  if (srh->segments_left == 0)
    return;

  srh->segments_left --;

  struct in6_addr* sid_list = (struct in6_addr*)(srh+1);
  rte_memcpy(&ipv6->dst_addr, &sid_list[srh->segments_left], 16);
}

#endif /*__SRV6_H__*/
