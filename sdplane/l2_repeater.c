#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

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

#include <zcmdsh/command.h>

#include <zcmdsh/debug_log.h>
#include <zcmdsh/debug_category.h>
#include <zcmdsh/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "l2fwd_export.h"
#include "sdplane.h"
#include "tap_handler.h"

static void
l2_repeat (struct rte_mbuf *m, unsigned rx_portid)
{
  struct rte_eth_dev_tx_buffer *buffer;
  uint16_t nb_ports;
  int tx_portid;
  int sent;

  nb_ports = rte_eth_dev_count_avail ();
  for (tx_portid = 0; tx_portid < nb_ports; tx_portid++)
    {
      if (rx_portid == tx_portid)
        continue;

      buffer = tx_buffer[tx_portid];
      sent = rte_eth_tx_buffer (tx_portid, 0, buffer, m);
      if (sent)
        port_statistics[tx_portid].tx += sent;
    }
}

int
l2_repeater (__rte_unused void *dummy)
{
  struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
  struct rte_mbuf *m;
  int sent;
  unsigned lcore_id;
  uint64_t prev_tsc, diff_tsc, cur_tsc;
  unsigned i, j, portid, nb_rx;
  struct lcore_queue_conf *qconf;
  const uint64_t drain_tsc =
    (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S * BURST_TX_DRAIN_US;
  struct rte_eth_dev_tx_buffer *buffer;

  uint64_t loop_counter = 0;

  prev_tsc = 0;
  lcore_id = rte_lcore_id ();
  qconf = &lcore_queue_conf[lcore_id];

  per_thread_tap_ring_init ();

  if (qconf->n_rx_port == 0)
    {
      DEBUG_SDPLANE_LOG (TAPHANDLER, "lcore %u has nothing to do.",
                         lcore_id);
      return 0;
    }

  DEBUG_SDPLANE_LOG (TAPHANDLER, "entering main loop on lcore %u",
                     lcore_id);

  while (! force_quit && ! force_stop[lcore_id])
    {
      cur_tsc = rte_rdtsc ();

      diff_tsc = cur_tsc - prev_tsc;
      if (unlikely (diff_tsc > drain_tsc))
        {
          for (i = 0; i < qconf->n_rx_port; i++)
            {
              /* l2_repeater flushes(tx) only on its corresponding rx-ports. */
              portid = qconf->rx_port_list[i];
              buffer = tx_buffer[portid];
              sent = rte_eth_tx_buffer_flush (portid, 0, buffer);
              if (sent)
                port_statistics[portid].tx += sent;
            }

          prev_tsc = cur_tsc;
        }

      for (i = 0; i < qconf->n_rx_port; i++)
        {
          portid = qconf->rx_port_list[i];
          nb_rx = rte_eth_rx_burst (portid, 0, pkts_burst,
                                    MAX_PKT_BURST);
          if (unlikely (nb_rx == 0))
            continue;

          port_statistics[portid].rx += nb_rx;

          for (j = 0; j < nb_rx; j++)
            {
              m = pkts_burst[j];
              rte_prefetch0 (rte_pktmbuf_mtod (m, void *));
              if (enable_tap_copy)
                l2fwd_copy_to_tap_ring (m, portid);

              l2_repeat (m, portid);
            }
        }

      loop_counter++;
    }
}

