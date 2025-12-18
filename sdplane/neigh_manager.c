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

struct rte_ring *msg_queue_neigh = NULL;

static __thread struct neigh_table master_neigh_tables[NEIGH_NR_TABLES];

static __thread uint64_t loop_counter = 0;

const int neigh_key_lengths[NEIGH_NR_TABLES] = {
  [NEIGH_ARP_TABLE] = sizeof (struct in_addr),
  [NEIGH_ND_TABLE] = sizeof (struct in6_addr),
};

const char *
neigh_manager_table_str (int type)
{
  switch (type)
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

const char *
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

static inline __attribute__ ((always_inline)) int
neigh_manager_add_entry (struct neigh_table *neigh_table, const int type,
                         const void *key, const struct neigh_entry *data)
{
  if (neigh_table->num_entries >= MAX_NEIGHBOR_ENTRY_SIZE)
    {
      DEBUG_SDPLANE_LOG (NEIGH, "master: neigh_table %s is full.",
                         neigh_manager_table_str (type));
      return -1;
    }

  struct neigh_entry *found;
  int pos;

  pos = neigh_manager_lookup (neigh_table, type, key, &found);
  if (found)
    {
      /* update existing entry */
      DEBUG_SDPLANE_LOG (NEIGH, "master: update: %s[%d] (total %d)",
                         neigh_manager_table_str (type), pos, neigh_table->num_entries);
      memcpy (found, data, sizeof *found);
      return pos;
    }
  else
    {
      if (pos < 0 || pos > neigh_table->num_entries)
        {
          DEBUG_SDPLANE_LOG (NEIGH, "invalid insertion position in %s.",
                             neigh_manager_table_str (type));
          return -1;
        }
      memmove (&neigh_table->entries[pos + 1], &neigh_table->entries[pos],
               sizeof (struct neigh_entry) *
                   (neigh_table->num_entries - pos));
      memcpy (&neigh_table->entries[pos], data, sizeof (struct neigh_entry));
      neigh_table->num_entries++;
      DEBUG_SDPLANE_LOG (NEIGH, "master: add: %s[%d] (total %d)",
                         neigh_manager_table_str (type), pos, neigh_table->num_entries);
      return pos;
    }

  return -1;
}

static inline __attribute__ ((always_inline)) int
neigh_manager_delete_entry (struct neigh_table *neigh_table, const int type,
                            const void *key)
{
  struct neigh_entry *found;
  int pos;

  pos = neigh_manager_lookup (neigh_table, type, key, &found);
  if (found)
    {
      neigh_table->num_entries--;
      memmove (&neigh_table->entries[pos], &neigh_table->entries[pos + 1],
               sizeof (struct neigh_entry) *
                   (neigh_table->num_entries - pos));
      DEBUG_SDPLANE_LOG (NEIGH, "master: del: %s[%d] (total %d)",
                         neigh_manager_table_str (type), pos, neigh_table->num_entries);
      return pos;
    }
  else
    DEBUG_SDPLANE_LOG (NEIGH, "master: no such entry in %s.",
                       neigh_manager_table_str (type));

  return -1;
}

int
neigh_manager_lookup (const struct neigh_table *neigh_table, const int type,
                      const void *key, struct neigh_entry **found)
{
  *found = NULL;

  int left = 0;
  int right = neigh_table->num_entries - 1;
  int mid, ret;
  struct neigh_entry *tmp;

  /* binary search */
  while (left <= right)
    {
      mid = (left + right) / 2;
      tmp = &neigh_table->entries[mid];
      ret = memcmp (&tmp->ip_addr, key, neigh_key_lengths[type]);
      if (ret == 0)
        {
          /* found */
          *found = tmp;
          DEBUG_SDPLANE_LOG (NEIGH, "lookup hit in %s[%d].",
                             neigh_manager_table_str (type), mid);
          return mid;
        }
      else if (ret < 0)
        left = mid + 1;
      else
        right = mid - 1;
    }

  /* not found */
  return left; // return the position to insert/delete the entry.
}

void
neigh_manager_show_table (const int type, const struct shell *shell)
{
  int i, ifindex;
  char addr[64];
  char lladdr[RTE_ETHER_ADDR_FMT_SIZE];
  struct rib *rib = rib_tlocal;

  fprintf (shell->terminal, "table type: %s (total %d)%s",
           neigh_manager_table_str (type),
           rib->rib_info->neigh_tables[type].num_entries, shell->NL);

  for (i = 0; i < rib->rib_info->neigh_tables[type].num_entries; i++)
    {
      if (rib->rib_info->neigh_tables[type].entries[i].state ==
          NEIGH_STATE_NONE)
        continue;
      inet_ntop (rib->rib_info->neigh_tables[type].entries[i].family,
                 &rib->rib_info->neigh_tables[type].entries[i].ip_addr, addr,
                 sizeof (addr));
      ifindex = rib->rib_info->neigh_tables[type].entries[i].ifindex;
      rte_ether_format_addr (
          lladdr, sizeof (lladdr),
          &rib->rib_info->neigh_tables[type].entries[i].mac_addr);
      fprintf (shell->terminal, "[%d] %s dev %d lladdr %s state %s%s", i, addr,
               ifindex, lladdr,
               neigh_manager_state_str (
                   rib->rib_info->neigh_tables[type].entries[i].state),
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
      /* ret is the index where the entry was added. */
      ret = neigh_manager_add_entry (
          &neigh_tables[msg_neigh_entry->type], msg_neigh_entry->type,
          &msg_neigh_entry->data.ip_addr, &msg_neigh_entry->data);
      /* if ret < 0, the table is full or entry already exists. */
      if (ret < 0)
        break;
      msg_neigh_entry->pos = ret;
      msg_neigh_entry->num_entries =
          neigh_tables[msg_neigh_entry->type].num_entries;
      new_msgp =
          internal_msg_create (INTERNAL_MSG_TYPE_NEIGH_ENTRY_ADD,
                               msg_neigh_entry, sizeof (*msg_neigh_entry));
      internal_msg_send_to (msg_queue_rib, new_msgp, NULL);
      break;

    case INTERNAL_MSG_TYPE_NEIGH_ENTRY_DEL:
      DEBUG_SDPLANE_LOG (NEIGH, "recv msg_neigh_del_entry: %p.", msgp);
      msg_neigh_entry = (struct internal_msg_neigh_entry *) (msg_header + 1);
      /* ret is the index where the entry was deleted. */
      ret = neigh_manager_delete_entry (&neigh_tables[msg_neigh_entry->type],
                                        msg_neigh_entry->type,
                                        &msg_neigh_entry->data.ip_addr);
      /* if ret < 0, no such entry. */
      if (ret < 0)
        break;
      msg_neigh_entry->pos = ret;
      msg_neigh_entry->num_entries =
          neigh_tables[msg_neigh_entry->type].num_entries;
      new_msgp =
          internal_msg_create (INTERNAL_MSG_TYPE_NEIGH_ENTRY_DEL,
                               msg_neigh_entry, sizeof (*msg_neigh_entry));
      internal_msg_send_to (msg_queue_rib, new_msgp, NULL);
      break;

    /* address resolution requests, etc. */
    // case INTERNAL_MSG_TYPE_NEIGH_XXX:

    default:
      DEBUG_SDPLANE_LOG (NEIGH, "recv msg unknown: %p.", msgp);
      break;
    }

  free (msgp);
}

void
neigh_manager_init ()
{
  /* initialize */
  if (! msg_queue_neigh)
    msg_queue_neigh =
        rte_ring_create ("msg_queue_neigh", 32, SOCKET_ID_ANY, RING_F_SC_DEQ);
}

int
neigh_manager (void *arg __rte_unused)
{
  int i;
  void *msgp;
  unsigned lcore_id = rte_lcore_id ();

  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);
  DEBUG_SDPLANE_LOG (NEIGH, "%s: started.", __func__);

  neigh_manager_init ();

  int thread_id;
  thread_id = thread_lookup (neigh_manager);
  thread_register_loop_counter (thread_id, &loop_counter);

  /* initialize master neigh tables */
  memset (master_neigh_tables, 0, sizeof (master_neigh_tables));

  while (! force_quit && ! force_stop[lcore_id])
    {
      lthread_sleep (0); // yield.
      // DEBUG_SDPLANE_LOG (NEIGH, "%s: schedule.", __func__);

      msgp = internal_msg_recv (msg_queue_neigh);
      if (msgp)
        neigh_manager_process_message (msgp, master_neigh_tables);

      loop_counter++;
    }

  rte_ring_free (msg_queue_neigh);

  DEBUG_SDPLANE_LOG (NEIGH, "%s: terminating.", __func__);
  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);

  return 0;
}
