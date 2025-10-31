#ifndef __DHCP_SERVER_H__
#define __DHCP_SERVER_H__

bool is_dhcp_packet (struct rte_mbuf *m);
void dhcp_server_init ();

int dhcp_server (__rte_unused void *dummy);

#endif /*__DHCP_SERVER_H__*/
