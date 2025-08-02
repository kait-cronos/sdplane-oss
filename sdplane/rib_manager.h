#ifndef __RIB_MANAGER_H__
#define __RIB_MANAGER_H__

#include <rte_ethdev.h>
#include "queue_config.h"

#include "rib.h"

struct rib
{
#if 0
  uint64_t ver;
  struct rte_eth_link link[RTE_MAX_ETHPORTS];
  struct sdplane_queue_conf qconf[RTE_MAX_LCORE];
#endif
  struct rib_info *rib_info;
} __rte_cache_aligned;

extern void *rcu_global_ptr_rib;
extern uint64_t rib_rcu_replace;

extern struct rte_ring *ring_up[RTE_MAX_ETHPORTS][MAX_RX_QUEUE_PER_LCORE];
extern struct rte_ring *ring_dn[RTE_MAX_ETHPORTS][MAX_RX_QUEUE_PER_LCORE];

extern struct rte_ring *router_if_ring_up[MAX_VSWITCH_ID];
extern struct rte_ring *router_if_ring_dn[MAX_VSWITCH_ID];
extern struct rte_ring *capture_if_ring_up[MAX_VSWITCH_ID];
extern struct rte_ring *capture_if_ring_dn[MAX_VSWITCH_ID];

void rib_manager_recv_message (void *msgp);
void rib_manager_send_message (void *msgp, struct shell *shell);

#endif /*__RIB_MANAGER_H__*/
