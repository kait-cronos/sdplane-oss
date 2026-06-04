#ifndef __RIB_MANAGER_H__
#define __RIB_MANAGER_H__

#include <rte_ethdev.h>
#include "queue_config.h"

#include "rib.h"
#include "sdplane.h"

#include "debug_sdplane.h"

extern struct rib_tree *rib_tree_master[ROUTE_TREE_SIZE];
extern struct hash_table nh_ht;

struct rib
{
  struct rib_info *rib_info;
} __rte_cache_aligned;

extern void *rcu_global_ptr_rib;
extern uint64_t rib_rcu_replace;

extern struct rte_ring *ring_up[RTE_MAX_ETHPORTS][MAX_RX_QUEUE_PER_LCORE];
extern struct rte_ring *ring_dn[RTE_MAX_ETHPORTS][MAX_RX_QUEUE_PER_LCORE];

extern struct rte_ring *router_if_ring_up[MAX_VSWITCH];
extern struct rte_ring *router_if_ring_dn[MAX_VSWITCH];
extern struct rte_ring *capture_if_ring_up[MAX_VSWITCH];
extern struct rte_ring *capture_if_ring_dn[MAX_VSWITCH];

void rib_manager_recv_message (void *msgp);
//void rib_manager_send_message (void *msgp, struct shell *shell);

#if 1
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
fdb_lookup_entry (const struct rib_info *rib_info,
                  const struct rte_ether_addr *mac_addr, uint16_t vlan_id)
{
  uint32_t hash, offset;
  int ret = -1;
  char mac_str[32] = { 0 };

  hash = fdb_jenkins_hash (mac_addr, vlan_id);
  offset = hash;

  if (IS_DEBUG (FDB_LOOKUP))
    {
      rte_ether_format_addr (mac_str, sizeof (mac_str), mac_addr);
#if 0
      DEBUG_LOG_MSG ("fdb_lookup: %s vlan: %u offset: %u.",
                     mac_str, vlan_id, offset);
#endif
    }

  //while (rib_info->fdb[offset].state != FDB_STATE_NONE)
  while (1)
    {
      //DEBUG_NEW (FDB, "check fdb[%u].", offset);
      if (rte_is_same_ether_addr (&rib_info->fdb[offset].l2addr, mac_addr) &&
          rib_info->fdb[offset].vlan_id == vlan_id)
        {
          if (rib_info->fdb[offset].state == FDB_STATE_ACTIVE)
            {
#if 0
              DEBUG_NEW (FDB, "lookup hit at fdb[%u]: port %d",
                         offset, rib_info->fdb[offset].port);
#endif
              ret = rib_info->fdb[offset].port;
              break;
            }
        }

      ++offset;
      if (offset >= FDB_SIZE)
        offset = 0;
      if (offset == hash)
        break;
    }

  if (ret < 0)
    DEBUG_NEW (FDB_LOOKUP,
               "fdb_lookup: %s vlan: %u hash: %u missed.",
               mac_str, vlan_id, hash);
  else
    DEBUG_NEW (FDB_LOOKUP,
               "fdb_lookup: %s vlan: %u hash: %u offset: %u port: %d.",
               mac_str, vlan_id, hash, offset, ret);

  return ret;
}
#else
int fdb_lookup_entry (const struct rib_info *rib_info,
                      const struct rte_ether_addr *mac_addr, uint16_t vlan_id);
#endif

#include <sdplane/debug.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include "debug_sdplane.h"

#define rib_send_message(msgp) \
do { \
  if (msg_queue_rib) \
    { \
      DEBUG_SDPLANE_LOG (RIB, "sending message to rib: %p.", (msgp)); \
      if (rte_ring_enqueue (msg_queue_rib, (msgp)) != 0) \
        { \
          WARNING ("rte_ring_enqueue: msg_queue_rib full. " \
                   "message %p lost.", (msgp)); \
          free (msgp); \
        } \
    } \
  else \
    { \
      WARNING ("can't send message to rib: queue: NULL."); \
      free (msgp); \
    } \
} while (0)

#define shell_rib_send_message(msgp, shell) \
do { \
  if (msg_queue_rib) \
    { \
      DEBUG_SDPLANE_LOG (RIB, "sending message to rib: %p.", (msgp)); \
      if (rte_ring_enqueue (msg_queue_rib, (msgp)) != 0) \
        { \
          WARNING ("rte_ring_enqueue: msg_queue_rib full. " \
                   "message %p lost.", (msgp)); \
          free (msgp); \
        } \
    } \
  else if (shell) \
    { \
      fprintf ((shell)->terminal, "can't send message to rib: queue: NULL.%s", \
               (shell)->NL); \
      free (msgp); \
    } \
  else \
    { \
      WARNING ("can't send message to rib: rib_queue: NULL."); \
      free (msgp); \
    } \
} while (0)

#endif /*__RIB_MANAGER_H__*/
