#include "include.h"

#include <lthread.h>

#include <sdplane/debug.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "sdplane.h"
#include "thread_info.h"

#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>

extern int lthread_core;
extern volatile bool force_stop[RTE_MAX_LCORE];
static __thread uint64_t loop_counter = 0;

struct netlink_sock
{
  int sockfd;
  int seq;
  struct sockaddr_nl snl;
  char *name;
};

struct netlink_sock netlink_kernel = { -1, 0, { 0 }, "netlink-kernel" };
struct netlink_sock netlink_cmd = { -1, 0, { 0 }, "netlink-cmd" };

int
netlink_socket (struct netlink_sock *nlsock, unsigned long groups)
{
  int sockfd;
  int ret;
  struct sockaddr_nl snl;
  int namelen;

  sockfd = socket (AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sockfd < 0)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "can't open %s socket: %s",
                         nlsock->name, strerror (errno));
      return -1;
    }

  ret = fcntl (sockfd, F_SETFL, O_NONBLOCK);
  if (ret < 0)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "can't set %s socket O_NONBLOCK: %s",
                         nlsock->name, strerror (errno));
      close (sockfd);
      return -1;
    }

  memset (&snl, 0, sizeof (snl));
  snl.nl_family = AF_NETLINK;
  snl.nl_groups = groups;

  ret = bind (sockfd, (struct sockaddr *) &snl, sizeof (snl));
  if (ret < 0)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "can't bind %s socket to groups 0x%x: %s",
                         nlsock->name, groups, strerror (errno));
      close (sockfd);
      return -1;
    }

  namelen = sizeof (snl);
  ret = getsockname (sockfd, (struct sockaddr *) &snl, &namelen);
  if (ret < 0 || namelen != sizeof (snl))
    {
      DEBUG_SDPLANE_LOG (NETLINK, "can't bind %s socket name: %s",
                         nlsock->name, strerror (errno));
      close (sockfd);
      return -1;
    }

  nlsock->snl = snl;
  nlsock->sockfd = sockfd;

  return 0;
}

void
netlink_thread (void *arg)
{
  unsigned lcore_id = rte_lcore_id ();

  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);
  DEBUG_SDPLANE_LOG (NETLINK, "%s: started.", __func__);

  int thread_id;
  thread_id = thread_lookup (netlink_thread);
  thread_register_loop_counter (thread_id, &loop_counter);

  unsigned long groups;
  groups = RTMGRP_LINK;
  groups |= RTMGRP_IPV4_ROUTE | RTMGRP_IPV4_IFADDR;
  groups |= RTMGRP_IPV6_ROUTE | RTMGRP_IPV6_IFADDR;
  netlink_socket (&netlink_kernel, groups);
  netlink_socket (&netlink_cmd, 0);

  while (! force_quit && ! force_stop[lthread_core])
    {
      lthread_sleep (100); // yield.
      //DEBUG_SDPLANE_LOG (NETLINK, "%s: schedule.", __func__);

      loop_counter++;
    }

  DEBUG_SDPLANE_LOG (NETLINK, "%s: terminating.", __func__);
  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);
}
