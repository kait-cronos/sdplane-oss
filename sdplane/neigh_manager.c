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

static __thread uint64_t loop_counter = 0;
static __thread struct rib *rib = NULL;

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

void
neigh_manager_create_table (struct rte_hash **neigh_tables, int index,
                            int key_len)
{
  struct rte_hash_parameters params = { .name = NULL,
                                        .entries = MAX_NEIGHBOR_TABLE_SIZE,
                                        .key_len = key_len,
                                        .hash_func = rte_jhash,
                                        .hash_func_init_val = 0,
                                        .socket_id = rte_socket_id () };

  params.name = neigh_manager_str (index);
  neigh_tables[index] = rte_hash_create (&params);
  if (neigh_tables[index] == NULL)
    {
      DEBUG_SDPLANE_LOG (NEIGH, "failed to create neighbor table: %s", params.name);
      return;
    }

  DEBUG_SDPLANE_LOG (NEIGH, "neighbor table %s created.", params.name);
}

void
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

int
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

int
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

int
neigh_manager_lookup (const int index, const void *key,
                      struct neigh_entry_data *out)
{
  int ret;

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_read_lock ();
  rib = (struct rib *) rcu_dereference (rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

  ret = rte_hash_lookup_data (rib->rib_info->neigh_tables[index], key,
                              (void *) out);

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_read_unlock ();
  urcu_qsbr_quiescent_state ();
#endif /*HAVE_LIBURCU_QSBR*/

  return ret;
}

void
neigh_manager_show_table (const int index, const struct shell *shell)
{
  struct neigh_entry_data *data;
  void *key;
  uint32_t iter = 0;
  char addr[64];
  char buf[RTE_ETHER_ADDR_FMT_SIZE];

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_read_lock ();
  rib = (struct rib *) rcu_dereference (rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

  while (rte_hash_iterate (rib->rib_info->neigh_tables[index], (void *) &key,
                           (void **) &data, &iter) >= 0)
    {
      inet_ntop (data->family, &data->ip_addr_key, addr, sizeof (addr));
      rte_ether_format_addr (buf, sizeof (buf), &data->lladdr);
      fprintf (shell->terminal, "%s lladdr %s\n", addr, buf);
    }

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_read_unlock ();
  urcu_qsbr_quiescent_state ();
#endif /*HAVE_LIBURCU_QSBR*/
}

int
neigh_manager (void *arg __rte_unused)
{
  unsigned lcore_id = rte_lcore_id ();

  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);
  DEBUG_SDPLANE_LOG (NEIGH, "%s: started.", __func__);

  int thread_id;
  thread_id = thread_lookup (neigh_manager);
  thread_register_loop_counter (thread_id, &loop_counter);

  void *msgp;
  msgp = internal_msg_create (INTERNAL_MSG_TYPE_NEIGH_CREATE_TABLE, NULL, 0);
  internal_msg_send_to (msg_queue_rib, msgp, NULL);

  while (! force_quit && ! force_stop[lcore_id])
    {
      lthread_sleep (100); // yield.
      // DEBUG_SDPLANE_LOG (NEIGH, "%s: schedule.", __func__);

      loop_counter++;
    }

  DEBUG_SDPLANE_LOG (NEIGH, "%s: terminating.", __func__);
  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_NEIGH_FREE_TABLE, NULL, 0);
  internal_msg_send_to (msg_queue_rib, msgp, NULL);

  return 0;
}
