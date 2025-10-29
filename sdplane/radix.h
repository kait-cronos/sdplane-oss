#ifndef RADIX_H
#define RADIX_H

#include "rib.h"

struct rib_node
{
  int valid;
  uint8_t key[16];
  int keylen;
  int num_routes;
  int route_idx[MAX_ECMP_ENTRY];
  struct rib_node *left;
  struct rib_node *right;
};
struct rib_tree
{
  int family;
  int table_id;
  struct rib_node *root;
};

/* RIB node management */
struct rib_tree *rib_new (struct rib_tree *t);
void rib_free (struct rib_tree *t);

/* RIB operations */
int rib_route_add (struct rib_tree *t, const uint8_t *key, int keylen,
                   int idx);
int rib_route_delete (struct rib_tree *t, const uint8_t *key, int keylen,
                      int idx);
struct rib_node *rib_route_lookup (struct rib_tree *t, const uint8_t *key);

/* RIB traversal */
typedef int (*rib_traverse_callback) (struct rib_node *n, void *arg);
int rib_traverse (struct rib_tree *t, rib_traverse_callback callback,
                  void *arg);

/* FIB rebuild from RIB */
int rebuild_fib_from_rib (struct rib_tree *rib_tree,
                          struct fib_tree *fib_tree);

int rib_show_route (struct rib_node *n, void *arg);

#endif /* RADIX_H */
