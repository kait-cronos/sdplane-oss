#include "include.h"

#include <linux/if.h>
#include <linux/if_tun.h>

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

#include "sdplane.h"
#include "thread_info.h"

#include "rib_manager.h"

#include "tap.h"

#include "log_packet.h"

static __thread uint64_t loop_counter = 0;
static __thread struct rib *rib = NULL;

struct rte_ring *ring_dhcp_rx = NULL;
//struct rte_ring *ring_dhcp_tx = NULL;

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
    DEBUG_SDPLANE_LOG (DHCP_SERVER, "m: %p warning: multi-seg mbuf: %u < %u",
                       m, data_len, pkt_len);

  DEBUG_SDPLANE_LOG (DHCP_SERVER, "m: %p packet [%d/%d] received.",
                     m, data_len, pkt_len);
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
  DEBUG_SDPLANE_LOG (DHCP_SERVER, "m: %p size: %d -> vswitch %d ring_dn %d",
                     m, size, vswitch_id, vswitch->router_if.tap_ring_id);
}

static inline __attribute__ ((always_inline)) void
dhcp_server_rx ()
{
  struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
  unsigned int dequeued = 0, avail = 0;
  struct rte_mbuf *m;
  int i;
 
  dequeued = rte_ring_dequeue_burst (ring_dhcp_rx,
                                     (void **) pkts_burst,
                                     MAX_PKT_BURST, &avail);
 
  for (i = 0; i < dequeued; i++)
    {
      m = pkts_burst[i];
      if (! m)
        continue;
 
      DEBUG_SDPLANE_LOG (PACKET, "m: %p received from ring_dhcp_rx.", m);
      log_packet (m, 0, 0);
 
      dhcp_server_read (m);
 
      rte_pktmbuf_free (m);
    }
}

int
dhcp_server (__rte_unused void *dummy)
{
  int ret;

  unsigned lcore_id;
  struct rte_ring *tap_ring;

  DEBUG_SDPLANE_LOG (DHCP_SERVER, "start thread on lcore[%d].",
                     rte_lcore_id ());

  int i, j;
  memset (fdb, 0, sizeof (fdb));

  unsigned lcore_id = rte_lcore_id ();

  int thread_id;
  thread_id = thread_lookup_by_lcore (dhcp_server, lcore_id);
  thread_register_loop_counter (thread_id, &loop_counter);

  DEBUG_SDPLANE_LOG (DHCP_SERVER, "start main loop on lcore[%d].",
                     lcore_id);

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_register_thread ();
#endif /*HAVE_LIBURCU_QSBR*/

  while (! force_quit && ! force_stop[tap_handler_id])
    {
      // lthread_sleep (0); // yield.
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

      loop_counter++;
    }

  printf ("%s on lcore[%d]: finished.\n", __func__, rte_lcore_id ());

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_unregister_thread ();
#endif /*HAVE_LIBURCU_QSBR*/

  return 0;
}
