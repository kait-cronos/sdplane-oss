#ifndef __MDB_H__
#define __MDB_H__

#include "rib.h"
#include "sdplane.h"
#include "jhash.h"
#include "hash_table.h"

extern volatile time_t mdb_now;

static inline __attribute__ ((always_inline)) uint32_t
mdb_hash (const void *key)
{
  uint64_t k = (uint64_t) (uintptr_t) key;
  return jhash ((const int32_t *) &k, (int) (sizeof (k) / 4), JHASH_SEED);
}

static inline __attribute__ ((always_inline)) uint64_t
mdb_pack_key (uint32_t group_addr, uint16_t vswitch_id)
{
  uint64_t key = 0;

  key = (key << 32) | group_addr;
  key = (key << 16) | vswitch_id;

  return key;
}

static inline __attribute__ ((always_inline)) void *
mdb_lookup (const struct hash_table *ht, const void *key)
{
  uint32_t idx = mdb_hash (key) & ht->mask;
  const struct ht_node *cur;

  for (cur = ht->buckets[idx]; cur; cur = cur->next)
    {
      if (cur->key == key)
        return cur->value;
    }
  return NULL;
}

static inline __attribute__ ((always_inline)) uint64_t
mdb_lookup_entry (const struct rib_info *rib_info, uint32_t group_addr,
                  uint16_t vswitch_id)
{
  uint64_t key = mdb_pack_key (group_addr, vswitch_id);
  struct mdb_entry *entry =
      mdb_lookup (&rib_info->mdb_table, (void *) (uintptr_t) key);
  uint64_t ret = entry ? entry->port_bitmap : 0;

  return ret;
}

int mdb_table_init (struct hash_table *ht);
int mdb_table_copy (struct hash_table *dst, struct hash_table *src);
void mdb_table_destroy (struct hash_table *ht);

int mdb_add_entry (struct rib_info *rib_info, uint32_t group_addr,
                   uint16_t vswitch_id, int port);

int mdb_del_entry (struct rib_info *rib_info, uint32_t group_addr,
                   uint16_t vswitch_id, int port);

void show_mdb_entry_callback (void *key, void *value, void *arg);

#endif /*__MDB_H__*/
