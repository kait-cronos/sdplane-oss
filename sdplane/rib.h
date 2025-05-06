#ifndef __RIB_H__
#define __RIB_H__

#define MAX_TAP_IF 8
#define MAX_ROUTER_IF 8
#define MAX_VSWITCH_PORTS 4
#define MAX_VSWITCH_ID 4
#define MAX_VSWITCH_LINK 32
#define MAX_VLAN_PER_PORT 4
#define MAX_ETH_PORTS 8

struct tap_if_conf
{
  int sockfd; //tap sockfd.
};

struct router_if
{
  int sockfd; //tap sockfd.

  struct in_addr ipv4_addr;
  struct in6_addr ipv6_addr;
};

struct vswitch_link
{
  uint16_t port_id;
  uint16_t vlan_id;
  uint16_t tag_id; //0 indicates untag. tag_id != vlan_id is tag-modify.
  uint16_t vswitch_id;
  uint16_t vswitch_port;
};

struct vswitch_conf
{
  uint16_t vswitch_port_size;
  struct vswitch_link vswitch_port[MAX_VSWITCH_PORTS];
};

struct port_conf
{
  uint16_t nb_rxd;
  uint16_t nb_txd;
  struct rte_eth_link link;
  struct rte_eth_dev_info dev_info;

  /* vlan support. */
  uint16_t vlan_size; //configured vlan size.
  struct vswitch_link vlan_link[MAX_VLAN_PER_PORT];
  struct vswitch_link switch_link; //for untag.
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
  uint8_t vswitch_link_size;
  uint8_t port_size;
  uint8_t lcore_size;
  struct tap_if_conf tap_if[MAX_TAP_IF];
  struct vswitch_conf vswitch[MAX_VSWITCH_ID];
  struct vswitch_link vswitch_link[MAX_VSWITCH_LINK];
  struct port_conf port[MAX_ETH_PORTS];
  struct lcore_qconf lcore_qconf[RTE_MAX_LCORE];
} __rte_cache_aligned;

EXTERN_COMMAND (show_rib);

#endif /*__RIB_H__*/
