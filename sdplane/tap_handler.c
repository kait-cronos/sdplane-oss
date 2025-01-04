#include "include.h"

#include <linux/if.h>
#include <linux/if_tun.h>

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
#include "thread_info.h"

#include "rib_manager.h"

static __thread struct rib *rib;

int peek_fd = -1;
int port_fd[RTE_MAX_ETHPORTS];

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
      DEBUG_SDPLANE_LOG (TAPHANDLER, "socket() failed: %s", strerror (errno));
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

struct vswitch vswitch0;
struct fdb_entry fdb[FDB_SIZE];

static __thread uint64_t loop_counter = 0;

static inline __attribute__ ((always_inline)) void
vswitch_port_update ()
{
  int i;
  int lcore_id;
  struct rte_ring *tap_ring_up, *tap_ring_down;
  char port_name[64];

  struct vswitch *vswitch = &vswitch0;
  struct vswitch_port *vswport, *nextport;
  int vswport_id;

  if (! rib)
    return;

  for (i = 0; i < vswitch->size; i++)
    {
      vswport = &vswitch->port[i];
      DEBUG_SDPLANE_LOG (VSWITCH,
                         "vswport[%d]: id: %d type: %d name: %s sockfd: %d "
                         "lcore_id: %d ring[0/1]: %p/%p ",
                         i, vswport->id, vswport->type, vswport->name,
                         vswport->sockfd, vswport->lcore_id,
                         vswport->ring[0], vswport->ring[1]);
    }

  DEBUG_SDPLANE_LOG (VSWITCH, "removing DPDK port in vswitch: size %d",
                     vswitch->size);
  for (i = vswitch->size - 1; i >= 0; i--)
    {
      vswport = &vswitch->port[i];
      DEBUG_SDPLANE_LOG (VSWITCH, "remove vswport[%d]", i);
      if (vswport->type == VSWITCH_PORT_TYPE_DPDK_LCORE)
        {
          if (i + 1 < vswitch->size)
            {
              nextport = &vswitch->port[i + 1];
              memmove (vswport, nextport,
                       (vswitch->size - (i + 1)) * sizeof (struct vswitch_port));
            }
          else
            {
              memset (vswport, 0, sizeof (struct vswitch_port));
            }
          vswitch->size--;
        }
    }

  DEBUG_SDPLANE_LOG (VSWITCH, "removed DPDK port in vswitch: size %d",
                     vswitch->size);
  for (i = 0; i < vswitch->size; i++)
    {
      vswport = &vswitch->port[i];
      DEBUG_SDPLANE_LOG (VSWITCH,
                         "vswport[%d]: id: %d type: %d name: %s sockfd: %d "
                         "lcore_id: %d ring[0/1]: %p/%p ",
                         i, vswport->id, vswport->type, vswport->name,
                         vswport->sockfd, vswport->lcore_id,
                         vswport->ring[0], vswport->ring[1]);
    }

  for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++)
    {
      struct sdplane_queue_conf *sdplane_qconf;

      sdplane_qconf = &rib->qconf[lcore_id];
      for (i = 0; i < sdplane_qconf->nrxq; i++)
        {
          uint16_t portid, queueid;
          portid = sdplane_qconf->rx_queue_list[i].port_id;
          queueid = sdplane_qconf->rx_queue_list[i].queue_id;

          tap_ring_up = ring_up[portid][queueid];
          tap_ring_down = ring_dn[portid][queueid];

          if (tap_ring_up && vswitch->size < vswitch->limit)
            {
              vswport_id = vswitch->size;
              vswport = &vswitch->port[vswport_id];
              vswport->id = vswport_id;
              vswport->type = VSWITCH_PORT_TYPE_DPDK_LCORE;
#if 0
              snprintf (port_name, sizeof (port_name), "tap_ring_lcore%d",
                        lcore_id);
#else
              snprintf (port_name, sizeof (port_name),
                        "ring_up[port-%d][queue-%d]",
                        portid, queueid);
#endif
              vswport->dpdk_port_id = portid;
              vswport->dpdk_queue_id = queueid;
              vswport->name = strdup (port_name);
              vswport->lcore_id = lcore_id;
              vswport->ring[TAPDIR_UP] = tap_ring_up;
              vswport->ring[TAPDIR_DOWN] = tap_ring_down;
              DEBUG_SDPLANE_LOG (VSWITCH,
                                 "vswitch->port[%d]: "
                                 "type: dpdk-lcore name: %s lcore_id: %d "
                                 "ring[up(%d)/down(%d)]: %p/%p",
                                 vswport_id, vswport->name, vswport->lcore_id,
                                 TAPDIR_UP, TAPDIR_DOWN, vswport->ring[TAPDIR_UP],
                                 vswport->ring[TAPDIR_DOWN]);
              vswitch->size++;
            }
        }
    }

  DEBUG_SDPLANE_LOG (VSWITCH, "updated DPDK port in vswitch: size %d",
                     vswitch->size);
  for (i = 0; i < vswitch->size; i++)
    {
      vswport = &vswitch->port[i];
      DEBUG_SDPLANE_LOG (VSWITCH,
                         "vswport[%d]: id: %d type: %d name: %s sockfd: %d lcore_id: %d ring[0/1]: %p/%p ",
                         i, vswport->id, vswport->type, vswport->name,
                         vswport->sockfd, vswport->lcore_id,
                         vswport->ring[0], vswport->ring[1]);
    }
}

static inline __attribute__ ((always_inline)) void
tap_handler_log_packet (struct rte_mbuf *m)
{
  /* analyze packet */
  struct rte_ether_hdr *eth;
  struct rte_ipv4_hdr *ipv4;
  struct rte_ipv6_hdr *ipv6;
  char eth_dst[32];
  char eth_src[32];
  unsigned short eth_type;
  eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  rte_ether_format_addr (eth_dst, sizeof (eth_dst),
                         &eth->dst_addr);
  rte_ether_format_addr (eth_src, sizeof (eth_src),
                         &eth->src_addr);
  eth_type = rte_be_to_cpu_16 (eth->ether_type);

  if (RTE_ETH_IS_IPV4_HDR (m->packet_type))
    {
      ipv4 = (struct rte_ipv4_hdr *) (eth + 1);
      DEBUG_SDPLANE_LOG (PACKET, "m: %p ether[%#hx]: %s -> %s "
                         "ipv4: len: %d",
                         m, eth_type, eth_src, eth_dst,
                         rte_be_to_cpu_16 (ipv4->total_length));
    }
  else if (RTE_ETH_IS_IPV6_HDR (m->packet_type))
    {
      ipv6 = (struct rte_ipv6_hdr *) (eth + 1);
      DEBUG_SDPLANE_LOG (PACKET, "m: %p ether[%#hx]: %s -> %s "
                         "ipv6: len: %d",
                         m, eth_type, eth_src, eth_dst,
                         rte_be_to_cpu_16 (ipv6->payload_len));
    }
  else
    {
      DEBUG_SDPLANE_LOG (PACKET, "m: %p ether[%#hx]: %s -> %s",
                         m, eth_type, eth_src, eth_dst);
    }
}

static inline __attribute__ ((always_inline)) void
tap_handler_register_fdb (struct rte_mbuf *m)
{
  int j;
  char eth_src[32];
  struct rte_ether_hdr *eth;

  eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  rte_ether_format_addr (eth_src, sizeof (eth_src),
                         &eth->src_addr);

  /* register in FDB */
  for (j = 0; j < FDB_SIZE; j++)
    {
      if (rte_is_zero_ether_addr (&fdb[j].l2addr))
        {
          fdb[j].l2addr = eth->src_addr;
          fdb[j].port = m->port;
          DEBUG_SDPLANE_LOG (
              FDB_CHANGE,
              "m: %p new: in fdb[%d]: addr: %s port: %d", m, j,
              eth_src, m->port);
          break;
        }
      if (rte_is_same_ether_addr (&fdb[j].l2addr, &eth->src_addr))
        {
          fdb[j].port = m->port;
          DEBUG_SDPLANE_LOG (
              FDB, "m: %p found: in fdb[%d]: addr: %s port: %d", m,
              j, eth_src, m->port);
          break;
        }
      char buf[32];
      rte_ether_format_addr (buf, sizeof (buf), &fdb[j].l2addr);
      DEBUG_SDPLANE_LOG (FDB, "m: %p fdb[%d]: addr: %s port: %d",
                         m, j, buf, fdb[j].port);
    }
}

static inline __attribute__ ((always_inline)) void
tap_handler_write_peek (struct rte_mbuf *m)
{
  uint32_t pkt_len;
  uint16_t data_len;
  char *pkt;
  int ret;

  pkt_len = rte_pktmbuf_pkt_len (m);
  data_len = rte_pktmbuf_data_len (m);
  pkt = rte_pktmbuf_mtod (m, char *);

  /* write to peek0 for packet capture. */
  if (peek_fd >= 0)
    {
      ret = write (peek_fd, pkt, data_len);
      if (ret < 0)
        DEBUG_SDPLANE_LOG (TAPHANDLER,
                           "warning: write () failed: %s",
                           strerror (errno));
      else
        DEBUG_SDPLANE_LOG (
            TAPHANDLER,
            "packet [%d/%d] (in_port: %d) written to peek0.",
            data_len, pkt_len, m->port);
    }
}

static inline __attribute__ ((always_inline)) void
tap_handler_write_port (struct rte_mbuf *m)
{
  uint32_t pkt_len;
  uint16_t data_len;
  char *pkt;
  int ret;

  pkt_len = rte_pktmbuf_pkt_len (m);
  data_len = rte_pktmbuf_data_len (m);
  pkt = rte_pktmbuf_mtod (m, char *);

  if (data_len < pkt_len)
    DEBUG_SDPLANE_LOG (TAPHANDLER,
                       "warning: multi-seg mbuf: %u < %u",
                       data_len, pkt_len);

  /* write to port-dpdkX. */
  if (port_fd[m->port] >= 0)
    {
      ret = write (port_fd[m->port], pkt, data_len);
      if (ret < 0)
        DEBUG_SDPLANE_LOG (
            TAPHANDLER,
            "write() failed: port_fd[%d]: %d error: %s.", m->port,
            port_fd[m->port], strerror (errno));
      else
        DEBUG_SDPLANE_LOG (
            TAPHANDLER,
            "packet [%d/%d] (in_port: %d) to port-dpdk%d.",
            data_len, pkt_len, m->port, m->port);
    }
}

static inline __attribute__ ((always_inline)) void
tap_handler_write_port_all (struct rte_mbuf *m)
{
  uint32_t pkt_len;
  uint16_t data_len;
  char *pkt;
  int ret;
  int port_id;
  uint16_t nb_ports;

  pkt_len = rte_pktmbuf_pkt_len (m);
  data_len = rte_pktmbuf_data_len (m);
  pkt = rte_pktmbuf_mtod (m, char *);

  if (data_len < pkt_len)
    DEBUG_SDPLANE_LOG (TAPHANDLER,
                       "warning: multi-seg mbuf: %u < %u",
                       data_len, pkt_len);

  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      /* write to port-dpdkX. */
      if (port_fd[port_id] >= 0)
        {
          ret = write (port_fd[port_id], pkt, data_len);
          if (ret < 0)
            DEBUG_SDPLANE_LOG (
                TAPHANDLER,
                "write() failed: port_fd[%d]: %d error: %s.", port_id,
                port_fd[port_id], strerror (errno));
          else
            DEBUG_SDPLANE_LOG (
                TAPHANDLER,
                "packet [%d/%d] (in_port: %d) to port-dpdk%d.",
                data_len, pkt_len, m->port, port_id);
        }
    }
}

static inline __attribute__ ((always_inline)) void
tap_handler_handle_packet_up ()
{
  struct vswitch *vswitch;
  struct vswitch_port *vswport;
  int vswport_id;
  int port_id;
  struct rte_ring *tap_ring;
  int i;

  vswitch = &vswitch0;

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

          m = pkts_burst[i];
          if (! m)
            continue;

          tap_handler_log_packet (m);
          tap_handler_register_fdb (m);
          tap_handler_write_peek (m);
          tap_handler_write_port_all (m);

          rte_pktmbuf_free (m);
        }
    }
}

static inline __attribute__ ((always_inline)) void
tap_handler_handle_packet_down ()
{
  int nfds = 0;
  struct pollfd fds[16];
  struct vswitch *vswitch;
  struct vswitch_port *vswport;
  int vswport_id;
  int port_id;
  int ret;
  int i;
  char data[9000];
  char *pkt;

  vswitch = &vswitch0;

  nfds = 0;
  for (port_id = 0; port_id < vswitch->size; port_id++)
    {
      vswport = &vswitch->port[port_id];
      if (vswport->type != VSWITCH_PORT_TYPE_LINUX_TAP)
        continue;

      if (nfds >= 16)
        break;

      fds[nfds].fd = vswport->sockfd;
      fds[nfds].events = POLL_IN;
      nfds++;
    }

  ret = poll (fds, nfds, 0);

  for (i = 0; i < nfds; i++)
    {
      if ((fds[i].revents & (POLLIN | POLLERR)) == 0)
        continue;

      /* find the corresponding vswport */
      for (port_id = 0; port_id < vswitch->size; port_id++)
        {
          vswport = &vswitch->port[port_id];
          if (vswport->type != VSWITCH_PORT_TYPE_LINUX_TAP)
            continue;
          if (vswport->sockfd == fds[i].fd)
            break;
        }
      if (port_id == vswitch->size)
        vswport = NULL;

      if (vswport == NULL)
        continue;

      ret = read (vswport->sockfd, data, sizeof (data));
      if (ret < 0)
        {
          DEBUG_SDPLANE_LOG (TAPHANDLER, "read() failed: %s",
                             strerror (errno));
          continue;
        }
      if (ret == 0)
        continue;

      for (port_id = 0; port_id < vswitch->size; port_id++)
        {
          int socket;
          struct rte_mempool *mp;
          struct rte_mbuf *m;
          struct rte_ring *ring;

          vswport = &vswitch->port[port_id];
          if (vswport->type != VSWITCH_PORT_TYPE_DPDK_LCORE)
            continue;

          socket = rte_lcore_to_socket_id (rte_lcore_id ());
          //mp = pktmbuf_pool[vswport->dpdk_port_id][socket];
          mp = l2fwd_pktmbuf_pool;
          m = rte_pktmbuf_alloc (mp);
          rte_pktmbuf_append (m, ret);
          pkt = rte_pktmbuf_mtod (m, char *);
          memcpy (pkt, data, ret);

          ring = ring_dn[vswport->dpdk_port_id][vswport->dpdk_queue_id];
          rte_ring_enqueue (ring, m);
          DEBUG_SDPLANE_LOG (TAPHANDLER,
                             "packet: sockfd %d -> ring_dn[%d][%d]",
                             vswport->sockfd, vswport->dpdk_port_id,
                             vswport->dpdk_queue_id);
        }
    }
}

int
tap_handler (__rte_unused void *dummy)
{
  int ret;

  int port_id;
  uint16_t nb_ports;
  char port_name[64];

  unsigned lcore_id;
  struct rte_ring *tap_ring;

  struct vswitch *vswitch;

  struct vswitch_port *vswport;
  int vswport_id;

  DEBUG_SDPLANE_LOG (TAPHANDLER, "start thread on lcore[%d].",
                     rte_lcore_id ());

  snprintf (port_name, sizeof (port_name), "peek0");
  peek_fd = tap_open (port_name);
  tap_admin_up (port_name);
  DEBUG_SDPLANE_LOG (TAPHANDLER, "create %s and make it up.", port_name);

  memset (&vswitch0, 0, sizeof (vswitch));
  vswitch = &vswitch0;
  vswitch->limit = VSWITCH_PORT_SIZE;

  for (port_id = 0; port_id < RTE_MAX_ETHPORTS; port_id++)
    port_fd[port_id] = -1;

  /* create linux-tap: port-dpdk0 */
  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      snprintf (port_name, sizeof (port_name), "port-dpdk%d", port_id);
      port_fd[port_id] = tap_open (port_name);
      tap_admin_up (port_name);
      DEBUG_SDPLANE_LOG (TAPHANDLER, "create %s and make it up.", port_name);

      if (vswitch->size < vswitch->limit)
        {
          vswport_id = vswitch->size;
          vswport = &vswitch->port[vswport_id];
          vswport->id = vswport_id;
          vswport->type = VSWITCH_PORT_TYPE_LINUX_TAP;
          vswport->name = strdup (port_name);
          vswport->sockfd = port_fd[port_id];
          vswport->dpdk_port_id = -1;
          vswport->dpdk_queue_id = -1;
          DEBUG_SDPLANE_LOG (TAPHANDLER,
                             "vswitch->port[%d]: "
                             "type: linux-tap name: %s sockfd: %d",
                             vswport_id, vswport->name, vswport->sockfd);
          vswitch->size++;
        }
    }

  int i, j;
  memset (fdb, 0, sizeof (fdb));

  unsigned tap_handler_id = rte_lcore_id ();

  int thread_id;
  thread_id = thread_lookup_by_lcore (tap_handler, tap_handler_id);
  thread_register_loop_counter (thread_id, &loop_counter);

  DEBUG_SDPLANE_LOG (TAPHANDLER, "start main loop on lcore[%d].",
                     tap_handler_id);

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_register_thread ();
#endif /*HAVE_LIBURCU_QSBR*/

  while (! force_quit && ! force_stop[tap_handler_id])
    {
      // lthread_sleep (0); // yield.
      // printf ("%s: schedule: %lu.\n", __func__, loop_counter);

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_lock ();
      rib = (struct rib *) rcu_dereference (rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

      uint64_t rib_ver;
      if (rib && rib_ver < rib->ver)
        {
          vswitch_port_update ();
          rib_ver = rib->ver;
        }

      tap_handler_handle_packet_up ();
      tap_handler_handle_packet_down ();

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_unlock ();
      urcu_qsbr_quiescent_state ();
#endif /*HAVE_LIBURCU_QSBR*/

      loop_counter++;
    }

  close (peek_fd);
  peek_fd = -1;

  printf ("%s on lcore[%d]: finished.\n", __func__, rte_lcore_id ());
  return 0;
}
