#ifndef __RIB_H__
#define __RIB_H__

#define MAX_TAP_IF              8
#define MAX_ROUTER_IF           8
#define MAX_VSWITCH_PORTS       16
#define MAX_VSWITCH             4
#define MAX_VSWITCH_LINK        32
#define MAX_VLAN_PER_PORT       8
#define MAX_ETH_PORTS           8
#define MAX_NEIGHBOR_ENTRY_SIZE 1024
#define FDB_SIZE                1024
#define FDB_HASH_MASK           0x3FF
#define FDB_STATE_NONE          0
#define FDB_STATE_ACTIVE        1
#define FDB_AGING_TIME_DEFAULT  300 /* Default aging time: 300 seconds */
#define MAX_NEXTHOP_OBJ_SIZE    1024
#define NEXTHOP_HASH_TAB_SIZE   1024
#define NEXTHOP_HASH_MASK       0x3FF
#define MAX_ECMP_ENTRY          4
#define ROUTE_TREE_SIZE         2
#define K                       2
#define BRANCH_SZ               (1 << K)

#define ETH_LINK_DUPLEX_STR(v)  ((v) ? "full" : "half")
#define ETH_LINK_AUTONEG_STR(v) ((v) ? "on" : "off")
#define ETH_LINK_STATUS_STR(v)  ((v) ? "up" : "down")

#include <rte_ether.h>
#include <rte_ethdev.h>

struct router_if
{
  int sockfd; // tap sockfd.
  uint16_t tap_ring_id;
  struct rte_ring *ring_up;
  struct rte_ring *ring_dn;

  uint16_t vlan_id;
  int ifindex;
  struct rte_ether_addr mac_addr;
  struct in_addr ipv4_addr;
  struct in6_addr ipv6_addr;
  struct in6_addr ll_addr;
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
  uint16_t mtu;
  uint16_t nrxq;
  uint16_t ntxq;
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

/* mapping between address family and neighbor table type */
#define AF_TO_NEIGH_TABLE(af) \
  ((af) == AF_INET ? NEIGH_ARP_TABLE : NEIGH_ND_TABLE)

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
  int ifindex;
};

struct neigh_table
{
  struct neigh_entry entries[MAX_NEIGHBOR_ENTRY_SIZE];
  int num_entries;
};

struct fdb_entry
{
  struct rte_ether_addr l2addr;
  int port;
  uint16_t vlan_id;
  uint8_t state;
  time_t last_seen;
};

#define APPLI_SLOT_SIZE 32
struct application_slot_entry
{
  char *name;
  struct rte_ring *ring;
  bool (*is_packet_match) (struct rte_mbuf *m);
};

enum route_type
{
  ROUTE_TYPE_CONNECTED,
  ROUTE_TYPE_NEXTHOP,
};

struct nh_info
{
  int family; // nexthop address family
  enum route_type type;
  union
  {
    struct in_addr ipv4_addr;
    struct in6_addr ipv6_addr;
  } nh_ip_addr;
  uint32_t oif;
  /**
   * If you need the interface *name* instead of its index (oif),
   * use:
   *     if_indextoname(index, ifname);
   */
};

enum nexthop_object_type
{
  NH_OBJ_TYPE_OBJECT,
  NH_OBJ_TYPE_GROUP,
};

/**
 * legacy: multipath is handled as a single object,
 * and nexthop objects are not reused to compose groups.
 */
struct nh_info_group
{
  int num;
  struct nh_info nh_info_list[MAX_ECMP_ENTRY];
};

struct nh_legacy
{
  int ref_count;
  /**
   * if ref_count reaches zero, the legacy nexthop object
   * is considered unused and can be deleted.
   */
  enum nexthop_object_type type;
  union
  {
    struct nh_info nh_info;      // NH_TYPE_OBJECT
    struct nh_info_group nh_grp; // NH_TYPE_GROUP
  };
};

/**
 * object-capable: multipath is handled as a group object,
 * composed of reusable nexthop objects represented as a list of "IDs".
 */
struct nh_id_group
{
  int num;
  int nh_id_list[MAX_ECMP_ENTRY];
};

struct nh_object
{
  enum nexthop_object_type type;
  union
  {
    struct nh_info nh_info;    // NH_TYPE_OBJECT
    struct nh_id_group nh_grp; // NH_TYPE_GROUP
  };
};

enum nexthop_type
{
  NH_TYPE_LEGACY,
  NH_TYPE_OBJECT_CAP,
};

/* nh_common stored in FIB entries */
struct nh_common
{
  enum nexthop_type nh_type;
  int nh_id;
  /**
   * nh_id's meaning depends on nh_type:
   * - NH_TYPE_LEGACY: index of rib_info->nh_legacy[],
   *   which is internal nhid.
   * - NH_TYPE_OBJECT_CAP: index of rib_info->nh_object[],
   *   which is also used as the kernel's nhid.
   */
};

struct nh_hash_node
{
  int nh_id;
  struct nh_hash_node *next;
};

struct nexthop
{
  struct
  {
    int top;
    struct nh_legacy object[MAX_NEXTHOP_OBJ_SIZE];
  } legacy;
  struct nh_object object[MAX_NEXTHOP_OBJ_SIZE];
};

struct rib_info
{
  uint32_t ver;
  uint8_t tapif_size;
  uint8_t vswitch_size;
  uint8_t vswitch_link_size;
  uint8_t port_size;
  uint8_t lcore_size;
  uint8_t application_slot_size;
  struct vswitch_conf vswitch[MAX_VSWITCH];
  struct vswitch_link vswitch_link[MAX_VSWITCH_LINK];
  struct port_conf port[MAX_ETH_PORTS];
  struct lcore_qconf lcore_qconf[RTE_MAX_LCORE];
  struct neigh_table neigh_tables[NEIGH_NR_TABLES];
  struct fdb_entry fdb[FDB_SIZE];
  struct application_slot_entry application_slot[APPLI_SLOT_SIZE];
  struct nexthop nexthop;
  struct fib_tree *fib_tree[ROUTE_TREE_SIZE];
  struct in6_addr srv6_local_sid_addr;
} __rte_cache_aligned;

EXTERN_COMMAND (show_rib);
EXTERN_COMMAND (show_rib_vswitch);
EXTERN_COMMAND (show_rib_vswitch_link);
EXTERN_COMMAND (show_rib_router_if);
EXTERN_COMMAND (show_rib_capture_if);
EXTERN_COMMAND (show_fdb);
EXTERN_COMMAND (set_vswitch);
EXTERN_COMMAND (set_vswitch_port);
EXTERN_COMMAND (set_vswitch_port_tag_swap);
EXTERN_COMMAND (set_router_if);
EXTERN_COMMAND (set_capture_if);
EXTERN_COMMAND (no_set_vswitch);
EXTERN_COMMAND (no_set_vswitch_port);
EXTERN_COMMAND (no_set_router_if);
EXTERN_COMMAND (no_set_capture_if);

void rib_cmd_init (struct command_set *cmdset);

extern __thread struct rib *rib_tlocal;


#endif /*__RIB_H__*/
