// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2023-2026 Yasuhiro Ohara

#ifndef __VTY_SERVER_H__
#define __VTY_SERVER_H__

#include <lthread.h>

struct vty_client
{
  int id;
  struct sockaddr_in peer_addr;
  int fd;
  lthread_t *lt;
};
typedef struct vty_client vty_client_t;

#define VTY_CLIENT_MAX 5

void vty_server (void *arg);

#endif /*__VTY_SERVER_H__*/
