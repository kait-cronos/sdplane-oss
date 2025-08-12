/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef __STAT_COLLECTOR_H__
#define __STAT_COLLECTOR_H__

extern uint64_t loop_console_prev, loop_console_current, loop_console_pps;
extern uint64_t loop_vty_shell_prev, loop_vty_shell_current,
    loop_vty_shell_pps;

extern uint64_t *loop_l2fwd_ptr[RTE_MAX_LCORE];
extern uint64_t loop_l2fwd_prev[RTE_MAX_LCORE];
extern uint64_t loop_l2fwd_current[RTE_MAX_LCORE];
extern uint64_t loop_l2fwd_pps[RTE_MAX_LCORE];

extern struct rte_eth_stats stats_prev[RTE_MAX_ETHPORTS];
extern struct rte_eth_stats stats_current[RTE_MAX_ETHPORTS];
extern struct rte_eth_stats stats_per_sec[RTE_MAX_ETHPORTS];

#endif /*__STAT_COLLECTOR_H__*/
