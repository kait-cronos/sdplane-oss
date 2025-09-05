#ifndef __INTERNAL_MESSAGE_H__
#define __INTERNAL_MESSAGE_H__

#include <rte_ring.h>

#include <rte_ethdev.h>
#include <stdint.h>
#include "queue_config.h"
#include "neigh_manager.h"

struct internal_msg_header
{
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

struct internal_msg_route_entry
{
  struct in_addr dst4;
  struct in6_addr dst6;
  struct in_addr gw4;
  struct in6_addr gw6;
  uint32_t oif; // output interface index
  uint32_t prefixlen;
};

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
};

struct internal_msg_neigh_entry
{
  int index; // NEIGH_ARP_TABLE or NEIGH_ND_TABLE.
  int hash;
  struct neigh_entry data;
};

struct internal_msg_fdb_entry
{
  struct rte_ether_addr mac_addr;
  uint16_t vlan_id;
  uint16_t port;
};

void *internal_msg_body (struct internal_msg_header *msgp);

struct internal_msg_header *internal_msg_create (uint16_t type, void *contentp,
                                                 uint32_t content_length);

void internal_msg_delete (struct internal_msg_header *msgp);

int internal_msg_send_to (struct rte_ring *ring,
                          struct internal_msg_header *msgp,
                          struct shell *shell);
struct internal_msg_header *internal_msg_recv (struct rte_ring *ring);

#endif /*__INTERNAL_MESSAGE_H__*/
