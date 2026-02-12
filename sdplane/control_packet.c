#include "include.h"

#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_mbuf.h>
#include <rte_byteorder.h>
#include <rte_udp.h>
#include <rte_tcp.h>
#include <rte_mbuf.h>

#include <sdplane/debug_log.h>

#include "control_packet.h"
#include "debug_sdplane.h"


static bool
is_isis (void *payload, uint16_t eth_type)
{
  return eth_type == 0xFEFE;
}

static bool
is_arp (void *payload, uint16_t eth_type)
{
  return eth_type == RTE_ETHER_TYPE_ARP;
}

static bool
is_icmp (void *payload, uint16_t eth_type)
{
  if (eth_type == RTE_ETHER_TYPE_IPV4)
    {
      struct rte_ipv4_hdr *ipv4_hdr = (struct rte_ipv4_hdr *)payload;
      return ipv4_hdr->next_proto_id == IPPROTO_ICMP;
    }
  if (eth_type == RTE_ETHER_TYPE_IPV6)
    {
      struct rte_ipv6_hdr *ipv6_hdr = (struct rte_ipv6_hdr *)payload;
      return ipv6_hdr->proto == IPPROTO_ICMPV6;
    }
  return false;
}

static bool
is_pim (void *payload, uint16_t eth_type)
{
  if (eth_type == RTE_ETHER_TYPE_IPV4)
    {
      struct rte_ipv4_hdr *ipv4_hdr = (struct rte_ipv4_hdr *)payload;
      return ipv4_hdr->next_proto_id == IPPROTO_PIM;
    }
  if (eth_type == RTE_ETHER_TYPE_IPV6)
    {
      struct rte_ipv6_hdr *ipv6_hdr = (struct rte_ipv6_hdr *)payload;
      return ipv6_hdr->proto == IPPROTO_PIM;
    }
  return false;
}

static bool
is_ospf (void *payload, uint16_t eth_type)
{
  if (eth_type == RTE_ETHER_TYPE_IPV4)
    {
      struct rte_ipv4_hdr *ipv4_hdr = (struct rte_ipv4_hdr *)payload;
      return ipv4_hdr->next_proto_id == IPPROTO_OSPF;
    }
  if (eth_type == RTE_ETHER_TYPE_IPV6)
    {
      struct rte_ipv6_hdr *ipv6_hdr = (struct rte_ipv6_hdr *)payload;
      return ipv6_hdr->proto == IPPROTO_OSPF;
    }
  return false;
}

static bool
is_bgp (void *payload, uint16_t eth_type)
{
  if (eth_type == RTE_ETHER_TYPE_IPV4)
    {
      struct rte_ipv4_hdr *ipv4_hdr = (struct rte_ipv4_hdr *) (payload + 1);
      if (ipv4_hdr->next_proto_id == IPPROTO_TCP)
        {
          char *l4_hdr = (char *) ipv4_hdr + sizeof (struct rte_ipv4_hdr);
          struct rte_tcp_hdr *tcp_hdr = (struct rte_tcp_hdr *) l4_hdr;
          uint16_t src_port = rte_be_to_cpu_16 (tcp_hdr->src_port);
          uint16_t dst_port = rte_be_to_cpu_16 (tcp_hdr->dst_port);
          return src_port == BGP_PORT || dst_port == BGP_PORT;
        }
    }
  if (eth_type == RTE_ETHER_TYPE_IPV6)
    {
      struct rte_ipv6_hdr *ipv6_hdr = (struct rte_ipv6_hdr *)payload;
      if (ipv6_hdr->proto == IPPROTO_TCP)
        {
          char *l4_hdr = (char *) ipv6_hdr + sizeof (struct rte_ipv6_hdr);
          struct rte_tcp_hdr *tcp_hdr = (struct rte_tcp_hdr *) l4_hdr;
          uint16_t src_port = rte_be_to_cpu_16 (tcp_hdr->src_port);
          uint16_t dst_port = rte_be_to_cpu_16 (tcp_hdr->dst_port);
          return src_port == BGP_PORT || dst_port == BGP_PORT;
        }
    }
  return false;
}

static bool
is_ldp (void *payload, uint16_t eth_type)
{
  if (eth_type == RTE_ETHER_TYPE_IPV4)
    {
      struct rte_ipv4_hdr *ipv4_hdr = (struct rte_ipv4_hdr *)payload;
      char *l4_hdr = (char *) ipv4_hdr + sizeof (struct rte_ipv4_hdr);
      if (ipv4_hdr->next_proto_id == IPPROTO_TCP)
        {
          struct rte_tcp_hdr *tcp_hdr = (struct rte_tcp_hdr *) l4_hdr;
          uint16_t src_port = rte_be_to_cpu_16 (tcp_hdr->src_port);
          uint16_t dst_port = rte_be_to_cpu_16 (tcp_hdr->dst_port);
          if (src_port == 646 || dst_port == 646)
            return true;
        }
      if (ipv4_hdr->next_proto_id == IPPROTO_UDP)
        {
          struct rte_udp_hdr *udp_hdr = (struct rte_udp_hdr *) l4_hdr;
          uint16_t src_port = rte_be_to_cpu_16 (udp_hdr->src_port);
          uint16_t dst_port = rte_be_to_cpu_16 (udp_hdr->dst_port);
          if (src_port == 646 || dst_port == 646)
            return true;
        }
    }
  if (eth_type == RTE_ETHER_TYPE_IPV6)
    {
      struct rte_ipv6_hdr *ipv6_hdr = (struct rte_ipv6_hdr *)payload;
      char *l4_hdr = (char *) ipv6_hdr + sizeof (struct rte_ipv6_hdr);
      if (ipv6_hdr->proto == IPPROTO_TCP)
        {
          struct rte_tcp_hdr *tcp_hdr = (struct rte_tcp_hdr *) l4_hdr;
          uint16_t src_port = rte_be_to_cpu_16 (tcp_hdr->src_port);
          uint16_t dst_port = rte_be_to_cpu_16 (tcp_hdr->dst_port);
          if (src_port == 646 || dst_port == 646)
            return true;
        }
      if (ipv6_hdr->proto == IPPROTO_UDP)
        {
          struct rte_udp_hdr *udp_hdr = (struct rte_udp_hdr *) l4_hdr;
          uint16_t src_port = rte_be_to_cpu_16 (udp_hdr->src_port);
          uint16_t dst_port = rte_be_to_cpu_16 (udp_hdr->dst_port);
          if (src_port == 646 || dst_port == 646)
            return true;
        }
    }
  return false;
}

static bool
is_rip (void *payload, uint16_t eth_type)
{
  if (eth_type == RTE_ETHER_TYPE_IPV4)
    {
      struct rte_ipv4_hdr *ipv4_hdr = (struct rte_ipv4_hdr *)payload;
      if (ipv4_hdr->next_proto_id == IPPROTO_UDP)
        {
          char *l4_hdr = (char *) ipv4_hdr + sizeof (struct rte_ipv4_hdr);
          struct rte_udp_hdr *udp_hdr = (struct rte_udp_hdr *) l4_hdr;
          uint16_t src_port = rte_be_to_cpu_16 (udp_hdr->src_port);
          uint16_t dst_port = rte_be_to_cpu_16 (udp_hdr->dst_port);
          return src_port == RIP_PORT || dst_port == RIP_PORT;
        }
    }
  if (eth_type == RTE_ETHER_TYPE_IPV6)
    {
      struct rte_ipv6_hdr *ipv6_hdr = (struct rte_ipv6_hdr *)payload;
      if (ipv6_hdr->proto == IPPROTO_UDP)
        {
          char *l4_hdr = (char *) ipv6_hdr + sizeof (struct rte_ipv6_hdr);
          struct rte_udp_hdr *udp_hdr = (struct rte_udp_hdr *) l4_hdr;
          uint16_t src_port = rte_be_to_cpu_16 (udp_hdr->src_port);
          uint16_t dst_port = rte_be_to_cpu_16 (udp_hdr->dst_port);
          return src_port == RIPNG_PORT || dst_port == RIPNG_PORT ||
                 src_port == RIP_PORT || dst_port == RIP_PORT;
        }
    }
  return false;
}

static bool
is_babel (void *payload, uint16_t eth_type)
{
  if (eth_type == RTE_ETHER_TYPE_IPV4)
    {
      struct rte_ipv4_hdr *ipv4_hdr = (struct rte_ipv4_hdr *)payload;
      if (ipv4_hdr->next_proto_id == IPPROTO_UDP)
        {
          char *l4_hdr = (char *) ipv4_hdr + sizeof (struct rte_ipv4_hdr);
          struct rte_udp_hdr *udp_hdr = (struct rte_udp_hdr *) l4_hdr;
          uint16_t src_port = rte_be_to_cpu_16 (udp_hdr->src_port);
          uint16_t dst_port = rte_be_to_cpu_16 (udp_hdr->dst_port);
          return src_port == 6696 || dst_port == 6696;
        }
    }
  if (eth_type == RTE_ETHER_TYPE_IPV4)
    {
      struct rte_ipv4_hdr *ipv4_hdr = (struct rte_ipv4_hdr *)payload;
      if (ipv4_hdr->next_proto_id == IPPROTO_UDP)
        {
          char *l4_hdr = (char *) ipv4_hdr + sizeof (struct rte_ipv4_hdr);
          struct rte_udp_hdr *udp_hdr = (struct rte_udp_hdr *) l4_hdr;
          uint16_t src_port = rte_be_to_cpu_16 (udp_hdr->src_port);
          uint16_t dst_port = rte_be_to_cpu_16 (udp_hdr->dst_port);
          return src_port == BABEL_PORT || dst_port == BABEL_PORT;
        }
    }
  if (eth_type == RTE_ETHER_TYPE_IPV6)
    {
      struct rte_ipv6_hdr *ipv6_hdr = (struct rte_ipv6_hdr *)payload;
      if (ipv6_hdr->proto == IPPROTO_UDP)
        {
          char *l4_hdr = (char *) ipv6_hdr + sizeof (struct rte_ipv6_hdr);
          struct rte_udp_hdr *udp_hdr = (struct rte_udp_hdr *) l4_hdr;
          uint16_t src_port = rte_be_to_cpu_16 (udp_hdr->src_port);
          uint16_t dst_port = rte_be_to_cpu_16 (udp_hdr->dst_port);
          return src_port == BABEL_PORT || dst_port == BABEL_PORT;
        }
    }
  return false;
}

static bool
is_nd (void *payload, uint16_t eth_type)
{
  if (eth_type == RTE_ETHER_TYPE_IPV6)
    {
      struct rte_ipv6_hdr *ipv6_hdr = (struct rte_ipv6_hdr *)payload;
      if (ipv6_hdr->proto == 58)
        {
          char *l4_hdr = (char *) ipv6_hdr + sizeof (struct rte_ipv6_hdr);
          struct icmp6_hdr
          {
            uint8_t icmp6_type;
            uint8_t icmp6_code;
            uint16_t icmp6_cksum;
            uint32_t icmp6_data;
          };
          struct icmp6_hdr *icmp6 = (struct icmp6_hdr *) l4_hdr;
          return icmp6->icmp6_type >= 133 && icmp6->icmp6_type <= 137;
        }
    }
  return false;
}


int
is_control_packet (struct rte_mbuf *m)
{
  struct rte_ether_hdr *eth_hdr;
  uint16_t eth_type;
  uint16_t vlan_tci;
  struct rte_vlan_hdr *vlan_hdr;
  void *payload;

  eth_hdr = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  eth_type = rte_be_to_cpu_16 (eth_hdr->ether_type);
  payload = (char *) (eth_hdr + 1);

  if (eth_type == RTE_ETHER_TYPE_VLAN)
    {
      vlan_hdr = (struct rte_vlan_hdr *) (eth_hdr + 1);
      vlan_tci = rte_be_to_cpu_16 (vlan_hdr->vlan_tci);
      eth_type = rte_be_to_cpu_16 (vlan_hdr->eth_proto);
      payload = (struct rte_ether_hdr *) ((char *) eth_hdr +
                                          sizeof (struct rte_ether_hdr) +
                                          sizeof (struct rte_vlan_hdr));
    }

  typedef bool (*filter_func_t) (void *, uint16_t);
  filter_func_t filter_funcs[] = { is_isis, is_arp, is_icmp, is_ospf,  is_pim,
                                   is_bgp,  is_ldp, is_rip,  is_babel, is_nd };
  int filter_funcs_size = sizeof (filter_funcs) / sizeof (filter_func_t);
  for (int i = 0; i < filter_funcs_size; i++)
    {
      if (filter_funcs[i](payload, eth_type))
        {
          DEBUG_NEW (L2_SWITCH, "filter_funcs[%d] = true", i);
          return 1;
        }
    }
  return 0;
}
