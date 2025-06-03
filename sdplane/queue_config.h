#ifndef __QUEUE_CONFIG_H__
#define __QUEUE_CONFIG_H__

struct port_queue_conf
{
  uint16_t port_id;
  uint16_t queue_id;
} __rte_cache_aligned;

#ifndef MAX_RX_QUEUE_PER_LCORE
#define MAX_RX_QUEUE_PER_LCORE 16
#endif
#define MAX_TX_QUEUE_PER_LCORE 1

struct sdplane_queue_conf
{
  uint16_t nrxq;
  struct port_queue_conf rx_queue_list[MAX_RX_QUEUE_PER_LCORE];

  /* unused */
  uint16_t ntxq;
  struct port_queue_conf tx_queue_list[MAX_TX_QUEUE_PER_LCORE];
} __rte_cache_aligned;

extern struct sdplane_queue_conf thread_qconf[RTE_MAX_LCORE];

EXTERN_COMMAND (set_thread_lcore_port_queue);

void queue_config_cmd_init (struct command_set *cmdset);

#endif /*__QUEUE_CONFIG_H__*/
