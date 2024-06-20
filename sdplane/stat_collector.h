#ifndef __STAT_COLLECTOR_H__
#define __STAT_COLLECTOR_H__

extern uint64_t loop_console_prev, loop_console_current, loop_console_pps;

extern struct rte_eth_stats stats_prev[RTE_MAX_ETHPORTS];
extern struct rte_eth_stats stats_current[RTE_MAX_ETHPORTS];
extern struct rte_eth_stats stats_per_sec[RTE_MAX_ETHPORTS];

#endif /*__STAT_COLLECTOR_H__*/
