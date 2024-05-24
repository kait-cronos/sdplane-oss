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

#if 0
#include <rte_common.h>
#include <rte_vect.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_random.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_udp.h>
#include <rte_string_fns.h>
#include <rte_cpuflags.h>

#include <cmdline_parse.h>
#include <cmdline_parse_etheraddr.h>

#else

#include <rte_common.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>

#endif

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

bool enable_tap_copy = true;

void
per_thread_tap_ring_init ()
{
#define RING_TO_TAP_SIZE 64
  char ring_name[32];
  int lcore_id = rte_lcore_id ();
  snprintf (ring_name, sizeof (ring_name), "ring_to_tap_%d", lcore_id);
  thread_ring_to_tap =
    rte_ring_create (ring_name, RING_TO_TAP_SIZE, rte_socket_id (),
                     (RING_F_SP_ENQ | RING_F_SC_DEQ));
  tap_ring_by_lcore[lcore_id] = thread_ring_to_tap;
}

int
tap_handler (__rte_unused void *dummy)
{
  int fd;
  struct ifreq ifr;
  int ret;
  uint64_t loop_counter = 0;

  printf ("%s[%d]: %s: enter.\n", __FILE__, __LINE__, __func__);

  fd = open ("/dev/net/tun", O_RDWR);
  if (fd < 0)
    {
      printf ("%s: can't open /dev/net/tun. quit", __func__);
      return -1;
    }

  memset (&ifr, 0, sizeof (ifr));
  snprintf (ifr.ifr_name, IFNAMSIZ, "peek0");
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

  ret = ioctl (fd, TUNSETIFF, (void *) &ifr);
  if (ret < 0)
    {
      printf ("%s: ioctl (TUNSETIFF) failed: %s\n",
              __func__, strerror (errno));
      close (fd);
      return -1;
    }

#if 1
  int sockfd;
  sockfd = socket (AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    {
      printf ("%s: socket() failed: %s\n",
              __func__, strerror (errno));
    }
  else
    {
      memset (&ifr, 0, sizeof (ifr));
      snprintf (ifr.ifr_name, IFNAMSIZ, "peek0");

      ret = ioctl (sockfd, SIOCGIFFLAGS, &ifr);
      if (ret < 0)
        {
          printf ("%s: ioctl (SOICGIFFLAG) failed: %s\n",
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
          printf ("%s: ioctl (SOICSIFFLAG) failed: %s\n",
                  __func__, strerror (errno));
          close (sockfd);
          sockfd = -1;
        }
    }
#endif

  printf ("%s on lcore[%d]: started.\n",
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
                    if (FLAG_CHECK (debug_config, DEBUG_SDPLANE_WIRETAP))
                      printf ("%s: capture pkt: len: %d(%d) from lcore[%d]\n",
                              __func__, pkt_len, data_len, lcore_id);
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


