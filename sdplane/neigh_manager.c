/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#include "include.h"

#include <lthread.h>

#include <rte_hash.h>
#include <rte_jhash.h>
#include <rte_ether.h>

#include <sdplane/debug.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_log.h>

#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "sdplane.h"
#include "thread_info.h"

#include "rib_manager.h"
#include "neigh_manager.h"

#include "internal_message.h"

struct rte_ring *msg_queue_neigh;

static __thread struct neigh_table primary_neigh_tables[NEIGH_NR_TABLES];

static __thread uint64_t loop_counter = 0;

const int neigh_key_lengths[NEIGH_NR_TABLES] = {
  [NEIGH_ARP_TABLE] = sizeof (struct in_addr),
  [NEIGH_ND_TABLE] = sizeof (struct in6_addr),
};

static inline __attribute__ ((always_inline)) const char *
neigh_manager_table_str (int index)
{
  switch (index)
    {
    case NEIGH_ARP_TABLE:
      return "arp_table";
    case NEIGH_ND_TABLE:
      return "nd_table";
    default:
      return "unknown";
    }

  return NULL;
}

static inline __attribute__ ((always_inline)) const char *
neigh_manager_state_str (uint8_t state)
{
  switch (state)
    {
    case NEIGH_STATE_NONE:
      return "none";
    case NEIGH_STATE_INCOMPLETE:
      return "incomplete";
    case NEIGH_STATE_REACHABLE:
      return "reachable";
    case NEIGH_STATE_STALE:
      return "stale";
    case NEIGH_STATE_DELAY:
      return "delay";
    case NEIGH_STATE_PROBE:
      return "probe";
    case NEIGH_STATE_FAILED:
      return "failed";
    case NEIGH_STATE_NOARP:
      return "noarp";
    case NEIGH_STATE_PERMANENT:
      return "permanent";
    default:
      return "unknown";
    }
}

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

  return hash % MAX_NEIGHBOR_TABLE_SIZE;
}

static inline __attribute__ ((always_inline)) int
neigh_manager_add_entry (struct neigh_table *neigh_table, const int index,
                         const void *key, const struct neigh_entry *data)
{
  uint32_t hash, offset;

  hash = jenkins_hash ((uint8_t *) key, neigh_key_lengths[index]);
  offset = hash;
  while (neigh_table->entries[offset].state != NEIGH_STATE_NONE)
    {
      if (! memcmp (&neigh_table->entries[offset].ip_addr, key,
                    neigh_key_lengths[index]))
        {
          DEBUG_SDPLANE_LOG (NEIGH, "duplicate entry at %s[%u]",
                             neigh_manager_table_str (index), offset);
          return -1;
        }

      ++offset;
      if (offset >= MAX_NEIGHBOR_TABLE_SIZE)
        offset = 0;
      if (offset == hash)
        {
          DEBUG_SDPLANE_LOG (NEIGH, "neigh_table is full.");
          return -1;
        }
    }
  memcpy (&neigh_table->entries[offset], data, sizeof (struct neigh_entry));

  DEBUG_SDPLANE_LOG (NEIGH, "added entry at %s[%u]",
                     neigh_manager_table_str (index), offset);
  return (int) offset;
}

static inline __attribute__ ((always_inline)) int
neigh_manager_delete_entry (struct neigh_table *neigh_table, const int index,
                            const void *key)
{
  uint32_t hash, offset;

  hash = jenkins_hash ((uint8_t *) key, neigh_key_lengths[index]);
  offset = hash;

  while (neigh_table->entries[offset].state != NEIGH_STATE_NONE)
    {
      if (! memcmp (&neigh_table->entries[offset].ip_addr, key,
                    neigh_key_lengths[index]))
        {
          memset (&neigh_table->entries[offset], 0,
                  sizeof (struct neigh_entry));
          DEBUG_SDPLANE_LOG (NEIGH, "deleted entry at %s[%u]",
                             neigh_manager_table_str (index), offset);
          return (int) offset;
        }

      ++offset;
      if (offset >= MAX_NEIGHBOR_TABLE_SIZE)
        offset = 0;
      if (offset == hash)
        break;
    }

  DEBUG_SDPLANE_LOG (NEIGH, "entry not found in %s",
                     neigh_manager_table_str (index));
  return -1;
}

int
neigh_manager_lookup (const struct neigh_table *neigh_table, const int index,
                      const void *key, struct neigh_entry *out)
{
  uint32_t hash, offset;

  hash = jenkins_hash ((uint8_t *) key, neigh_key_lengths[index]);
  offset = hash;

  while (neigh_table->entries[offset].state != NEIGH_STATE_NONE)
    {
      if (! memcmp (&neigh_table->entries[offset].ip_addr, key,
                    neigh_key_lengths[index]))
        {
          memcpy (out, &neigh_table->entries[offset],
                  sizeof (struct neigh_entry));
          DEBUG_SDPLANE_LOG (NEIGH, "lookup hit at %s[%u]",
                             neigh_manager_table_str (index), offset);
          return (int) offset;
        }

      ++offset;
      if (offset >= MAX_NEIGHBOR_TABLE_SIZE)
        offset = 0;
      if (offset == hash)
        break;
    }

  DEBUG_SDPLANE_LOG (NEIGH, "lookup failed in %s",
                     neigh_manager_table_str (index));
  return -1;
}

void
neigh_manager_show_table (const int index, const struct shell *shell)
{
  int i;
  char addr[64];
  char lladdr[RTE_ETHER_ADDR_FMT_SIZE];
  struct rib *rib = rib_tlocal;

  for (i = 0; i < MAX_NEIGHBOR_TABLE_SIZE; i++)
    {
      if (rib->rib_info->neigh_tables[index].entries[i].state ==
          NEIGH_STATE_NONE)
        continue;
      inet_ntop (rib->rib_info->neigh_tables[index].entries[i].family,
                 &rib->rib_info->neigh_tables[index].entries[i].ip_addr, addr,
                 sizeof (addr));
      rte_ether_format_addr (
          lladdr, sizeof (lladdr),
          &rib->rib_info->neigh_tables[index].entries[i].mac_addr);
      fprintf (shell->terminal, "%s lladdr %s state %s%s", addr, lladdr,
               neigh_manager_state_str (
                   rib->rib_info->neigh_tables[index].entries[i].state),
               shell->NL);
    }
}

void
neigh_manager_process_message (void *msgp, struct neigh_table *neigh_tables)
{
  int i, ret;
  DEBUG_SDPLANE_LOG (NEIGH, "%s: msg: %p.", __func__, msgp);

  void *new_msgp;
  struct internal_msg_header *msg_header;
  struct internal_msg_neigh_entry *msg_neigh_entry;

  msg_header = (struct internal_msg_header *) msgp;
  switch (msg_header->type)
    {
    case INTERNAL_MSG_TYPE_NEIGH_ENTRY_ADD:
      DEBUG_SDPLANE_LOG (NEIGH, "recv msg_neigh_add_entry: %p.", msgp);
      msg_neigh_entry = (struct internal_msg_neigh_entry *) (msg_header + 1);
      ret = neigh_manager_add_entry (
          &neigh_tables[msg_neigh_entry->index], msg_neigh_entry->index,
          &msg_neigh_entry->data.ip_addr, &msg_neigh_entry->data);
      if (ret < 0)
        return;
      msg_neigh_entry->hash = ret;
      new_msgp =
          internal_msg_create (INTERNAL_MSG_TYPE_NEIGH_ENTRY_ADD,
                               msg_neigh_entry, sizeof (*msg_neigh_entry));
      internal_msg_send_to (msg_queue_rib, new_msgp, NULL);
      break;

    case INTERNAL_MSG_TYPE_NEIGH_ENTRY_DEL:
      DEBUG_SDPLANE_LOG (NEIGH, "recv msg_neigh_del_entry: %p.", msgp);
      msg_neigh_entry = (struct internal_msg_neigh_entry *) (msg_header + 1);
      neigh_manager_delete_entry (&neigh_tables[msg_neigh_entry->index],
                                  msg_neigh_entry->index,
                                  &msg_neigh_entry->data.ip_addr);
      msg_neigh_entry->hash = ret;
      new_msgp =
          internal_msg_create (INTERNAL_MSG_TYPE_NEIGH_ENTRY_DEL,
                               msg_neigh_entry, sizeof (*msg_neigh_entry));
      internal_msg_send_to (msg_queue_rib, new_msgp, NULL);
      break;

      // address resolution requests, etc.

    default:
      DEBUG_SDPLANE_LOG (NEIGH, "recv msg unknown: %p.", msgp);
      break;
    }

  free (msgp);
}

int
neigh_manager (void *arg __rte_unused)
{
  int i;
  void *msgp;
  unsigned lcore_id = rte_lcore_id ();

  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);
  DEBUG_SDPLANE_LOG (NEIGH, "%s: started.", __func__);

  /* initialize */
  msg_queue_neigh =
      rte_ring_create ("msg_queue_neigh", 32, SOCKET_ID_ANY, RING_F_SC_DEQ);

  int thread_id;
  thread_id = thread_lookup (neigh_manager);
  thread_register_loop_counter (thread_id, &loop_counter);

  /* initialize primary neigh tables */
  memset (primary_neigh_tables, 0, sizeof (primary_neigh_tables));

  while (! force_quit && ! force_stop[lcore_id])
    {
      lthread_sleep (100); // yield.
      // DEBUG_SDPLANE_LOG (NEIGH, "%s: schedule.", __func__);

      msgp = internal_msg_recv (msg_queue_neigh);
      if (msgp)
        neigh_manager_process_message (msgp, primary_neigh_tables);

      loop_counter++;
    }

  rte_ring_free (msg_queue_neigh);

  DEBUG_SDPLANE_LOG (NEIGH, "%s: terminating.", __func__);
  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);

  return 0;
}
