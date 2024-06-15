#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <string.h>
#include <sys/queue.h>
#include <stdarg.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>

#include <locale.h>

#include <rte_ethdev.h>
#include <rte_bus_pci.h>

#include "dpdk_flag.h"

#if 0
struct flag_name link_speeds[] =
{
  { "Fix",      RTE_ETH_LINK_SPEED_FIXED },
  { "10M-hd",   RTE_ETH_LINK_SPEED_10M_HD },
  { "10M",      RTE_ETH_LINK_SPEED_10M },
  { "100M-hd",  RTE_ETH_LINK_SPEED_10M_HD },
  { "100M",     RTE_ETH_LINK_SPEED_10M },
  { "1G",       RTE_ETH_LINK_SPEED_1G },
  { "2.5G",     RTE_ETH_LINK_SPEED_2_5G },
  { "5G",       RTE_ETH_LINK_SPEED_5G },
  { "10G",      RTE_ETH_LINK_SPEED_10G },
  { "20G",      RTE_ETH_LINK_SPEED_20G },
  { "25G",      RTE_ETH_LINK_SPEED_25G },
  { "40G",      RTE_ETH_LINK_SPEED_40G },
  { "50G",      RTE_ETH_LINK_SPEED_50G },
  { "56G",      RTE_ETH_LINK_SPEED_56G },
  { "100G",     RTE_ETH_LINK_SPEED_100G },
  { "200G",     RTE_ETH_LINK_SPEED_200G },
  { "400G",     RTE_ETH_LINK_SPEED_400G },
};

struct flag_name rx_offload_capa[] =
{
  { "VLAN_STRIP", RTE_ETH_RX_OFFLOAD_VLAN_STRIP       },
  { "IPV4_CKSUM", RTE_ETH_RX_OFFLOAD_IPV4_CKSUM       },
  { "UDP_CKSUM", RTE_ETH_RX_OFFLOAD_UDP_CKSUM         },
  { "TCP_CKSUM", RTE_ETH_RX_OFFLOAD_TCP_CKSUM         },
  { "TCP_LRO", RTE_ETH_RX_OFFLOAD_TCP_LRO             },
  { "QINQ_STRIP", RTE_ETH_RX_OFFLOAD_QINQ_STRIP       },
  { "OUTER_IPV4_CKSUM", RTE_ETH_RX_OFFLOAD_OUTER_IPV4_CKSUM },
  { "MACSEC_STRIP", RTE_ETH_RX_OFFLOAD_MACSEC_STRIP   },
  { "VLAN_FILTER", RTE_ETH_RX_OFFLOAD_VLAN_FILTER     },
  { "VLAN_EXTEND", RTE_ETH_RX_OFFLOAD_VLAN_EXTEND     },
  { "SCATTER", RTE_ETH_RX_OFFLOAD_SCATTER             },

  { "TIMESTAMP", RTE_ETH_RX_OFFLOAD_TIMESTAMP         },
  { "SECURITY", RTE_ETH_RX_OFFLOAD_SECURITY           },
  { "KEEP_CRC", RTE_ETH_RX_OFFLOAD_KEEP_CRC           },
  { "SCTP_CKSUM", RTE_ETH_RX_OFFLOAD_SCTP_CKSUM       },
  { "OUTER_UDP_CKSUM", RTE_ETH_RX_OFFLOAD_OUTER_UDP_CKSUM  },
  { "RSS_HASH", RTE_ETH_RX_OFFLOAD_RSS_HASH           },
  { "BUFFER_SPLIT", RTE_ETH_RX_OFFLOAD_BUFFER_SPLIT   },
};

struct flag_name tx_offload_capa[] =
{
  { "VLAN_INSERT", RTE_ETH_TX_OFFLOAD_VLAN_INSERT      },
  { "IPV4_CKSUM", RTE_ETH_TX_OFFLOAD_IPV4_CKSUM       },
  { "UDP_CKSUM", RTE_ETH_TX_OFFLOAD_UDP_CKSUM        },
  { "TCP_CKSUM", RTE_ETH_TX_OFFLOAD_TCP_CKSUM        },
  { "SCTP_CKSUM", RTE_ETH_TX_OFFLOAD_SCTP_CKSUM       },
  { "TCP_TSO", RTE_ETH_TX_OFFLOAD_TCP_TSO          },
  { "UDP_TSO", RTE_ETH_TX_OFFLOAD_UDP_TSO          },
  { "OUTER_IPV4_CKSUM", RTE_ETH_TX_OFFLOAD_OUTER_IPV4_CKSUM },
  { "QINQ_INSERT", RTE_ETH_TX_OFFLOAD_QINQ_INSERT      },
  { "VXLAN_TNL_TSO", RTE_ETH_TX_OFFLOAD_VXLAN_TNL_TSO    },
  { "GRE_TNL_TSO", RTE_ETH_TX_OFFLOAD_GRE_TNL_TSO      },
  { "IPIP_TNL_TSO", RTE_ETH_TX_OFFLOAD_IPIP_TNL_TSO     },
  { "GENEVE_TNL_TSO", RTE_ETH_TX_OFFLOAD_GENEVE_TNL_TSO   },
  { "MACSEC_INSERT", RTE_ETH_TX_OFFLOAD_MACSEC_INSERT    },

  { "MT_LOCKFREE", RTE_ETH_TX_OFFLOAD_MT_LOCKFREE      },
  { "MULTI_SEGS", RTE_ETH_TX_OFFLOAD_MULTI_SEGS       },
  { "MBUF_FAST_FREE", RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE   },
  { "SECURITY", RTE_ETH_TX_OFFLOAD_SECURITY         },
  { "UDP_TNL_TSO", RTE_ETH_TX_OFFLOAD_UDP_TNL_TSO      },
  { "IP_TNL_TSO", RTE_ETH_TX_OFFLOAD_IP_TNL_TSO       },
  { "OUTER_UDP_CKSUM", RTE_ETH_TX_OFFLOAD_OUTER_UDP_CKSUM  },
  { "SEND_ON_TIMESTAMP", RTE_ETH_TX_OFFLOAD_SEND_ON_TIMESTAMP},
};
#endif


int
snprintf_flags (char *buf, int size, uint64_t flags,
                struct flag_name *flag_names,
                char *delim, int flag_names_size)
{
  char *p = buf;
  int bufsize = size;
  int ret = 0;
  int num = 0;
  int i;

  if (! delim)
    delim = "|";

  for (i = flag_names_size - 1; i >= 0; i--)
    {
      if (! (flags & flag_names[i].val))
        continue;
      if (bufsize <= 0)
        continue;

      if (num == 0)
        ret = snprintf (p, bufsize, "%s", flag_names[i].name);
      else
        ret = snprintf (p, bufsize, "%s%s", delim, flag_names[i].name);

      p += ret;
      bufsize -= ret;
      num++;
    }
  return num;
}

