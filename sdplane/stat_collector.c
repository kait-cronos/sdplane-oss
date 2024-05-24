#include <stdio.h>

#include <lthread.h>

//#include <rte_common.h>
#include <rte_ether.h>
#include <rte_ethdev.h>

extern volatile bool force_quit;
extern volatile bool force_stop[RTE_MAX_LCORE];

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

int
stat_collector (__rte_unused void *dummy)
{
  int i, port_id;
  uint16_t nb_ports;

  memset (stats_prev, 0, sizeof (stats_prev));
  memset (stats_current, 0, sizeof (stats_current));
  memset (stats_per_sec, 0, sizeof (stats_per_sec));

  nb_ports = rte_eth_dev_count_avail ();
  unsigned stat_collector_id = rte_lcore_id ();
  while (! force_quit && ! force_stop[stat_collector_id])
    {
      lthread_sleep (1000); // yield.
      //printf ("%s: schedule.\n", __func__);
      for (port_id = 0; port_id < nb_ports; port_id++)
        stats_prev[port_id] = stats_current[port_id];
      for (port_id = 0; port_id < nb_ports; port_id++)
        rte_eth_stats_get (port_id, &stats_current[port_id]);
      for (port_id = 0; port_id < nb_ports; port_id++)
        rte_eth_stats_per_sec (&stats_per_sec[port_id],
                               &stats_current[port_id],
                               &stats_prev[port_id]);
      //printf ("%s: stats collected.\n", __func__);
    }
  return 0;
}

