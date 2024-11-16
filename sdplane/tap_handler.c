#include "include.h"

#include <lthread.h>

#include <rte_common.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>

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
      DEBUG_SDPLANE_LOG (TAPHANDLER, "can't open /dev/net/tun. quit");
      return -1;
    }

  memset (&ifr, 0, sizeof (ifr));
  snprintf (ifr.ifr_name, IFNAMSIZ, "%s", ifname);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

  ret = ioctl (fd, TUNSETIFF, (void *) &ifr);
  if (ret < 0)
    {
      DEBUG_SDPLANE_LOG (TAPHANDLER, "ioctl (TUNSETIFF) failed: %s",
                         strerror (errno));
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
      DEBUG_SDPLANE_LOG (TAPHANDLER, "socket() failed: %s",
                         strerror (errno));
    }
  else
    {
      memset (&ifr, 0, sizeof (ifr));
      snprintf (ifr.ifr_name, IFNAMSIZ, "%s", ifname);

      ret = ioctl (sockfd, SIOCGIFFLAGS, &ifr);
      if (ret < 0)
        {
          DEBUG_SDPLANE_LOG (TAPHANDLER, "ioctl (SOICGIFFLAG) failed: %s",
                             strerror (errno));
          close (sockfd);
          sockfd = -1;
        }
    }

  if (sockfd >= 0)
    {
      ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
      ret = ioctl (sockfd, SIOCSIFFLAGS, &ifr);
      if (ret < 0)
        {
          DEBUG_SDPLANE_LOG (TAPHANDLER, "ioctl (SOICSIFFLAG) failed: %s",
                             strerror (errno));
          close (sockfd);
          sockfd = -1;
        }
    }
}

#define VSWITCH_PORT_TYPE_NONE       0
#define VSWITCH_PORT_TYPE_DPDK_LCORE 1
#define VSWITCH_PORT_TYPE_LINUX_TAP  2

struct vswitch_port
{
  uint8_t id;
  uint8_t type;
  char *name;
  int sockfd;
  int lcore_id;
  struct rte_ring *ring[TAPDIR_SIZE];
};

#define VSWITCH_PORT_SIZE 16
struct vswitch
{
  int limit;
  int size;
  struct vswitch_port port[VSWITCH_PORT_SIZE];
};

int
tap_handler (__rte_unused void *dummy)
{
  int peek_fd;
  int ret;
  uint64_t loop_counter = 0;

  int port_id;
  uint16_t nb_ports;
  char port_name[64];

  unsigned lcore_id;
  struct rte_ring *tap_ring;

  struct vswitch vswitch0;
  struct vswitch *vswitch;

  struct vswitch_port *vswport;
  int vswport_id;

  DEBUG_SDPLANE_LOG (TAPHANDLER, "start thread on lcore[%d].",
                     rte_lcore_id ());

  snprintf (port_name, sizeof (port_name), "peek0");
  peek_fd = tap_open (port_name);
  tap_admin_up (port_name);
  DEBUG_SDPLANE_LOG (TAPHANDLER, "create %s and make it up.",
                     port_name);

  memset (&vswitch0, 0, sizeof (vswitch));
  vswitch = &vswitch0;
  vswitch->limit = VSWITCH_PORT_SIZE;

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

      if (vswitch->size < vswitch->limit)
        {
          vswport_id = vswitch->size;
          vswport = &vswitch->port[vswport_id];
          vswport->id = vswport_id;
          vswport->type = VSWITCH_PORT_TYPE_LINUX_TAP;
          vswport->name = strdup (port_name);
          vswport->sockfd = port_fd[port_id];
          DEBUG_SDPLANE_LOG (TAPHANDLER, "vswitch->port[%d]: "
                             "type: linux-tap name: %s sockfd: %d",
                             vswport_id, vswport->name, vswport->sockfd);
          vswitch->size++;
        }
    }

  struct rte_ring *tap_ring_up, *tap_ring_down;
  for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++)
    {
      tap_ring_up = tap_ring_lcore_dir[lcore_id][TAPDIR_UP];
      tap_ring_down = tap_ring_lcore_dir[lcore_id][TAPDIR_DOWN];

      if (tap_ring_up && vswitch->size < vswitch->limit)
        {
          vswport_id = vswitch->size;
          vswport = &vswitch->port[vswport_id];
          vswport->id = vswport_id;
          vswport->type = VSWITCH_PORT_TYPE_DPDK_LCORE;
          snprintf (port_name, sizeof (port_name),
                    "tap_ring_lcore%d", lcore_id);
          vswport->name = strdup (port_name);
          vswport->lcore_id = lcore_id;
          vswport->ring[TAPDIR_UP] = tap_ring_up;
          vswport->ring[TAPDIR_DOWN] = tap_ring_down;
          DEBUG_SDPLANE_LOG (TAPHANDLER, "vswitch->port[%d]: "
                             "type: dpdk-lcore name: %s lcore_id: %d "
                             "ring[up(%d)/down(%d)]: %p/%p",
                             vswport_id, vswport->name, vswport->lcore_id,
                             TAPDIR_UP, TAPDIR_DOWN,
                             vswport->ring[TAPDIR_UP],
                             vswport->ring[TAPDIR_DOWN]);
          vswitch->size++;
        }
    }

#define FDB_SIZE 16
  int i, j;
  struct fdb_entry
  {
    struct rte_ether_addr l2addr;
    int port;
  };
  struct fdb_entry fdb[FDB_SIZE];
  memset (fdb, 0, sizeof (fdb));

  DEBUG_SDPLANE_LOG (TAPHANDLER, "start main loop on lcore[%d].",
                     rte_lcore_id ());

  unsigned tap_handler_id = rte_lcore_id ();
  while (! force_quit && ! force_stop[tap_handler_id])
    {
      //lthread_sleep (0); // yield.
      //printf ("%s: schedule: %lu.\n", __func__, loop_counter);

      for (port_id = 0; port_id < vswitch->size; port_id++)
        {
          struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
          unsigned int dequeued = 0, avail = 0;

          vswport = &vswitch->port[port_id];
          if (vswport->type != VSWITCH_PORT_TYPE_DPDK_LCORE)
            continue;

          tap_ring = vswport->ring[TAPDIR_UP];
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
                DEBUG_SDPLANE_LOG (TAPHANDLER,
                        "warning: multi-seg mbuf: %u < %u",
                        data_len, pkt_len);

              /* analyze packet */
              struct rte_ether_hdr *eth;
              struct rte_ipv4_hdr *ipv4;
              struct rte_ipv6_hdr *ipv6;
              char eth_dst[32];
              char eth_src[32];
              eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
              rte_ether_format_addr (eth_dst, sizeof (eth_dst),
                                     &eth->dst_addr);
              rte_ether_format_addr (eth_src, sizeof (eth_src),
                                     &eth->src_addr);
              DEBUG_SDPLANE_LOG (PACKET,
                  "m: %p ether: dst: %s src: %s type: %#hx",
                  m, eth_dst, eth_src,
                  rte_be_to_cpu_16 (eth->ether_type));

              if (RTE_ETH_IS_IPV4_HDR (m->packet_type))
                {
                  ipv4 = (struct rte_ipv4_hdr *) (eth + 1);
                  DEBUG_SDPLANE_LOG (PACKET,
                      "m: %p ipv4: len: %d",
                      m, rte_be_to_cpu_16 (ipv4->total_length));
                }
              else if (RTE_ETH_IS_IPV6_HDR (m->packet_type))
                {
                  ipv6 = (struct rte_ipv6_hdr *) (eth + 1);
                  DEBUG_SDPLANE_LOG (PACKET,
                      "m: %p ipv6: len: %d",
                      m, rte_be_to_cpu_16 (ipv6->payload_len));
                }

              for (j = 0; j < FDB_SIZE; j++)
                {
                  if (rte_is_zero_ether_addr (&fdb[j].l2addr))
                    {
                      fdb[j].l2addr = eth->src_addr;
                      fdb[j].port = m->port;
                      DEBUG_SDPLANE_LOG (FDB_CHANGE,
                          "m: %p new: in fdb[%d]: addr: %s port: %d",
                          m, j, eth_src, m->port);
                      break;
                    }
                  if (rte_is_same_ether_addr (&fdb[j].l2addr, &eth->src_addr))
                    {
                      fdb[j].port = m->port;
                      DEBUG_SDPLANE_LOG (FDB,
                          "m: %p found: in fdb[%d]: addr: %s port: %d",
                          m, j, eth_src, m->port);
                      break;
                    }
                  char buf[32];
                  rte_ether_format_addr (buf, sizeof (buf), &fdb[j].l2addr);
                  DEBUG_SDPLANE_LOG (FDB,
                      "m: %p fdb[%d]: addr: %s port: %d",
                      m, j, buf, fdb[i].port);
                }

              if (peek_fd >= 0)
                {
                  ret = write (peek_fd, pkt, data_len);
                  if (ret < 0)
                    DEBUG_SDPLANE_LOG (TAPHANDLER,
                            "warning: write () failed: %s",
                            strerror (errno));
                  else
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
        }

      loop_counter++;
    }

  close (peek_fd);
  printf ("%s on lcore[%d]: finished.\n",
          __func__, rte_lcore_id ());
  return 0;
}


