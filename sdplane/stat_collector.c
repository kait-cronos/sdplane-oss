/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#include <stdio.h>

#include <lthread.h>

// #include <rte_common.h>
#include <rte_ether.h>
#include <rte_ethdev.h>

#include <sdplane/log.h>
#include <sdplane/debug.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "thread_info.h"

extern volatile bool force_quit;
extern volatile bool force_stop[RTE_MAX_LCORE];

extern int lthread_core;

extern uint64_t loop_console;
uint64_t loop_console_prev, loop_console_current, loop_console_pps;

extern uint64_t loop_vty_shell;
uint64_t loop_vty_shell_prev, loop_vty_shell_current, loop_vty_shell_pps;

uint64_t *loop_l2fwd_ptr[RTE_MAX_LCORE] = { NULL };
uint64_t loop_l2fwd_prev[RTE_MAX_LCORE];
uint64_t loop_l2fwd_current[RTE_MAX_LCORE];
uint64_t loop_l2fwd_pps[RTE_MAX_LCORE];

struct rte_eth_stats stats_prev[RTE_MAX_ETHPORTS];
struct rte_eth_stats stats_current[RTE_MAX_ETHPORTS];
struct rte_eth_stats stats_per_sec[RTE_MAX_ETHPORTS];

static inline void
rte_eth_stats_per_sec (struct rte_eth_stats *per_sec,
                       struct rte_eth_stats *stats_current,
                       struct rte_eth_stats *stats_prev)
{
  per_sec->ipackets = stats_current->ipackets - stats_prev->ipackets;
  per_sec->opackets = stats_current->opackets - stats_prev->opackets;
  per_sec->ibytes = stats_current->ibytes - stats_prev->ibytes;
  per_sec->obytes = stats_current->obytes - stats_prev->obytes;
  per_sec->ierrors = stats_current->ierrors - stats_prev->ierrors;
  per_sec->oerrors = stats_current->oerrors - stats_prev->oerrors;
}

static uint64_t loop_stat_collector = 0;

int
stat_collector (__rte_unused void *dummy)
{
  int i, port_id;
  uint16_t nb_ports;

  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);
  DEBUG_SDPLANE_LOG (STAT_COLLECTOR, "%s: started.", __func__);

  int thread_id;
  thread_id = thread_lookup (stat_collector);
  thread_register_loop_counter (thread_id, &loop_stat_collector);

  memset (stats_prev, 0, sizeof (stats_prev));
  memset (stats_current, 0, sizeof (stats_current));
  memset (stats_per_sec, 0, sizeof (stats_per_sec));

  while (! force_quit && ! force_stop[lthread_core])
    {
      lthread_sleep (1000); // yield.
      // printf ("%s: schedule.\n", __func__);
      nb_ports = rte_eth_dev_count_avail ();

      for (port_id = 0; port_id < nb_ports; port_id++)
        stats_prev[port_id] = stats_current[port_id];
      for (port_id = 0; port_id < nb_ports; port_id++)
        rte_eth_stats_get (port_id, &stats_current[port_id]);
      for (port_id = 0; port_id < nb_ports; port_id++)
        rte_eth_stats_per_sec (&stats_per_sec[port_id],
                               &stats_current[port_id], &stats_prev[port_id]);
      // printf ("%s: stats collected.\n", __func__);

      if (FLAG_CHECK (DEBUG_CONFIG (SDPLANE), DEBUG_SDPLANE_STAT_COLLECTOR))
        {
          for (port_id = 0; port_id < nb_ports; port_id++)
            {
              struct rte_eth_stats *s;
              struct rte_eth_link link;
              bool link_status;
              rte_eth_link_get_nowait (port_id, &link);
              link_status = ! ! link.link_status;
              if (! link_status)
                continue;

              s = &stats_per_sec[port_id];
              DEBUG_SDPLANE_LOG (STAT_COLLECTOR,
                                 "port[%d]: "
                                 "pps: in: %'lu out: %'lu "
                                 "bps: in %'lu out: %'lu",
                                 port_id, s->ipackets, s->opackets,
                                 s->ibytes * 8, s->obytes * 8);
#if 0
              log_info ("port[%d]: "
                      "imiss: %'lu ierr: %'lu oerr: %'lu nombuf: %'lu",
                      s->imissed, s->ierrors, s->oerrors, s->rx_nombuf);
#endif
            }
        }

      for (i = 0; i < THREAD_INFO_LIMIT; i++)
        {
          struct thread_counter *tc;
          tc = &thread_counters[i];
          if (! tc->loop_counter_ptr)
            continue;
          tc->prev = tc->current;
          tc->current = *tc->loop_counter_ptr;
          tc->persec = tc->current - tc->prev;
        }

      loop_vty_shell_prev = loop_vty_shell_current;
      loop_vty_shell_current = loop_vty_shell;
      loop_vty_shell_pps = loop_vty_shell_current - loop_vty_shell_prev;

      for (i = 0; i < RTE_MAX_LCORE; i++)
        {
          if (loop_l2fwd_ptr[i])
            {
              loop_l2fwd_prev[i] = loop_l2fwd_current[i];
              loop_l2fwd_current[i] = *loop_l2fwd_ptr[i];
              loop_l2fwd_pps[i] = loop_l2fwd_current[i] - loop_l2fwd_prev[i];
            }
        }

      loop_stat_collector++;
    }

  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);
  return 0;
}
