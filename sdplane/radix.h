#ifndef RADIX_H
#define RADIX_H

#include "rib.h"
#include "route_info.h"

/* RIB node management */
struct rib_node *create_rib_node (void);

/* RIB operations */
int rib_route_add (struct rib_tree *t, const uint8_t *key, int plen,
                   struct route_info *route);
int rib_route_delete (struct rib_tree *t, const uint8_t *key, int plen,
                      struct nexthop_info *nh);
int rebuild_fib_from_rib (struct rib_tree *rib_tree, struct fib_tree *fib_tree);

#endif /* RADIX_H */
