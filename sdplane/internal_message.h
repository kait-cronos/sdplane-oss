#ifndef __INTERNAL_MESSAGE_H__
#define __INTERNAL_MESSAGE_H__

#include <rte_ring.h>

#include <rte_ethdev.h>
#include "queue_config.h"

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
#define INTERNAL_MSG_TYPE_VSWITCH_CREATE        8
#define INTERNAL_MSG_TYPE_VSWITCH_DELETE        9
#define INTERNAL_MSG_TYPE_VSWITCH_LINK_CREATE   10
#define INTERNAL_MSG_TYPE_VSWITCH_LINK_DELETE   11

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

struct internal_msg_vswitch_create
{
  uint16_t vlan_id;
};

struct internal_msg_vswitch_delete
{
  uint16_t vswitch_id;
};

struct internal_msg_vswitch_link_create
{
  uint16_t vswitch_id;
  uint16_t port_id;
  uint16_t tag_id;
};

struct internal_msg_vswitch_link_delete
{
  uint16_t vswitch_link_id;
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
