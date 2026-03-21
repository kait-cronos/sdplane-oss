// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2023-2026 Yasuhiro Ohara

#ifndef __INTERNAL_MESSAGE_H__
#define __INTERNAL_MESSAGE_H__

#include <stdint.h>
#include <rte_ring.h>
#include <rte_ethdev.h>
#include <sdplane/shell.h>

#include "queue_config.h"
#include "radix.h"

struct internal_msg_header
{
  struct rte_ring *ring_response;
  uint32_t type;
  uint32_t length; // not including the header size.
};

#define INTERNAL_MSG_TYPE_NONE                  0
#define INTERNAL_MSG_TYPE_QCONF                 1
#define INTERNAL_MSG_TYPE_ETH_LINK              2
#define INTERNAL_MSG_TYPE_PORT_STATUS           3
#define INTERNAL_MSG_TYPE_NETTLP_SEND_DMA_WRITE 4
#define INTERNAL_MSG_TYPE_NETTLP_SEND_DMA_READ  5
#define INTERNAL_MSG_TYPE_QCONF2                6
#define INTERNAL_MSG_TYPE_TXRX_DESC             7
#define INTERNAL_MSG_TYPE_NEIGH_ENTRY_ADD       8
#define INTERNAL_MSG_TYPE_NEIGH_ENTRY_DEL       9
#define INTERNAL_MSG_TYPE_VSWITCH_SET           10
#define INTERNAL_MSG_TYPE_VSWITCH_PORT_SET      11
#define INTERNAL_MSG_TYPE_ROUTER_IF_SET         12
#define INTERNAL_MSG_TYPE_CAPTURE_IF_SET        13
#define INTERNAL_MSG_TYPE_VSWITCH_NO_SET        14
#define INTERNAL_MSG_TYPE_VSWITCH_PORT_NO_SET   15
#define INTERNAL_MSG_TYPE_ROUTER_IF_NO_SET      16
#define INTERNAL_MSG_TYPE_CAPTURE_IF_NO_SET     17
#define INTERNAL_MSG_TYPE_FDB_ENTRY_ADD         18
#define INTERNAL_MSG_TYPE_APPLICATION_SLOT      19
#define INTERNAL_MSG_TYPE_ROUTE_ENTRY_ADD       20
#define INTERNAL_MSG_TYPE_ROUTE_ENTRY_DEL       21
#define INTERNAL_MSG_TYPE_NEXTHOP_ENTRY_ADD     22
#define INTERNAL_MSG_TYPE_NEXTHOP_ENTRY_DEL     23
#define INTERNAL_MSG_TYPE_PORT_GET_REQUEST      24
#define INTERNAL_MSG_TYPE_PORT_GET_RESPONSE     25
#define INTERNAL_MSG_TYPE_CMD_SUCCESS           26
#define INTERNAL_MSG_TYPE_CMD_ERROR             27
#define INTERNAL_MSG_TYPE_MAC_ADDR_ADD          28
#define INTERNAL_MSG_TYPE_MAC_ADDR_DEL          29
#define INTERNAL_MSG_TYPE_IP_ADDR_ADD           30
#define INTERNAL_MSG_TYPE_IP_ADDR_DEL           31
#define INTERNAL_MSG_TYPE_SRV6_LOCAL_SID        32

struct internal_msg_eth_link
{
  struct rte_eth_link link[RTE_MAX_ETHPORTS];
};

struct internal_msg_qconf
{
  struct sdplane_queue_conf qconf[RTE_MAX_LCORE];
};

struct internal_msg_txrx_desc
{
  uint16_t portid;
  uint16_t mtu;
  uint16_t nrxq;
  uint16_t ntxq;
  uint16_t nb_rxd;
  uint16_t nb_txd;
};

struct internal_msg_vswitch
{
  uint16_t vswitch_id;
  uint16_t vlan_id;
};

struct internal_msg_vswitch_port
{
  uint16_t vswitch_id;
  uint16_t port_id;
  uint16_t tag_id;
  bool is_tagged;
};

struct internal_msg_tap_dev
{
  uint16_t vswitch_id;
  char tap_name[16];
  uint16_t vlan_id;
};

struct internal_msg_neigh_entry
{
  int type; // NEIGH_ARP_TABLE or NEIGH_ND_TABLE.
  int pos;
  int num_entries;
  struct neigh_entry data;
};

struct internal_msg_fdb_entry
{
  struct rte_ether_addr mac_addr;
  uint16_t vlan_id;
  uint16_t port;
};

struct internal_msg_nh_member
{
  int kernel_nh_id;
  struct nh_info info;
  uint32_t weight;
};

struct internal_msg_nh_entry
{
  int kernel_nh_id;
  int nhcnt;
  struct internal_msg_nh_member members[MAX_ECMP_ENTRY];
};

struct internal_msg_route_entry
{
  int family;
  int table_id;
  struct route_dst_info dst;
  bool is_nhid;
  struct internal_msg_nh_entry nh;
};

struct internal_msg_port_info
{
  uint16_t port_id;
  struct rte_eth_dev_info dev_info;
  struct rte_eth_link link;
};

struct internal_msg_cmd_success
{
  char message[64];
};

struct internal_msg_cmd_error
{
  char message[64];
};

struct internal_msg_mac_addr
{
  char ifname[16];
  struct rte_ether_addr mac_addr;
};

struct internal_msg_ip_addr
{
  char ifname[16];
  int family;
  bool is_ll_addr;
  union
    {
      struct in_addr ipv4_addr;
      struct in6_addr ipv6_addr;
    } ip_addr;
};

struct internal_msg_srv6_local_sid
{
  struct in6_addr srv6_local_sid_addr;
  uint8_t srv6_local_sid_addr_index;
};

void *internal_msg_body (struct internal_msg_header *msgp);

struct internal_msg_header *internal_msg_create (uint16_t type, void *contentp,
                                                 uint32_t content_length);

void internal_msg_delete (struct internal_msg_header *msgp);

int internal_msg_send_to (struct rte_ring *ring,
                          struct internal_msg_header *msgp,
                          struct shell *shell);
struct internal_msg_header *internal_msg_recv (struct rte_ring *ring);
int internal_msg_recv_burst (struct rte_ring *ring,
                             struct internal_msg_header **msg_table,
                             int size);

#endif /*__INTERNAL_MESSAGE_H__*/
