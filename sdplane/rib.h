/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef __RIB_H__
#define __RIB_H__

#define MAX_TAP_IF              8
#define MAX_ROUTER_IF           8
#define MAX_VSWITCH_PORTS       4
#define MAX_VSWITCH             4
#define MAX_VSWITCH_LINK        32
#define MAX_VLAN_PER_PORT       4
#define MAX_ETH_PORTS           8
#define MAX_NEIGHBOR_TABLE_SIZE 1024

#define ETH_LINK_DUPLEX_STR(v)  ((v) ? "full" : "half")
#define ETH_LINK_AUTONEG_STR(v) ((v) ? "on"   : "off")
#define ETH_LINK_STATUS_STR(v)  ((v) ? "up"   : "down")

#include <rte_ether.h>

struct router_if
{
  int sockfd; // tap sockfd.
  uint16_t tap_ring_id;
  struct rte_ring *ring_up;
  struct rte_ring *ring_dn;

  struct rte_ether_addr mac_addr;
  struct in_addr ipv4_addr;
  struct in6_addr ipv6_addr;
  char tap_name[16];
};

struct capture_if
{
  int sockfd; // tap sockfd.
  uint16_t tap_ring_id;
  struct rte_ring *ring_up;
  struct rte_ring *ring_dn;
  char tap_name[16];
};

struct vswitch_link
{
  uint16_t vswitch_link_id;
  uint16_t port_id;
  // uint16_t queue_id; //queue_id of rx_queue.
  uint16_t vlan_id;
  uint16_t tag_id; // 0 indicates untag. tag_id != vlan_id is tag-modify.
  uint16_t vswitch_id;
  uint16_t vswitch_port;
  bool is_deleted;
};

struct vswitch_conf
{
  uint16_t vswitch_id;
  uint16_t vlan_id;
  uint16_t vswitch_port_size;
  uint16_t vswitch_link_id[MAX_VSWITCH_PORTS];
  struct router_if router_if;
  struct capture_if capture_if;
  bool is_deleted;
};

struct port_conf
{
  uint16_t dpdk_port_id;
  uint16_t nb_rxd;
  uint16_t nb_txd;
  struct rte_eth_link link;
  struct rte_eth_dev_info dev_info;
  bool is_stopped;

  /* vlan support. */
  uint16_t vlan_size; // configured vlan size.
  uint16_t vswitch_link_id_of_vlan[MAX_VLAN_PER_PORT];
  uint16_t vswitch_link_id_of_native_vlan;
};

#include "queue_config.h"

struct lcore_qconf
{
  uint16_t nrxq;
  struct port_queue_conf rx_queue_list[MAX_RX_QUEUE_PER_LCORE];
};

enum
{
  NEIGH_ARP_TABLE = 0,
  NEIGH_ND_TABLE = 1,
  NEIGH_NR_TABLES,
};

extern const int neigh_key_lengths[];

struct neigh_entry
{
  int family;
  union
  {
    struct in_addr ipv4_addr;
    struct in6_addr ipv6_addr;
  } ip_addr;
  struct rte_ether_addr mac_addr;
  uint8_t state;
  // e.g. router_if, state, timer
};

struct neigh_table
{
  struct neigh_entry entries[MAX_NEIGHBOR_TABLE_SIZE];
};

struct rib_info
{
  uint32_t ver;
  uint8_t tapif_size;
  uint8_t vswitch_size;
  uint8_t vswitch_link_size;
  uint8_t port_size;
  uint8_t lcore_size;
  struct vswitch_conf vswitch[MAX_VSWITCH];
  struct vswitch_link vswitch_link[MAX_VSWITCH_LINK];
  struct port_conf port[MAX_ETH_PORTS];
  struct lcore_qconf lcore_qconf[RTE_MAX_LCORE];
  struct neigh_table neigh_tables[NEIGH_NR_TABLES];
} __rte_cache_aligned;

EXTERN_COMMAND (show_rib);
EXTERN_COMMAND (set_vswitch);
EXTERN_COMMAND (delete_vswitch);
EXTERN_COMMAND (show_vswitch_rib);
EXTERN_COMMAND (set_vswitch_link);
EXTERN_COMMAND (delete_vswitch_link);
EXTERN_COMMAND (show_vswitch_link);
EXTERN_COMMAND (set_router_if);
EXTERN_COMMAND (delete_router_if);
EXTERN_COMMAND (show_router_if);
EXTERN_COMMAND (set_capture_if);
EXTERN_COMMAND (delete_capture_if);
EXTERN_COMMAND (show_capture_if);

void rib_cmd_init (struct command_set *cmdset);

extern __thread struct rib *rib_tlocal;


#endif /*__RIB_H__*/
