#include "include.h"

#include <linux/if.h>
#include <linux/if_tun.h>
#include <netinet/icmp6.h>

#include <lthread.h>

#include <rte_common.h>
#include <rte_ip.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_ether.h>

#include <sdplane/debug.h>
#include <sdplane/termio.h>
#include <sdplane/vector.h>
#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>

#include "l2fwd_export.h"
#include "sdplane.h"
#include "thread_info.h"
#include "rib_manager.h"
#include "internal_message.h"
#include "log_packet.h"

#include <netinet/igmp.h>

static __thread uint64_t loop_counter_igmp_snooping = 0;
static __thread struct rib *rib = NULL;

struct rte_ring *ring_igmp_snooping_rx = NULL;
//struct rte_ring *ring_igmp_snooping_tx = NULL;

bool
is_igmp_packet (struct rte_mbuf *m)
{
  struct rte_ether_hdr *eth;
  struct rte_vlan_hdr *vlan = NULL;
  struct llc_snap_hdr *snap = NULL;
  struct rte_ipv4_hdr *ipv4 = NULL;
  struct rte_ipv6_hdr *ipv6 = NULL;
  struct rte_ipv6_routing_ext *srh = NULL;
  struct rte_icmp_hdr *icmp = NULL;
  struct rte_udp_hdr *udp = NULL;
  struct rte_tcp_hdr *tcp = NULL;

  struct igmp *igmp = NULL;

  __parse_packet (m, &eth, &vlan, &snap, &ipv4, &ipv6, &srh, &icmp, &udp, &tcp);

  uint8_t ip_proto;
  if (ipv4)
    {
      ip_proto = ipv4->next_proto_id;
      if (ip_proto == IPPROTO_IGMP)
        igmp = (struct  igmp *) (ipv4 + 1);
    }
  else if (ipv6)
    {
      ip_proto = ipv6->proto;
      if (ip_proto == IPPROTO_IGMP)
        igmp = (struct igmp *) (ipv6 + 1);
    }

  if (! igmp)
    return false;

  uint8_t type;
  uint8_t code;
  uint8_t cksum;
  char group_addr[64] = { 0 };

  type = igmp->igmp_type;
  code = igmp->igmp_code;
  cksum = rte_be_to_cpu_16 (igmp->igmp_cksum);
  if (ipv4)
    inet_ntop (AF_INET, &igmp->igmp_group, group_addr, sizeof (group_addr));

  DEBUG_NEW (IGMP, "IPv%s IGMP type: %d code: %d cksum: %#hx"
             "gaddr: %s",
             (ipv4 ? "4" : "6"), type, code, cksum,
             group_addr);

  return true;
}

static bool
igmp_report_parse (const struct rte_mbuf *m, uint32_t *group_addr)
{
  struct rte_ether_hdr eth_buf;
  struct rte_vlan_hdr vlan_buf;
  struct rte_ipv4_hdr ipv4_buf;
  struct igmp igmp_buf;

  const struct rte_ether_hdr *eth;
  const struct rte_vlan_hdr *vlan;
  const struct rte_ipv4_hdr *ipv4;
  const struct igmp *igmp;

  uint32_t eth_type;
  uint32_t offset;
  uint32_t ip_hlen;
  uint32_t ip_len;
  uint32_t group;

  if (! m || ! group_addr)
    return false;

  eth = rte_pktmbuf_read (m, 0, sizeof (eth_buf), &eth_buf);
  if (! eth)
    return false;

  eth_type = rte_be_to_cpu_16 (eth->ether_type);
  offset = sizeof (eth_buf);

  if (eth_type == RTE_ETHER_TYPE_VLAN)
    {
      vlan = rte_pktmbuf_read (m, offset, sizeof (vlan_buf), &vlan_buf);
      if (! vlan)
        return false;

      eth_type = rte_be_to_cpu_16 (vlan->eth_proto);
      offset += sizeof (vlan_buf);
    }

  if (eth_type != RTE_ETHER_TYPE_IPV4)
    return false;

  ipv4 = rte_pktmbuf_read (m, offset, sizeof (ipv4_buf), &ipv4_buf);
  if (! ipv4)
    return false;

  if (ipv4->next_proto_id != IPPROTO_IGMP)
    return false;

  ip_hlen = rte_ipv4_hdr_len (ipv4);
  if (ip_hlen < sizeof (ipv4_buf))
    return false;

  offset += ip_hlen;

  igmp = rte_pktmbuf_read (m, offset, sizeof (igmp_buf), &igmp_buf);
  if (! igmp)
    return false;

  if (igmp->igmp_type != IGMP_V2_MEMBERSHIP_REPORT)
    return false;

  group = rte_be_to_cpu_32 (igmp->igmp_group.s_addr);
  if (! IN_MULTICAST (group))
    return false;

  *group_addr = igmp->igmp_group.s_addr;
  return true;
}

void
igmp_snooping_learn (const struct rte_mbuf *m, unsigned rx_portid,
                     uint16_t vswitch_id)
{
  struct internal_msg_mdb_entry entry;
  struct internal_msg_header *msgp;
  uint32_t group_addr;

  if (! m || rx_portid >= MAX_ETH_PORTS)
    return;

  if (rx_portid >= MAX_ETH_PORTS || ! igmp_report_parse (m, &group_addr))
    return;

  entry.group_addr = group_addr;
  entry.vswitch_id = vswitch_id;
  entry.port = rx_portid;

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_MDB_ENTRY_ADD, &entry,
                              sizeof (entry));

  if (msgp)
    rib_send_message (msgp);
}

static inline __attribute__ ((always_inline)) void
igmp_snooping_read (struct rte_mbuf *m)
{
  uint32_t pkt_len;
  uint16_t data_len;

  pkt_len = rte_pktmbuf_pkt_len (m);
  data_len = rte_pktmbuf_data_len (m);

  if (data_len < pkt_len)
    ERROR_MSG ("m: %p warning: multi-seg mbuf: %u < %u",
               m, data_len, pkt_len);

  DEBUG_NEW (IGMP, "DEBUG_NEW: m: %p packet [size: %d/%d] received.",
             m, data_len, pkt_len);
  if (IS_DEBUG (IGMP))
    log_packet (m, 0, 0);
}

static inline __attribute__ ((always_inline)) void
igmp_snooping_rx ()
{
  struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
  unsigned int dequeued = 0, avail = 0;
  struct rte_mbuf *m;
  int i;

  if (! ring_igmp_snooping_rx)
    return;

  dequeued = rte_ring_dequeue_burst (ring_igmp_snooping_rx,
                                     (void **) pkts_burst,
                                     MAX_PKT_BURST, &avail);

  for (i = 0; i < dequeued; i++)
    {
      m = pkts_burst[i];
      if (! m)
        continue;

      igmp_snooping_read (m);
      rte_pktmbuf_free (m);
    }
}

int
igmp_snooping_init ()
{
  ring_igmp_snooping_rx = rte_ring_create ("igmp_snooping_rx", 32,
                                      SOCKET_ID_ANY, RING_F_SC_DEQ);
  if (! ring_igmp_snooping_rx)
    {
      printf ("%s[%d]: %s: failed to start.\n", __FILE__, __LINE__, __func__);
      DEBUG_NEW (IGMP, "rte_ring_create(igmp_snooping_rx) failed: %s",
                 rte_strerror (rte_errno));
      return -1;
    }
  return 0;
}

int
igmp_snooping (__rte_unused void *dummy)
{
  unsigned lcore_id;
  int ret;

  lcore_id = rte_lcore_id ();
  DEBUG_NEW (IGMP, "start thread on lcore[%d].", lcore_id);

  ret = igmp_snooping_init ();
  if (ret < 0)
    {
      DEBUG_NEW (IGMP, "igmp_snooping_init() failed: "
                 "unable to create rx ring");
      return -1;
    }

  /* register an application slot entry in rib,
     to receive packets from enhanced_repeater. */
  struct internal_msg_header *msgp;
  struct application_slot_entry msg_appli_slot, *app;
  msg_appli_slot.name = "igmp-snooping";
  msg_appli_slot.ring = ring_igmp_snooping_rx;
  msg_appli_slot.is_packet_match = is_igmp_packet;
  app = &msg_appli_slot;
  DEBUG_NEW (IGMP, "registering appli_slot: %s ring: %p is_packet_match: %p",
             app->name, app->ring, app->is_packet_match);
  msgp = internal_msg_create (INTERNAL_MSG_TYPE_APPLICATION_SLOT,
                              &msg_appli_slot, sizeof (msg_appli_slot));
  rib_send_message (msgp);

  /* register the thread loop_counter. */
  int thread_id;
  thread_id = thread_lookup_by_lcore (igmp_snooping, lcore_id);
  thread_register_loop_counter (thread_id, &loop_counter_igmp_snooping);

  DEBUG_NEW (IGMP, "start main loop on lcore[%d].", lcore_id);
  if (IS_LTHREAD ())
    DEBUG_NEW (IGMP, "started as a lthread.");

#if HAVE_LIBURCU_QSBR
  if (! IS_LTHREAD ())
    urcu_qsbr_register_thread ();
#endif /*HAVE_LIBURCU_QSBR*/

  while (! force_quit && ! force_stop[lcore_id])
    {
      if (IS_LTHREAD ())
        lthread_sleep (0); // yield.
      // printf ("%s: schedule: %lu.\n", __func__, loop_counter);

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_lock ();
      rib = (struct rib *) rcu_dereference (rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

      igmp_snooping_rx ();

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_unlock ();
      urcu_qsbr_quiescent_state ();
#endif /*HAVE_LIBURCU_QSBR*/

      loop_counter_igmp_snooping++;
    }

  printf ("%s on lcore[%d]: finished.\n", __func__, rte_lcore_id ());

#if HAVE_LIBURCU_QSBR
  if (! IS_LTHREAD ())
    urcu_qsbr_unregister_thread ();
#endif /*HAVE_LIBURCU_QSBR*/

  return 0;
}
