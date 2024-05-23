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

#include <zcmdsh/debug.h>
#include <zcmdsh/termio.h>
#include <zcmdsh/vector.h>
#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>
#include <zcmdsh/command_shell.h>
#include <zcmdsh/debug_cmd.h>

#include "l2fwd_export.h"

#include "sdplane.h"

volatile bool force_stop[RTE_MAX_LCORE];

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

struct flag_name rx_offload_capa[] =
{
  { "VLAN_STRIP", RTE_ETH_RX_OFFLOAD_VLAN_STRIP       },
  { "IPV4_CKSUM", RTE_ETH_RX_OFFLOAD_IPV4_CKSUM       },
  { "UDP_CKSUM", RTE_ETH_RX_OFFLOAD_UDP_CKSUM         },
  { "TCP_CKSUM", RTE_ETH_RX_OFFLOAD_TCP_CKSUM         },
  { "TCP_LRO", RTE_ETH_RX_OFFLOAD_TCP_LRO             },
  { "QINQ_STRIP", RTE_ETH_RX_OFFLOAD_QINQ_STRIP       },
  { "OUTER_IPV4_CKSUM", RTE_ETH_RX_OFFLOAD_OUTER_IPV4_CKSUM },
  { "MACSEC_STRIP", RTE_ETH_RX_OFFLOAD_MACSEC_STRIP   },
  { "VLAN_FILTER", RTE_ETH_RX_OFFLOAD_VLAN_FILTER     },
  { "VLAN_EXTEND", RTE_ETH_RX_OFFLOAD_VLAN_EXTEND     },
  { "SCATTER", RTE_ETH_RX_OFFLOAD_SCATTER             },

  { "TIMESTAMP", RTE_ETH_RX_OFFLOAD_TIMESTAMP         },
  { "SECURITY", RTE_ETH_RX_OFFLOAD_SECURITY           },
  { "KEEP_CRC", RTE_ETH_RX_OFFLOAD_KEEP_CRC           },
  { "SCTP_CKSUM", RTE_ETH_RX_OFFLOAD_SCTP_CKSUM       },
  { "OUTER_UDP_CKSUM", RTE_ETH_RX_OFFLOAD_OUTER_UDP_CKSUM  },
  { "RSS_HASH", RTE_ETH_RX_OFFLOAD_RSS_HASH           },
  { "BUFFER_SPLIT", RTE_ETH_RX_OFFLOAD_BUFFER_SPLIT   },
};

struct flag_name tx_offload_capa[] =
{
  { "VLAN_INSERT", RTE_ETH_TX_OFFLOAD_VLAN_INSERT      },
  { "IPV4_CKSUM", RTE_ETH_TX_OFFLOAD_IPV4_CKSUM       },
  { "UDP_CKSUM", RTE_ETH_TX_OFFLOAD_UDP_CKSUM        },
  { "TCP_CKSUM", RTE_ETH_TX_OFFLOAD_TCP_CKSUM        },
  { "SCTP_CKSUM", RTE_ETH_TX_OFFLOAD_SCTP_CKSUM       },
  { "TCP_TSO", RTE_ETH_TX_OFFLOAD_TCP_TSO          },
  { "UDP_TSO", RTE_ETH_TX_OFFLOAD_UDP_TSO          },
  { "OUTER_IPV4_CKSUM", RTE_ETH_TX_OFFLOAD_OUTER_IPV4_CKSUM },
  { "QINQ_INSERT", RTE_ETH_TX_OFFLOAD_QINQ_INSERT      },
  { "VXLAN_TNL_TSO", RTE_ETH_TX_OFFLOAD_VXLAN_TNL_TSO    },
  { "GRE_TNL_TSO", RTE_ETH_TX_OFFLOAD_GRE_TNL_TSO      },
  { "IPIP_TNL_TSO", RTE_ETH_TX_OFFLOAD_IPIP_TNL_TSO     },
  { "GENEVE_TNL_TSO", RTE_ETH_TX_OFFLOAD_GENEVE_TNL_TSO   },
  { "MACSEC_INSERT", RTE_ETH_TX_OFFLOAD_MACSEC_INSERT    },

  { "MT_LOCKFREE", RTE_ETH_TX_OFFLOAD_MT_LOCKFREE      },
  { "MULTI_SEGS", RTE_ETH_TX_OFFLOAD_MULTI_SEGS       },
  { "MBUF_FAST_FREE", RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE   },
  { "SECURITY", RTE_ETH_TX_OFFLOAD_SECURITY         },
  { "UDP_TNL_TSO", RTE_ETH_TX_OFFLOAD_UDP_TNL_TSO      },
  { "IP_TNL_TSO", RTE_ETH_TX_OFFLOAD_IP_TNL_TSO       },
  { "OUTER_UDP_CKSUM", RTE_ETH_TX_OFFLOAD_OUTER_UDP_CKSUM  },
  { "SEND_ON_TIMESTAMP", RTE_ETH_TX_OFFLOAD_SEND_ON_TIMESTAMP},
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

          char rx_offload_str[128];
          char tx_offload_str[128];
          snprintf_flags (rx_offload_str, sizeof (rx_offload_str),
                      dev_info.rx_offload_capa, rx_offload_capa, "|",
                      sizeof (rx_offload_capa) / sizeof (struct flag_name));
          snprintf_flags (tx_offload_str, sizeof (tx_offload_str),
                      dev_info.tx_offload_capa, tx_offload_capa, "|",
                      sizeof (tx_offload_capa) / sizeof (struct flag_name));
          fprintf (t, "  rx_offload_capa: <%s>\n",
                   rx_offload_str);
          fprintf (t, "  tx_offload_capa: <%s>\n",
                   tx_offload_str);

          char rx_conf_str[128];
          char tx_conf_str[128];
          memset (rx_conf_str, 0, sizeof (rx_conf_str));
          memset (tx_conf_str, 0, sizeof (tx_conf_str));
          snprintf_flags (rx_conf_str, sizeof (rx_conf_str),
                      dev_info.default_rxconf.offloads, rx_offload_capa, "|",
                      sizeof (rx_offload_capa) / sizeof (struct flag_name));
          snprintf_flags (tx_conf_str, sizeof (tx_conf_str),
                      dev_info.default_txconf.offloads, tx_offload_capa, "|",
                      sizeof (tx_offload_capa) / sizeof (struct flag_name));
          fprintf (t, "  default_rxconf.offloads: <%s>\n", rx_conf_str);
          fprintf (t, "  default_txconf.offloads: <%s>\n", tx_conf_str);

          int i;
          uint32_t ptypes[32];
          char ptypes_name[32];
          ret = rte_eth_dev_get_supported_ptypes (port_id, RTE_PTYPE_ALL_MASK,
                  ptypes, 32);
          for (i = 0; i < ret && i < 32; i++)
            {
              rte_get_ptype_name (ptypes[i], ptypes_name,
                                  sizeof (ptypes_name));
              fprintf (t, "  ptypes[%d]: %s\n", i, ptypes_name);
            }
        }
    }
}

DEFINE_COMMAND (show_port_statistics,
                "show port statistics (pps|total|bps|total-bytes)",
                SHOW_HELP
                PORT_HELP
                "statistics\n"
                "pps\n"
                "total packets\n"
                "bps\n"
                "total bytes\n"
                )
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  int i, port_id;
  uint16_t nb_ports;
  char name[16];
  bool packets = false;
  bool total = false;
  struct rte_eth_stats *stats, *stats_array;

  if (! strcmp (argv[3], "pps"))
    {
      packets = true; total = false;
      stats_array = stats_per_sec;
    }
  else if (! strcmp (argv[3], "total"))
    {
      packets = true; total = true;
      stats_array = stats_current;
    }
  else if (! strcmp (argv[3], "bps"))
    {
      packets = false; total = false;
      stats_array = stats_per_sec;
    }
  else if (! strcmp (argv[3], "total-bytes"))
    {
      packets = false; total = true;
      stats_array = stats_current;
    }

  if (packets)
    fprintf (t, "%16s %8s %8s %8s %8s\n",
             "port name:", "rx", "tx", "ierrors", "oerrors");
  else
    fprintf (t, "%16s %8s %8s\n",
             "port name:", "bytes-in", "bytes-out");

  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      stats = &stats_array[port_id];
      snprintf (name, sizeof (name), "port[%d]:", port_id);
      if (packets)
      fprintf (t, "%16s %'8lu %'8lu %'8lu %'8lu\n", name,
               stats->ipackets, stats->opackets,
               stats->ierrors, stats->oerrors);
      else
      fprintf (t, "%16s %'8lu %'8lu\n", name,
               stats->ibytes, stats->obytes);
    }
}

DEFINE_COMMAND (set_port_promiscuous,
                "set port (<0-16>|all) promiscuous (enable|disable)",
                SET_HELP
                PORT_HELP
                PORT_NUMBER_HELP
                ALL_HELP
                "promiscuous\n"
                ENABLE_HELP
                DISABLE_HELP
                )
{
  struct shell *shell = (struct shell *) context;
  int i, port_spec = -1;
  uint16_t port_id, nb_ports;
  int ret;

  if (strcmp (argv[2], "all"))
    port_spec = strtol (argv[2], NULL, 0);

  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      if (port_spec != -1 && port_spec != port_id)
        continue;
      if (! strcmp (argv[4], "enable"))
        ret = rte_eth_promiscuous_enable (port_id);
      else
        ret = rte_eth_promiscuous_disable (port_id);
      if (ret < 0)
        fprintf (shell->terminal, "set promiscuous error: ret: %d\n", ret);
    }
}

DEFINE_COMMAND (show_port_promiscuous,
                "show port (<0-16>|all) promiscuous",
                SHOW_HELP
                PORT_HELP
                PORT_NUMBER_HELP
                ALL_HELP
                "promiscuous\n"
                )
{
  struct shell *shell = (struct shell *) context;
  int i, port_spec = -1;
  uint16_t port_id, nb_ports;
  int ret;

  if (strcmp (argv[2], "all"))
    port_spec = strtol (argv[2], NULL, 0);

  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      if (port_spec != -1 && port_spec != port_id)
        continue;
      ret = rte_eth_promiscuous_get (port_id);
      if (ret < 0)
        fprintf (shell->terminal, "get promiscuous error: ret: %d\n", ret);
      else if (ret == 1)
        fprintf (shell->terminal, "port[%d]: promiscuous: enabled.\n",
                 port_id);
      else
        fprintf (shell->terminal, "port[%d]: promiscuous: disabled.\n",
                 port_id);
    }
}

DEFINE_COMMAND (show_port_flowcontrol,
                "show port (<0-16>|all) flowcontrol",
                SHOW_HELP
                PORT_HELP
                PORT_NUMBER_HELP
                ALL_HELP
                "flowcontrol\n"
                )
{
  struct shell *shell = (struct shell *) context;
  int i, port_spec = -1;
  uint16_t port_id, nb_ports;
  int ret;
  struct rte_eth_fc_conf fc_conf;
  bool rx_enabled = false;
  bool tx_enabled = false;

  if (strcmp (argv[2], "all"))
    port_spec = strtol (argv[2], NULL, 0);

  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      if (port_spec != -1 && port_spec != port_id)
        continue;

      ret = rte_eth_dev_flow_ctrl_get (port_id, &fc_conf);
      if (ret < 0)
        {
          fprintf (shell->terminal, "get flow_ctrl error: ret: %d\n", ret);
          continue;
        }

      if (fc_conf.mode == RTE_ETH_FC_FULL)
        {
          rx_enabled = tx_enabled = true;
        }
      else
        {
          if (fc_conf.mode == RTE_ETH_FC_RX_PAUSE)
            rx_enabled = true;
          if (fc_conf.mode == RTE_ETH_FC_TX_PAUSE)
            tx_enabled = true;
        }

      fprintf (shell->terminal, "port[%d]: flow control:\n", port_id);
      fprintf (shell->terminal, "rx pause: %s\n", (rx_enabled ? "on" : "off"));
      fprintf (shell->terminal, "tx pause: %s\n", (tx_enabled ? "on" : "off"));
      fprintf (shell->terminal, "autoneg: %s\n",
               (fc_conf.autoneg ? "on" : "off"));
      fprintf (shell->terminal, "pause time: %'u\n", fc_conf.pause_time);
      fprintf (shell->terminal, "high waterline: %'u\n", fc_conf.high_water);
      fprintf (shell->terminal, "low waterline: %'u\n", fc_conf.low_water);
      fprintf (shell->terminal, "send xon: %s\n",
               (fc_conf.send_xon ? "on" : "off"));
      fprintf (shell->terminal, "forward mac control frames: %s\n",
               (fc_conf.mac_ctrl_frame_fwd ? "on" : "off"));
    }
}

DEFINE_COMMAND (set_port_flowcontrol,
                "set port (<0-16>|all) flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)",
                SET_HELP
                PORT_HELP
                PORT_NUMBER_HELP
                ALL_HELP
                "flowcontrol\n"
                "flowcontrol rx\n"
                "flowcontrol tx\n"
                "flowcontrol autoneg\n"
                "flowcontrol send-xon\n"
                "flowcontrol forward mac control frames\n"
                "flowcontrol on\n"
                "flowcontrol off\n"
                )
{
  struct shell *shell = (struct shell *) context;
  int i, port_spec = -1;
  uint16_t port_id, nb_ports;
  int ret;
  struct rte_eth_fc_conf fc_conf;

  bool rx_enabled = false;
  bool tx_enabled = false;

  if (strcmp (argv[2], "all"))
    port_spec = strtol (argv[2], NULL, 0);

  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      if (port_spec != -1 && port_spec != port_id)
        continue;

      ret = rte_eth_dev_flow_ctrl_get (port_id, &fc_conf);
      if (ret < 0)
        {
          fprintf (shell->terminal, "get flow_ctrl error: ret: %d\n", ret);
          continue;
        }

      /* read the current flow control config. */
      if (fc_conf.mode == RTE_ETH_FC_FULL)
        {
          rx_enabled = tx_enabled = true;
        }
      else
        {
          if (fc_conf.mode == RTE_ETH_FC_RX_PAUSE)
            rx_enabled = true;
          if (fc_conf.mode == RTE_ETH_FC_TX_PAUSE)
            tx_enabled = true;
        }

      /* update the config */
      bool newval;
      if (! strcmp (argv[5], "on"))
        newval = true;
      else
        newval = false;

      if (! strcmp (argv[4], "rx"))
        rx_enabled = newval;
      else if (! strcmp (argv[4], "tx"))
        tx_enabled = newval;
      else if (! strcmp (argv[4], "autoneg"))
        fc_conf.autoneg = newval;
      else if (! strcmp (argv[4], "send-xon"))
        fc_conf.send_xon = newval;
      else if (! strcmp (argv[4], "fwd-mac-ctrl"))
        fc_conf.mac_ctrl_frame_fwd = newval;
      else if (! strcmp (argv[4], "all"))
        {
          rx_enabled = newval;
          tx_enabled = newval;
          fc_conf.autoneg = newval;
          fc_conf.send_xon = newval;
          fc_conf.mac_ctrl_frame_fwd = newval;
        }

      /* fill in back the mode to the fc_conf */
      if (rx_enabled && tx_enabled)
        fc_conf.mode = RTE_ETH_FC_FULL;
      else if (rx_enabled)
        fc_conf.mode = RTE_ETH_FC_RX_PAUSE;
      else if (tx_enabled)
        fc_conf.mode = RTE_ETH_FC_TX_PAUSE;
      else
        fc_conf.mode = RTE_ETH_FC_NONE;

      fprintf (shell->terminal, "port[%d]: flow control:\n", port_id);
      fprintf (shell->terminal, "rx pause: %s\n", (rx_enabled ? "on" : "off"));
      fprintf (shell->terminal, "tx pause: %s\n", (tx_enabled ? "on" : "off"));
      fprintf (shell->terminal, "autoneg: %s\n", (fc_conf.autoneg ? "on" : "off"));
      fprintf (shell->terminal, "send-xon: %s\n", (fc_conf.send_xon ? "on" : "off"));
      fprintf (shell->terminal, "fwd-mac-ctrl: %s\n", (fc_conf.send_xon ? "on" : "off"));
    }
}

void
soft_dplane_cmd_init (struct command_set *cmdset)
{
  setlocale (LC_ALL, "en_US.utf8");
  INSTALL_COMMAND2 (cmdset, show_port);
  INSTALL_COMMAND2 (cmdset, show_port_statistics);
  INSTALL_COMMAND2 (cmdset, set_locale);
  INSTALL_COMMAND2 (cmdset, start_stop_port);
  INSTALL_COMMAND2 (cmdset, set_port_promiscuous);
  INSTALL_COMMAND2 (cmdset, show_port_promiscuous);
  INSTALL_COMMAND2 (cmdset, show_port_flowcontrol);
  INSTALL_COMMAND2 (cmdset, set_port_flowcontrol);
}

extern struct rte_ring *tap_ring_by_lcore[RTE_MAX_LCORE];

void
soft_dplane_init ()
{
  int lcore_id;
  for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++)
    tap_ring_by_lcore[lcore_id] = NULL;
}

