#ifndef __NEIGH_MANAGER_H__
#define __NEIGH_MANAGER_H__

#include <rte_byteorder.h>

#include "rib_manager.h"

#define NEIGH_STATE_NONE       0x00
#define NEIGH_STATE_INCOMPLETE 0x01
#define NEIGH_STATE_REACHABLE  0x02
#define NEIGH_STATE_STALE      0x04
#define NEIGH_STATE_DELAY      0x08
#define NEIGH_STATE_PROBE      0x10
#define NEIGH_STATE_FAILED     0x20
#define NEIGH_STATE_NOARP      0x40
#define NEIGH_STATE_PERMANENT  0x80

/*
 * Binary search over the table sorted in memcmp() order of the keys.
 * Keys are compared as big-endian integers, which is equivalent to
 * memcmp() but avoids the libc call: memcmp() is not inline-expanded
 * when its three-way result (<0, ==0, >0) is used.
 */
static inline __attribute__ ((always_inline)) int
neigh_lookup_pos4 (const struct neigh_table *neigh_table, const void *key,
                   struct neigh_entry **found)
{
  int left = 0;
  int right = neigh_table->num_entries - 1;
  int mid;
  uint32_t x;
  const struct neigh_entry *tmp;
  /* Read the IPv4 address (NBO) as an integer.
   * The search key is loaded only once, before the loop. */
  const uint32_t k = rte_be_to_cpu_32 (*(const uint32_t *) key);

  *found = NULL;
  while (left <= right)
    {
      mid = (left + right) / 2;
      tmp = &neigh_table->entries[mid];
      x = rte_be_to_cpu_32 (*(const uint32_t *) &tmp->ip_addr);
      if (x == k)
        {
          *found = (struct neigh_entry *) tmp;
          return mid;
        }
      if (x < k)
        left = mid + 1;
      else
        right = mid - 1;
    }

  return left; /* position to insert the key */
}

static inline __attribute__ ((always_inline)) int
neigh_lookup_pos6 (const struct neigh_table *neigh_table, const void *key,
                   struct neigh_entry **found)
{
  int left = 0;
  int right = neigh_table->num_entries - 1;
  int mid;
  uint64_t xh, xl;
  const struct neigh_entry *tmp;
  /* Read the IPv6 address (NBO) as two 64-bit integers.
   * The search key is loaded only once, before the loop. */
  const uint64_t kh = rte_be_to_cpu_64 (*(const uint64_t *) key);
  const uint64_t kl =
      rte_be_to_cpu_64 (*(const uint64_t *) ((const uint8_t *) key + 8));

  *found = NULL;
  while (left <= right)
    {
      mid = (left + right) / 2;
      tmp = &neigh_table->entries[mid];
      xh = rte_be_to_cpu_64 (*(const uint64_t *) &tmp->ip_addr);
      xl = rte_be_to_cpu_64 (
          *(const uint64_t *) ((const uint8_t *) &tmp->ip_addr + 8));
      if (xh == kh)
        {
          if (xl == kl)
            {
              *found = (struct neigh_entry *) tmp;
              return mid;
            }
          if (xl < kl)
            left = mid + 1;
          else
            right = mid - 1;
        }
      else if (xh < kh)
        left = mid + 1;
      else
        right = mid - 1;
    }

  return left; /* position to insert the key */
}

static inline __attribute__ ((always_inline)) int
neigh_lookup_pos (const struct neigh_table *neigh_table, int type,
                  const void *key, struct neigh_entry **found)
{
  if (type == NEIGH_ARP_TABLE)
    return neigh_lookup_pos4 (neigh_table, key, found);
  else
    return neigh_lookup_pos6 (neigh_table, key, found);
}

static inline __attribute__ ((always_inline)) struct neigh_entry *
neigh_lookup (const struct neigh_table *neigh_table, int type, const void *key)
{
  struct neigh_entry *found;
  neigh_lookup_pos (neigh_table, type, key, &found);
  return found;
}

const char *neigh_manager_table_str (int type);
const char *neigh_manager_state_str (uint8_t state);
void neigh_manager_show_table (const int type, const struct shell *shell);
void neigh_manager_process_message (void *msgp,
                                    struct neigh_table *neigh_tables);
int neigh_manager (void *arg __rte_unused);

#endif /*__NEIGH_MANAGER_H__*/
