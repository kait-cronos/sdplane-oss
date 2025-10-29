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

struct fib_tree *fib_new (struct fib_tree *t);
void free_fib_node (struct fib_node *n);
int count_nonzero (const int *arr, int len);
struct fib_node *create_fib_node (void);
typedef int (*fib_traverse_callback) (struct fib_node *n, void *arg);
int fib_traverse (struct fib_tree *t, fib_traverse_callback callback,
                  void *arg);
int fib_show_route (struct fib_node *n, void *arg);

/* IPv4 */
int fib_route_add4 (struct fib_tree *t, const uint8_t *key, int keylen, int *route_idx);
struct fib_node *fib_route_lookup4 (struct fib_tree *t, const uint8_t *key);

/* IPv6 */
int fib_route_add6 (struct fib_tree *t, const uint8_t *key, int keylen, int *route_idx);
struct fib_node *fib_route_lookup6 (struct fib_tree *t, const uint8_t *key);

#endif /* FIB_H */
