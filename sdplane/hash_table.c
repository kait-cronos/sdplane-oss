#include "include.h"

#include "hash_table.h"

int
hash_table_init (struct hash_table *ht, int size,
                 uint32_t (*hash_fn) (const void *key),
                 int (*key_eq) (const void *a, const void *b))
{
  ht->buckets = calloc (size, sizeof (struct ht_node *));
  if (! ht->buckets)
    return -1;

  ht->size = size;
  ht->mask = size - 1;
  ht->count = 0;
  ht->hash_fn = hash_fn;
  ht->key_eq = key_eq;
  return 0;
}

void
hash_table_destroy (struct hash_table *ht)
{
  int i;
  struct ht_node *cur, *next;

  for (i = 0; i < ht->size; i++)
    {
      cur = ht->buckets[i];
      while (cur)
        {
          next = cur->next;
          free (cur);
          cur = next;
        }
    }
  free (ht->buckets);
  ht->buckets  = NULL;
  ht->size = 0;
  ht->mask = 0;
  ht->count = 0;
}

int
hash_table_insert (struct hash_table *ht, void *key, void *value)
{
  uint32_t idx = ht->hash_fn (key) & ht->mask;
  struct ht_node *cur;

  /* update existing entry if key already present */
  for (cur = ht->buckets[idx]; cur; cur = cur->next)
    {
      if (ht->key_eq (cur->key, key))
        {
          cur->value = value;
          return 0;
        }
    }

  /* allocate a new node and prepend to bucket */
  struct ht_node *node = malloc (sizeof (struct ht_node));
  if (! node)
    return -1;

  node->key = key;
  node->value = value;
  node->next = ht->buckets[idx];
  ht->buckets[idx] = node;
  ht->count++;
  return 0;
}

int
hash_table_remove (struct hash_table *ht, const void *key)
{
  uint32_t idx = ht->hash_fn (key) & ht->mask;
  struct ht_node *prev = NULL, *cur = ht->buckets[idx];

  while (cur)
    {
      if (ht->key_eq (cur->key, key))
        {
          if (prev)
            prev->next = cur->next;
          else
            ht->buckets[idx] = cur->next;
          free (cur);
          ht->count--;
          return 0;
        }
      prev = cur;
      cur = cur->next;
    }
  return -1; /* not found */
}

void *
hash_table_lookup (const struct hash_table *ht, const void *key)
{
  uint32_t idx = ht->hash_fn (key) & ht->mask;
  const struct ht_node *cur;

  for (cur = ht->buckets[idx]; cur; cur = cur->next)
    {
      if (ht->key_eq (cur->key, key))
        return cur->value;
    }
  return NULL;
}