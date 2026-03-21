// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2023-2026 Yasuhiro Ohara

#ifndef __DHCP_SERVER_H__
#define __DHCP_SERVER_H__

#include <stdbool.h>
#include <rte_common.h>
#include <rte_mbuf.h>

bool is_dhcp_packet (struct rte_mbuf *m);
void dhcp_server_init ();

int dhcp_server (__rte_unused void *dummy);

#endif /*__DHCP_SERVER_H__*/
