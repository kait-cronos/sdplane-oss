#ifndef __RIB_MANAGER_H__
#define __RIB_MANAGER_H__

#include <rte_ethdev.h>
#include "queue_config.h"

struct rib {
  struct rte_eth_link link[RTE_MAX_ETHPORTS];
  struct sdplane_queue_conf qconf[RTE_MAX_LCORE];
} __rte_cache_aligned;

extern void *rcu_global_ptr_rib;
extern uint64_t rib_rcu_replace;

void rib_manager_recv_message (void *msgp);
void rib_manager_send_message (void *msgp, struct shell *shell);

#endif /*__RIB_MANAGER_H__*/
