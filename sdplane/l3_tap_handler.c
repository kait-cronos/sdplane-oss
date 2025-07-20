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

#include "l3fwd.h"
#include "l3fwd_event.h"
#include "l3fwd_route.h"

#include "l2fwd_export.h"
#include "l2fwd_cmd.h"

#include "sdplane.h"
#include "thread_info.h"

#include "rib_manager.h"

#include "tap.h"

#include "log_packet.h"

extern struct fdb_entry fdb[FDB_SIZE];

static __thread uint64_t loop_counter = 0;
static __thread struct rib *rib = NULL;

static inline __attribute__ ((always_inline)) void
l3_tap_handler_register_fdb (struct rte_mbuf *m)
{
  int j;
  char eth_src[32];
  struct rte_ether_hdr *eth;

  eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  rte_ether_format_addr (eth_src, sizeof (eth_src), &eth->src_addr);

  /* register in FDB */
  for (j = 0; j < FDB_SIZE; j++)
    {
      if (rte_is_zero_ether_addr (&fdb[j].l2addr))
        {
          fdb[j].l2addr = eth->src_addr;
          fdb[j].port = m->port;
          DEBUG_SDPLANE_LOG (FDB_CHANGE,
                             "m: %p new: in fdb[%d]: addr: %s port: %d", m, j,
                             eth_src, m->port);
          break;
        }
      if (rte_is_same_ether_addr (&fdb[j].l2addr, &eth->src_addr))
        {
          fdb[j].port = m->port;
          DEBUG_SDPLANE_LOG (FDB, "m: %p found: in fdb[%d]: addr: %s port: %d",
                             m, j, eth_src, m->port);
          break;
        }
      char buf[32];
      rte_ether_format_addr (buf, sizeof (buf), &fdb[j].l2addr);
      DEBUG_SDPLANE_LOG (FDB, "m: %p fdb[%d]: addr: %s port: %d", m, j, buf,
                         fdb[j].port);
    }
}

static inline __attribute__ ((always_inline)) void
l3_tap_handler_write_capture_if (int capture_fd, struct rte_mbuf *m)
{
  uint32_t pkt_len;
  uint16_t data_len;
  char *pkt;
  int ret;

  pkt_len = rte_pktmbuf_pkt_len (m);
  data_len = rte_pktmbuf_data_len (m);
  pkt = rte_pktmbuf_mtod (m, char *);
  ;

  /* write to capture_fd for packet capture. */
  if (capture_fd >= 0)
    {
      ret = write (capture_fd, pkt, data_len);
      if (ret < 0)
        DEBUG_SDPLANE_LOG (TAPHANDLER, "warning: write () failed: %s",
                           strerror (errno));
      else
        DEBUG_SDPLANE_LOG (
            TAPHANDLER, "packet [%d/%d] (in_port: %d) written to capture I/F.",
            data_len, pkt_len, m->port);
    }
}

static inline __attribute__ ((always_inline)) void
l3_tap_handler_write_router_if (int router_fd, struct rte_mbuf *m)
{
  uint32_t pkt_len;
  uint16_t data_len;
  char *pkt;
  int ret;

  pkt_len = rte_pktmbuf_pkt_len (m);
  data_len = rte_pktmbuf_data_len (m);
  pkt = rte_pktmbuf_mtod (m, char *);

  if (data_len < pkt_len)
    DEBUG_SDPLANE_LOG (TAPHANDLER, "warning: multi-seg mbuf: %u < %u",
                       data_len, pkt_len);

  ret = write (router_fd, pkt, data_len);
  if (ret < 0)
    DEBUG_SDPLANE_LOG (TAPHANDLER, "write() failed: router_fd: %d error: %s.",
                       router_fd, strerror (errno));
  else
    DEBUG_SDPLANE_LOG (TAPHANDLER, "packet [%d/%d] router_fd: %d.", data_len,
                       pkt_len, router_fd);
}

static inline __attribute__ ((always_inline)) void
l3_tap_handler_handle_packet_up ()
{
  struct rte_ring *tap_ring;
  int i;
  int vswitch_id;

  for (vswitch_id = 0; vswitch_id < rib->rib_info->vswitch_size; vswitch_id++)
    {
      struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
      unsigned int dequeued = 0, avail = 0;

      struct vswitch_conf *vswitch = &rib->rib_info->vswitch[vswitch_id];

      int router_fd = vswitch->router_if.sockfd;
      int capture_fd = vswitch->capture_if.sockfd;

      if (vswitch->router_if.tap_ring_id != vswitch_id)
        continue;

      tap_ring = vswitch->router_if.ring_up;
      dequeued = rte_ring_dequeue_burst (tap_ring, (void **) pkts_burst,
                                         MAX_PKT_BURST, &avail);

      for (i = 0; i < dequeued; i++)
        {
          struct rte_mbuf *m;

          m = pkts_burst[i];
          if (! m)
            continue;

          DEBUG_SDPLANE_LOG (PACKET, "m: %p received from tap_ring: %d", m,
                             vswitch->router_if.tap_ring_id);
          log_packet (m, vswitch_id, vswitch->router_if.tap_ring_id);

          l3_tap_handler_register_fdb (m);
          if (capture_fd >= 0)
            l3_tap_handler_write_capture_if (capture_fd, m);
          if (router_fd >= 0)
            l3_tap_handler_write_router_if (router_fd, m);

          rte_pktmbuf_free (m);
        }
    }
}

static inline __attribute__ ((always_inline)) void
l3_tap_handler_handle_packet_down ()
{
  int nfds = 0;
  struct pollfd fds[MAX_VSWITCH_ID];
  int ret;
  int i;
  char data[9000];
  char *pkt;

  struct vswitch_conf *vswitch;
  int vswitch_id;

  nfds = 0;
  for (vswitch_id = 0; vswitch_id < rib->rib_info->vswitch_size; vswitch_id++)
    {
      vswitch = &rib->rib_info->vswitch[vswitch_id];

      if (nfds >= MAX_VSWITCH_ID)
        break;

      fds[nfds].fd = vswitch->router_if.sockfd;
      fds[nfds].events = POLLIN;
      fds[nfds].revents = 0;
      nfds++;
    }

  ret = poll (fds, nfds, 0);

  for (i = 0; i < nfds; i++)
    {
      if ((fds[i].revents & (POLLIN | POLLERR)) == 0)
        continue;

      vswitch_id = -1;
      for (int j = 0; j < rib->rib_info->vswitch_size; j++)
        {
          vswitch = &rib->rib_info->vswitch[j];
          if (vswitch->router_if.sockfd == fds[i].fd)
            {
              vswitch_id = j;
              break;
            }
        }

      if (vswitch_id < 0)
        continue;

      vswitch = &rib->rib_info->vswitch[vswitch_id];

      ret = read (vswitch->router_if.sockfd, data, sizeof (data));
      if (ret < 0)
        {
          DEBUG_SDPLANE_LOG (TAPHANDLER,
                             "read() failed on router_if vswitch[%d]: %s",
                             vswitch_id, strerror (errno));
          continue;
        }
      if (ret == 0)
        continue;

      if (vswitch->router_if.ring_dn)
        {
          int socket = rte_lcore_to_socket_id (rte_lcore_id ());
          struct rte_mempool *mp = l2fwd_pktmbuf_pool;
          struct rte_mbuf *m = rte_pktmbuf_alloc (mp);
          rte_pktmbuf_append (m, ret);
          pkt = rte_pktmbuf_mtod (m, char *);
          memcpy (pkt, data, ret);
          rte_ring_enqueue (vswitch->router_if.ring_dn, m);
          DEBUG_SDPLANE_LOG (TAPHANDLER, "packet: sockfd %d -> ring_dn %d",
                             vswitch->router_if.sockfd,
                             vswitch->router_if.tap_ring_id);
        }
    }
}

int
l3_tap_handler (__rte_unused void *dummy)
{
  int ret;

  unsigned lcore_id;
  struct rte_ring *tap_ring;

  DEBUG_SDPLANE_LOG (TAPHANDLER, "start thread on lcore[%d].",
                     rte_lcore_id ());

  int i, j;
  memset (fdb, 0, sizeof (fdb));

  unsigned tap_handler_id = rte_lcore_id ();

  int thread_id;
  thread_id = thread_lookup_by_lcore (l3_tap_handler, tap_handler_id);
  thread_register_loop_counter (thread_id, &loop_counter);

  DEBUG_SDPLANE_LOG (TAPHANDLER, "start main loop on lcore[%d].",
                     tap_handler_id);

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

      l3_tap_handler_handle_packet_up ();
      l3_tap_handler_handle_packet_down ();

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
