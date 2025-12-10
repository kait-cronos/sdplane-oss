#ifndef __LOG_PACKET_H__
#define __LOG_PACKET_H__

#include "rte_override.h"

#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <rte_version.h>
#if RTE_VERSION < RTE_VERSION_NUM(24, 0, 0, 0)
  #include <rte_ip.h>
#else
  #include <rte_ip6.h>
#endif

struct llc_snap_hdr
{
  uint8_t dsap;
  uint8_t ssap;
  uint8_t control;
  uint8_t oui[3];
  uint16_t snap_ether_type;
} __attribute__ ((__packed__));

struct stp_bpdu
{
  uint16_t protocol_id;
  uint8_t version;
  uint8_t bpdu_type;
  uint8_t flags;
  uint8_t root_id[8];
  uint32_t root_path_cost;
  uint8_t bridge_id[8];
  uint16_t port_id;
  uint16_t message_age;
  uint16_t max_age;
  uint16_t hello_time;
  uint16_t forward_delay;
} __attribute__ ((__packed__));

static inline __attribute__ ((always_inline)) char *
__icmp_type_str (uint8_t type)
{
  switch (type)
    {
    case ICMP_ECHOREPLY:     return "echo-reply";    //0
    case ICMP_DEST_UNREACH:  return "dest-unreach";  //3
    case ICMP_SOURCE_QUENCH: return "source-quench"; //4
    case ICMP_REDIRECT:      return "redirect";      //5
    case ICMP_ECHO:          return "echo-request";  //8
    case ICMP_TIME_EXCEEDED: return "time-exceeded"; //11
    case ICMP_PARAMETERPROB: return "param-problem"; //12
    case ICMP_TIMESTAMP:     return "timestamp";     //13 
    case ICMP_TIMESTAMPREPLY: return "timestamp-reply"; //14 
    case ICMP_INFO_REQUEST:  return "info-request";  //15 
    case ICMP_INFO_REPLY:    return "info-reply";    //16 
    case ICMP_ADDRESS:       return "address";       //17 
    case ICMP_ADDRESSREPLY:  return "address-reply"; //18 
    default: break;
    }
  return "unknown";
}

static inline __attribute__ ((always_inline)) char *
__icmp6_type_str (uint8_t type)
{
  switch (type)
    {
    case ICMP6_DST_UNREACH:      return "unreach";        //  1
    case ICMP6_PACKET_TOO_BIG:   return "too-big";        //  2
    case ICMP6_TIME_EXCEEDED:    return "time-exceeded";  //  3
    case ICMP6_PARAM_PROB:       return "param-problem";  //  4
    case ICMP6_ECHO_REQUEST:     return "echo-request";   //128
    case ICMP6_ECHO_REPLY:       return "echo-reply";     //129
    case MLD_LISTENER_QUERY:     return "mld-query";      //130
    case MLD_LISTENER_REPORT:    return "mld-report";     //131
    case MLD_LISTENER_REDUCTION: return "mld-reduction";  //132
    case ND_ROUTER_SOLICIT:      return "router-solicit"; //133
    case ND_ROUTER_ADVERT:       return "router-advert";  //134
    case ND_NEIGHBOR_SOLICIT:    return "neigh-solicit";  //135
    case ND_NEIGHBOR_ADVERT:     return "neigh-advert";   //136
    case ND_REDIRECT:            return "redirect";       //137
    case ICMPV6_EXT_ECHO_REQUEST:return "ext-echo-request"; //160
    case ICMPV6_EXT_ECHO_REPLY:  return "ext-echo-reply"; //161
    default: break;
    }
  return "unknown";
}

inline __attribute__ ((always_inline)) void
__parse_packet (struct rte_mbuf *m, struct rte_ether_hdr **eth,
                struct rte_vlan_hdr **vlan, struct llc_snap_hdr **snap,
                struct rte_ipv4_hdr **ipv4, struct rte_ipv6_hdr **ipv6,
                struct rte_ipv6_routing_ext **srh, struct rte_icmp_hdr **icmp,
                struct rte_udp_hdr **udp, struct rte_tcp_hdr **tcp)
{
  *eth = NULL; *vlan = NULL; *snap = NULL; *ipv4 = NULL;
  *ipv6 = NULL; *srh = NULL; *icmp = NULL; *udp = NULL;
  *tcp = NULL;
  
  unsigned short eth_type;

  *eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  eth_type = rte_be_to_cpu_16 ((*eth)->ether_type);

  if (eth_type == RTE_ETHER_TYPE_VLAN)
    {
      uint16_t eth_proto;
      *vlan = (struct rte_vlan_hdr *) ((*eth) + 1);
      eth_proto = rte_be_to_cpu_16 ((*vlan)->eth_proto);
      if (eth_proto < 0x600)
        *snap = (struct llc_snap_hdr *) ((*vlan) + 1);
      else if (eth_proto == RTE_ETHER_TYPE_IPV4)
        *ipv4 = (struct rte_ipv4_hdr *) ((*vlan) + 1);
      else if (eth_proto == RTE_ETHER_TYPE_IPV6)
        *ipv6 = (struct rte_ipv6_hdr *) ((*vlan) + 1);
    }
  else if (eth_type < 0x0600)
    {
      // 802.3 / LLC フレーム（IPではない）
      *snap = (struct llc_snap_hdr *) ((*eth) + 1);
    }
  else
    {
      if (eth_type == RTE_ETHER_TYPE_IPV4)
        *ipv4 = (struct rte_ipv4_hdr *) ((*eth) + 1);
      else if (eth_type == RTE_ETHER_TYPE_IPV6)
        *ipv6 = (struct rte_ipv6_hdr *) ((*eth) + 1);
    }

  uint8_t ip_proto;
  if (*ipv4)
    {
      ip_proto = (*ipv4)->next_proto_id;
      if (ip_proto == IPPROTO_ICMP)
        *icmp = (struct rte_icmp_hdr *) ((*ipv4) + 1);
      else if (ip_proto == IPPROTO_UDP)
        *udp = (struct rte_udp_hdr *) ((*ipv4) + 1);
      else if (ip_proto == IPPROTO_TCP)
        *tcp = (struct rte_tcp_hdr *) ((*ipv4) + 1);
    }
  else if (*ipv6)
    {
      ip_proto = (*ipv6)->proto;
      void* transport_hdr = (*ipv6) + 1;
      if (ip_proto == IPPROTO_ROUTING)
        {
          *srh = (struct rte_ipv6_routing_ext *) ((*ipv6) + 1);
          if ((*srh)->type == RTE_IPV6_SRCRT_TYPE_4)
            {
              ip_proto = (*srh)->next_hdr;
              uint16_t srh_len = ((*srh)->hdr_len +1) * 8;
              transport_hdr = (void *)((char *)(*srh) + srh_len);
            }
        }
      if (ip_proto == IPPROTO_ICMPV6)
        *icmp = (struct rte_icmp_hdr *) transport_hdr;
      else if (ip_proto == IPPROTO_UDP)
        *udp = (struct rte_udp_hdr *) transport_hdr ;
      else if (ip_proto == IPPROTO_TCP)
        *tcp = (struct rte_tcp_hdr *) transport_hdr ;
    }
}

static inline __attribute__ ((always_inline)) void
__log_packet (char *file, int line, const char *func, struct rte_mbuf *m,
              uint16_t rx_portid, uint16_t rx_queueid)
{
  char ether_str[512];
  char vlan_str[128];
  char snap_str[512];
  char pvst_str[512];
  char ip_str[512];
  char srh_str[512];
  char transport_str[512];
  char payload_str[512];

  struct rte_ether_hdr *eth;
  struct rte_vlan_hdr *vlan = NULL;
  struct llc_snap_hdr *snap = NULL;
  struct rte_ipv4_hdr *ipv4 = NULL;
  struct rte_ipv6_hdr *ipv6 = NULL;
  struct rte_ipv6_routing_ext *srh = NULL;
  struct rte_icmp_hdr *icmp = NULL;
  struct rte_udp_hdr *udp = NULL;
  struct rte_tcp_hdr *tcp = NULL;

  memset (ether_str, 0, sizeof (ether_str));
  memset (vlan_str, 0, sizeof (vlan_str));
  memset (snap_str, 0, sizeof (snap_str));
  memset (pvst_str, 0, sizeof (pvst_str));
  memset (ip_str, 0, sizeof (ip_str));
  memset (srh_str, 0, sizeof (srh_str));
  memset (transport_str, 0, sizeof (transport_str));
  memset (payload_str, 0, sizeof (payload_str));

  __parse_packet (m, &eth, &vlan, &snap, &ipv4, &ipv6, &srh, &icmp, &udp, &tcp);

  /* ether part */
  unsigned short eth_type;
  char eth_dst[32];
  char eth_src[32];
  rte_ether_format_addr (eth_dst, sizeof (eth_dst), &eth->dst_addr);
  rte_ether_format_addr (eth_src, sizeof (eth_src), &eth->src_addr);
  eth_type = rte_be_to_cpu_16 (eth->ether_type);
  snprintf (ether_str, sizeof (ether_str), "ether: type: 0x%04hx %s -> %s",
            eth_type, eth_src, eth_dst);

  if (vlan && !snap)
    {
      uint16_t vlan_tci, eth_proto;
      vlan_tci = rte_be_to_cpu_16 (vlan->vlan_tci);
      eth_proto = rte_be_to_cpu_16 (vlan->eth_proto);
      snprintf (vlan_str, sizeof (vlan_str),
                " vlan: pri: %d dei: %d vlan-id: %d proto: %#x",
                RTE_VLAN_TCI_PRI (vlan_tci), RTE_VLAN_TCI_DEI (vlan_tci),
                RTE_VLAN_TCI_ID (vlan_tci), eth_proto);
    }

  uint8_t ip_proto;
  char ip_src[64];
  char ip_dst[64];

  if (snap)
    // LLC単独フレーム(STP/CDPなど)
    {
      uint16_t proto = rte_be_to_cpu_16 (snap->snap_ether_type);
      snprintf (snap_str, sizeof (snap_str),
                "LLC frame: (non-IP, DSAP: 0x%02x SSAP: 0x%02x) "
                "SNAP frame: (snap_ether_type: 0x%04X OUI=%02X:%02X:%02X)",
                snap->dsap, snap->ssap, proto, snap->oui[0], snap->oui[1], snap->oui[2]);

      if (proto == 0x10B)
        {
          const struct stp_bpdu *bpdu = (const struct stp_bpdu *) snap;

          uint16_t proto_id = rte_be_to_cpu_16 (bpdu->protocol_id);
          uint16_t port_id = rte_be_to_cpu_16 (bpdu->port_id);

          snprintf (pvst_str, sizeof (pvst_str),
                    " PVST+ BPDU: "
                    "(Protocol ID: 0x%04x Version: %u BPDU Type: 0x%02x)",
                    proto_id, bpdu->version, bpdu->bpdu_type);
        }
    }
  else if (ipv4)
    {
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
  else if (ipv6)
    {
      inet_ntop (AF_INET6, &ipv6->src_addr, ip_src, sizeof (ip_src));
      inet_ntop (AF_INET6, &ipv6->dst_addr, ip_dst, sizeof (ip_dst));
      snprintf (ip_str, sizeof (ip_str),
                "ipv6: vtc_flow: %#x length: %d "
                "proto: %d hop_limits: %d %s -> %s",
                rte_be_to_cpu_32 (ipv6->vtc_flow),
                rte_be_to_cpu_16 (ipv6->payload_len), ipv6->proto,
                ipv6->hop_limits, ip_src, ip_dst);
      if (srh)
        {
          snprintf (srh_str, sizeof (srh_str),
                    " srv6: next_hdr: %d routing_type: %d"
                    " segments_left: %d last_entry: %d tag: %08x",
                    srh->next_hdr, srh->type, srh->segments_left,
                    srh->last_entry, rte_be_to_cpu_32 (srh->flags));

          size_t offset = strlen(srh_str);
          char sid_str[INET6_ADDRSTRLEN];
          struct in6_addr* sid_list = (struct in6_addr*)(srh+1);
          for (int i=0; i < srh->last_entry + 1; i++)
            {
              size_t rem_len = sizeof(srh_str) - offset;
              if (rem_len <= 1) break;
              struct in6_addr* sid = &sid_list[i];
              inet_ntop (AF_INET6, sid, sid_str, sizeof(sid_str));
              const char* marker = (i == srh->segments_left) ? " (active)" : "";
              int written = snprintf(srh_str+offset, rem_len,
                                     " SL[%d]: %s%s ", i, marker, sid_str);
              offset += written;
            }
        }
    }

  uint16_t src_port;
  uint16_t dst_port;
  if (icmp)
    {
      snprintf (transport_str, sizeof (transport_str),
                "icmp: type: %d (%s) code: %d "
                "cksum: %#x ident: %d seqnum: %d",
                icmp->icmp_type,
                (ipv4 ? __icmp_type_str (icmp->icmp_type) :
                        __icmp6_type_str (icmp->icmp_type)),
                icmp->icmp_code,
                rte_be_to_cpu_16 (icmp->icmp_cksum),
                rte_be_to_cpu_16 (icmp->icmp_ident),
                rte_be_to_cpu_16 (icmp->icmp_seq_nb));
    }
  else if (udp)
    {
      src_port = rte_be_to_cpu_16 (udp->src_port);
      dst_port = rte_be_to_cpu_16 (udp->dst_port);
      snprintf (transport_str, sizeof (transport_str),
                "udp: src-port: %d dst-port: %d",
                src_port, dst_port);
    }
  else if (tcp)
    {
      src_port = rte_be_to_cpu_16 (tcp->src_port);
      dst_port = rte_be_to_cpu_16 (tcp->dst_port);
      snprintf (transport_str, sizeof (transport_str),
                "tcp: src-port: %d dst-port: %d",
                src_port, dst_port);
    }

  //if (FLAG_CHECK (DEBUG_CONFIG (SDPLANE), DEBUG_TYPE (SDPLANE, PACKET)))
    debug_log ("%s[%d] %s(): m: %p rx_port: %d rx_queue: %d "
               "%s%s %s%s%s%s %s %s",
               file, line, func, m, rx_portid, rx_queueid,
               ether_str, vlan_str, snap_str, pvst_str,
               ip_str, srh_str, transport_str, payload_str);
}

#define log_packet(m, port, queue)                                            \
  __log_packet (__FILE__, __LINE__, __func__, m, port, queue)

#endif /*__LOG_PACKET_H__*/
