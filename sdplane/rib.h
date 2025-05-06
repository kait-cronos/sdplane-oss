#ifndef __RIB_H__
#define __RIB_H__

#define MAX_TAPIF_PORTS 16
#define MAX_VSWITCH_PORTS 4
#define MAX_VSWITCH_ID 4

#define MAX_VLAN_PER_PORT 4
#define MAX_ETH_PORTS 8

struct tapif_conf
{
  int sockfd; //tap sockfd.
};

struct switch_port
{
  uint8_t type; // dpdk or tapif
  uint8_t portid;
  uint8_t queueid;
};

struct router_if
{
  int sockfd; //tap sockfd.

  struct in_addr ipv4_addr;
  struct in6_addr ipv6_addr;
};

struct vswitch_conf
{
  uint16_t port_size;
  struct switch_port port[MAX_VSWITCH_PORTS];
};

struct vswitch_link
{
  uint16_t vlan_id;
  uint16_t vswitch_id;
  uint16_t vswitch_port;
};

struct port_conf
{
  uint16_t nb_rxd;
  uint16_t nb_txd;
  struct rte_eth_link link;
  struct rte_eth_dev_info dev_info;

  /* link to upper device. */
  uint16_t type; //l2_vswitch_port or l3_router_if (direct).
  uint16_t router_if;
  struct vswitch_link switch_link; //for untag.

  /* tx/rx vlan config */
  uint16_t vlan_size; //configured vlan size.
  struct vswitch_link vlan_link[MAX_VLAN_PER_PORT];
};

#include "queue_config.h"

struct lcore_qconf
{
  uint16_t nrxq;
  struct port_queue_conf rx_queue_list[MAX_RX_QUEUE_PER_LCORE];
};

struct rib_info {
  uint32_t ver;
  uint8_t tapif_size;
  uint8_t vswitch_size;
  uint8_t port_size;
  uint8_t lcore_size;
  struct tapif_conf tapif[MAX_TAPIF_PORTS];
  struct vswitch_conf vswitch[MAX_VSWITCH_ID];
  struct port_conf port[MAX_ETH_PORTS];
  struct lcore_qconf lcore_qconf[RTE_MAX_LCORE];
} __rte_cache_aligned;

EXTERN_COMMAND (show_rib);

#endif /*__RIB_H__*/
