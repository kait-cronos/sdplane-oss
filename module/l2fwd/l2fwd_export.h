#ifndef __L2FWD_EXPORT_H__
#define __L2FWD_EXPORT_H__

//extern volatile bool force_stop[RTE_MAX_LCORE];

#define MAX_PKT_BURST      32
#define BURST_TX_DRAIN_US  100 /* TX drain every ~100us */
#define MEMPOOL_CACHE_SIZE 256

struct port_pair_params
{
#define NUM_PORTS 2
  uint16_t port[NUM_PORTS];
} __rte_cache_aligned;

#define MAX_RX_QUEUE_PER_LCORE 16
#define MAX_TX_QUEUE_PER_PORT  16
/* List of queues to be polled for a given lcore. 8< */
struct lcore_queue_conf
{
  unsigned n_rx_port;
  unsigned rx_port_list[MAX_RX_QUEUE_PER_LCORE];
} __rte_cache_aligned;
extern struct lcore_queue_conf lcore_queue_conf[RTE_MAX_LCORE];
/* >8 End of list of queues to be polled for a given lcore. */

extern uint32_t l2fwd_dst_ports[RTE_MAX_ETHPORTS];
int l2fwd_launch_one_lcore (__rte_unused void *dummy);

extern struct rte_mempool *l2fwd_pktmbuf_pool;

struct l2fwd_port_statistics
{
  uint64_t tx;
  uint64_t rx;
  uint64_t dropped;
} __rte_cache_aligned;
extern struct l2fwd_port_statistics port_statistics[RTE_MAX_ETHPORTS];


extern int mac_updating;
extern int promiscuous_on;
//extern uint16_t nb_rxd;
//extern uint16_t nb_txd;
extern struct rte_ether_addr l2fwd_ports_eth_addr[RTE_MAX_ETHPORTS];
extern uint32_t l2fwd_enabled_port_mask;
extern struct port_pair_params port_pair_params_array[RTE_MAX_ETHPORTS / 2];
extern struct port_pair_params *port_pair_params;
extern uint16_t nb_port_pair_params;
extern unsigned int l2fwd_rx_queue_per_lcore;
extern struct rte_eth_dev_tx_buffer *tx_buffer[RTE_MAX_ETHPORTS];
extern struct rte_eth_conf port_conf;
extern uint64_t timer_period;

int check_port_pair_config (void);
void check_all_ports_link_status (uint32_t port_mask);

int l2fwd_init (int argc, char **argv);
void print_stats (void);

#endif /*__L2FWD_EXPORT_H__*/
