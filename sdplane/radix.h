#ifndef RADIX_H
#define RADIX_H

#include "fib.h"
#include "rib.h"

struct route_dst_info
{
  int family; // destination address family
  union
  {
    struct in_addr ipv4_addr;
    struct in6_addr ipv6_addr;
  } dst_ip_addr;
  int plen;
};

struct route_entry
{
  struct route_dst_info dst;
  struct nh_common nh;
};

struct rib_node
{
  int valid;
  struct route_entry entry;
  /*
   * entry.dst_ip_addr is key
   * entry.plen is key length
   * entry.nh is nexthop info
   */
  struct rib_node *left;
  struct rib_node *right;
};

/*
 * rib_tree->family: table family
 *   └─ rib_node->entry.family: destination address family
 *         └─ nh_common->...->nh_info.family: nexthop address family
 */
struct rib_tree
{
  int family; // table family
  int table_id;
  struct rib_node *root;
};

/* RIB node management */
struct rib_tree *rib_new (struct rib_tree *t);
void rib_free (struct rib_tree *t);

/* RIB operations */
int rib_route_add (struct rib_tree *t, const uint8_t *key, int keylen,
                   struct route_entry *data);
int rib_route_delete (struct rib_tree *t, const uint8_t *key, int keylen);
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
