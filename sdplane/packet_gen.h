#ifndef __PACKET_GEN_H__
#define __PACKET_GEN_H__

#include <rte_ethdev.h>

#include "l2fwd_export.h"
#include "debug_sdplane.h"
#include "rte_override.h"

static inline __attribute__ ((always_inline)) struct rte_mbuf *
arp_req_pkt_gen (struct rte_ether_addr *src_mac, struct in_addr *src_ip,
                 struct in_addr *target_ip)
{
  struct rte_mempool *mp = l2fwd_pktmbuf_pool;
  struct rte_mbuf *m;
  struct rte_ether_hdr *eth;
  struct rte_arp_hdr *arp;
  size_t pkt_size;

  m = rte_pktmbuf_alloc (mp);
  if (! m)
    {
      DEBUG_NEW (ROUTER, "failed to allocate mbuf for ARP request");
      return NULL;
    }

  pkt_size = sizeof (struct rte_ether_hdr) + sizeof (struct rte_arp_hdr);
  if (rte_pktmbuf_append (m, pkt_size) == NULL)
    {
      DEBUG_NEW (ROUTER, "failed to append data to mbuf");
      rte_pktmbuf_free (m);
      return NULL;
    }

  /* setup eth_hdr */
  eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  memset (&eth->dst_addr, 0xff, RTE_ETHER_ADDR_LEN); // broadcast
  rte_ether_addr_copy (src_mac, &eth->src_addr);
  eth->ether_type = rte_cpu_to_be_16 (RTE_ETHER_TYPE_ARP);

  /* setup arp_hdr */
  arp = (struct rte_arp_hdr *) (eth + 1);
  arp->arp_hardware = rte_cpu_to_be_16 (RTE_ARP_HRD_ETHER);
  arp->arp_protocol = rte_cpu_to_be_16 (RTE_ETHER_TYPE_IPV4);
  arp->arp_hlen = RTE_ETHER_ADDR_LEN;
  arp->arp_plen = sizeof (struct in_addr);
  arp->arp_opcode = rte_cpu_to_be_16 (RTE_ARP_OP_REQUEST);

  rte_ether_addr_copy (src_mac, &arp->arp_data.arp_sha);
  memcpy (&arp->arp_data.arp_sip, src_ip, sizeof (struct in_addr));
  memset (&arp->arp_data.arp_tha, 0, RTE_ETHER_ADDR_LEN);
  memcpy (&arp->arp_data.arp_tip, target_ip, sizeof (struct in_addr));

  return m;
}

#define ICMPV6_NS 135
#define ICMPV6_NA 136

static inline __attribute__ ((always_inline)) struct rte_mbuf *
ns_pkt_gen (struct rte_ether_addr *src_mac, struct in6_addr *src_ip,
            struct in6_addr *target_ip)
{
  struct rte_mempool *mp = l2fwd_pktmbuf_pool;
  struct rte_mbuf *m;
  struct rte_ether_hdr *eth;
  struct rte_ipv6_hdr *ipv6;
  uint8_t *icmpv6_data;
  uint16_t icmpv6_len;
  size_t pkt_size;

  /*
   * ICMPv6 NS message structure:
   * - ICMPv6 header (4 bytes: type, code, checksum)
   * - Reserved (4 bytes)
   * - Target Address (16 bytes)
   * - Options: Source Link-Layer Address (8 bytes: type, length, MAC)
   */
  icmpv6_len = 4 + 4 + 16 + 8; /* 32 bytes */
  pkt_size = sizeof (struct rte_ether_hdr) + sizeof (struct rte_ipv6_hdr) +
             icmpv6_len;

  m = rte_pktmbuf_alloc (mp);
  if (! m)
    {
      DEBUG_NEW (ROUTER, "failed to allocate mbuf for NS");
      return NULL;
    }

  if (rte_pktmbuf_append (m, pkt_size) == NULL)
    {
      DEBUG_NEW (ROUTER, "failed to append data to mbuf");
      rte_pktmbuf_free (m);
      return NULL;
    }

  /*
   * solicited-node multicast address: ff02::1:ffxx:xxxx
   * (lower 24 bits of target_ip)
   */
  uint8_t solicited_node_addr[16];
  memset (solicited_node_addr, 0, 16);
  solicited_node_addr[0] = 0xff;
  solicited_node_addr[1] = 0x02;
  solicited_node_addr[11] = 0x01;
  solicited_node_addr[12] = 0xff;
  solicited_node_addr[13] = ((uint8_t *) target_ip)[13];
  solicited_node_addr[14] = ((uint8_t *) target_ip)[14];
  solicited_node_addr[15] = ((uint8_t *) target_ip)[15];

  /* setup eth_hdr */
  eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  /* IPv6 multicast MAC: 33:33:xx:xx:xx:xx (lower 32 bits of IPv6 address) */
  eth->dst_addr.addr_bytes[0] = 0x33;
  eth->dst_addr.addr_bytes[1] = 0x33;
  eth->dst_addr.addr_bytes[2] = solicited_node_addr[12];
  eth->dst_addr.addr_bytes[3] = solicited_node_addr[13];
  eth->dst_addr.addr_bytes[4] = solicited_node_addr[14];
  eth->dst_addr.addr_bytes[5] = solicited_node_addr[15];
  rte_ether_addr_copy (src_mac, &eth->src_addr);
  eth->ether_type = rte_cpu_to_be_16 (RTE_ETHER_TYPE_IPV6);

  /* setup ipv6_hdr */
  ipv6 = (struct rte_ipv6_hdr *) (eth + 1);
  ipv6->vtc_flow = rte_cpu_to_be_32 (0x60000000);
  ipv6->payload_len = rte_cpu_to_be_16 (icmpv6_len);
  ipv6->proto = IPPROTO_ICMPV6;
  ipv6->hop_limits = 255;
  memcpy (IPV6_ADDR_BYTES (ipv6->src_addr), src_ip, sizeof (struct in6_addr));
  memcpy (IPV6_ADDR_BYTES (ipv6->dst_addr), solicited_node_addr,
          sizeof (struct in6_addr));

  /* setup icmp_hdr*/
  icmpv6_data = (uint8_t *) (ipv6 + 1);

  /* ICMPv6 header */
  icmpv6_data[0] = ICMPV6_NS; /* type */
  icmpv6_data[1] = 0;         /* code */
  icmpv6_data[2] = 0;         /* checksum */
  icmpv6_data[3] = 0;

  /* Reserved (4 bytes) */
  memset (&icmpv6_data[4], 0, 4);

  /* Target Address (16 bytes) */
  memcpy (&icmpv6_data[8], target_ip, sizeof (struct in6_addr));

  /* Option: Source Link-Layer Address (8 bytes) */
  icmpv6_data[24] = 1; /* type: Source LLA */
  icmpv6_data[25] = 1; /* length: 1 (in units of 8 bytes) */
  memcpy (&icmpv6_data[26], src_mac, RTE_ETHER_ADDR_LEN);

  /* calculate ICMPv6 checksum using IPv6 pseudo-header */
  struct rte_icmp_hdr *icmp = (struct rte_icmp_hdr *) icmpv6_data;
  icmp->icmp_cksum = 0;
  uint16_t cksum = rte_ipv6_udptcp_cksum (ipv6, icmpv6_data);
  icmp->icmp_cksum = cksum;

  return m;
}

#endif /*__PACKET_GEN_H__*/
