#ifndef __IGMP_SNOOPING_H__
#define __IGMP_SNOOPING_H__

#include <stdint.h>

#define FLOOD_ALL_PORTS 0

struct rte_mbuf;
struct rib_info;
struct vswitch_conf;

void igmp_snooping_learn (const struct rte_mbuf *m, unsigned rx_portid,
                          uint16_t vswitch_id);

uint64_t igmp_snooping_select_ports (const struct rte_mbuf *m,
                                     const struct rib_info *rib_info,
                                     const struct vswitch_conf *vswitch);

#endif /* __IGMP_SNOOPING_H__ */
