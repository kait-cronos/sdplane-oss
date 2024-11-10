#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <string.h>
#include <sys/queue.h>
#include <stdarg.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>

#include <rte_common.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <lthread.h>

#include <zcmdsh/debug.h>
#include <zcmdsh/termio.h>
#include <zcmdsh/vector.h>
#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>
#include <zcmdsh/command_shell.h>
#include <zcmdsh/debug_cmd.h>

#include <zcmdsh/debug_log.h>
#include <zcmdsh/debug_category.h>
#include <zcmdsh/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "l3fwd.h"
#include "l3fwd_event.h"
#include "l3fwd_route.h"

#include "l2fwd_export.h"
#include "l2fwd_cmd.h"

#include "sdplane.h"

#include <linux/if.h>
#include <linux/if_tun.h>

struct rte_ring *tap_ring_by_lcore[RTE_MAX_LCORE];
__thread struct rte_ring *thread_ring_to_tap;

__thread struct rte_ring *thread_ring_from_tap;

#define TAPDIR_UP   0
#define TAPDIR_DOWN 1
#define TAPDIR_SIZE 2
struct rte_ring *tap_ring_lcore_dir[RTE_MAX_LCORE][TAPDIR_SIZE];

bool enable_tap_copy = true;

int port_fd[RTE_MAX_ETHPORTS];

void
per_thread_tap_ring_init ()
{
#define RING_TO_TAP_SIZE 64
  char ring_name[32];
  int lcore_id = rte_lcore_id ();
  snprintf (ring_name, sizeof (ring_name), "tap_ring_lcore%d_up", lcore_id);
  thread_ring_to_tap =
    rte_ring_create (ring_name, RING_TO_TAP_SIZE, rte_socket_id (),
                     (RING_F_SP_ENQ | RING_F_SC_DEQ));
  tap_ring_by_lcore[lcore_id] = thread_ring_to_tap;

  snprintf (ring_name, sizeof (ring_name), "tap_ring_lcore%d_down", lcore_id);
  thread_ring_from_tap =
    rte_ring_create (ring_name, RING_TO_TAP_SIZE, rte_socket_id (),
                     (RING_F_SP_ENQ | RING_F_SC_DEQ));

  tap_ring_lcore_dir[lcore_id][TAPDIR_UP] = thread_ring_to_tap;
  tap_ring_lcore_dir[lcore_id][TAPDIR_DOWN] = thread_ring_from_tap;
}

int
tap_open (char *ifname)
{
  int ret;
  int fd;
  struct ifreq ifr;

  fd = open ("/dev/net/tun", O_RDWR);
  if (fd < 0)
    {
      DEBUG_SDPLANE_LOG (TAPHANDLER, "%s: can't open /dev/net/tun. quit", __func__);
      return -1;
    }

  memset (&ifr, 0, sizeof (ifr));
  snprintf (ifr.ifr_name, IFNAMSIZ, "%s", ifname);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

  ret = ioctl (fd, TUNSETIFF, (void *) &ifr);
  if (ret < 0)
    {
      DEBUG_SDPLANE_LOG (TAPHANDLER, "%s: ioctl (TUNSETIFF) failed: %s\n",
              __func__, strerror (errno));
      close (fd);
      return -1;
    }

  return fd;
}

void
tap_admin_up (char *ifname)
{
  int ret;
  int sockfd;
  struct ifreq ifr;

  sockfd = socket (AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    {
      DEBUG_SDPLANE_LOG (TAPHANDLER, "%s: socket() failed: %s\n",
              __func__, strerror (errno));
    }
  else
    {
      memset (&ifr, 0, sizeof (ifr));
      snprintf (ifr.ifr_name, IFNAMSIZ, ifname);

      ret = ioctl (sockfd, SIOCGIFFLAGS, &ifr);
      if (ret < 0)
        {
          DEBUG_SDPLANE_LOG (TAPHANDLER, "%s: ioctl (SOICGIFFLAG) failed: %s\n",
                  __func__, strerror (errno));
          close (sockfd);
          sockfd = -1;
        }
    }

  if (sockfd > 0)
    {
      ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
      ret = ioctl (sockfd, SIOCSIFFLAGS, &ifr);
      if (ret < 0)
        {
          DEBUG_SDPLANE_LOG (TAPHANDLER, "%s: ioctl (SOICSIFFLAG) failed: %s\n",
                  __func__, strerror (errno));
          close (sockfd);
          sockfd = -1;
        }
    }
}

int
tap_handler (__rte_unused void *dummy)
{
  int fd;
  int ret;
  uint64_t loop_counter = 0;

  int port_id;
  uint16_t nb_ports;
  char port_name[64];

  DEBUG_SDPLANE_LOG (TAPHANDLER, "%s[%d]: %s: enter.\n", __FILE__, __LINE__, __func__);

  snprintf (port_name, sizeof (port_name), "peek0");
  fd = tap_open (port_name);
  tap_admin_up (port_name);
  DEBUG_SDPLANE_LOG (TAPHANDLER, "create %s and make it up.",
                     port_name);

  for (port_id = 0; port_id < RTE_MAX_ETHPORTS; port_id++)
    port_fd[port_id] = -1;

  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      snprintf (port_name, sizeof (port_name), "port-dpdk%d", port_id);
      port_fd[port_id] = tap_open (port_name);
      tap_admin_up (port_name);
      DEBUG_SDPLANE_LOG (TAPHANDLER, "create %s and make it up.",
                         port_name);
    }

  DEBUG_SDPLANE_LOG (TAPHANDLER, "%s on lcore[%d]: started.",
          __func__, rte_lcore_id ());

  unsigned tap_handler_id = rte_lcore_id ();
  while (! force_quit && ! force_stop[tap_handler_id])
    {
      unsigned lcore_id;
      //lthread_sleep (0); // yield.
      //printf ("%s: schedule: %lu.\n", __func__, loop_counter);
      for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++)
        {
          struct rte_ring *tap_ring;
          struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
          unsigned int dequeued = 0, avail = 0;
          int i;

          tap_ring = tap_ring_by_lcore[lcore_id];
          do {
            if (tap_ring)
              dequeued = rte_ring_dequeue_burst (tap_ring, (void **) pkts_burst,
                                                 MAX_PKT_BURST, &avail);
            for (i = 0; i < dequeued; i++)
              {
                struct rte_mbuf *m;
                uint32_t pkt_len;
                uint16_t data_len;
                char *pkt;

                m = pkts_burst[i];
                if (! m)
                  continue;
                pkt_len = rte_pktmbuf_pkt_len (m);
                data_len = rte_pktmbuf_data_len (m);
                pkt = rte_pktmbuf_mtod (m, char *);
                if (data_len < pkt_len)
                  printf ("%s: warning: multi-seg mbuf: %u < %u\n",
                          __func__, data_len, pkt_len);
                ret = write (fd, pkt, data_len);
                if (ret < 0)
                  printf ("%s: warning: write () failed: %s\n",
                          __func__, strerror (errno));
                else
                  {
                    DEBUG_SDPLANE_LOG (TAPHANDLER,
                        "packet [%d/%d] (in_port: %d) written to peek0.",
                        data_len, pkt_len, m->port);
                  }

                if (port_fd[m->port] >= 0)
                  {
                    ret = write (port_fd[m->port], pkt, data_len);
                    if (ret < 0)
                      DEBUG_SDPLANE_LOG (TAPHANDLER,
                                         "write() failed: port_fd[%d]: %d error: %s.",
                                         m->port, port_fd[m->port], strerror (errno));
                    else
                      DEBUG_SDPLANE_LOG (TAPHANDLER,
                        "packet [%d/%d] (in_port: %d) to port-dpdk%d.",
                        data_len, pkt_len, m->port, m->port);
                  }

                rte_pktmbuf_free (m);
              }
          } while (avail);
        }

      loop_counter++;
    }

  close (fd);
  printf ("%s on lcore[%d]: finished.\n",
          __func__, rte_lcore_id ());
  return 0;
}


