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

#include <locale.h>

#include <rte_ethdev.h>
#include <rte_bus_pci.h>

#include "debug.h"
#include "termio.h"
#include "vector.h"
#include "shell.h"
#include "command.h"
#include "command_shell.h"
#include "debug_cmd.h"

#include "l2fwd.h"

#include "soft_dplane.h"

struct flag_name
{
  char *name;
  uint64_t val;
};

struct flag_name link_speeds[] =
{
  { "Fix",      RTE_ETH_LINK_SPEED_FIXED },
  { "10M-hd",   RTE_ETH_LINK_SPEED_10M_HD },
  { "10M",      RTE_ETH_LINK_SPEED_10M },
  { "100M-hd",  RTE_ETH_LINK_SPEED_10M_HD },
  { "100M",     RTE_ETH_LINK_SPEED_10M },
  { "1G",       RTE_ETH_LINK_SPEED_1G },
  { "2.5G",     RTE_ETH_LINK_SPEED_2_5G },
  { "5G",       RTE_ETH_LINK_SPEED_5G },
  { "10G",      RTE_ETH_LINK_SPEED_10G },
  { "20G",      RTE_ETH_LINK_SPEED_20G },
  { "25G",      RTE_ETH_LINK_SPEED_25G },
  { "40G",      RTE_ETH_LINK_SPEED_40G },
  { "50G",      RTE_ETH_LINK_SPEED_50G },
  { "56G",      RTE_ETH_LINK_SPEED_56G },
  { "100G",     RTE_ETH_LINK_SPEED_100G },
  { "200G",     RTE_ETH_LINK_SPEED_200G },
  { "400G",     RTE_ETH_LINK_SPEED_400G },
};

int
snprintf_flags (char *buf, int size, uint64_t flags,
                struct flag_name *flag_names,
                char *delim, int flag_names_size)
{
  char *p = buf;
  int bufsize = size;
  int ret = 0;
  int num = 0;
  int i;

  if (! delim)
    delim = "|";

  for (i = flag_names_size - 1; i >= 0; i--)
    {
      if (! (flags & flag_names[i].val))
        continue;
      if (bufsize <= 0)
        continue;

      if (num == 0)
        ret = snprintf (p, bufsize, "%s", flag_names[i].name);
      else
        ret = snprintf (p, bufsize, "%s%s", delim, flag_names[i].name);

      p += ret;
      bufsize -= ret;
      num++;
    }
  return num;
}

DEFINE_COMMAND (set_locale,
                //"set locale (LC_ALL|LC_NUMERIC) (C|C.utf8|en_US.utf8|POSIX)",
                "set locale (C|C.utf8|en_US.utf8|POSIX)",
                SET_HELP
                "locale information\n"
                "C\n"
                "C.utf8\n"
                "en_US.utf8\n"
                "POSIX")
{
  struct shell *shell = (struct shell *) context;
  char *ret;
  ret = setlocale (LC_ALL, argv[2]);
  if (! ret)
    fprintf (shell->terminal, "setlocale(): failed.\n");
  else
    fprintf (shell->terminal, "setlocale(): %s.\n", ret);
}

DEFINE_COMMAND (start_stop_port,
                "(start|stop|reset) port (|<0-16>|all)",
                START_HELP
                STOP_HELP
                RESET_HELP
                PORT_HELP
                PORT_NUMBER_HELP
                PORT_ALL_HELP
               )
{
  struct shell *shell = (struct shell *) context;
  int port_id;
  int port_spec = -1;
  uint16_t nb_ports;
  int ret;
  bool all = false;

  if (argc == 2)
    all = true;
  else if (strcmp (argv[2], "all"))
    port_spec = strtol (argv[2], NULL, 0);

  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      if (! all && port_spec != -1 && port_spec != port_id)
        continue;
      if (! strcmp (argv[0], "start"))
        ret = rte_eth_dev_start (port_id);
      else if (! strcmp (argv[0], "stop"))
        ret = rte_eth_dev_stop (port_id);
      else if (! strcmp (argv[0], "reset"))
        ret = rte_eth_dev_reset (port_id);
      printf ("rte_eth_dev_%s (): ret: %d port: %u\n",
              argv[0], ret, port_id);
    }
}

DEFINE_COMMAND (show_port,
                "show port (|<0-16>|all)",
                SHOW_HELP
                PORT_HELP
                PORT_NUMBER_HELP
                PORT_ALL_HELP
               )
{
  struct shell *shell = (struct shell *) context;
  int port_id;
  int port_spec = -1;
  uint16_t nb_ports;
  int ret;
  struct rte_eth_dev_info dev_info;
  struct rte_eth_dev_info *dev = &dev_info;
  const struct rte_pci_device *pci_dev = NULL;
  FILE *t = shell->terminal;
  char link_capa[32];
  bool brief = false;
  const char *devname;
  const char *businfo;
  struct rte_eth_link link;
  char *status;

  if (argc == 2)
    brief = true;
  else if (strcmp (argv[2], "all"))
    port_spec = strtol (argv[2], NULL, 0);

  if (brief)
    fprintf (t, "%-8s %-12s %6s %7s %-13s <%-24s>\n",
             "port:", "device", "status", "speed",
             "driver", "capability");

  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      if (port_spec != -1 && port_spec != port_id)
        continue;

      ret = rte_eth_dev_info_get (port_id, &dev_info);
      if (ret != 0)
        {
          fprintf (t, "rte_eth_dev_info_get() returned %d.\n", ret);
          continue;
        }

      memset (&link, 0, sizeof (link));
      ret = rte_eth_link_get_nowait (port_id, &link);
      if (ret < 0)
        {
          fprintf (t, "port[%d]: rte_eth_link_get_nowait() failed: %d.\n",
                   port_id, ret);
        }
      status = (link.link_status ? "up" : "down");

      devname = rte_dev_name (dev->device);
      businfo = rte_dev_bus_info (dev->device);
      snprintf_flags (link_capa, sizeof (link_capa),
                      dev_info.speed_capa, link_speeds, "|",
                      sizeof (link_speeds) / sizeof (struct flag_name));

      if (brief)
        {
          char port_name[16];
          snprintf (port_name, sizeof (port_name), "port[%d]:", port_id);
          fprintf (t, "%-8s %-12s %6s %'7d %-13s <%-24s>\n",
                   port_name, devname, status, link.link_speed,
                   dev->driver_name, link_capa);
        }
      else
        {
          fprintf (t, "port[%d]:\n", port_id);
          fprintf (t, "  link speed: %'d\n", link.link_speed);
          fprintf (t, "  link duplex: %d\n", link.link_duplex);
          fprintf (t, "  link autoneg: %d\n", link.link_autoneg);
          fprintf (t, "  link status: %d\n", link.link_status);
          fprintf (t, "  device name: %s\n", devname);
          fprintf (t, "  bus info: %s\n", businfo);
          fprintf (t, "  driver_name: %s\n", dev->driver_name);
          fprintf (t, "  if_index: %'u\n", dev->if_index);
          fprintf (t, "  min_mtu: %'u max_mtu: %'u\n",
                   dev->min_mtu, dev->max_mtu);
          fprintf (t, "  min_rx_bufsize: %'u max_rx_bufsize: %'u\n",
                   dev->min_rx_bufsize, dev->max_rx_bufsize);
          fprintf (t, "  max_rx_pktlen: %'u max_lro_pkt_size: %'u\n",
                   dev->max_rx_pktlen, dev->max_lro_pkt_size);
          fprintf (t, "  max_rx_queues: %'u max_tx_queues: %'u\n",
                   dev->max_rx_queues, dev->max_tx_queues);
          fprintf (t, "  speed_capa: <%s>\n", link_capa);
          fprintf (t, "  nb_rx_queues: %'u nb_tx_queues: %'u\n",
                   dev->nb_rx_queues, dev->nb_tx_queues);
        }
    }
}

DEFINE_COMMAND (clear_cmd,
                "clear",
                CLEAR_HELP)
{
  struct shell *shell = (struct shell *) context;
  const char clr[] = { 27, '[', '2', 'J', '\0' };
  const char topLeft[] = { 27, '[', '1', ';', '1', 'H', '\0' };

  /* Clear screen and move to top left */
  fprintf (shell->terminal, "%s%s", clr, topLeft);
  fflush (shell->terminal);
}

void
soft_dplane_cmd_init (struct command_set *cmdset)
{
  setlocale (LC_ALL, "en_US.utf8");
  INSTALL_COMMAND2 (cmdset, clear_cmd);
  INSTALL_COMMAND2 (cmdset, show_port);
  INSTALL_COMMAND2 (cmdset, set_locale);
  INSTALL_COMMAND2 (cmdset, start_stop_port);
}

extern struct rte_ring *tap_ring_by_lcore[RTE_MAX_LCORE];

void
soft_dplane_init ()
{
  int lcore_id;
  for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++)
    tap_ring_by_lcore[lcore_id] = NULL;
}

