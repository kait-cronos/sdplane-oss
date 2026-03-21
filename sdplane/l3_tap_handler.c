// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2023-2026 Yasuhiro Ohara

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

#include "l3fwd.h"
#include "l3fwd_route.h"
#include "l2fwd_export.h"
#include "sdplane.h"
#include "thread_info.h"
#include "rib_manager.h"
#include "log_packet.h"
#include "tap_handler.h"

static __thread uint64_t loop_counter = 0;
static __thread struct rib *rib = NULL;

static inline __attribute__ ((always_inline)) void
l3_tap_handler_write (int tap_fd, struct rte_mbuf *m)
{
  uint32_t pkt_len;
  uint16_t data_len;
  char *pkt;
  int ret;

  struct rte_ether_hdr *eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  if (rte_be_to_cpu_16 (eth->ether_type) == RTE_ETHER_TYPE_VLAN)
    rte_vlan_strip (m);

  pkt_len = rte_pktmbuf_pkt_len (m);
  data_len = rte_pktmbuf_data_len (m);
  pkt = rte_pktmbuf_mtod (m, char *);

  if (data_len < pkt_len)
    DEBUG_NEW (TAPHANDLER, "warning: multi-seg mbuf: %u < %u",
               data_len, pkt_len);

  ret = write (tap_fd, pkt, data_len);
  if (ret < 0)
    DEBUG_NEW (TAPHANDLER, "write() failed: tap_fd: %d error: %s.",
               tap_fd, strerror (errno));
  else
    DEBUG_NEW (TAPHANDLER, "packet [%d/%d] tap_fd: %d.", data_len,
               pkt_len, tap_fd);
}

static inline __attribute__ ((always_inline)) void
l3_tap_handler_dequeue_burst (int tap_fd, struct rte_ring *tap_ring,
                              struct vswitch_conf *vswitch)
{
  int i;
  struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
  unsigned int dequeued = 0, avail = 0;

  if (! tap_ring || tap_fd < 0)
    return;

  dequeued = rte_ring_dequeue_burst (tap_ring, (void **) pkts_burst,
                                      MAX_PKT_BURST, &avail);

  for (i = 0; i < dequeued; i++)
    {
      struct rte_mbuf *m;

      m = pkts_burst[i];
      if (! m)
        continue;

      if (IS_DEBUG (PACKET))
        log_packet (m, vswitch->vswitch_id, vswitch->router_if.tap_ring_id);

      if (rib && rib->rib_info &&
          is_fdb_send_add (rib->rib_info, m, vswitch->vlan_id))
        send_fdb_entry_add_msg (m, vswitch->vlan_id);
      if (tap_fd >= 0)
        l3_tap_handler_write (tap_fd, m);

      rte_pktmbuf_free (m);
    }
}

static inline __attribute__ ((always_inline)) void
l3_tap_handler_handle_packet_up ()
{
  int vswitch_id;

  if (! rib || ! rib->rib_info)
    return;

  for (vswitch_id = 0; vswitch_id < rib->rib_info->vswitch_size; vswitch_id++)
    {
      struct vswitch_conf *vswitch = &rib->rib_info->vswitch[vswitch_id];

      int router_fd = vswitch->router_if.sockfd;
      struct rte_ring *router_tap_ring = vswitch->router_if.ring_up;
      int capture_fd = vswitch->capture_if.sockfd;
      struct rte_ring *capture_tap_ring = vswitch->capture_if.ring_up;

      if (vswitch->router_if.tap_ring_id != vswitch->vswitch_id)
        continue;

      l3_tap_handler_dequeue_burst (router_fd, router_tap_ring, vswitch);
      l3_tap_handler_dequeue_burst (capture_fd, capture_tap_ring, vswitch);
    }
}

static inline __attribute__ ((always_inline)) void
l3_tap_handler_handle_packet_down ()
{
  int nfds = 0;
  struct pollfd fds[MAX_VSWITCH];
  int ret;
  int i;
  char data[9000];
  char *pkt;
  int size;

  struct vswitch_conf *vswitch;
  int vswitch_id;

  if (! rib || ! rib->rib_info)
    return;

  nfds = 0;
  for (vswitch_id = 0; vswitch_id < rib->rib_info->vswitch_size; vswitch_id++)
    {
      vswitch = &rib->rib_info->vswitch[vswitch_id];

      if (nfds >= MAX_VSWITCH)
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
          DEBUG_NEW (TAPHANDLER, "read() failed on router_if vswitch[%d]: %s",
                     vswitch_id, strerror (errno));
          continue;
        }
      if (ret == 0)
        {
          DEBUG_NEW (TAPHANDLER, "read() ret: 0: vswitch[%d]", vswitch_id);
          continue;
        }

      if (vswitch->router_if.ring_dn)
        {
          struct rte_mempool *mp = l2fwd_pktmbuf_pool;
          struct rte_mbuf *m = rte_pktmbuf_alloc (mp);

	  if (m)
            {
              size = ret;
              if (size >= m->buf_len)
                {
                  WARNING ("m: %p insufficient buf: %d bytes read, "
                           "m->buf_len: %d", m, ret, m->buf_len);
                  size = m->buf_len;
                }

              rte_pktmbuf_append (m, size);
              pkt = rte_pktmbuf_mtod (m, char *);
              memcpy (pkt, data, size);

              rte_ring_enqueue (vswitch->router_if.ring_dn, m);
              DEBUG_NEW (TAPHANDLER,
                         "vswitch[%d]: packet: sockfd %d -> ring_dn %d",
                         vswitch_id,
                         vswitch->router_if.sockfd,
                         vswitch->router_if.tap_ring_id);

              if (IS_DEBUG (PACKET))
                log_packet (m, vswitch_id, vswitch->router_if.tap_ring_id);
            }
	  else
              WARNING ("rte_pktmbuf_alloc() failed: ring_dn: drop %d bytes",
                       ret);
        }
    }
}

int
l3_tap_handler (__rte_unused void *dummy)
{
  DEBUG_NEW (TAPHANDLER, "start thread on lcore[%d].",
             rte_lcore_id ());

  unsigned tap_handler_id = rte_lcore_id ();

  int thread_id;
  thread_id = thread_lookup_by_lcore (l3_tap_handler, tap_handler_id);
  thread_register_loop_counter (thread_id, &loop_counter);

  DEBUG_NEW (TAPHANDLER, "start main loop on lcore[%d].",
             tap_handler_id);
  if (IS_LTHREAD ())
    DEBUG_NEW (TAPHANDLER, "started as a lthread.");

#if HAVE_LIBURCU_QSBR
  if (! IS_LTHREAD ())
    urcu_qsbr_register_thread ();
#endif /*HAVE_LIBURCU_QSBR*/

  while (! force_quit && ! force_stop[tap_handler_id])
    {
      if (IS_LTHREAD ())
        lthread_sleep (0); // yield.
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
  if (! IS_LTHREAD ())
    urcu_qsbr_unregister_thread ();
#endif /*HAVE_LIBURCU_QSBR*/

  return 0;
}

bool
should_send_to_tap(struct rte_mbuf *m)
{
  return true;
}
