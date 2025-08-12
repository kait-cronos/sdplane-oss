/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#include "include.h"

#include <lthread.h>

#include <rte_common.h>
#include <rte_launch.h>
#include <rte_ether.h>

#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>

#include <sdplane/debug.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "sdplane.h"

#include "l3fwd_cmd.h"
#include "l2fwd_cmd.h"

#include "vty_server.h"
#include "vty_shell.h"

#include "thread_info.h"

extern int lthread_core;
extern volatile bool force_stop[RTE_MAX_LCORE];

vty_client_t client_info[VTY_CLIENT_MAX];

static __thread uint64_t loop_counter = 0;

void
vty_server (void *arg)
{
  int sockfd;
  int ret;
  int client_fd;

  sockfd = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd == -1)
    {
      fprintf (stderr, "lthread_socket() failed.\n");
      return;
    }

  int optval = 1;
  ret =
      setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval));
  if (ret < 0)
    {
      fprintf (stderr, "setsockopt(REUSEADDR) failed: %s.\n",
               strerror (errno));
      return;
    }

  struct sockaddr_in peer_addr;
  struct sockaddr_in sin;
  socklen_t addrlen = sizeof (peer_addr);

  sin.sin_family = PF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons (9882);

  ret = bind (sockfd, (struct sockaddr *) &sin, sizeof (sin));
  if (ret < 0)
    {
      fprintf (stderr, "bind() failed: %s.\n", strerror (errno));
      return;
    }

  DEBUG_SDPLANE_LOG (VTY_SERVER, "Starting vty_server on port 9882.");

  int client_size = 0;
  int i, client_id;
  lthread_t *client_lt = NULL;

  /* initialize */
  for (i = 0; i < VTY_CLIENT_MAX; i++)
    {
      client_info[i].id = i;
      client_info[i].fd = -1;
    }

  listen (sockfd, 128);

  struct pollfd fds[2];

  int thread_id;
  thread_id = thread_lookup (vty_server);
  thread_register_loop_counter (thread_id, &loop_counter);

  while (! force_quit && ! force_stop[lthread_core])
    {
      lthread_sleep (100); // yield.

      fds[0].fd = sockfd;
      fds[0].events = POLLIN;
      poll (fds, 1, 0);
      if ((fds[0].revents & (POLLIN | POLLERR)) != 0)
        {
          DEBUG_SDPLANE_LOG (VTY_SERVER, "accept() start.");
          client_fd =
              accept (sockfd, (struct sockaddr *) &peer_addr, &addrlen);
          if (client_fd < 0)
            {
              DEBUG_SDPLANE_LOG (VTY_SERVER, "accept() failed.");
              continue;
            }
          else
            {
              DEBUG_SDPLANE_LOG (VTY_SERVER, "accept(): client_fd: %d.",
                                 client_fd);
            }

          client_id = -1;
          for (i = 0; i < VTY_CLIENT_MAX; i++)
            {
              if (client_info[i].fd == -1)
                {
                  client_id = i;
                  break;
                }
            }
          if (client_id == -1)
            {
              DEBUG_SDPLANE_LOG (VTY_SERVER,
                                 "can't create new client: "
                                 "already served max (%d).",
                                 VTY_CLIENT_MAX);
              close (client_fd);
              continue;
            }

          DEBUG_SDPLANE_LOG (VTY_SERVER, "lthread_accept: client[%d].",
                             client_id);
          client_info[client_id].peer_addr = peer_addr;
          client_info[client_id].fd = client_fd;
          ret = lthread_create (&client_info[client_id].lt, vty_shell,
                                &client_info[client_id]);
          DEBUG_SDPLANE_LOG (VTY_SERVER, "lthread_create: ret: %d.", ret);
          if (client_size < client_id)
            client_size = client_id + 1;
        }

      loop_counter++;
    }

  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);
}
