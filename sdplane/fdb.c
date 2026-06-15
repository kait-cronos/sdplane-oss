#include "include.h"

#include "debug_sdplane.h"
#include "rib_manager.h"
#include "sdplane.h"
#include "fdb.h"

volatile time_t fdb_now = 0;

static uint32_t
fdb_hash (const void *key)
{
  uint64_t k = (uint64_t) (uintptr_t) key;
  return jhash ((const int32_t *) &k, (int) (sizeof (k) / 4), JHASH_SEED);
}

static int
fdb_eq_key (const void *a, const void *b)
{
  return a == b;
}

int
fdb_table_init (struct hash_table *ht)
{
  return hash_table_init (ht, FDB_HASH_BUCKETS, fdb_hash, fdb_eq_key);
}

static void
fdb_free_value_callback (void *key, void *value, void *arg)
{
  free (value);
}

void
fdb_table_destroy (struct hash_table *ht)
{
  /* free each value, then free bucket array */
  hash_table_foreach (ht, fdb_free_value_callback, NULL);
  hash_table_destroy (ht);
}

/*
 * FDB table deep-copy: for each entry in src, allocate a new entry and insert into dst.
 */

/* context carried into fdb_table_copy's foreach callback. */
struct fdb_copy_arg
{
  struct hash_table *dst;
  int err; /* set to -1 if any error occurs during copying */
};

static void
fdb_copy_callback (void *key, void *value, void *arg)
{
  struct fdb_copy_arg *copy_arg = arg;
  struct fdb_entry *dst, *src = value;

  if (copy_arg->err)
    return;

  dst = malloc (sizeof (struct fdb_entry));
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
fdb_table_copy (struct hash_table *dst, struct hash_table *src)
{
  struct fdb_copy_arg copy_arg = { .dst = dst, .err = 0 };

  if (fdb_table_init (dst) < 0)
    return -1;

  hash_table_foreach (src, fdb_copy_callback, &copy_arg);
  if (copy_arg.err)
    {
      fdb_table_destroy (dst);
      return -1;
    }
  return 0;
}

/*
 * Add a new entry to the FDB table.
 */

int
fdb_add_entry (struct rib_info *rib_info, const struct rte_ether_addr *mac_addr,
               uint16_t vlan_id, int port)
{
  uint64_t key = fdb_pack_key (mac_addr, vlan_id);
  void *kptr = (void *) (uintptr_t) key;
  time_t current_time = time (NULL);
  struct fdb_entry *entry;

  /* update existing entry */
  entry = hash_table_lookup (&rib_info->fdb_table, kptr);
  if (entry)
    {
      entry->port = port;
      entry->last_seen = current_time;
      DEBUG_NEW (FDB, "updated fdb: vlan:%u port:%d", vlan_id, port);
      return 0;
    }

  /* insert new entry */
  entry = malloc (sizeof (struct fdb_entry));
  if (! entry)
    {
      DEBUG_NEW (FDB, "fdb_add_entry: malloc failed.");
      return -1;
    }
  entry->l2addr = *mac_addr;
  entry->vlan_id = vlan_id;
  entry->port = port;
  entry->last_seen = current_time;

  if (hash_table_insert (&rib_info->fdb_table, kptr, entry) < 0)
    {
      free (entry);
      DEBUG_NEW (FDB, "fdb_add_entry: insert failed.");
      return -1;
    }

  char mac_str[32];
  rte_ether_format_addr (mac_str, sizeof (mac_str), mac_addr);
  DEBUG_NEW (FDB, "added fdb: %s vlan:%u port:%d", mac_str, vlan_id, port);
  return 0;
}

/*
 * Age out stale entries from the FDB table.
 */

/* shared context for the aging / count-expired foreach callbacks. */
struct fdb_aging_arg
{
  struct hash_table *ht;
  time_t current_time;
  time_t aging_time;
  int count;
};

static void
fdb_aging_callback (void *key, void *value, void *arg)
{
  struct fdb_aging_arg *aging_arg = arg;
  struct fdb_entry *entry = value;

  if (aging_arg->current_time - entry->last_seen > aging_arg->aging_time)
    {
      if (IS_DEBUG (FDB))
        {
          char mac_str[32];
          rte_ether_format_addr (mac_str, sizeof (mac_str), &entry->l2addr);
          DEBUG_SDPLANE_LOG (FDB, "aged out fdb: %s vlan:%u",
                             mac_str, entry->vlan_id);
        }
      /* free the value, then remove the (current) node from the table */
      free (entry);
      hash_table_remove (aging_arg->ht, key);
      aging_arg->count++;
    }
}

int
fdb_aging_process (struct rib_info *rib_info, time_t aging_time)
{
  struct fdb_aging_arg aging_arg = {
    .ht = &rib_info->fdb_table,
    .current_time = time (NULL),
    .aging_time = aging_time,
    .count = 0,
  };

  hash_table_foreach (&rib_info->fdb_table, fdb_aging_callback, &aging_arg);
  return aging_arg.count;
}

/*
 * Count the number of expired entries in the FDB table.
 */

static void
fdb_count_expired_callback (void *key, void *value, void *arg)
{
  struct fdb_entry *entry = value;
  struct fdb_aging_arg *aging_arg = arg;

  if (aging_arg->current_time - entry->last_seen > aging_arg->aging_time)
    aging_arg->count++;
}

int
fdb_count_expired (struct rib_info *rib_info, time_t aging_time)
{
  struct fdb_aging_arg aging_arg = {
    .ht = &rib_info->fdb_table,
    .current_time = time (NULL),
    .aging_time = aging_time,
    .count = 0,
  };

  hash_table_foreach (&rib_info->fdb_table, fdb_count_expired_callback, &aging_arg);
  return aging_arg.count;
}

/*
 * Callback function to display FDB entries.
 */

void
show_fdb_entry_callback (void *key, void *value, void *arg)
{
  struct fdb_entry *entry = value;
  struct shell *shell = arg;
  char mac_str[32];
  char time_str[32];
  struct tm tm_buf;

  rte_ether_format_addr (mac_str, sizeof (mac_str), &entry->l2addr);

  localtime_r (&entry->last_seen, &tm_buf);
  strftime (time_str, sizeof (time_str), "%Y-%m-%d %H:%M:%S", &tm_buf);

  fprintf (shell->terminal, "  fdb: %s vlan:%u port:%d last_seen:%s%s",
           mac_str, entry->vlan_id, entry->port, time_str, shell->NL);
}
