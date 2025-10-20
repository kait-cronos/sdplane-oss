#ifndef ROUTE_INFO_H
#define ROUTE_INFO_H

#include "rib.h"

/* route info management */
struct route_info *route_info_create (void);
void route_info_free (struct route_info *ri);
struct route_info *copy_route_info (struct route_info *src);
int route_info_add_nexthop (struct route_info *ri, struct nexthop_info *nh);
int route_info_del_nexthop (struct route_info *ri, struct nexthop_info *nh);

/* nexthop management */
struct nexthop_info *nexthop_create (int family, const void *nexthop, uint32_t oif);
void nexthop_free (struct nexthop_info *nh);

#endif /* ROUTE_INFO_H */
