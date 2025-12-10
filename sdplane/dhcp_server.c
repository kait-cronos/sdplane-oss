#include "include.h"

#include <linux/if.h>
#include <linux/if_tun.h>
#include <netinet/icmp6.h>

#include <lthread.h>

#include <rte_common.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>

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
#include "debug_sdplane.h"

#include <rte_ether.h>
#include "l2fwd_export.h"

#include "sdplane.h"
#include "thread_info.h"
#include "rib_manager.h"
#include "tap.h"
#include "internal_message.h"

#include "log_packet.h"

static __thread uint64_t loop_counter_dhcp = 0;
static __thread struct rib *rib = NULL;

struct rte_ring *ring_dhcp_rx = NULL;
//struct rte_ring *ring_dhcp_tx = NULL;

bool
is_dhcp_packet (struct rte_mbuf *m)
{
  struct rte_ether_hdr *eth;
  uint16_t eth_type;
  struct rte_vlan_hdr *vlan = NULL;
  struct llc_snap_hdr *snap = NULL;
  struct rte_ipv4_hdr *ipv4 = NULL;
  struct rte_ipv6_hdr *ipv6 = NULL;
  struct rte_ipv6_routing_ext *srh = NULL;
  struct rte_icmp_hdr *icmp = NULL;
  struct rte_udp_hdr *udp = NULL;
  struct rte_tcp_hdr *tcp = NULL;

  __parse_packet (m, &eth, &vlan, &snap, &ipv4, &ipv6, &srh, &icmp, &udp, &tcp);

  uint16_t src_port;
  uint16_t dst_port;
  if (udp)
    {
      src_port = rte_be_to_cpu_16 (udp->src_port);
      dst_port = rte_be_to_cpu_16 (udp->dst_port);
    }
  else if (tcp)
    {
      src_port = rte_be_to_cpu_16 (tcp->src_port);
      dst_port = rte_be_to_cpu_16 (tcp->dst_port);
    }

  /* ipv6 ND router-solicit/router-advert */
  if (ipv6 && icmp &&
      (icmp->icmp_type == ND_ROUTER_SOLICIT ||
       icmp->icmp_type == ND_ROUTER_ADVERT))
    return true;

  /* ipv4 bootps/bootpc (DHCP) */
  if (ipv4 && udp &&
      ((src_port == 67 || src_port == 68) ||
      (dst_port == 67 || dst_port == 68)))
    return true;

  /* ipv6 dhcpv6-client/dhcpv6-server */
  if (ipv6 && udp &&
      ((src_port == 546 || src_port == 547) ||
      (dst_port == 546 || dst_port == 547)))
    return true;

  return false;
}

static inline __attribute__ ((always_inline)) void
dhcp_server_read (struct rte_mbuf *m)
{
  uint32_t pkt_len;
  uint16_t data_len;
  char *pkt;
  int ret;

  pkt_len = rte_pktmbuf_pkt_len (m);
  data_len = rte_pktmbuf_data_len (m);
  pkt = rte_pktmbuf_mtod (m, char *);

  if (data_len < pkt_len)
    ERROR_MSG ("m: %p warning: multi-seg mbuf: %u < %u",
               m, data_len, pkt_len);

  DEBUG_NEW (DHCP_SERVER, "DEBUG_NEW: m: %p packet [size: %d/%d] received.",
                     m, data_len, pkt_len);
  if (IS_DEBUG (DHCP_SERVER))
    log_packet (m, 0, 0);
}

static inline __attribute__ ((always_inline)) void
dhcp_server_write (char *data, int size, unsigned vswitch_id)
{
  char *pkt;
  int ret;
  struct vswitch_conf *vswitch;

  vswitch = &rib->rib_info->vswitch[vswitch_id];

  if (! vswitch)
    return;

  if (! vswitch->router_if.ring_dn)
    return;

  int socket = rte_lcore_to_socket_id (rte_lcore_id ());
  struct rte_mempool *mp = l2fwd_pktmbuf_pool;
  struct rte_mbuf *m = rte_pktmbuf_alloc (mp);
  rte_pktmbuf_append (m, size);
  pkt = rte_pktmbuf_mtod (m, char *);
  memcpy (pkt, data, size);

  rte_ring_enqueue (vswitch->router_if.ring_dn, m);
  DEBUG_NEW (DHCP_SERVER, "m: %p size: %d -> vswitch %d ring_dn %d",
             m, size, vswitch_id, vswitch->router_if.tap_ring_id);
}

static inline __attribute__ ((always_inline)) void
dhcp_server_rx ()
{
  struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
  unsigned int dequeued = 0, avail = 0;
  struct rte_mbuf *m;
  int i;

  if (! ring_dhcp_rx)
    return;
 
  dequeued = rte_ring_dequeue_burst (ring_dhcp_rx,
                                     (void **) pkts_burst,
                                     MAX_PKT_BURST, &avail);
 
  for (i = 0; i < dequeued; i++)
    {
      m = pkts_burst[i];
      if (! m)
        continue;
 
      dhcp_server_read (m);
      rte_pktmbuf_free (m);
    }
}

void
dhcp_server_init ()
{
  ring_dhcp_rx = rte_ring_create ("dhcp_rx", 32, SOCKET_ID_ANY,
                                  RING_F_SC_DEQ);
}

int
dhcp_server (__rte_unused void *dummy)
{
  unsigned lcore_id;

  lcore_id = rte_lcore_id ();
  DEBUG_NEW (DHCP_SERVER, "start thread on lcore[%d].", lcore_id);

  dhcp_server_init ();

  /* register an application slot entry in rib,
     to receive packets from enhanced_repeater. */
  struct internal_msg_header *msgp;
  struct application_slot_entry msg_appli_slot, *app;
  msg_appli_slot.name = "dhcp-server";
  msg_appli_slot.ring = ring_dhcp_rx;
  msg_appli_slot.is_packet_match = is_dhcp_packet;
  app = &msg_appli_slot;
  DEBUG_NEW (DHCP_SERVER,
             "registering appli_slot: %s ring: %p is_packet_match: %p",
             app->name, app->ring, app->is_packet_match);
  msgp = internal_msg_create (INTERNAL_MSG_TYPE_APPLICATION_SLOT,
                              &msg_appli_slot, sizeof (msg_appli_slot));
  rib_send_message (msgp);

  /* register the thread loop_counter. */
  int thread_id;
  thread_id = thread_lookup_by_lcore (dhcp_server, lcore_id);
  thread_register_loop_counter (thread_id, &loop_counter_dhcp);

  DEBUG_NEW (DHCP_SERVER, "start main loop on lcore[%d].", lcore_id);
  if (IS_LTHREAD ())
    DEBUG_NEW (DHCP_SERVER, "started as a lthread.");

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

      dhcp_server_rx ();

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_unlock ();
      urcu_qsbr_quiescent_state ();
#endif /*HAVE_LIBURCU_QSBR*/

      loop_counter_dhcp++;
    }

  printf ("%s on lcore[%d]: finished.\n", __func__, rte_lcore_id ());

#if HAVE_LIBURCU_QSBR
  if (! IS_LTHREAD ())
    urcu_qsbr_unregister_thread ();
#endif /*HAVE_LIBURCU_QSBR*/

  return 0;
}
