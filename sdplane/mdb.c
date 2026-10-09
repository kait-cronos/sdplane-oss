#include "include.h"

#include "debug_sdplane.h"
#include "rib_manager.h"
#include "sdplane.h"
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
