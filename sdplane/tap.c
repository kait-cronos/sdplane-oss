#include "include.h"

#include <fcntl.h>

#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/if_arp.h>

#include <rte_ether.h>

#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

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
        }
      close (sockfd);
      sockfd = -1;
    }
}

void
tap_set_hwaddr (char *ifname, struct rte_ether_addr *hwaddr)
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

      ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
      memcpy (&ifr.ifr_hwaddr.sa_data[0], hwaddr,
              sizeof (struct rte_ether_addr));

      ret = ioctl (sockfd, SIOCSIFHWADDR, &ifr);
      if (ret < 0)
        {
          DEBUG_SDPLANE_LOG (TAPHANDLER, "ioctl (SOICSIFHWADDR) failed: %s",
                             strerror (errno));
          close (sockfd);
          sockfd = -1;
        }
    }
}

int tap_get_ifindex (char *ifname)
{
  int ret;
  int sockfd;
  struct ifreq ifr;
  int ifindex;

  sockfd = socket (AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    {
      DEBUG_SDPLANE_LOG (TAPHANDLER, "socket() failed: %s", strerror (errno));
      return -1;
    }

  memset (&ifr, 0, sizeof (ifr));
  snprintf (ifr.ifr_name, IFNAMSIZ, "%s", ifname);

  ret = ioctl (sockfd, SIOCGIFINDEX, &ifr);
  if (ret < 0)
    {
      DEBUG_SDPLANE_LOG (TAPHANDLER, "ioctl (SOICGIFINDEX) failed: %s",
                          strerror (errno));
      close (sockfd);
      return -1;
    }

  ifindex = ifr.ifr_ifindex;
  close (sockfd);
  return ifindex;
}
