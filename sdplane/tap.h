#ifndef __TAP_H__
#define __TAP_H__

#include <rte_ether.h>

int tap_open (char *ifname);
void tap_admin_up (char *ifname);

void tap_set_hwaddr (char *ifname, struct rte_ether_addr *hwaddr);

#endif /*__TAP_H__*/
