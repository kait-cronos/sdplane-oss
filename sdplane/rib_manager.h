// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2023-2026 Yasuhiro Ohara

#ifndef __RIB_MANAGER_H__
#define __RIB_MANAGER_H__

#include <rte_ethdev.h>
#include "queue_config.h"

#include "rib.h"
#include "sdplane.h"

extern struct rib_tree *rib_tree_master[ROUTE_TREE_SIZE];
extern struct hash_table nh_ht;

struct rib
{
  struct rib_info *rib_info;
} __rte_cache_aligned;

extern void *rcu_global_ptr_rib;
extern uint64_t rib_rcu_replace;

extern struct rte_ring *ring_up[RTE_MAX_ETHPORTS][MAX_RX_QUEUE_PER_LCORE];
extern struct rte_ring *ring_dn[RTE_MAX_ETHPORTS][MAX_RX_QUEUE_PER_LCORE];

extern struct rte_ring *router_if_ring_up[MAX_VSWITCH];
extern struct rte_ring *router_if_ring_dn[MAX_VSWITCH];
extern struct rte_ring *capture_if_ring_up[MAX_VSWITCH];
extern struct rte_ring *capture_if_ring_dn[MAX_VSWITCH];

void rib_manager_recv_message (void *msgp);
//void rib_manager_send_message (void *msgp, struct shell *shell);

int fdb_lookup_entry (const struct rib_info *rib_info,
                      const struct rte_ether_addr *mac_addr, uint16_t vlan_id);

#include <sdplane/debug.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include "debug_sdplane.h"

#define rib_send_message(msgp) \
do { \
  if (msg_queue_rib) \
    { \
      DEBUG_SDPLANE_LOG (RIB, "sending message to rib: %p.", (msgp)); \
      if (rte_ring_enqueue (msg_queue_rib, (msgp)) != 0) \
        { \
          WARNING ("rte_ring_enqueue: msg_queue_rib full. " \
                   "message %p lost.", (msgp)); \
          free (msgp); \
        } \
    } \
  else \
    { \
      WARNING ("can't send message to rib: queue: NULL."); \
      free (msgp); \
    } \
} while (0)

#define shell_rib_send_message(msgp, shell) \
do { \
  if (msg_queue_rib) \
    { \
      DEBUG_SDPLANE_LOG (RIB, "sending message to rib: %p.", (msgp)); \
      if (rte_ring_enqueue (msg_queue_rib, (msgp)) != 0) \
        { \
          WARNING ("rte_ring_enqueue: msg_queue_rib full. " \
                   "message %p lost.", (msgp)); \
          free (msgp); \
        } \
    } \
  else if (shell) \
    { \
      fprintf ((shell)->terminal, "can't send message to rib: queue: NULL.%s", \
               (shell)->NL); \
      free (msgp); \
    } \
  else \
    { \
      WARNING ("can't send message to rib: rib_queue: NULL."); \
      free (msgp); \
    } \
} while (0)

#endif /*__RIB_MANAGER_H__*/
