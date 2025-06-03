#ifndef __LOG_PACKET_H__
#define __LOG_PACKET_H__

static inline __attribute__ ((always_inline)) void
__log_packet (char *file, int line, const char *func, struct rte_mbuf *m,
              uint16_t rx_portid, uint16_t rx_queueid)
{
  char ether_str[512];
  char ip_str[512];
  char transport_str[512];
  char payload_str[512];

  struct rte_ether_hdr *eth;
  char eth_dst[32];
  char eth_src[32];
  unsigned short eth_type;

  struct rte_ipv4_hdr *ipv4;
  struct rte_ipv6_hdr *ipv6;
  char ip_src[64];
  char ip_dst[64];

  memset (ether_str, 0, sizeof (ether_str));
  memset (ip_str, 0, sizeof (ip_str));
  memset (transport_str, 0, sizeof (transport_str));
  memset (payload_str, 0, sizeof (payload_str));

  /* ether part */
  eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  rte_ether_format_addr (eth_dst, sizeof (eth_dst), &eth->dst_addr);
  rte_ether_format_addr (eth_src, sizeof (eth_src), &eth->src_addr);
  eth_type = rte_be_to_cpu_16 (eth->ether_type);
  snprintf (ether_str, sizeof (ether_str), "ether: type: 0x%04hx %s -> %s",
            eth_type, eth_src, eth_dst);

  if (RTE_ETH_IS_IPV4_HDR (m->packet_type))
    {
      ipv4 = (struct rte_ipv4_hdr *) (eth + 1);
      inet_ntop (AF_INET, &ipv4->src_addr, ip_src, sizeof (ip_src));
      inet_ntop (AF_INET, &ipv4->dst_addr, ip_dst, sizeof (ip_dst));
      snprintf (ip_str, sizeof (ip_str),
                "ipv4: ver_ihl: %#x tos: %d length: %d id: %d off: %d "
                "ttl: %d proto: %d cksum: %#x %s -> %s",
                ipv4->version_ihl, ipv4->type_of_service,
                rte_be_to_cpu_16 (ipv4->total_length),
                rte_be_to_cpu_16 (ipv4->packet_id),
                rte_be_to_cpu_16 (ipv4->fragment_offset), ipv4->time_to_live,
                ipv4->next_proto_id, rte_be_to_cpu_16 (ipv4->hdr_checksum),
                ip_src, ip_dst);
    }
  else if (RTE_ETH_IS_IPV6_HDR (m->packet_type))
    {
      ipv6 = (struct rte_ipv6_hdr *) (eth + 1);
      inet_ntop (AF_INET6, &ipv6->src_addr, ip_src, sizeof (ip_src));
      inet_ntop (AF_INET6, &ipv6->dst_addr, ip_dst, sizeof (ip_dst));
      snprintf (ip_str, sizeof (ip_str),
                "ipv6: vtc_flow: %#x length: %d "
                "proto: %d hop_limits: %d %s -> %s",
                rte_be_to_cpu_32 (ipv6->vtc_flow),
                rte_be_to_cpu_16 (ipv6->payload_len), ipv6->proto,
                ipv6->hop_limits, ip_src, ip_dst);
    }

  if (FLAG_CHECK (DEBUG_CONFIG (SDPLANE), DEBUG_TYPE (SDPLANE, PACKET)))
    debug_log ("%s[%d] %s(): m: %p rx_port: %d rx_queue: %d "
               "%s %s %s %s",
               file, line, func, m, rx_portid, rx_queueid, ether_str, ip_str,
               transport_str, payload_str);
}

#define log_packet(m, port, queue)                                            \
  __log_packet (__FILE__, __LINE__, __func__, m, port, queue)

#endif /*__LOG_PACKET_H__*/
