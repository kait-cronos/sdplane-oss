#ifndef FIB_H
#define FIB_H

#include "rib.h"
#include "route_info.h"

int fib_route_add (struct fib_tree *t, const uint8_t *key, int plen, struct route_info *ri);
struct fib_node *fib_route_lookup (struct fib_tree *t, const uint8_t *key);

#endif /* FIB_H */
