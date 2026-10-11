#include "include.h"

#include "debug_sdplane.h"
#include "rib.h"
#include "rib_manager.h"
#include "sdplane.h"
#include <netinet/in.h>
#include "mdb.h"

volatile time_t mdb_now = 0;

static int
mdb_eq_key (const void *a, const void *b)
{
  return a == b;
}

int
mdb_table_init (struct hash_table *ht)
{
  return hash_table_init (ht, MDB_HASH_BUCKETS, mdb_hash, mdb_eq_key);
}

static void
mdb_free_value_callback (void *key, void *value, void *arg)
{
  free (value);
}

void
mdb_table_destroy (struct hash_table *ht)
{
  hash_table_foreach (ht, mdb_free_value_callback, NULL);
  hash_table_destroy (ht);
}

/*
 * MDB table deep-copy: for each entry in src, allocate a new entry and insert into dst.
 */

/* context carried into mdb_table_copy's foreach callback. */
struct mdb_copy_arg
{
  struct hash_table *dst;
  int err; /* set to -1 if any error occurs during copying */
};

static void
mdb_copy_callback (void *key, void *value, void *arg)
{
  struct mdb_copy_arg *copy_arg = arg;
  struct mdb_entry *dst, *src = value;

  if (copy_arg->err)
    return;

  dst = malloc (sizeof (struct mdb_entry));
  if (! dst)
    {
      copy_arg->err = -1;
      return;
    }
  *dst = *src;
  if (hash_table_insert (copy_arg->dst, key, dst) < 0)
    {
      free (dst);
      copy_arg->err = -1;
    }
}

int
mdb_table_copy (struct hash_table *dst, struct hash_table *src)
{
  struct mdb_copy_arg copy_arg = { .dst = dst, .err = 0 };

  if (mdb_table_init (dst) < 0)
    return -1;

  hash_table_foreach (src, mdb_copy_callback, &copy_arg);
  if (copy_arg.err)
    {
      mdb_table_destroy (dst);
      return -1;
    }
  return 0;
}

/*
 * Add a new entry to the FDB table.
 */

int
mdb_add_entry (struct rib_info *rib_info, uint32_t group_addr,
               uint16_t vswitch_id, int port)
{
  if (port < 0 || port >= MAX_ETH_PORTS)
    {
      DEBUG_NEW (IGMP, "mdb_add_entry: invalid port: %d", port);
      return -1;
    }

  uint64_t key = mdb_pack_key (group_addr, vswitch_id);
  void *kptr = (void *) (uintptr_t) key;
  time_t current_time = time (NULL);
  struct mdb_entry *entry;

  /* update existing entry */
  entry = hash_table_lookup (&rib_info->mdb_table, kptr);
  if (entry)
    {
      entry->port_bitmap |= (uint32_t) 1U << port;
      entry->port_last_seen[port] = current_time;
      DEBUG_NEW (IGMP, "updated mdb: vswitch:%u port:%d bitmap:%#x",
                 vswitch_id, port, entry->port_bitmap);
      return 0;
    }

  /* insert new entry */
  entry = malloc (sizeof (struct mdb_entry));
  if (! entry)
    {
      DEBUG_NEW (IGMP, "mdb_add_entry: malloc failed.");
      return -1;
    }
  entry->group_addr = group_addr;
  entry->vswitch_id = vswitch_id;
  entry->port_bitmap = (uint32_t) 1U << port;
  entry->port_last_seen[port] = current_time;

  if (hash_table_insert (&rib_info->mdb_table, kptr, entry) < 0)
    {
      free (entry);
      DEBUG_NEW (IGMP, "mdb_add_entry: insert failed.");
      return -1;
    }

  char group_str[INET_ADDRSTRLEN];
  inet_ntop (AF_INET, &group_addr, group_str, sizeof (group_str));
  DEBUG_NEW (IGMP, "added mdb: %s vswitch:%u port:%d bitmap:%#x",
              group_str, vswitch_id, port, entry->port_bitmap);
  return 0;
}

int
mdb_del_entry (struct rib_info *rib_info, uint32_t group_addr,
               uint16_t vswitch_id, int port)
{
  if (port < 0 || port >= MAX_ETH_PORTS)
    return -1;

  uint64_t key = mdb_pack_key (group_addr, vswitch_id);
  void *kptr = (void *) (uintptr_t) key;
  struct mdb_entry *entry;

  entry = hash_table_lookup (&rib_info->mdb_table, kptr);
  if (! entry)
    return 0;
  entry->port_bitmap &= ~((uint32_t) 1U << port);
  entry->port_last_seen[port] = 0;

  if (entry->port_bitmap == 0)
    {
      free (entry);
      hash_table_remove (&rib_info->mdb_table, kptr);
    }
  return 0;
}

void
show_mdb_entry_callback (void *key, void *value, void *arg)
{
  struct mdb_entry *entry = value;
  struct shell *shell = arg;
  char group_str[INET_ADDRSTRLEN];

  inet_ntop (AF_INET, &entry->group_addr, group_str,
    sizeof (group_str));

  fprintf (shell->terminal,
    "  mdb: %s vswitch:%u bitmap:0x%08" PRIx32 " ports:",
    group_str, (unsigned int) entry->vswitch_id,
    entry->port_bitmap);

  for (int port = 0; port < MAX_ETH_PORTS; port++)
  {
    if (entry->port_bitmap & (1U << port))
      fprintf (shell->terminal, " %d", port);
  }

  fprintf (shell->terminal, "%s", shell->NL);
}
