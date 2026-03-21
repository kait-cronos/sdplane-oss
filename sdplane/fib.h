// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2023-2026 Yasuhiro Ohara

#ifndef FIB_H
#define FIB_H

#include "rib.h"

#define KEY_SIZE(len) (((len) + 7) / 8)

struct show_route_arg
{
  struct shell *shell;
  struct rib_info *rib_info;
  int family;
};

struct fib_node
{
  int leaf; // 0: non-leaf, 1: leaf
  union
  {
    struct in_addr ipv4_addr;
    struct in6_addr ipv6_addr;
  } key_ip_addr;
  int keylen;
  int sdplane_nh_id;
  struct fib_node *child[BRANCH_SZ];
};

struct fib_tree
{
  int family;
  int table_id;
  struct fib_node *root;
};

struct fib_tree *fib_new (struct fib_tree *t);
void fib_free (struct fib_tree *t);

/* IPv4/v6 */
int fib_route_add (struct fib_tree *t, const uint8_t *key, int keylen,
                   int sdplane_nh_id);
struct fib_node *fib_route_lookup (struct fib_tree *t, const uint8_t *key);

/* FIB traverse method */
typedef int (*fib_traverse_callback) (struct fib_node *n, void *arg);
int fib_traverse (struct fib_tree *t, fib_traverse_callback callback,
                  void *arg);
int fib_show_route (struct fib_node *n, void *arg);

#endif /* FIB_H */
