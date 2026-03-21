// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2023-2026 Yasuhiro Ohara

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

struct ht_node
{
  void *key;
  void *value;
  struct ht_node *next;
};

struct hash_table
{
  struct ht_node **buckets;
  int size; /* number of buckets (power-of-2) */
  int mask; /* size - 1  */
  int count; /* total number of entries */
  uint32_t (*hash_fn) (const void *key);
  int (*key_eq) (const void *a, const void *b);
};

int hash_table_init (struct hash_table *ht, int size,
                     uint32_t (*hash_fn) (const void *key),
                     int (*key_eq) (const void *a, const void *b));
void hash_table_destroy (struct hash_table *ht);


int hash_table_insert (struct hash_table *ht, void *key, void *value);
int hash_table_remove (struct hash_table *ht, const void *key);

void *hash_table_lookup (const struct hash_table *ht, const void *key);

#endif /* HASH_TABLE_H */
