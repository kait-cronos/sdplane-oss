#ifndef __FDB_H__
#define __FDB_H__

#include "rib.h"
#include "sdplane.h"
#include "jhash.h"
#include "hash_table.h"

#include "debug_sdplane.h"

/* cached time, refreshed once per data-path loop */
extern volatile time_t fdb_now;

static inline __attribute__ ((always_inline)) uint64_t
fdb_pack_key (const struct rte_ether_addr *mac_addr, uint16_t vlan_id)
{
  uint64_t key = 0;
  int i;

  for (i = 0; i < RTE_ETHER_ADDR_LEN; i++)
    key = (key << 8) | mac_addr->addr_bytes[i];
  key = (key << 16) | vlan_id;

  return key;
}

static inline __attribute__ ((always_inline)) int
fdb_lookup_entry (const struct rib_info *rib_info,
                  const struct rte_ether_addr *mac_addr, uint16_t vlan_id)
{
  uint64_t key = fdb_pack_key (mac_addr, vlan_id);
  struct fdb_entry *entry =
    hash_table_lookup (&rib_info->fdb_table, (void *) (uintptr_t) key);
  int ret = entry ? entry->port : -1;

  /* refresh active MAC so aging keeps it */
  if (entry && fdb_now)
    entry->last_seen = fdb_now;

  /* Note: per-packet path; do NOT enable FDB_LOOKUP debug under load. */
  if (IS_DEBUG (FDB_LOOKUP))
    {
      char mac_str[32] = { 0 };
      rte_ether_format_addr (mac_str, sizeof (mac_str), mac_addr);
      if (ret < 0)
        DEBUG_NEW (FDB_LOOKUP, "fdb_lookup: %s vlan: %u missed.", mac_str,
                   vlan_id);
      else
        DEBUG_NEW (FDB_LOOKUP, "fdb_lookup: %s vlan: %u port: %d.", mac_str,
                   vlan_id, ret);
    }

  return ret;
}

int fdb_table_init (struct hash_table *ht);
int fdb_table_copy (struct hash_table *dst, struct hash_table *src);
void fdb_table_destroy (struct hash_table *ht);

int fdb_add_entry (struct rib_info *rib_info,
                   const struct rte_ether_addr *mac_addr, uint16_t vlan_id,
                   int port);

int fdb_aging_process (struct rib_info *rib_info, time_t aging_time);
int fdb_count_expired (struct rib_info *rib_info, time_t aging_time);

void show_fdb_entry_callback (void *key, void *value, void *arg);

#endif /*__FDB_H__*/
