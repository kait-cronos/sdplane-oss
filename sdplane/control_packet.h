#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_mbuf.h>
#include <rte_byteorder.h>
#include <rte_udp.h>
#include <rte_mbuf.h>

#define IPPROTO_OSPFIGP 89
#define IPPROTO_PIM 103

/* Common IP protocol numbers (guarded to avoid redefinition) */
#ifndef IPPROTO_TCP
#define IPPROTO_TCP 6
#endif
#ifndef IPPROTO_UDP
#define IPPROTO_UDP 17
#endif
#ifndef IPPROTO_ICMP
#define IPPROTO_ICMP 1
#endif
#ifndef IPPROTO_ICMPV6
#define IPPROTO_ICMPV6 58
#endif
#ifndef IPPROTO_OSPF
#define IPPROTO_OSPF 89
#endif

/* Well-known ports used by control protocols (guarded) */
#ifndef BGP_PORT
#define BGP_PORT 179
#endif
#ifndef LDP_PORT
#define LDP_PORT 646
#endif
#ifndef RIP_PORT
#define RIP_PORT 520
#endif
#ifndef RIPNG_PORT
#define RIPNG_PORT 521
#endif
#ifndef BABEL_PORT
#define BABEL_PORT 6696
#endif

int is_control_packet (struct rte_mbuf *m);
