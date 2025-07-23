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

static __thread struct rte_hash *primary_neigh_tables[NEIGH_NR_TABLES];

static __thread uint64_t loop_counter = 0;

const int neigh_key_lengths[NEIGH_NR_TABLES] = {
  [NEIGH_ARP_TABLE] = sizeof(struct in_addr),
  [NEIGH_ND_TABLE]  = sizeof(struct in6_addr),
};

static inline __attribute__ ((always_inline)) const char *
neigh_manager_str (int index)
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

/*
 * NOTE:
 * The rte_hash library does not manage data entities.
 * sdplane must be responsible for allocating, storing, and freeing associated
 * data.
 */

static inline __attribute__ ((always_inline)) void
neigh_manager_create_table (struct rte_hash **neigh_tables, int index,
                            int key_len)
{
  char name[64];
  static int global_table_counter = 0;
  int id = __sync_fetch_and_add (&global_table_counter, 1);

  struct rte_hash_parameters params = { .name = NULL,
                                        .entries = MAX_NEIGHBOR_TABLE_SIZE,
                                        .key_len = key_len,
                                        .hash_func = rte_jhash,
                                        .hash_func_init_val = 0,
                                        .socket_id = rte_socket_id () };

  snprintf(name, sizeof(name), "%s(%d)", neigh_manager_str (index), id);
  params.name = name;
  neigh_tables[index] = rte_hash_create (&params);
  if (neigh_tables[index] == NULL)
    {
      DEBUG_SDPLANE_LOG (NEIGH,
              "Failed to create neighbor table: %s (rte_errno=%d: %s)\n",
              params.name, rte_errno, rte_strerror(rte_errno));
      return;
    }

  DEBUG_SDPLANE_LOG (NEIGH, "neighbor table %s created.", params.name);
}

static inline __attribute__ ((always_inline)) void
neigh_manager_free_table (struct rte_hash **neigh_tables, int index)
{
  struct neigh_entry_data *data;
  void *key;
  uint32_t iter = 0;

  while (rte_hash_iterate (neigh_tables[index], (void *) &key, (void **) &data,
                           &iter) >= 0)
    free (data);
  rte_hash_free (neigh_tables[index]);

  DEBUG_SDPLANE_LOG (NEIGH, "neighbor table %s freed.",
                     neigh_manager_str (index));
}

static inline __attribute__ ((always_inline)) int
neigh_manager_add_entry (struct rte_hash **neigh_tables, const int index,
                         const void *key, const struct neigh_entry_data *data)
{
  struct neigh_entry_data *data_copy =
      malloc (sizeof (struct neigh_entry_data));
  if (! data_copy)
    {
      DEBUG_SDPLANE_LOG (NEIGH,
                         "failed to allocate memory for neighbor entry data.");
      return -1;
    }
  memcpy (data_copy, data, sizeof (struct neigh_entry_data));

  return rte_hash_add_key_data (neigh_tables[index], key, (void *) data_copy);
}

static inline __attribute__ ((always_inline)) int
neigh_manager_delete_entry (struct rte_hash **neigh_tables, const int index,
                            const void *key)
{
  struct neigh_entry_data *data;
  if (rte_hash_lookup_data (neigh_tables[index], key, (void *) &data) < 0)
    {
      DEBUG_SDPLANE_LOG (NEIGH, "neighbor entry not found for key.");
      return -1;
    }
  free (data);

  return rte_hash_del_key (neigh_tables[index], key);
}

/**
 * Usage example:
 *
 *   struct neigh_entry_data *entry;
 *   char buf[RTE_ETHER_ADDR_FMT_SIZE];
 *   int ret;
 *
 *   int ret = neigh_manager_lookup (index, key, &entry);
 *   if (ret < 0)
 *       return -1;
 *
 *   rte_ether_format_addr (buf, sizeof (buf), &entry->lladdr);
 *   fprintf (stderr, "result: %s\n", buf);
 */
int
neigh_manager_lookup (const int index, const void *key,
                      struct neigh_entry_data **out)
{
  struct rib *rib = rib_tlocal;

  return rte_hash_lookup_data (rib->rib_info->neigh_tables[index], key, (void **) out);
}

void
neigh_manager_show_table (const int index, const struct shell *shell)
{
  struct neigh_entry_data *data;
  void *key;
  uint32_t iter = 0;
  char addr[64];
  char buf[RTE_ETHER_ADDR_FMT_SIZE];
  struct rib *rib = rib_tlocal;

  while (rte_hash_iterate (rib->rib_info->neigh_tables[index], (void *) &key,
                           (void **) &data, &iter) >= 0)
    {
      inet_ntop (data->family, key, addr, sizeof (addr));
      rte_ether_format_addr (buf, sizeof (buf), &data->lladdr);
      fprintf (shell->terminal, "%s lladdr %s\n", addr, buf);
    }
}

void
neigh_manager_process_message (void *msgp, struct rte_hash **neigh_tables, struct rte_ring *msg_queue)
{
  int i, ret;
  DEBUG_SDPLANE_LOG (NEIGH, "%s: msg: %p.", __func__, msgp);

  bool forward_to_rib = false;
  struct internal_msg_header *msg_header;
  struct internal_msg_neigh_entry *msg_neigh_entry;

  msg_header = (struct internal_msg_header *) msgp;
  switch (msg_header->type)
    {
    case INTERNAL_MSG_TYPE_NEIGH_CREATE_TABLE:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_neigh_create_table: %p.", msgp);
      for (i = 0; i < NEIGH_NR_TABLES; i++)
        neigh_manager_create_table (neigh_tables, i, neigh_key_lengths[i]);
      forward_to_rib = true;
      break;

    case INTERNAL_MSG_TYPE_NEIGH_FREE_TABLE:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_neigh_free_table: %p.", msgp);
      for (i = 0; i < NEIGH_NR_TABLES; i++)
          neigh_manager_free_table (neigh_tables, i);
      forward_to_rib = true;
      break;

    case INTERNAL_MSG_TYPE_NEIGH_ENTRY_ADD:
      DEBUG_SDPLANE_LOG (NEIGH, "recv msg_neigh_add_entry: %p.", msgp);
      msg_neigh_entry = (struct internal_msg_neigh_entry *) (msg_header + 1);
      ret = neigh_manager_add_entry (
          neigh_tables, msg_neigh_entry->index,
          &msg_neigh_entry->ip_addr_key, &msg_neigh_entry->data);
      if (ret == EINVAL) /* entry already exists. */
        DEBUG_SDPLANE_LOG (NEIGH, "neigh_manager_add_entry: EINVAL.");
      else if (ret == ENOSPC) /* table is full. */
        DEBUG_SDPLANE_LOG (NEIGH, "neigh_manager_add_entry: ENOSPC.");
      forward_to_rib = true;
      break;

    case INTERNAL_MSG_TYPE_NEIGH_ENTRY_DEL:
      DEBUG_SDPLANE_LOG (NEIGH, "recv msg_neigh_del_entry: %p.", msgp);
      msg_neigh_entry = (struct internal_msg_neigh_entry *) (msg_header + 1);
      neigh_manager_delete_entry (neigh_tables,
                                  msg_neigh_entry->index,
                                  &msg_neigh_entry->ip_addr_key);
      forward_to_rib = true;
      break;

    // address resolution requests, etc.

    default:
      DEBUG_SDPLANE_LOG (NEIGH, "recv msg unknown: %p.", msgp);
      break;
    }


  if (msg_queue == msg_queue_neigh)
    {
      if (forward_to_rib)
        {
          /* reflect updates from primary_neigh_tables into rib's copy. (rib->rib_info->neigh_tables) */
          DEBUG_SDPLANE_LOG (NEIGH, "forwarding msg to rib: %p.", msgp);
          /* msgp will be freed by rib_manager_process_message() */
          internal_msg_send_to (msg_queue_rib, msgp, NULL);
        }
      else
        free (msgp);
    }

    /* if msg_queue == msg_queue_rib, msgp will also be freed by rib_manager_process_message() */
}

int
neigh_manager (void *arg __rte_unused)
{
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

  /* create the neighbor tables */
  msgp = internal_msg_create (INTERNAL_MSG_TYPE_NEIGH_CREATE_TABLE, NULL, 0);
  internal_msg_send_to (msg_queue_neigh, msgp, NULL);

  while (! force_quit && ! force_stop[lcore_id])
    {
      lthread_sleep (100); // yield.
      // DEBUG_SDPLANE_LOG (NEIGH, "%s: schedule.", __func__);

      msgp = internal_msg_recv (msg_queue_neigh);
      if (msgp)
        neigh_manager_process_message (msgp, primary_neigh_tables, msg_queue_neigh);

      loop_counter++;
    }

  /* free the neighbor tables */
  msgp = internal_msg_create (INTERNAL_MSG_TYPE_NEIGH_FREE_TABLE, NULL, 0);
  internal_msg_send_to (msg_queue_neigh, msgp, NULL);

  rte_ring_free (msg_queue_neigh);

  DEBUG_SDPLANE_LOG (NEIGH, "%s: terminating.", __func__);
  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);

  return 0;
}
