#include "include.h"

#include <rte_common.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>

#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

#include <sdplane/command.h>

#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "l2fwd_export.h"
#include "sdplane.h"
#include "tap_handler.h"

#include "rib_manager.h"
#include "thread_info.h"

static __thread unsigned lcore_id;
static __thread struct rib *rib = NULL;

static uint64_t l2_repeat_pkt_copy_failure = 0;

extern struct rte_eth_dev_tx_buffer
    *tx_buffer_per_q[RTE_MAX_ETHPORTS][RTE_MAX_LCORE];

/* l2_repeater_tx_flush() flushes the queue'ed packets
   in tx_buffer_per_q[] onto the NIC. */
static inline __attribute__ ((always_inline)) void
l2_repeater_tx_flush ()
{
  uint16_t nb_ports;
  int tx_portid;
  struct rte_eth_dev_tx_buffer *buffer;
  int sent;
  uint16_t tx_queueid;

  tx_queueid = lcore_id;

  nb_ports = rte_eth_dev_count_avail ();
  for (tx_portid = 0; tx_portid < nb_ports; tx_portid++)
    {
      buffer = tx_buffer_per_q[tx_portid][tx_queueid];
      sent = 0;
      if (buffer)
        {
          sent = rte_eth_tx_buffer_flush (tx_portid, tx_queueid, buffer);

          if (sent || buffer->length)
            DEBUG_SDPLANE_LOG (LINKFLAP_GENERATOR,
                               "lcore[%d]: port %d queue %d flush: "
                               "sent: %d buffer->length: %d",
                               lcore_id, tx_portid, tx_queueid, sent,
                               buffer->length);
        }
      if (sent)
        {
          port_statistics[tx_portid].tx += sent;
        }
    }
}

/* l2_repeater_tx_burst() reads from ring_dn[] and
   tx_bursts it directly to the NIC. */
static inline __attribute__ ((always_inline)) void
l2_repeater_tx_burst ()
{
  struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
  struct rte_mbuf *m;
  unsigned i, nb_rx;
  uint16_t portid, queueid;
  uint16_t tx_queueid;

  tx_queueid = lcore_id;

  if (unlikely (! rib || ! rib->rib_info))
    return;

  struct lcore_qconf *lcore_qconf;
  lcore_qconf = &rib->rib_info->lcore_qconf[lcore_id];
  for (i = 0; i < lcore_qconf->nrxq; i++)
    {
      portid = lcore_qconf->rx_queue_list[i].port_id;
      queueid = lcore_qconf->rx_queue_list[i].queue_id;

      nb_rx = rte_ring_dequeue_burst (
          ring_dn[portid][queueid], (void **) pkts_burst, MAX_PKT_BURST, NULL);

      if (unlikely (nb_rx == 0))
        continue;

      rte_eth_tx_burst (portid, tx_queueid, pkts_burst, nb_rx);
      DEBUG_SDPLANE_LOG (LINKFLAP_GENERATOR,
                         "lcore[%d]: tx_burst: port: %d queue: %d pkts: %d",
                         lcore_id, portid, tx_queueid, nb_rx);
    }
}

/* l2_repeater_tap_up() copies the packet to the tap. */
static inline __attribute__ ((always_inline)) void
l2_repeater_tap_up (struct rte_mbuf *m, unsigned portid, unsigned queueid)
{
  struct rte_mbuf *c;
  uint32_t pkt_len;
  uint16_t data_len;
  int ret;
  pkt_len = rte_pktmbuf_pkt_len (m);
  data_len = rte_pktmbuf_data_len (m);

  DEBUG_SDPLANE_LOG (LINKFLAP_GENERATOR,
                     "lcore[%d]: m: %p port %d queue %d to ring_up: %p",
                     lcore_id, m, portid, queueid, ring_up[portid][queueid]);
  c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
  ret = rte_ring_enqueue (ring_up[portid][queueid], c);
  if (ret)
    {
      if (ret == -ENOBUFS)
        DEBUG_SDPLANE_LOG (LINKFLAP_GENERATOR,
                           "lcore[%d]: m: %p port %d queue %d to ring: "
                           "ENOBUFS: %d",
                           lcore_id, m, portid, queueid, ret);
      else
        DEBUG_SDPLANE_LOG (LINKFLAP_GENERATOR,
                           "lcore[%d]: m: %p port %d queue %d to ring: "
                           "failed: %d",
                           lcore_id, m, portid, queueid, ret);
      rte_pktmbuf_free (c);
    }
#if 0
  else
    DEBUG_SDPLANE_LOG (LINKFLAP_GENERATOR,
                       "lcore[%d]: m: %p port %d queue %d to ring: %d",
                       lcore_id, m, portid, queueid, ret);
#endif
}

static inline __attribute__ ((always_inline)) void
l2_repeat (struct rte_mbuf *m, unsigned rx_portid, unsigned rx_queueid)
{
  struct rte_eth_dev_tx_buffer *buffer;
  uint16_t nb_ports;
  int tx_portid;
  int sent;
  struct rte_mbuf *c;
  uint16_t tx_queueid;

  tx_queueid = lcore_id;

  nb_ports = rte_eth_dev_count_avail ();
  for (tx_portid = 0; tx_portid < nb_ports; tx_portid++)
    {
      if (rx_portid == tx_portid)
        continue;

      if (! rib->rib_info->port[tx_portid].link.link_status)
        continue;

      buffer = tx_buffer_per_q[tx_portid][tx_queueid];
      if (! buffer)
        continue;

      /* copy the packet */
      c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
      if (c)
        {
          /* send the packet-copy */
          sent = rte_eth_tx_buffer (tx_portid, tx_queueid, buffer, c);
          if (sent)
            port_statistics[tx_portid].tx += sent;
        }
      else
        {
          l2_repeat_pkt_copy_failure++;
          DEBUG_LOG_MSG ("lcore[%d]: m: %p port %d -> %d "
                         "rte_pktmbuf_copy() failed.",
                         lcore_id, m, rx_portid, tx_portid);
        }
    }
}

static inline __attribute__ ((always_inline)) void
link_flap (uint16_t port_id)
{
  rte_eth_dev_set_link_down (port_id);
  rte_eth_dev_set_link_up (port_id);
  DEBUG_SDPLANE_LOG (LINKFLAP_GENERATOR,
                     "link down/up on port: %d by lcore %u", port_id,
                     lcore_id);
}

static uint32_t nb_rx_burst = 0;

static inline __attribute__ ((always_inline)) void
l2_repeater_rx_burst ()
{
  struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
  struct rte_mbuf *m;
  unsigned i, j, nb_rx;
  uint16_t portid, queueid;

  if (unlikely (! rib || ! rib->rib_info))
    return;

  struct lcore_qconf *lcore_qconf;
  lcore_qconf = &rib->rib_info->lcore_qconf[lcore_id];
  for (i = 0; i < lcore_qconf->nrxq; i++)
    {
      portid = lcore_qconf->rx_queue_list[i].port_id;
      queueid = lcore_qconf->rx_queue_list[i].queue_id;

      //link_flap (portid);

      nb_rx = rte_eth_rx_burst (portid, queueid, pkts_burst, MAX_PKT_BURST);
      nb_rx_burst++;

      if (unlikely (nb_rx == 0))
        continue;

      port_statistics[portid].rx += nb_rx;

      for (j = 0; j < nb_rx; j++)
        {
          m = pkts_burst[j];
          rte_prefetch0 (rte_pktmbuf_mtod (m, void *));

          l2_repeater_tap_up (m, portid, queueid);
          l2_repeat (m, portid, queueid);

          rte_pktmbuf_free (m);
        }
    }
}

#define LINK_FLAP_INTERVAL_US (1000 * 100) //100ms

static inline __attribute__ ((always_inline)) void
link_flap_once ()
{
  static bool link_status[RTE_MAX_ETHPORTS];
  uint16_t portid, queueid;
  int i;

  struct lcore_qconf *lcore_qconf;
  lcore_qconf = &rib->rib_info->lcore_qconf[lcore_id];
  for (i = 0; i < lcore_qconf->nrxq && i < RTE_MAX_ETHPORTS; i++)
    {
      portid = lcore_qconf->rx_queue_list[i].port_id;
      queueid = lcore_qconf->rx_queue_list[i].queue_id;

      if (link_status[i])
        rte_eth_dev_set_link_up (portid);
      else
        rte_eth_dev_set_link_down (portid);

      DEBUG_SDPLANE_LOG (
          LINKFLAP_GENERATOR,
          "link %s(%d) on port: %d by lcore %u: interval: %'d us",
          (link_status[i] ? "up" : "down"), link_status[i], portid, lcore_id,
          LINK_FLAP_INTERVAL_US);

      link_status[i] ^= true;
    }
}

static __thread uint64_t loop_counter = 0;

int
linkflap_generator (__rte_unused void *dummy)
{
  uint64_t cur_tsc;
  uint64_t prev_tx, diff_tx;
  const uint64_t drain_tx =
      (rte_get_tsc_hz () + US_PER_S - 1) / US_PER_S * BURST_TX_DRAIN_US;
  struct lcore_queue_conf *qconf;

  uint64_t prev_linkflap, diff_linkflap;
  uint64_t interval_linkflap =
      (rte_get_tsc_hz () + US_PER_S - 1) / US_PER_S * LINK_FLAP_INTERVAL_US;

  uint16_t nb_ports;

  /* the tx_buffer_per_q is initialized in rib_manager. */

  prev_tx = 0;
  prev_linkflap = 0;

  lcore_id = rte_lcore_id ();
  qconf = &lcore_queue_conf[lcore_id];

  int thread_id;
  thread_id = thread_lookup_by_lcore (linkflap_generator, lcore_id);
  thread_register_loop_counter (thread_id, &loop_counter);

  DEBUG_SDPLANE_LOG (LINKFLAP_GENERATOR, "entering main loop on lcore %u",
                     lcore_id);

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_register_thread ();
#endif /*HAVE_LIBURCU_QSBR*/

  while (! force_quit && ! force_stop[lcore_id])
    {
      cur_tsc = rte_rdtsc ();

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_lock ();
      rib = (struct rib *) rcu_dereference (rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

      diff_tx = cur_tsc - prev_tx;
      if (unlikely (diff_tx > drain_tx))
        {
          l2_repeater_tx_flush ();
          prev_tx = cur_tsc;
        }

      diff_linkflap = cur_tsc - prev_linkflap;
      if (unlikely (diff_linkflap > interval_linkflap))
        {
          link_flap_once ();
          prev_linkflap = cur_tsc;
        }

      l2_repeater_rx_burst ();
      l2_repeater_tx_burst ();

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_unlock ();
      urcu_qsbr_quiescent_state ();
#endif /*HAVE_LIBURCU_QSBR*/

      loop_counter++;
    }

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_unregister_thread ();
#endif /*HAVE_LIBURCU_QSBR*/
}
