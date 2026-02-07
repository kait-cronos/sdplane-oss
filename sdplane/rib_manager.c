#include "include.h"

#include <linux/if.h>
#include <linux/if_tun.h>

#include <lthread.h>

#include <rte_common.h>
#include <rte_launch.h>
#include <rte_ether.h>
#include <rte_malloc.h>

#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>

#include <sdplane/debug.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "rib_manager.h"
#include "sdplane.h"
#include "thread_info.h"
#include "tap.h"
#include "l3_tap_handler.h"

#include "l2fwd_export.h"

#include "internal_message.h"

#include "radix.h"
#include "fib.h"

#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

struct rte_ring *ring_up[RTE_MAX_ETHPORTS][MAX_RX_QUEUE_PER_LCORE];
struct rte_ring *ring_dn[RTE_MAX_ETHPORTS][MAX_RX_QUEUE_PER_LCORE];

struct rte_ring *router_if_ring_up[MAX_VSWITCH];
struct rte_ring *router_if_ring_dn[MAX_VSWITCH];
struct rte_ring *capture_if_ring_up[MAX_VSWITCH];
struct rte_ring *capture_if_ring_dn[MAX_VSWITCH];

struct rte_eth_dev_tx_buffer *tx_buffer_per_q[RTE_MAX_ETHPORTS][RTE_MAX_LCORE];

extern int lthread_core;
extern volatile bool force_stop[RTE_MAX_LCORE];

struct rte_ring *msg_queue_rib;

void *rcu_global_ptr_rib;
uint64_t rib_rcu_replace = 0;

static __thread struct rib *rib = NULL;

struct rib_tree *rib_tree_master[ROUTE_TREE_SIZE];
static int rib_tree_size;
struct nh_hash_node *nh_hash_table[NEXTHOP_HASH_TAB_SIZE];

static inline __attribute__ ((always_inline)) int
router_if_delete (struct rib_info *rib_info, uint16_t vswitch_id);
static inline __attribute__ ((always_inline)) int
capture_if_delete (struct rib_info *rib_info, uint16_t vswitch_id);

/*
 * common
 */

static inline __attribute__ ((always_inline)) uint32_t
jenkins_hash (uint8_t *key, int key_len)
{
  int i;
  uint32_t hash = 0;

  hash = 0;
  for (i = 0; i < key_len; i++)
    {
      hash += key[i];
      hash += hash << 10;
      hash ^= hash >> 6;
    }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;

  return hash;
}

/*
 * vswitch
 */

static inline __attribute__ ((always_inline)) struct vswitch_conf *
vswitch_lookup (struct rib_info *new, uint16_t vswitch_id)
{
  int i;
  for (i = 0; i < new->vswitch_size; i++)
    {
      if (new->vswitch[i].vswitch_id == vswitch_id &&
          ! new->vswitch[i].is_deleted)
        return &new->vswitch[i];
    }
  return NULL;
}

static inline __attribute__ ((always_inline)) struct vswitch_conf *
vswitch_new (struct rib_info *new, uint16_t vswitch_id, uint16_t vlan_id)
{
  int i;
  struct vswitch_conf *vswitch;
  bool is_slot_found = false;

  for (i = 0; i < new->vswitch_size; i++)
    {
      if (new->vswitch[i].is_deleted)
        {
          is_slot_found = true;
          memset (&new->vswitch[i], 0, sizeof (struct vswitch_conf));
          vswitch = &new->vswitch[i];
          break;
        }
    }

  if (! is_slot_found)
    {
      if (new->vswitch_size >= MAX_VSWITCH)
        return NULL;
      vswitch = &new->vswitch[new->vswitch_size];
      new->vswitch_size++;
    }

  vswitch->vswitch_id = vswitch_id;
  vswitch->vlan_id = vlan_id;
  vswitch->router_if.sockfd = -1;
  vswitch->capture_if.sockfd = -1;
  vswitch->router_if.tap_ring_id = -1;
  vswitch->router_if.vlan_id = vlan_id;
  vswitch->capture_if.tap_ring_id = -1;
  vswitch->is_deleted = false;
  return vswitch;
}

static inline __attribute__ ((always_inline)) struct vswitch_link *
vswitch_link_lookup (struct rib_info *new, struct vswitch_conf *vswitch,
                     struct port_conf *port)
{
  int i;
  struct vswitch_link *vswitch_link;
  for (i = 0; i < new->vswitch_link_size; i++)
    {
      vswitch_link = &new->vswitch_link[i];
      if (vswitch_link->vswitch_id == vswitch->vswitch_id &&
          vswitch_link->port_id == port->dpdk_port_id &&
          ! vswitch_link->is_deleted)
        return vswitch_link;
    }
  return NULL;
}

static inline __attribute__ ((always_inline)) struct vswitch_link *
vswitch_link_new (struct rib_info *new, struct vswitch_conf *vswitch,
                  struct port_conf *port)
{
  uint16_t vswitch_link_id;
  struct vswitch_link *vswitch_link;
  bool is_slot_found = false;

  for (vswitch_link_id = 0; vswitch_link_id < new->vswitch_link_size;
       vswitch_link_id++)
    {
      if (new->vswitch_link[vswitch_link_id].is_deleted)
        {
          is_slot_found = true;
          memset (&new->vswitch_link[vswitch_link_id], 0,
                  sizeof (struct vswitch_link));
          break;
        }
    }

  if (! is_slot_found)
    {
      if (new->vswitch_link_size >= MAX_VSWITCH_LINK)
        return NULL;
      vswitch_link_id = new->vswitch_link_size++;
    }

  vswitch_link = &new->vswitch_link[vswitch_link_id];
  vswitch_link->vswitch_link_id = vswitch_link_id;
  vswitch_link->port_id = port->dpdk_port_id;
  vswitch_link->vlan_id = vswitch->vlan_id;
  vswitch_link->tag_id = vswitch->vlan_id; // tagged.
  vswitch_link->vswitch_id = vswitch->vswitch_id;
  vswitch_link->is_deleted = false;

  uint16_t vswitch_port;
  if (vswitch->vswitch_port_size < MAX_VSWITCH_PORTS)
    {
      vswitch_port = vswitch->vswitch_port_size++;
      vswitch->vswitch_link_id[vswitch_port] = vswitch_link_id;
      vswitch_link->vswitch_port = vswitch_port;
    }

  return vswitch_link;
}

static inline __attribute__ ((always_inline)) void
vswitch_link_remove_from_port_vlan_array (struct port_conf *port,
                                          uint16_t vswitch_link_id)
{
  int i;
  for (i = 0; i < port->vlan_size; i++)
    {
      if (port->vswitch_link_id_of_vlan[i] == vswitch_link_id)
        {
          memmove (&port->vswitch_link_id_of_vlan[i],
                   &port->vswitch_link_id_of_vlan[i + 1],
                   (port->vlan_size - i - 1) * sizeof (uint16_t));
          port->vlan_size--;
          break;
        }
      else if (port->vswitch_link_id_of_native_vlan == vswitch_link_id)
        {
          port->vswitch_link_id_of_native_vlan = -1;
          break;
        }
    }
}

static inline __attribute__ ((always_inline)) void
vswitch_link_remove_from_vswitch_port_array (struct vswitch_conf *vswitch,
                                             uint16_t vswitch_link_id)
{
  int i;
  for (i = 0; i < vswitch->vswitch_port_size; i++)
    {
      if (vswitch->vswitch_link_id[i] == vswitch_link_id)
        {
          memmove (&vswitch->vswitch_link_id[i],
                   &vswitch->vswitch_link_id[i + 1],
                   (vswitch->vswitch_port_size - i - 1) * sizeof (uint16_t));
          vswitch->vswitch_port_size--;
        }
    }
}

static inline __attribute__ ((always_inline)) int
vswitch_link_delete (struct rib_info *rib_info, uint16_t vswitch_link_id)
{
  int i;
  uint16_t vswitch_id = rib_info->vswitch_link[vswitch_link_id].vswitch_id;
  uint16_t port_id = rib_info->vswitch_link[vswitch_link_id].port_id;

  vswitch_link_remove_from_vswitch_port_array (
      vswitch_lookup (rib_info, vswitch_id), vswitch_link_id);

  vswitch_link_remove_from_port_vlan_array (&rib_info->port[port_id],
                                            vswitch_link_id);

  for (i = vswitch_link_id; i < rib_info->vswitch_link_size; i++)
    {
      if (rib_info->vswitch_link[i].vswitch_id == vswitch_id)
        rib_info->vswitch_link[i].vswitch_port--;
    }

  rib_info->vswitch_link[vswitch_link_id].is_deleted = true;
  DEBUG_SDPLANE_LOG (RIB, "delete: link_id: %u", vswitch_link_id);
  return 0;
}

static inline __attribute__ ((always_inline)) int
vswitch_delete (struct rib_info *rib_info, uint16_t vswitch_id)
{
  struct vswitch_conf *vswitch = vswitch_lookup (rib_info, vswitch_id);
  if (! vswitch)
    {
      DEBUG_SDPLANE_LOG (RIB, "delete: vswitch: %u not found", vswitch_id);
      return 0;
    }

  /* delete vswitch's router_if and capture_if */
  if (vswitch->router_if.sockfd >= 0)
    router_if_delete (rib_info, vswitch_id);
  if (vswitch->capture_if.sockfd >= 0)
    capture_if_delete (rib_info, vswitch_id);

  /* delete all vswitch's vswitch_links from port_conf */
  while (vswitch->vswitch_port_size > 0)
    vswitch_link_delete (rib_info, vswitch->vswitch_link_id[0]);

  vswitch->is_deleted = true;
  DEBUG_SDPLANE_LOG (RIB, "delete: vswitch: %u", vswitch_id);
  return 0;
}

/*
 * router_if
 */

static inline __attribute__ ((always_inline)) int
router_if_delete (struct rib_info *rib_info, uint16_t vswitch_id)
{
  struct vswitch_conf *vswitch = vswitch_lookup (rib_info, vswitch_id);
  struct router_if *rif = &vswitch->router_if;

  if (rif->sockfd >= 0)
    {
      close (rif->sockfd);
      rif->sockfd = -1;
    }

  if (router_if_ring_up[vswitch_id])
    {
      rte_ring_free (router_if_ring_up[vswitch_id]);
      router_if_ring_up[vswitch_id] = NULL;
    }
  if (router_if_ring_dn[vswitch_id])
    {
      rte_ring_free (router_if_ring_dn[vswitch_id]);
      router_if_ring_dn[vswitch_id] = NULL;
    }

  memset (rif, 0, sizeof (struct router_if));
  rif->sockfd = -1;
  rif->tap_ring_id = -1;
  rif->ifindex = -1;
  DEBUG_SDPLANE_LOG (RIB, "delete: router_if: vswitch %u", vswitch_id);
  return 0;
}

/*
 * capture_if
 */

static inline __attribute__ ((always_inline)) int
capture_if_delete (struct rib_info *rib_info, uint16_t vswitch_id)
{
  struct vswitch_conf *vswitch = vswitch_lookup (rib_info, vswitch_id);
  struct capture_if *cif = &vswitch->capture_if;

  if (cif->sockfd >= 0)
    {
      close (cif->sockfd);
      cif->sockfd = -1;
    }

  if (capture_if_ring_up[vswitch_id])
    {
      rte_ring_free (capture_if_ring_up[vswitch_id]);
      capture_if_ring_up[vswitch_id] = NULL;
    }
  if (capture_if_ring_dn[vswitch_id])
    {
      rte_ring_free (capture_if_ring_dn[vswitch_id]);
      capture_if_ring_dn[vswitch_id] = NULL;
    }

  memset (cif, 0, sizeof (struct capture_if));
  cif->sockfd = -1;
  cif->tap_ring_id = -1;
  DEBUG_SDPLANE_LOG (RIB, "delete: capture_if: vswitch %u", vswitch_id);
  return 0;
}

/*
 * port
 */

static inline __attribute__ ((always_inline)) void
port_set_native_vlan (struct rib_info *new, struct port_conf *port,
                      struct vswitch_link *vswitch_link)
{
  vswitch_link->tag_id = 0;
  port->vswitch_link_id_of_native_vlan = vswitch_link->vswitch_link_id;
}

static inline __attribute__ ((always_inline)) void
port_add_tagged_vlan (struct rib_info *new, struct port_conf *port,
                      struct vswitch_link *vswitch_link)
{
  if (port->vlan_size >= MAX_VLAN_PER_PORT)
    return;
  uint16_t index = port->vlan_size++;
  port->vswitch_link_id_of_vlan[index] = vswitch_link->vswitch_link_id;
}

int
port_qconf_compare (const void *a, const void *b)
{
  struct port_queue_conf *pa = (struct port_queue_conf *) a;
  struct port_queue_conf *pb = (struct port_queue_conf *) b;
  if (pa->port_id < pb->port_id)
    return -1;
  else if (pa->port_id > pb->port_id)
    return 1;
  else if (pa->queue_id < pb->queue_id)
    return -1;
  else if (pa->queue_id > pb->queue_id)
    return 1;
  return 0;
}

/*
 * fdb
 */

static inline __attribute__ ((always_inline)) uint32_t
fdb_jenkins_hash (const struct rte_ether_addr *mac_addr, uint16_t vlan_id)
{
  uint8_t data[RTE_ETHER_ADDR_LEN + sizeof (uint16_t)];

  // Copy MAC address
  memcpy (data, mac_addr->addr_bytes, RTE_ETHER_ADDR_LEN);

  // Copy VLAN ID in network byte order for consistent hashing
  uint16_t vlan_be = rte_cpu_to_be_16 (vlan_id);
  memcpy (data + RTE_ETHER_ADDR_LEN, &vlan_be, sizeof (uint16_t));

  return jenkins_hash (data, sizeof (data)) & FDB_HASH_MASK;
}

static inline __attribute__ ((always_inline)) int
fdb_add_entry (struct rib_info *rib_info,
               const struct rte_ether_addr *mac_addr, uint16_t vlan_id,
               int port)
{
  uint32_t hash, offset;
  time_t current_time = time (NULL);

  hash = fdb_jenkins_hash (mac_addr, vlan_id);
  offset = hash;

  while (rib_info->fdb[offset].state != FDB_STATE_NONE)
    {
      if (rte_is_same_ether_addr (&rib_info->fdb[offset].l2addr, mac_addr) &&
          rib_info->fdb[offset].vlan_id == vlan_id)
        {
          rib_info->fdb[offset].port = port;
          rib_info->fdb[offset].last_seen = current_time;
          rib_info->fdb[offset].state = FDB_STATE_ACTIVE;
          DEBUG_SDPLANE_LOG (FDB, "updated fdb[%u]: port %d", offset, port);
          return offset;
        }

      ++offset;
      if (offset >= FDB_SIZE)
        offset = 0;
      if (offset == hash)
        {
          DEBUG_SDPLANE_LOG (FDB, "fdb table is full.");
          return -1;
        }
    }

  rib_info->fdb[offset].l2addr = *mac_addr;
  rib_info->fdb[offset].port = port;
  rib_info->fdb[offset].vlan_id = vlan_id;
  rib_info->fdb[offset].last_seen = current_time;
  rib_info->fdb[offset].state = FDB_STATE_ACTIVE;

  char mac_str[32];
  rte_ether_format_addr (mac_str, sizeof (mac_str), mac_addr);
  DEBUG_SDPLANE_LOG (FDB, "added fdb[%u]: %s vlan:%u port:%d", offset, mac_str,
                     vlan_id, port);
  return offset;
}

int
fdb_lookup_entry (const struct rib_info *rib_info,
                  const struct rte_ether_addr *mac_addr, uint16_t vlan_id)
{
  uint32_t hash, offset;

  hash = fdb_jenkins_hash (mac_addr, vlan_id);
  offset = hash;

  char mac_str[32];
  rte_ether_format_addr (mac_str, sizeof (mac_str), mac_addr);
  DEBUG_NEW (FDB, "fdb_lookup: %s vlan: %u offset: %u.",
             mac_str, vlan_id, offset);

  //while (rib_info->fdb[offset].state != FDB_STATE_NONE)
  while (1)
    {
      //DEBUG_NEW (FDB, "check fdb[%u].", offset);
      if (rte_is_same_ether_addr (&rib_info->fdb[offset].l2addr, mac_addr) &&
          rib_info->fdb[offset].vlan_id == vlan_id)
        {
          if (rib_info->fdb[offset].state == FDB_STATE_ACTIVE)
            {
              DEBUG_SDPLANE_LOG (FDB, "lookup hit at fdb[%u]: port %d", offset,
                                 rib_info->fdb[offset].port);
              return rib_info->fdb[offset].port;
            }
        }

      ++offset;
      if (offset >= FDB_SIZE)
        offset = 0;
      if (offset == hash)
        break;
    }

  DEBUG_SDPLANE_LOG (FDB, "lookup missed");
  return -1;
}

static inline __attribute__ ((always_inline)) void
fdb_aging_process (struct rib_info *rib_info, time_t max_age)
{
  int i;
  time_t current_time = time (NULL);

  for (i = 0; i < FDB_SIZE; i++)
    {
      if (rib_info->fdb[i].state == FDB_STATE_ACTIVE &&
          current_time - rib_info->fdb[i].last_seen > max_age)
        {
          char mac_str[32];
          rte_ether_format_addr (mac_str, sizeof (mac_str),
                                 &rib_info->fdb[i].l2addr);
          DEBUG_SDPLANE_LOG (FDB, "aged out fdb[%d]: %s", i, mac_str);
          rib_info->fdb[i].state = FDB_STATE_NONE;
        }
    }
}

/*
 * route_table
 */

static inline __attribute__ ((always_inline)) int
route_table_lookup_id (int table_id, int family)
{
  int i;

  for (i = 0; i < ROUTE_TREE_SIZE; i++)
    {
      if (rib_tree_master[i] == NULL)
        continue;
      if (rib_tree_master[i]->family == family &&
          rib_tree_master[i]->table_id == table_id)
        return i;
    }
  return -1;
}

static inline __attribute__ ((always_inline)) int
route_table_alloc_slot (int table_id, int family)
{
  int i;

  for (i = 0; i < ROUTE_TREE_SIZE; i++)
    {
      if (rib_tree_master[i] == NULL)
        {
          rib_tree_master[i] = rib_new (rib_tree_master[i]);
          if (! rib_tree_master[i])
            return -1;
          rib_tree_master[i]->family = family;
          rib_tree_master[i]->table_id = table_id;
          return i;
        }
    }
  return -1;
}

/*
 * nexthop
 */

static inline __attribute__ ((always_inline)) uint32_t
nexthop_legacy_object_jenkins_hash (struct nh_info *nh_info)
{
  uint8_t data[sizeof (nh_info->family) +
               sizeof (nh_info->nh_ip_addr) +
               sizeof (nh_info->oif)];
  memset (data, 0, sizeof (data));
  uint8_t *ptr = data;

  // copy family
  memcpy (ptr, &nh_info->family, sizeof (nh_info->family));
  ptr += sizeof (nh_info->family);
  // copy nexthop IP address
  memcpy (ptr, &nh_info->nh_ip_addr, sizeof (nh_info->nh_ip_addr));
  ptr += sizeof (nh_info->nh_ip_addr);
  // copy oif
  uint32_t oif_be = rte_cpu_to_be_32 (nh_info->oif);
  memcpy (ptr, &oif_be, sizeof (oif_be));

  return jenkins_hash (data, sizeof (data)) & NEXTHOP_HASH_MASK;
}

static inline __attribute__ ((always_inline)) uint32_t
nexthop_legacy_group_jenkins_hash (struct nh_info_group *nhg)
{
  uint32_t hash, combined_hash;
  int i;
  hash = combined_hash = 0;

  for (i = 0; i < nhg->num; i++)
    {
      hash = nexthop_legacy_object_jenkins_hash (&nhg->nh_info_list[i]);
      combined_hash ^= hash;
      combined_hash += combined_hash << 1;
    }

  return combined_hash & NEXTHOP_HASH_MASK;
}

// nh is a pointer to struct nh_info or struct nh_info_group
static inline __attribute__ ((always_inline)) uint32_t
nexthop_legacy_jenkins_hash (enum nexthop_object_type nh_type, void *nh)
{
  switch (nh_type)
    {
      case NH_OBJ_TYPE_OBJECT:
        return nexthop_legacy_object_jenkins_hash((struct nh_info *)nh);

      case NH_OBJ_TYPE_GROUP:
        return nexthop_legacy_group_jenkins_hash((struct nh_info_group *)nh);

      default:
        return 0;
    }
}

static inline __attribute__ ((always_inline)) struct nh_hash_node *
nexthop_legacy_alloc_hash_node (void)
{
  struct nh_hash_node *new;

  new = calloc (1, sizeof (struct nh_hash_node));
  new->nh_id = -1;
  new->next = NULL;

  return new;
}

static inline __attribute__ ((always_inline)) int
nexthop_legacy_remove_hash_node (struct nh_hash_node **head,
                                 struct nh_hash_node *target)
{
  struct nh_hash_node *prev = NULL, *cur;

  if (! head || ! target)
    return -1;

  cur = *head;
  while (cur)
    {
      if (cur == target)
        {
          if (prev)
            prev->next = cur->next;
          else
            *head = cur->next;

          free(cur);
          return 0;
        }
      prev = cur;
      cur = cur->next;
    }
  return -1; /* not found */
}

static inline __attribute__ ((always_inline)) int
nexthop_legacy_cleanup_hash_table (struct nh_hash_node **nh_hash_table)
{
  int i;
  struct nh_hash_node *current, *next;

  for (i = 0; i < NEXTHOP_HASH_TAB_SIZE; i++)
    {
      current = nh_hash_table[i];
      while (current != NULL)
        {
          next = current->next;
          free (current);
          current = next;
        }
      nh_hash_table[i] = NULL;
    }

  return 0;
}

static inline __attribute__ ((always_inline)) int
nexthop_legacy_create_object (struct rib_info *rib_info, const struct nh_legacy *new)
{
  int start, i;

  start = rib_info->nexthop.legacy.top;
  if (start < 0)
    {
      DEBUG_NEW (RIB, "nexthop legacy object limit exceeded");
      return -1;
    }

  /* store object at 'start' */
  rib_info->nexthop.legacy.object[start] = *new;
  rib_info->nexthop.legacy.object[start].ref_count = 1;

  /* find next free slot (ref_count == 0); mark full with -1 */
  i = start;
  do
    {
      i++;
      if (i == MAX_NEXTHOP_OBJ_SIZE)
        i = 0;
      if (i == start)
        {
          rib_info->nexthop.legacy.top = -1; /* no slot available */
          return start;
        }
    }
  while (rib_info->nexthop.legacy.object[i].ref_count != 0);

  rib_info->nexthop.legacy.top = i;
  return start;
}

// Note:
// nh_info.nh_ip_addr must be zero-initialized before use.
// this function compares the entire nh_ip_addr union.
static inline __attribute__ ((always_inline)) int
nexthop_legacy_compare_object (struct nh_legacy *a, struct nh_legacy *b)
{
  int i;

  /* null check */
  if (! a || ! b)
    return 0;

  /* compare type */
  if (a->type != b->type)
    return 0;

  /* compare object */
  switch (a->type)
    {
      case NH_OBJ_TYPE_OBJECT:
        if (a->nh_info.family != b->nh_info.family ||
            memcmp (&a->nh_info.nh_ip_addr, &b->nh_info.nh_ip_addr,
                    sizeof (a->nh_info.nh_ip_addr)) != 0 ||
            a->nh_info.oif != b->nh_info.oif)
          return 0;
        break;
      case NH_OBJ_TYPE_GROUP:
        if (a->nh_grp.num != b->nh_grp.num)
          return 0;
        /*
         * compare multipath nexthop groups in an order-sensitive manner.
         * groups with the same members but different ordering
         * are treated as distinct objects.
         */
        for (i = 0; i < a->nh_grp.num; i++)
          {
            if (a->nh_grp.nh_info_list[i].family != b->nh_grp.nh_info_list[i].family ||
                memcmp (&a->nh_grp.nh_info_list[i].nh_ip_addr,
                        &b->nh_grp.nh_info_list[i].nh_ip_addr,
                        sizeof (a->nh_grp.nh_info_list[i].nh_ip_addr)) != 0 ||
                a->nh_grp.nh_info_list[i].oif != b->nh_grp.nh_info_list[i].oif)
              return 0;
          }
        break;
      default:
        return 0;
    }

  return 1; // treat equal objects
}

static inline __attribute__ ((always_inline)) int
nexthop_common_add_entry (struct rib_info *rib_info, enum nexthop_type nh_type, void *nh)
{
  int nh_id; // nexthop ID for FIB
  int pos; // hash table index

  switch (nh_type)
    {
      case NH_TYPE_LEGACY:
        struct nh_legacy *nh_legacy = (struct nh_legacy *) nh;
        pos = nexthop_legacy_jenkins_hash (nh_legacy->type, nh);
        /* register nexthop object */
        if (nh_hash_table[pos] == NULL)
          {
            nh_hash_table[pos] = nexthop_legacy_alloc_hash_node ();
            nh_id = nexthop_legacy_create_object (rib_info, nh_legacy);
            nh_hash_table[pos]->nh_id = nh_id;
          }
        else
          {
            /* conflict: search existing nexthop objects */
            struct nh_hash_node *current = nh_hash_table[pos];
            while (current != NULL)
              {
                if (nexthop_legacy_compare_object (
                      &rib_info->nexthop.legacy.object[current->nh_id],
                      nh_legacy))
                  {
                    /* found existing nexthop */
                    rib_info->nexthop.legacy.object[current->nh_id].ref_count++;
                    nh_id = current->nh_id;
                    return nh_id;
                  }
                current = current->next;
              }
            /* not found: register new nexthop */
            struct nh_hash_node *new = nexthop_legacy_alloc_hash_node ();
            nh_id = nexthop_legacy_create_object (rib_info, nh_legacy);
            new->nh_id = nh_id;
            new->next = nh_hash_table[pos];
            nh_hash_table[pos] = new;
          }
        DEBUG_NEW (RIB, "nexthop legacy added: index=%d, nh_id=%d", pos, nh_id);
        break;

      case NH_TYPE_OBJECT_CAP:
        struct nh_object *nh_object = (struct nh_object *) nh;
        DEBUG_NEW (RIB, "not implemented yet");
        nh_id = -1;
        break;

      default:
        return -1;
    }

  return nh_id;
}

static inline __attribute__ ((always_inline)) int
nexthop_common_del_entry (struct rib_info *rib_info, enum nexthop_type nh_type, void *nh)
{
  int nh_id; // nexthop ID for FIB
  int pos; // hash table index

  switch (nh_type)
    {
      case NH_TYPE_LEGACY:
        struct nh_legacy *nh_legacy = (struct nh_legacy *) nh;
        pos = nexthop_legacy_jenkins_hash (nh_legacy->type, nh);
        /* nexthop object is not found */
        if (nh_hash_table[pos] == NULL)
          {
            DEBUG_NEW (RIB, "nexthop legacy not found in hash table");
            return -1;
          }
        /* search existing nexthop objects */
        struct nh_hash_node **head = &nh_hash_table[pos];
        struct nh_hash_node *current  = *head;

        while (current != NULL)
          {
            if (nexthop_legacy_compare_object(
                  &rib_info->nexthop.legacy.object[current->nh_id],
                  nh_legacy))
              {
                nh_id = current->nh_id;

                /* found existing nexthop (release one reference) */
                rib_info->nexthop.legacy.object[nh_id].ref_count--;

                /* delete hash node (unlink + free) */
                if (rib_info->nexthop.legacy.object[nh_id].ref_count == 0)
                  nexthop_legacy_remove_hash_node(head, current);

                return nh_id;
              }

            current = current->next;
          }
        break;

      case NH_TYPE_OBJECT_CAP:
        struct nh_object *nh_object = (struct nh_object *) nh;
        DEBUG_NEW (RIB, "not implemented yet");
        nh_id = -1;
        break;

      default:
        return -1;
    }

  return nh_id;
}

// nh is a pointer to struct nh_legacy or struct nh_object
static inline __attribute__ ((always_inline)) char *
nexthop_common_format_object (enum nexthop_type nh_type, void *nh,
                             char *buf, size_t buf_size)
{
  int i, len = 0;
  char nh_ip_str[INET6_ADDRSTRLEN] = { 0 };

  switch (nh_type)
    {
      case NH_TYPE_LEGACY:
        struct nh_legacy *nh_legacy = (struct nh_legacy *) nh;
        switch (nh_legacy->type)
          {
            case NH_OBJ_TYPE_OBJECT:
              struct nh_info *nh_info = &nh_legacy->nh_info;

              inet_ntop (nh_info->family, &nh_info->nh_ip_addr,
                         nh_ip_str, sizeof (nh_ip_str));
              len += snprintf (buf + len, buf_size - len,
                               "%s(oif=%d)",
                               nh_ip_str, nh_info->oif);
              break;

            case NH_OBJ_TYPE_GROUP:
              struct nh_info_group *nhg = &nh_legacy->nh_grp;

              len += snprintf (buf + len, buf_size - len, "[");
              for (i = 0; i < nhg->num; i++)
                {
                  inet_ntop (nhg->nh_info_list[i].family,
                             &nhg->nh_info_list[i].nh_ip_addr,
                             nh_ip_str, sizeof (nh_ip_str));
                  len += snprintf (buf + len, buf_size - len,
                                   "%s(oif=%d)%s",
                                   nh_ip_str,
                                   nhg->nh_info_list[i].oif,
                                   (i == nhg->num - 1) ? "" : ", ");
                }
              snprintf (buf + len, buf_size - len, "]");
              break;

            default:
              return buf;
          }
        break;

      case NH_TYPE_OBJECT_CAP:
        int *nh_id = (int *) nh;
        len += snprintf (buf + len, buf_size - len, "nhid=%d", *nh_id);
        break;

      default:
        return buf;
    }

  return buf;
}

/*
 * rib and rib_info
 */

static inline __attribute__ ((always_inline)) struct rib_info *
rib_info_create (struct rib_info *old)
{
  struct rib_info *new;
  int i;

  /* allocate new */
  new = malloc (sizeof (struct rib_info));
  if (! new)
    return NULL;

  if (! old)
    {
      memset (new, 0, sizeof (struct rib_info));
      for (i = 0; i < ROUTE_TREE_SIZE; i++)
        new->fib_tree[i] = fib_new (NULL);
    }
  else
    {
      memcpy (new, old, sizeof (struct rib_info));
      for (i = 0; i < ROUTE_TREE_SIZE; i++)
        new->fib_tree[i] = fib_new (NULL);
    }

  new->ver++;
  return new;
}

static inline __attribute__ ((always_inline)) void
rib_info_delete (struct rib_info *old)
{
  int i;

  /* free each FIB tree */
  for (i = 0; i < ROUTE_TREE_SIZE; i++)
    {
      if (old->fib_tree[i])
        {
          /* free all FIB nodes in the tree */
          fib_free (old->fib_tree[i]);
        }
    }
  free (old);
}

static inline __attribute__ ((always_inline)) struct rib *
rib_create (struct rib *old)
{
  struct rib *new;

  /* allocate new */
  new = malloc (sizeof (struct rib));
  if (! new)
    return NULL;

  if (! old)
    {
      memset (new, 0, sizeof (struct rib));
      new->rib_info = rib_info_create (NULL);
    }
  else
    {
      memcpy (new, old, sizeof (struct rib));
      new->rib_info = rib_info_create (old->rib_info);
    }

  return new;
}

static inline __attribute__ ((always_inline)) void
rib_delete (struct rib *old)
{
  if (old->rib_info)
    {
      rib_info_delete (old->rib_info);
      old->rib_info = NULL;
    }
  free (old);
}

static inline __attribute__ ((always_inline)) int
rib_check (struct rib *new)
{
  struct sdplane_queue_conf *qconf;
  struct lcore_qconf *lcore_qconf;
  int lcore;
  int i, ret;
  char ring_name[32];
  int j;

  DEBUG_SDPLANE_LOG (RIB, "ver: %d rib: %p rib_info: %p.", new->rib_info->ver,
                     new, new->rib_info);

  struct rte_eth_rxconf rxq_conf;
  struct rte_eth_txconf txq_conf;

#define RX_DESC_DEFAULT 1024
#define TX_DESC_DEFAULT 1024
  uint16_t nb_rxd = RX_DESC_DEFAULT;
  uint16_t nb_txd = TX_DESC_DEFAULT;

  for (lcore = 0; lcore < RTE_MAX_LCORE; lcore++)
    {
      lcore_qconf = &new->rib_info->lcore_qconf[lcore];
      for (i = 0; i < lcore_qconf->nrxq; i++)
        {
          struct port_queue_conf *rxq;
          rxq = &lcore_qconf->rx_queue_list[i];

          DEBUG_SDPLANE_LOG (
              RIB, "new rib: lcore: %d qconf[%d]: port: %d queue: %d", lcore,
              i, rxq->port_id, rxq->queue_id);
        }
    }

#define MAX_PORT_QCONF 256
  struct port_queue_conf port_qconf[MAX_PORT_QCONF];
  int port_qconf_size = 0;
  memset (port_qconf, 0, sizeof (port_qconf));

  /* check port's #rxq/#txq */
  int max_lcore = 0;
  for (lcore = 0; lcore < new->rib_info->lcore_size; lcore++)
    {
      lcore_qconf = &new->rib_info->lcore_qconf[lcore];
      for (i = 0; i < lcore_qconf->nrxq; i++)
        {
          struct port_queue_conf *rxq;
          rxq = &lcore_qconf->rx_queue_list[i];

          if (max_lcore < lcore)
            max_lcore = lcore;

          memcpy (&port_qconf[port_qconf_size], rxq,
                  sizeof (struct port_queue_conf));
          port_qconf_size++;
        }
    }

  qsort (port_qconf, port_qconf_size, sizeof (struct port_queue_conf),
         port_qconf_compare);

  int port_nrxq[RTE_MAX_ETHPORTS];
  memset (port_nrxq, 0, sizeof (port_nrxq));

  for (i = 0; i < port_qconf_size; i++)
    {
      struct port_queue_conf *rxq;
      rxq = &port_qconf[i];
      DEBUG_SDPLANE_LOG (RIB, "port_qconf[%d]: port: %d queue: %d", i,
                         rxq->port_id, rxq->queue_id);
      if (port_nrxq[rxq->port_id] == rxq->queue_id)
        {
          port_nrxq[rxq->port_id]++;
        }
      else
        {
          DEBUG_SDPLANE_LOG (RIB,
                             "unorderd port_qconf[%d]: port: %d queue: %d", i,
                             rxq->port_id, rxq->queue_id);
          return -1;
        }
    }

  struct rte_eth_conf port_conf =
    { .txmode = { .mq_mode = RTE_ETH_MQ_TX_NONE, },
    };
  struct rte_eth_dev_info dev_info;

  int ntxq;
  ntxq = max_lcore + 1;
  DEBUG_SDPLANE_LOG (RIB, "max_lcore: %d, ntxq: %d", max_lcore, ntxq);
  int nb_ports;
  nb_ports = rte_eth_dev_count_avail ();
  if (nb_ports > MAX_ETH_PORTS)
    nb_ports = MAX_ETH_PORTS;
  for (i = 0; i < nb_ports; i++)
    {
      int nrxq;
      nrxq = port_nrxq[i];
      rte_eth_dev_info_get (i, &dev_info);
      if (dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE)
        port_conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE;
      else
        port_conf.txmode.offloads &= (~RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE);
      if (*dev_info.dev_flags & RTE_ETH_DEV_INTR_LSC)
        {
          port_conf.intr_conf.lsc = 1;
        }

      if (new->rib_info->port[i].nrxq)
        {
          if (nrxq < new->rib_info->port[i].nrxq)
            nrxq = new->rib_info->port[i].nrxq;
        }
      if (new->rib_info->port[i].ntxq)
        {
          if (ntxq < new->rib_info->port[i].ntxq)
            ntxq = new->rib_info->port[i].ntxq;
        }

      DEBUG_NEW (RIB, "port[%d]: dev_configure: nrxq: %d ntxq: %d",
                 i, nrxq, ntxq);

      rte_eth_dev_stop (i);
      ret = rte_eth_dev_configure (i, nrxq, ntxq, &port_conf);
 
      if (new->rib_info->port[i].mtu)
        {
          uint16_t mtu = new->rib_info->port[i].mtu;
          DEBUG_NEW (RIB, "port[%d]: dev_set_mtu: %d", i, mtu);
          ret = rte_eth_dev_set_mtu (i, mtu);
          if (ret < 0)
            WARNING ("port[%d]: dev_set_mtu: %d failed: %d", i, mtu, ret);
        }

      nb_rxd = RX_DESC_DEFAULT;
      if (new->rib_info->port[i].nb_rxd)
        nb_rxd = new->rib_info->port[i].nb_rxd;
      nb_txd = TX_DESC_DEFAULT;
      if (new->rib_info->port[i].nb_txd)
        nb_txd = new->rib_info->port[i].nb_txd;

      rxq_conf = dev_info.default_rxconf;
      rxq_conf.offloads = port_conf.rxmode.offloads;

      int socket;
      socket = rte_eth_dev_socket_id (i);

      for (j = 0; j < nrxq; j++)
        {
          ret = rte_eth_rx_queue_setup (i, j, nb_rxd, socket,
                                        &rxq_conf, l2fwd_pktmbuf_pool);
          DEBUG_NEW (RIB, "port[%d]: rx_queue_setup: rxq: %d rxd: %d",
                     i, j, nb_rxd);
        }

      txq_conf = dev_info.default_txconf;
      txq_conf.offloads = port_conf.txmode.offloads;

      for (j = 0; j < ntxq; j++)
        {
          ret = rte_eth_tx_queue_setup (i, j, nb_txd, socket, &txq_conf);
          DEBUG_NEW (RIB, "port[%d]: tx_queue_setup: txq: %d txd: %d",
                     i, j, nb_txd);

          if (! tx_buffer_per_q[i][j])
            {
              DEBUG_SDPLANE_LOG (L2_REPEATER,
                                 "tx_buffer_init: port: %d queue: %d", i, j);
              tx_buffer_per_q[i][j] = rte_zmalloc_socket (
                  "tx_buffer", RTE_ETH_TX_BUFFER_SIZE (MAX_PKT_BURST), 0,
                  rte_eth_dev_socket_id (i));
              rte_eth_tx_buffer_init (tx_buffer_per_q[i][j], MAX_PKT_BURST);
            }
        }

      rte_eth_dev_start (i);
    }

    /* prepare rte_ring "ring_up/dn[][]" */
#define RING_TO_TAP_SIZE 64
  for (lcore = 0; lcore < RTE_MAX_LCORE; lcore++)
    {
      lcore_qconf = &new->rib_info->lcore_qconf[lcore];
      for (i = 0; i < lcore_qconf->nrxq; i++)
        {
          struct port_queue_conf *rxq;
          rxq = &lcore_qconf->rx_queue_list[i];

          if (! ring_up[rxq->port_id][rxq->queue_id])
            {
              snprintf (ring_name, sizeof (ring_name), "ring_up[%d][%d]",
                        rxq->port_id, rxq->queue_id);
              ring_up[rxq->port_id][rxq->queue_id] = rte_ring_create (
                  ring_name, RING_TO_TAP_SIZE, rte_socket_id (),
                  (RING_F_SP_ENQ | RING_F_SC_DEQ));
              DEBUG_SDPLANE_LOG (RIB, "rib: create: %s: %p", ring_name,
                                 ring_up[rxq->port_id][rxq->queue_id]);
            }

          if (! ring_dn[rxq->port_id][rxq->queue_id])
            {
              snprintf (ring_name, sizeof (ring_name), "ring_dn[%d][%d]",
                        rxq->port_id, rxq->queue_id);
              ring_dn[rxq->port_id][rxq->queue_id] = rte_ring_create (
                  ring_name, RING_TO_TAP_SIZE, rte_socket_id (),
                  (RING_F_SP_ENQ | RING_F_SC_DEQ));
              DEBUG_SDPLANE_LOG (RIB, "rib: create: %s: %p", ring_name,
                                 ring_dn[rxq->port_id][rxq->queue_id]);
            }
        }
    }

  for (i = 0; i < new->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vswitch = &new->rib_info->vswitch[i];

      if (vswitch->vlan_id == 0)
        continue;

      if (vswitch->router_if.sockfd >= 0)
        {
          if (! router_if_ring_up[i])
            {
              snprintf (ring_name, sizeof (ring_name), "router_up[%d]", i);
              router_if_ring_up[i] = rte_ring_create (
                  ring_name, RING_TO_TAP_SIZE, rte_socket_id (), 0);
              DEBUG_SDPLANE_LOG (RIB, "rib: create: %s: %p", ring_name,
                                 router_if_ring_up[i]);
            }
          if (! router_if_ring_dn[i])
            {
              snprintf (ring_name, sizeof (ring_name), "router_dn[%d]", i);
              router_if_ring_dn[i] = rte_ring_create (
                  ring_name, RING_TO_TAP_SIZE, rte_socket_id (), 0);
              DEBUG_SDPLANE_LOG (RIB, "rib: create: %s: %p", ring_name,
                                 router_if_ring_dn[i]);
            }

          vswitch->router_if.ring_up = router_if_ring_up[i];
          vswitch->router_if.ring_dn = router_if_ring_dn[i];
        }

      if (vswitch->capture_if.sockfd >= 0)
        {
          if (! capture_if_ring_up[i])
            {
              snprintf (ring_name, sizeof (ring_name), "capture_up[%d]", i);
              capture_if_ring_up[i] = rte_ring_create (
                  ring_name, RING_TO_TAP_SIZE, rte_socket_id (), 0);
              DEBUG_SDPLANE_LOG (RIB, "rib: create: %s: %p", ring_name,
                                 capture_if_ring_up[i]);
            }
          if (! capture_if_ring_dn[i])
            {
              snprintf (ring_name, sizeof (ring_name), "capture_dn[%d]", i);
              capture_if_ring_dn[i] = rte_ring_create (
                  ring_name, RING_TO_TAP_SIZE, rte_socket_id (), 0);
              DEBUG_SDPLANE_LOG (RIB, "rib: create: %s: %p", ring_name,
                                 capture_if_ring_dn[i]);
            }

          vswitch->capture_if.ring_up = capture_if_ring_up[i];
          vswitch->capture_if.ring_dn = capture_if_ring_dn[i];
        }
    }

  return 0;
}

static inline __attribute__ ((always_inline)) void
rib_replace (struct rib *new)
{
  struct rib *old;
  old = rcu_dereference (rcu_global_ptr_rib);

  /* assign new */
  rcu_assign_pointer (rcu_global_ptr_rib, new);
  DEBUG_SDPLANE_LOG (RIB,
                     "rib: replace: %'lu-th: "
                     "rib: %p -> %p "
                     "rib_info: ver.%d (%p) -> ver.%d (%p)",
                     rib_rcu_replace, old, new,
                     (old && old->rib_info ? old->rib_info->ver : -1),
                     (old ? old->rib_info : NULL),
                     (new &&new->rib_info ? new->rib_info->ver : -1),
                     (new ? new->rib_info : NULL));

  /* reclaim old */
  if (old)
    {
      urcu_qsbr_synchronize_rcu ();
      rib_delete (old);
    }

  rib_rcu_replace++;
}

void
update_port_status (struct rib *new)
{
  uint16_t nb_ports, port_id;
  DEBUG_SDPLANE_LOG (RIB, "update port status: ver: %d rib: %p rib_info: %p.",
                     new->rib_info->ver, new, new->rib_info);
  nb_ports = rte_eth_dev_count_avail ();
  new->rib_info->port_size = nb_ports;
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      rte_eth_dev_info_get (port_id, &new->rib_info->port[port_id].dev_info);
      rte_eth_link_get_nowait (port_id, &new->rib_info->port[port_id].link);
      DEBUG_SDPLANE_LOG (RIB, "port: %d link: %d ver: %d rib_info: %p.",
                         port_id,
                         new->rib_info->port[port_id].link.link_status,
                         new->rib_info->ver, new->rib_info);
      new->rib_info->port[port_id].dpdk_port_id = port_id;
    }

  uint16_t lcore_size;
  lcore_size = rte_lcore_count ();
  new->rib_info->lcore_size = lcore_size;
}

void
set_stop_flag (struct rib *old)
{
  uint16_t nb_ports, port_id;
  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      old->rib_info->port[port_id].is_stopped = true;
      DEBUG_SDPLANE_LOG (RIB, "port[%d]: is_stopped: %d", port_id,
                         old->rib_info->port[port_id].is_stopped);
    }
}

void
delete_stop_flag (struct rib *old)
{
  uint16_t nb_ports, port_id;
  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      old->rib_info->port[port_id].is_stopped = false;
      DEBUG_SDPLANE_LOG (RIB, "port[%d]: is_stopped: %d", port_id,
                         old->rib_info->port[port_id].is_stopped);
    }
}

void
application_slot_add (struct rib_info *rib_info,
                      struct application_slot_entry *msg_appli_slot)
{
  if (rib_info->application_slot_size == APPLI_SLOT_SIZE)
    {
      WARNING ("application_slot: full: %d entries.",
               rib_info->application_slot_size);
      return;
    }
  memcpy (&rib_info->application_slot[rib_info->application_slot_size],
          msg_appli_slot, sizeof (struct application_slot_entry));
  rib_info->application_slot_size++;
  DEBUG_NEW (RIB, "application_slot added: %d entries.",
             rib_info->application_slot_size);
}

#define RIB_RETFLAG_NONE               0
#define RIB_RETFLAG_RETURN_IMMEDIATELY 1

static uint16_t
rib_manager_process_port_get (struct internal_msg_header *imsghdr)
{
  uint16_t retflag = 0;
  struct internal_msg_port_info *port_info;
  assert (imsghdr->type == INTERNAL_MSG_TYPE_PORT_GET_REQUEST);
  port_info = (struct internal_msg_port_info *)
    internal_msg_body (imsghdr);

  DEBUG_NEW (RIB, "port_get_request: imsghdr: %p", imsghdr);

  struct rib *new, *old;
  old = rcu_dereference (rcu_global_ptr_rib);
  new = rib_create (old);

  /* get port info update. */
  uint16_t port_id;
  port_id = port_info->port_id;
  new->rib_info->port[port_id].dpdk_port_id = port_id;
  rte_eth_dev_info_get (port_id, &new->rib_info->port[port_id].dev_info);
  rte_eth_link_get_nowait (port_id, &new->rib_info->port[port_id].link);

  rib_replace (new);

  struct internal_msg_header *resp;
  struct internal_msg_port_info resp_port_info;
  resp_port_info.port_id = port_id;
  memcpy (&resp_port_info.dev_info,
          &new->rib_info->port[port_id].dev_info,
          sizeof (struct rte_eth_dev_info));
  memcpy (&resp_port_info.link,
          &new->rib_info->port[port_id].link,
          sizeof (struct rte_eth_link));
  resp = internal_msg_create (INTERNAL_MSG_TYPE_PORT_GET_RESPONSE,
                              &resp_port_info, sizeof (resp_port_info));
  if (imsghdr->ring_response)
    {
      internal_msg_send_to (imsghdr->ring_response, resp, NULL);
      DEBUG_NEW (RIB, "send response: %p to ring: %p",
                 resp, imsghdr->ring_response);
    }
  else
    {
      WARNING ("cannot send response: %p", resp);
      WARNING ("probably no shell ring response. update libsdplane.");
    }

  free (imsghdr);
  FLAG_SET (retflag, RIB_RETFLAG_RETURN_IMMEDIATELY);
  return retflag;
}

void
rib_manager_process_message (void *msgp)
{
  int ret;
  int i, j;
  //DEBUG_SDPLANE_LOG (RIB, "msg: %p.", msgp);

  int route_idx_add = -1, tree_idx_add = -1;

  struct rib *new, *old;

  /* retrieve old */
#if HAVE_LIBURCU_QSBR
  old = rcu_dereference (rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

  new = rib_create (old);

  /* related neigh table */
  struct neigh_table *neigh_table;

  /* related vswitch operation */
  struct vswitch_conf *vswitch;
  struct port_conf *port;
  struct vswitch_link *link;

  /* change something according to the update instruction message. */
  struct internal_msg_header *msg_header;
  struct internal_msg_eth_link *msg_eth_link;
  struct internal_msg_qconf *msg_qconf;
  struct internal_msg_neigh_entry *msg_neigh_entry;
  struct internal_msg_route_entry *msg_route_entry;
  struct internal_msg_mac_addr *msg_mac_addr;
  struct internal_msg_ip_addr *msg_ip_addr;

  msg_header = (struct internal_msg_header *) msgp;

  uint16_t retflag = 0;

  switch (msg_header->type)
    {
    case INTERNAL_MSG_TYPE_PORT_GET_REQUEST:
      DEBUG_NEW (RIB, "port_get_request: receive");
      retflag = rib_manager_process_port_get (msg_header);
      if (FLAG_CHECK (retflag, RIB_RETFLAG_RETURN_IMMEDIATELY))
        return;
      break;

    case INTERNAL_MSG_TYPE_PORT_STATUS:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_port_status: %p.", msgp);
      update_port_status (new);
      break;

    case INTERNAL_MSG_TYPE_QCONF:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_qconf: %p.", msgp);
      msg_qconf = (struct internal_msg_qconf *) (msg_header + 1);
      uint16_t lcore_size;
      lcore_size = rte_lcore_count ();
      new->rib_info->lcore_size = lcore_size;
      for (i = 0; i < lcore_size; i++)
        {
          if (msg_qconf->qconf[i].nrxq)
            {
              new->rib_info->lcore_qconf[i].nrxq = msg_qconf->qconf[i].nrxq;
              for (j = 0; j < msg_qconf->qconf[i].nrxq; j++)
                {
                  char *src, *dst;
                  int len;
                  dst =
                      (char *) &new->rib_info->lcore_qconf[i].rx_queue_list[j];
                  src = (char *) &msg_qconf->qconf[i].rx_queue_list[j];
                  len =
                      sizeof (new->rib_info->lcore_qconf[i].rx_queue_list[j]);
                  memcpy (dst, src, len);
                }
            }
        }

      /* for qconf change, we need strict rib_check(). */
      set_stop_flag (old);
      ret = rib_check (new);
      delete_stop_flag (old);

      /* if rib_check() is not a pass */
      if (ret < 0)
        {
          DEBUG_SDPLANE_LOG (RIB, "rib_check() failed: return.");
          free (msgp);
          return;
        }

      /* for qconf change, we need an NULL intermittent state
         to avoid a conflict between different cores. */
      /* XXX, we can use smarter intermittent state. */
      struct rib *zero;
      zero = malloc (sizeof (struct rib));
      if (zero)
        {
          memset (zero, 0, sizeof (struct rib));
          rib_replace (zero);
        }
      break;

    case INTERNAL_MSG_TYPE_TXRX_DESC:
      struct internal_msg_txrx_desc *msg_txrx_desc;
      int portid;
      DEBUG_NEW (RIB, "recv msg_txrx_desc: %p.", msgp);
      msg_txrx_desc = (struct internal_msg_txrx_desc *) (msg_header + 1);
      portid = msg_txrx_desc->portid;
      if (msg_txrx_desc->mtu)
        {
          new->rib_info->port[portid].mtu = msg_txrx_desc->mtu;
          DEBUG_NEW (RIB, "set port: %d mtu: %d",
                     portid, msg_txrx_desc->mtu);
        }
      if (msg_txrx_desc->nrxq)
        {
          new->rib_info->port[portid].nrxq = msg_txrx_desc->nrxq;
          DEBUG_NEW (RIB, "set port: %d nrxq: %d",
                     portid, msg_txrx_desc->nrxq);
        }
      if (msg_txrx_desc->ntxq)
        {
          new->rib_info->port[portid].ntxq = msg_txrx_desc->ntxq;
          DEBUG_NEW (RIB, "set port: %d ntxq: %d",
                     portid, msg_txrx_desc->ntxq);
        }
      if (msg_txrx_desc->nb_rxd)
        {
          new->rib_info->port[portid].nb_rxd = msg_txrx_desc->nb_rxd;
          DEBUG_NEW (RIB, "set port: %d nb_rxd: %d",
                     portid, msg_txrx_desc->nb_rxd);
        }
      if (msg_txrx_desc->nb_txd)
        {
          new->rib_info->port[portid].nb_txd = msg_txrx_desc->nb_txd;
          DEBUG_NEW (RIB, "set port: %d nb_txd: %d",
                     portid, msg_txrx_desc->nb_txd);
        }

      //if (msg_txrx_desc->nrxq || msg_txrx_desc->ntxq)
      if (startup_config_completed)
        {
          if (old)
            set_stop_flag (old);
          ret = rib_check (new);
          if (old)
            delete_stop_flag (old);
        }
      break;

    case INTERNAL_MSG_TYPE_NEIGH_ENTRY_ADD:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_neigh_entry_add: %p.", msgp);
      msg_neigh_entry = (struct internal_msg_neigh_entry *) (msg_header + 1);
      neigh_table = &new->rib_info->neigh_tables[msg_neigh_entry->type];
      /* If the master’s neighbor table has more entries than the RIB’s, 
       * it indicates that a new entry has been inserted. */
      if (msg_neigh_entry->num_entries > neigh_table->num_entries)
        {
          /* insert new entry */
          DEBUG_SDPLANE_LOG (NEIGH, "rib: add: %s[%d]",
                             neigh_manager_table_str (msg_neigh_entry->type),
                             msg_neigh_entry->pos);
          memmove (&neigh_table->entries[msg_neigh_entry->pos + 1],
                   &neigh_table->entries[msg_neigh_entry->pos],
                   sizeof (struct neigh_entry) *
                       (neigh_table->num_entries - msg_neigh_entry->pos));
        }
      else
        /* update entry */
        DEBUG_SDPLANE_LOG (NEIGH, "rib: update: %s[%d]",
                           neigh_manager_table_str (msg_neigh_entry->type),
                           msg_neigh_entry->pos);
      memcpy (&neigh_table->entries[msg_neigh_entry->pos],
              &msg_neigh_entry->data, sizeof (struct neigh_entry));
      // same as "neigh_table->num_entries++;"
      neigh_table->num_entries = msg_neigh_entry->num_entries;
      break;

    case INTERNAL_MSG_TYPE_NEIGH_ENTRY_DEL:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_neigh_entry_del: %p.", msgp);
      msg_neigh_entry = (struct internal_msg_neigh_entry *) (msg_header + 1);
      /* delete entry */
      DEBUG_SDPLANE_LOG (NEIGH, "rib: del: %s[%d]",
                         neigh_manager_table_str (msg_neigh_entry->type),
                         msg_neigh_entry->pos);
      neigh_table = &new->rib_info->neigh_tables[msg_neigh_entry->type];
      // same as "neigh_table->num_entries--;"
      neigh_table->num_entries = msg_neigh_entry->num_entries;
      memmove (&neigh_table->entries[msg_neigh_entry->pos],
               &neigh_table->entries[msg_neigh_entry->pos + 1],
               sizeof (struct neigh_entry) *
                   (neigh_table->num_entries - msg_neigh_entry->pos));
      break;

    case INTERNAL_MSG_TYPE_VSWITCH_SET:
      struct internal_msg_vswitch *msg_vswitch_set;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_vswitch_set: %p.", msgp);
      msg_vswitch_set = (struct internal_msg_vswitch *) (msg_header + 1);
      if (vswitch_lookup (new->rib_info, msg_vswitch_set->vswitch_id))
        {
          DEBUG_SDPLANE_LOG (RIB, "vswitch already exists.");
          break;
        }
      vswitch = vswitch_new (new->rib_info, msg_vswitch_set->vswitch_id,
                             msg_vswitch_set->vlan_id);
      if (vswitch)
        DEBUG_SDPLANE_LOG (RIB, "create succeeded: vswitch: %u vlan_id: %u",
                           msg_vswitch_set->vswitch_id,
                           msg_vswitch_set->vlan_id);
      else
        DEBUG_SDPLANE_LOG (RIB, "create failed: vswitch: %u vlan_id: %u",
                           msg_vswitch_set->vswitch_id,
                           msg_vswitch_set->vlan_id);
      break;

    case INTERNAL_MSG_TYPE_VSWITCH_NO_SET:
      struct internal_msg_vswitch *msg_vswitch_no_set;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_vswitch_no_set: %p.", msgp);
      msg_vswitch_no_set = (struct internal_msg_vswitch *) (msg_header + 1);
      if (! vswitch_lookup (new->rib_info, msg_vswitch_no_set->vswitch_id))
        {
          DEBUG_SDPLANE_LOG (RIB, "delete vswitch not found");
          break;
        }
      vswitch_delete (new->rib_info, msg_vswitch_no_set->vswitch_id);
      break;

    case INTERNAL_MSG_TYPE_VSWITCH_PORT_SET:
      struct internal_msg_vswitch_port *msg_vswitch_port_set;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_vswitch_port_set: %p.", msgp);
      msg_vswitch_port_set =
          (struct internal_msg_vswitch_port *) (msg_header + 1);

      vswitch =
          vswitch_lookup (new->rib_info, msg_vswitch_port_set->vswitch_id);
      if (! vswitch)
        {
          DEBUG_SDPLANE_LOG (RIB, "vswitch: %u not found",
                             msg_vswitch_port_set->vswitch_id);
          break;
        }
      port = &new->rib_info->port[msg_vswitch_port_set->port_id];
      link = vswitch_link_new (new->rib_info, vswitch, port);
      if (! link)
        {
          DEBUG_SDPLANE_LOG (RIB, "create failed: vswitch: %u port %u tag: %u",
                             msg_vswitch_port_set->vswitch_id,
                             msg_vswitch_port_set->port_id,
                             msg_vswitch_port_set->tag_id);
          break;
        }

      if (! msg_vswitch_port_set->is_tagged)
        {
          port_set_native_vlan (new->rib_info, port, link);
          DEBUG_SDPLANE_LOG (
              RIB, "create succeeded: link_id: %u vswitch: %u port: %u native",
              link->vswitch_link_id, msg_vswitch_port_set->vswitch_id,
              msg_vswitch_port_set->port_id);
        }
      else
        {
          if (msg_vswitch_port_set->tag_id != 0 &&
              msg_vswitch_port_set->tag_id != link->tag_id)
            link->tag_id = msg_vswitch_port_set->tag_id; // enable tag modified
          port_add_tagged_vlan (new->rib_info, port, link);
          DEBUG_SDPLANE_LOG (
              RIB,
              "create succeeded: link_id: %u vswitch: %u port: %u tag: %u",
              link->vswitch_link_id, msg_vswitch_port_set->vswitch_id,
              msg_vswitch_port_set->port_id, msg_vswitch_port_set->tag_id);
        }
      break;

    case INTERNAL_MSG_TYPE_VSWITCH_PORT_NO_SET:
      struct internal_msg_vswitch_port *msg_vswitch_port_no_set;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_vswitch_port_no_set: %p.", msgp);
      msg_vswitch_port_no_set =
          (struct internal_msg_vswitch_port *) (msg_header + 1);
      vswitch =
          vswitch_lookup (new->rib_info, msg_vswitch_port_no_set->vswitch_id);
      if (! vswitch)
        {
          DEBUG_SDPLANE_LOG (RIB, "vswitch: %u not found",
                             msg_vswitch_port_no_set->vswitch_id);
          break;
        }
      port = &new->rib_info->port[msg_vswitch_port_no_set->port_id];
      link = vswitch_link_lookup (new->rib_info, vswitch, port);
      if (! link)
        {
          DEBUG_SDPLANE_LOG (RIB,
                             "vswitch link: vswitch: %u port: %u not found",
                             msg_vswitch_port_no_set->vswitch_id,
                             msg_vswitch_port_no_set->port_id);
          break;
        }

      vswitch_link_delete (new->rib_info, link->vswitch_link_id);
      break;

    case INTERNAL_MSG_TYPE_ROUTER_IF_SET:
      struct internal_msg_tap_dev *msg_router_if_set;
      struct router_if *rif;
      struct application_slot_entry app_slot;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_router_if_set: %p.", msgp);
      msg_router_if_set = (struct internal_msg_tap_dev *) (msg_header + 1);

      vswitch = vswitch_lookup (new->rib_info, msg_router_if_set->vswitch_id);
      if (! vswitch)
        {
          DEBUG_SDPLANE_LOG (RIB, "vswitch: %u not found",
                             msg_router_if_set->vswitch_id);
          break;
        }
      rif = &vswitch->router_if;

      rif->sockfd = tap_open (msg_router_if_set->tap_name);
      ioctl (rif->sockfd, TUNSETPERSIST, 1);
      rif->tap_ring_id = msg_router_if_set->vswitch_id;
      snprintf (rif->tap_name, sizeof (rif->tap_name), "%s",
                msg_router_if_set->tap_name);
#define NO_VLAN_SPECIFIED 65535
      if (msg_router_if_set->vlan_id != NO_VLAN_SPECIFIED)
        rif->vlan_id = msg_router_if_set->vlan_id;
      else
        rif->vlan_id = vswitch->vlan_id;
      rif->ifindex = tap_get_ifindex (msg_router_if_set->tap_name);
      tap_admin_up (msg_router_if_set->tap_name);

      DEBUG_SDPLANE_LOG (RIB, "create succeeded: router_if: %s vswitch: %u: vlan %u",
                         msg_router_if_set->tap_name,
                         msg_router_if_set->vswitch_id,
                         msg_router_if_set->vlan_id);

      // set router_if ring
      set_stop_flag (old);
      ret = rib_check (new);
      delete_stop_flag (old);

      /* if rib_check() is not a pass */
      if (ret < 0)
        {
          DEBUG_SDPLANE_LOG (RIB, "rib_check() failed: return.");
          free (msgp);
          return;
        }

#if 0
      /*
       * TODO(#229):
       * Temporarily disabled due to duplicate TX on the same link
       * with enhanced repeater.
       * Expected to be resolved by future changes to the application_slot implementation.
       */

       app_slot.name = L3_TAP_HANDLER_APP_NAME;
       app_slot.ring = rif->ring_up;
       app_slot.is_packet_match = should_send_to_tap;
       application_slot_add (new->rib_info, &app_slot);
#endif

      break;

    case INTERNAL_MSG_TYPE_ROUTER_IF_NO_SET:
      struct internal_msg_tap_dev *msg_router_if_no_set;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_router_if_no_set: %p.", msgp);
      msg_router_if_no_set = (struct internal_msg_tap_dev *) (msg_header + 1);

      for (i = 0; i < new->rib_info->vswitch_size; i++)
        {
          if (! strcmp (msg_router_if_no_set->tap_name,
                        new->rib_info->vswitch[i].router_if.tap_name))
            router_if_delete (new->rib_info,
                              new->rib_info->vswitch[i].vswitch_id);
        }
      break;

    case INTERNAL_MSG_TYPE_CAPTURE_IF_SET:
      struct internal_msg_tap_dev *msg_capture_if_set;
      struct capture_if *cif;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_capture_if_set: %p.", msgp);
      msg_capture_if_set = (struct internal_msg_tap_dev *) (msg_header + 1);

      vswitch = vswitch_lookup (new->rib_info, msg_capture_if_set->vswitch_id);
      if (! vswitch)
        {
          DEBUG_SDPLANE_LOG (RIB, "vswitch: %u not found",
                             msg_capture_if_set->vswitch_id);
          break;
        }
      cif = &vswitch->capture_if;

      cif->sockfd = tap_open (msg_capture_if_set->tap_name);
      ioctl (cif->sockfd, TUNSETPERSIST, 1);
      cif->tap_ring_id = msg_capture_if_set->vswitch_id;
      snprintf (cif->tap_name, sizeof (cif->tap_name), "%s",
                msg_capture_if_set->tap_name);
      tap_admin_up (msg_capture_if_set->tap_name);

      DEBUG_SDPLANE_LOG (RIB, "create: capture_if: %s vswitch: %u",
                         msg_capture_if_set->tap_name,
                         msg_capture_if_set->vswitch_id);

      // set capture_if ring
      set_stop_flag (old);
      ret = rib_check (new);
      delete_stop_flag (old);

      /* if rib_check() is not a pass */
      if (ret < 0)
        {
          DEBUG_SDPLANE_LOG (RIB, "rib_check() failed: return.");
          free (msgp);
          return;
        }

      break;

    case INTERNAL_MSG_TYPE_CAPTURE_IF_NO_SET:
      struct internal_msg_tap_dev *msg_capture_if_no_set;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_capture_if_no_set: %p.", msgp);
      msg_capture_if_no_set = (struct internal_msg_tap_dev *) (msg_header + 1);
      for (i = 0; i < new->rib_info->vswitch_size; i++)
        {
          if (! strcmp (msg_capture_if_no_set->tap_name,
                        new->rib_info->vswitch[i].capture_if.tap_name))
            capture_if_delete (new->rib_info,
                               new->rib_info->vswitch[i].vswitch_id);
        }
      break;

    case INTERNAL_MSG_TYPE_FDB_ENTRY_ADD:
      struct internal_msg_fdb_entry *msg_fdb_entry_add;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_fdb_entry_add: %p.", msgp);

      msg_fdb_entry_add = (struct internal_msg_fdb_entry *) (msg_header + 1);

      fdb_add_entry (new->rib_info, &msg_fdb_entry_add->mac_addr,
                     msg_fdb_entry_add->vlan_id, msg_fdb_entry_add->port);
      break;

    case INTERNAL_MSG_TYPE_APPLICATION_SLOT:
      struct application_slot_entry *msg_appli_slot;
      DEBUG_NEW (RIB, "recv msg_appli_slot: %p.", msgp);
      msg_appli_slot =
        (struct application_slot_entry *) (msg_header + 1);
      application_slot_add (new->rib_info, msg_appli_slot);
      break;

    case INTERNAL_MSG_TYPE_ROUTE_ENTRY_ADD:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_route_entry_add: %p.", msgp);
      msg_route_entry = (struct internal_msg_route_entry *) (msg_header + 1);
      char dst_str_add[INET6_ADDRSTRLEN];
      char nexthop_str_add_buf[512];
      struct route_entry route_entry;
      int nh_id = 0;

      /* register nexthop entry to rib_info->nexthop */
      nh_id = nexthop_common_add_entry (
                                  new->rib_info,
                                  msg_route_entry->nh_type,
                                  &msg_route_entry->nh);
      if (nh_id < 0)
        break;
      DEBUG_NEW (RIB, "nexthop object added: nh_id=%d type=%s refcnt=%d",
                 nh_id, msg_route_entry->nh_type ? "OBJECT_CAPABLE" : "LEGACY",
                 new->rib_info->nexthop.legacy.object[nh_id].ref_count);

      /* search for existing RIB tree with matching family and table_id */
      i = route_table_lookup_id (msg_route_entry->table_id,
                                 msg_route_entry->family);
      if (i < 0)
        {
          /* if not found, search for empty slot and create new table */
          i = route_table_alloc_slot (msg_route_entry->table_id,
                                      msg_route_entry->family);
          if (i < 0)
            {
              DEBUG_SDPLANE_LOG (RIB, "failed to create route table");
              break;
            }
        }

      /* create route entry */
      memset (&route_entry, 0, sizeof (struct route_entry));
      memcpy (&route_entry.dst,
              &msg_route_entry->dst,
              sizeof (struct route_dst_info));
      route_entry.nh.nh_type = msg_route_entry->nh_type;
      route_entry.nh.nh_id = nh_id;

      /* register route entry to RIB */
      if (rib_route_add (rib_tree_master[i],
                         &msg_route_entry->dst.dst_ip_addr,
                         msg_route_entry->dst.plen,
                         &route_entry) > 0)
        {
          DEBUG_SDPLANE_LOG (RIB, "failed to add route to RIB");
          break;
        }
      inet_ntop (msg_route_entry->dst.family,
                 &msg_route_entry->dst.dst_ip_addr,
                 dst_str_add, sizeof (dst_str_add));
      DEBUG_SDPLANE_LOG (
          RIB, "route added: dst=%s/%d nexthop=%s",
          dst_str_add, msg_route_entry->dst.plen,
          nexthop_common_format_object (
              msg_route_entry->nh_type,
              &msg_route_entry->nh,
              nexthop_str_add_buf,
              sizeof (nexthop_str_add_buf)
          )
        );

      break;

    case INTERNAL_MSG_TYPE_ROUTE_ENTRY_DEL:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_route_entry_del: %p.", msgp);
      msg_route_entry = (struct internal_msg_route_entry *) (msg_header + 1);
      char dst_str_del[INET6_ADDRSTRLEN];

      /* delete nexthop entry from rib_info->nexthop */
      nh_id = nexthop_common_del_entry (
                                  new->rib_info,
                                  msg_route_entry->nh_type,
                                  &msg_route_entry->nh);
      if (nh_id < 0)
        break;
      DEBUG_NEW (RIB, "nexthop object del: nh_id=%d type=%s refcnt=%d",
                 nh_id, msg_route_entry->nh_type ? "OBJECT_CAPABLE" : "LEGACY",
                 new->rib_info->nexthop.legacy.object[nh_id].ref_count);

      /* search for existing RIB tree with matching family and table_id */
      i = route_table_lookup_id (msg_route_entry->table_id,
                                 msg_route_entry->family);

      if (i >= 0)
        {
          if (rib_route_delete (rib_tree_master[i],
                                &msg_route_entry->dst.dst_ip_addr,
                                msg_route_entry->dst.plen) != 0)
            {
              DEBUG_SDPLANE_LOG (RIB, "failed to delete route to RIB");
              break;
            }
          inet_ntop (msg_route_entry->family,
                     &msg_route_entry->dst.dst_ip_addr,
                     dst_str_del, sizeof (dst_str_del));
          DEBUG_SDPLANE_LOG (RIB, "route deleted: dst=%s/%d",
                             dst_str_del, msg_route_entry->dst.plen);
        }
      else
        {
          DEBUG_SDPLANE_LOG (RIB, "route table not found: table_id=%d family=%d",
                             msg_route_entry->table_id,
                             msg_route_entry->family);
          break;
        }

      break;

    case INTERNAL_MSG_TYPE_MAC_ADDR_ADD:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_mac_addr_add: %p.", msgp);
      msg_mac_addr = (struct internal_msg_mac_addr *) (msg_header + 1);
      char mac_str_add[RTE_ETHER_ADDR_FMT_SIZE];

      for (i = 0; i < new->rib_info->vswitch_size; i++)
        {
          if (memcmp (new->rib_info->vswitch[i].router_if.tap_name, msg_mac_addr->ifname,
                      sizeof (msg_mac_addr->ifname)) == 0)
            {
              memcpy (&new->rib_info->vswitch[i].router_if.mac_addr, &msg_mac_addr->mac_addr,
                      sizeof (struct rte_ether_addr));

              rte_ether_format_addr (mac_str_add, sizeof (mac_str_add), &msg_mac_addr->mac_addr);
              DEBUG_SDPLANE_LOG (RIB, "add MAC address: ifname=%s mac=%s",
                                 msg_mac_addr->ifname, mac_str_add);

              break;
            }
        }

      break;

    case INTERNAL_MSG_TYPE_MAC_ADDR_DEL:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_mac_addr_del: %p.", msgp);
      msg_mac_addr = (struct internal_msg_mac_addr *) (msg_header + 1);
      char mac_str_del[RTE_ETHER_ADDR_FMT_SIZE];

      for (i = 0; i < new->rib_info->vswitch_size; i++)
        {
          if (memcmp (new->rib_info->vswitch[i].router_if.tap_name, msg_mac_addr->ifname,
                      sizeof (msg_mac_addr->ifname)) == 0 && 
              memcmp (&new->rib_info->vswitch[i].router_if.mac_addr, &msg_mac_addr->mac_addr,
                      sizeof (struct rte_ether_addr)))
            {
              memset (&new->rib_info->vswitch[i].router_if.mac_addr, 0,
                      sizeof (struct rte_ether_addr));

              rte_ether_format_addr (mac_str_del, sizeof (mac_str_del), &msg_mac_addr->mac_addr);
              DEBUG_SDPLANE_LOG (RIB, "delete MAC address: ifname=%s mac=%s",
                                 msg_mac_addr->ifname, mac_str_del);

              break;
            }
        }

      break;

    case INTERNAL_MSG_TYPE_IP_ADDR_ADD:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_ip_addr_add: %p.", msgp);
      msg_ip_addr = (struct internal_msg_ip_addr *) (msg_header + 1);
      char ip_str_add[INET6_ADDRSTRLEN];

      for (i = 0; i < new->rib_info->vswitch_size; i++)
        {
          if (memcmp (new->rib_info->vswitch[i].router_if.tap_name, msg_ip_addr->ifname,
                      sizeof (msg_ip_addr->ifname)) == 0)
            {
              if (msg_ip_addr->family == AF_INET)
                {
                  memcpy (&new->rib_info->vswitch[i].router_if.ipv4_addr, &msg_ip_addr->ip_addr.ipv4_addr,
                          sizeof (struct in_addr));

                  inet_ntop (AF_INET, &msg_ip_addr->ip_addr.ipv4_addr, ip_str_add,
                             sizeof (ip_str_add));
                  DEBUG_SDPLANE_LOG (RIB, "add IPv4 address: ifname=%s ip=%s",
                                     msg_ip_addr->ifname, ip_str_add);
                }
              else
                {
                  if (msg_ip_addr->is_ll_addr)
                    memcpy (&new->rib_info->vswitch[i].router_if.ll_addr, &msg_ip_addr->ip_addr.ipv6_addr,
                            sizeof (struct in6_addr));
                  else
                    memcpy (&new->rib_info->vswitch[i].router_if.ipv6_addr, &msg_ip_addr->ip_addr.ipv6_addr,
                            sizeof (struct in6_addr));

                  inet_ntop (AF_INET6, &msg_ip_addr->ip_addr.ipv6_addr, ip_str_add,
                             sizeof (ip_str_add));
                  DEBUG_SDPLANE_LOG (RIB, "add IPv6 address: ifname=%s ip=%s",
                                     msg_ip_addr->ifname, ip_str_add);
                }

              break;
            }
        }

      break;

    case INTERNAL_MSG_TYPE_IP_ADDR_DEL:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_ip_addr_del: %p.", msgp);
      msg_ip_addr = (struct internal_msg_ip_addr *) (msg_header + 1);
      char ip_str_del[INET6_ADDRSTRLEN];

      for (i = 0; i < new->rib_info->vswitch_size; i++)
        {
          if (memcmp (new->rib_info->vswitch[i].router_if.tap_name, msg_ip_addr->ifname,
                      sizeof (msg_ip_addr->ifname)) == 0)
            {
              if (msg_ip_addr->family == AF_INET)
                {
                  if (memcmp (&new->rib_info->vswitch[i].router_if.ipv4_addr, &msg_ip_addr->ip_addr.ipv4_addr,
                              sizeof (struct in_addr)) == 0)
                    {
                      memset (&new->rib_info->vswitch[i].router_if.ipv4_addr, 0, sizeof (struct in_addr));

                      inet_ntop (AF_INET, &msg_ip_addr->ip_addr.ipv4_addr, ip_str_del,
                                 sizeof (ip_str_del));
                      DEBUG_SDPLANE_LOG (RIB, "delete IPv4 address: ifname=%s ip=%s",
                                         msg_ip_addr->ifname, ip_str_del);
                    }
                }
              else
                {
                  if (msg_ip_addr->is_ll_addr)
                    {
                      if (memcmp (&new->rib_info->vswitch[i].router_if.ll_addr, &msg_ip_addr->ip_addr.ipv6_addr,
                                  sizeof (struct in6_addr)) == 0)
                        {
                          memset (&new->rib_info->vswitch[i].router_if.ll_addr, 0, sizeof (struct in6_addr));

                          inet_ntop (AF_INET6, &msg_ip_addr->ip_addr.ipv6_addr, ip_str_del,
                                    sizeof (ip_str_del));
                          DEBUG_SDPLANE_LOG (RIB, "delete IPv6 address: ifname=%s ip=%s",
                                            msg_ip_addr->ifname, ip_str_del);
                        }
                    }
                  else
                    {
                      if (memcmp (&new->rib_info->vswitch[i].router_if.ipv6_addr, &msg_ip_addr->ip_addr.ipv6_addr,
                                  sizeof (struct in6_addr)) == 0)
                        {
                          memset (&new->rib_info->vswitch[i].router_if.ipv6_addr, 0, sizeof (struct in6_addr));

                          inet_ntop (AF_INET6, &msg_ip_addr->ip_addr.ipv6_addr, ip_str_del,
                                    sizeof (ip_str_del));
                          DEBUG_SDPLANE_LOG (RIB, "delete IPv6 address: ifname=%s ip=%s",
                                            msg_ip_addr->ifname, ip_str_del);
                        }
                    }
                }

              break;
            }
        }

      break;

    case INTERNAL_MSG_TYPE_SRV6_LOCAL_SID:
      struct internal_msg_srv6_local_sid *msg_srv6_local_sid;
      char addr_str[64];
      DEBUG_NEW (RIB, "recv msg_srv6_local_sid: %p.", msgp);
      msg_srv6_local_sid =
        (struct internal_msg_srv6_local_sid *) (msg_header + 1);
      inet_ntop (AF_INET6, &msg_srv6_local_sid->srv6_local_sid_addr,
                 addr_str, sizeof (addr_str));
      DEBUG_NEW (RIB, "recv msg_srv6_local_sid: addr: %s index: %d.",
                 addr_str, msg_srv6_local_sid->srv6_local_sid_addr_index);
      int num_sids = new->rib_info->srv6_local_sid_addr_num;
      int index = msg_srv6_local_sid->srv6_local_sid_addr_index;
      if (0 <= num_sids && num_sids < MAX_SRV6_LOCAL_SID_ADDR_NUM &&
          0 <= index && index <= num_sids)
        {
          DEBUG_NEW (RIB, "saving at local_sid_addr[%d/%d] success: addr: %s.",
                     index, num_sids, addr_str);
          memcpy (&new->rib_info->srv6_local_sid_addr[index],
                &msg_srv6_local_sid->srv6_local_sid_addr,
                sizeof (struct in6_addr));
          if (index == num_sids)
            new->rib_info->srv6_local_sid_addr_num++;
          DEBUG_NEW (RIB, "local_sid_addr_num: %d.",
                     new->rib_info->srv6_local_sid_addr_num);
        }
      else
        DEBUG_NEW (RIB, "saving at local_sid_addr[%d/%d] failed: addr: %s.",
                   index, num_sids, addr_str);
      break;

    default:
      DEBUG_SDPLANE_LOG (RIB, "recv msg unknown: %p.", msgp);
      break;
    }

  free (msgp);

  /* rebuild FIB from RIB for each routing table */
  for (i = 0; i < ROUTE_TREE_SIZE; i++)
    {
      /* skip NULL pointers and uninitialized RIB trees (family == 0) */
      if (rib_tree_master[i] && new->rib_info->fib_tree[i] &&
          rib_tree_master[i]->family != 0)
        {
          if (rebuild_fib_from_rib (rib_tree_master[i],
                                    new->rib_info->fib_tree[i]) != 0)
            DEBUG_SDPLANE_LOG (RIB, "failed to rebuild FIB[%d] (family=%d)", i,
                               rib_tree_master[i]->family);
        }
    }
  rib_replace (new);
}

#if 0
void
rib_manager_send_message (void *msgp, struct shell *shell)
{
  if (msg_queue_rib)
    {
      DEBUG_SDPLANE_LOG (RIB, "%s: sending message %p.", __func__, msgp);
      rte_ring_enqueue (msg_queue_rib, msgp);
    }
  else
    {
      fprintf (shell->terminal, "can't send message to rib: queue: NULL.%s",
               shell->NL);
    }
}
#endif

static __thread uint64_t loop_counter = 0;
static __thread time_t last_fdb_aging_time = 0;

int
rib_manager (void *arg)
{
  int ret, i;
  void *msgp;
  unsigned lcore_id = rte_lcore_id ();

  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);
  DEBUG_SDPLANE_LOG (RIB, "%s: started.", __func__);

  /* the tx_buffer_per_q is initialized in rib_manager. */
  memset (tx_buffer_per_q, 0, sizeof (tx_buffer_per_q));

  /* initialize */
  msg_queue_rib =
      rte_ring_create ("msg_queue_rib", 1024, SOCKET_ID_ANY, RING_F_SC_DEQ);
  DEBUG_NEW (RIB, "msg_queue_rib: %p", msg_queue_rib);

  int thread_id;
  thread_id = thread_lookup_by_lcore (rib_manager, lcore_id);
  thread_register_loop_counter (thread_id, &loop_counter);

  if (IS_LTHREAD ())
    DEBUG_NEW (RIB, "started as a lthread.");
  else
    DEBUG_NEW (RIB, "started on lcore: %d.", lcore_id);

#if 0
  /* This is the rcu producer. do we need to register ? */
#if HAVE_LIBURCU_QSBR
  if (! IS_LTHREAD ())
    urcu_qsbr_register_thread ();
#endif /*HAVE_LIBURCU_QSBR*/
#endif

  /* initialize fdb aging timer */
  last_fdb_aging_time = time (NULL);

  while (! force_quit && ! force_stop[lcore_id])
    {
      if (IS_LTHREAD ())
        lthread_sleep (0); // yield.
#if 0
      DEBUG_NEW (RIB, "%s: schedule (loop: %lu).",
                 __func__, loop_counter);
#endif

      /* rte_ring_dequeue_burst() or not */
#if 0
      msgp = internal_msg_recv (msg_queue_rib);
      if (msgp)
        rib_manager_process_message (msgp);
#else
#define RIB_RING_BURST_SIZE 512
      int num = 0;
      struct internal_msg_header *msg_table[RIB_RING_BURST_SIZE];
      num = internal_msg_recv_burst (msg_queue_rib, msg_table,
                                     RIB_RING_BURST_SIZE);
      for (int i = 0; i < num; i++)
        {
          msgp = msg_table[i];
          rib_manager_process_message (msgp);
        }
#endif

      /* fdb aging process - run every 60 seconds */
      time_t current_time = time (NULL);
      struct rib *current_rib = NULL;
#if HAVE_LIBURCU_QSBR
      /* This is the rcu producer.
         I think we don't need to lock/unlock/quiescent. */
        {
          //urcu_qsbr_read_lock ();
          current_rib = rcu_dereference (rcu_global_ptr_rib);
        }
#endif /*HAVE_LIBURCU_QSBR*/

      if (current_time - last_fdb_aging_time >= 60 && current_rib &&
          current_rib->rib_info)
        {
          fdb_aging_process (current_rib->rib_info, FDB_AGING_TIME_DEFAULT);
          DEBUG_SDPLANE_LOG (FDB, "fdb aging process executed");
          last_fdb_aging_time = current_time;
        }

#if HAVE_LIBURCU_QSBR
      /* This is the rcu producer.
         I think we don't need to lock/unlock/quiescent. */
        {
          //urcu_qsbr_read_unlock ();
          //urcu_qsbr_quiescent_state ();
        }
#endif /*HAVE_LIBURCU_QSBR*/

      loop_counter++;
    }

  rte_ring_free (msg_queue_rib);
  for (i = 0; i < ROUTE_TREE_SIZE; i++)
    {
      rib_free (rib_tree_master[i]);
    }
  nexthop_legacy_cleanup_hash_table (nh_hash_table);

  DEBUG_SDPLANE_LOG (RIB, "%s: terminating.", __func__);
  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);

#if 0
  /* This is the rcu producer. do we need to register ? */
#if HAVE_LIBURCU_QSBR
  if (! IS_LTHREAD ())
    urcu_qsbr_unregister_thread ();
#endif /*HAVE_LIBURCU_QSBR*/
#endif

  return 0;
}
