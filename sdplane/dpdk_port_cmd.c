/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#include "include.h"

#include <rte_ethdev.h>
#include <rte_bus_pci.h>
#include <rte_malloc.h>

#include <sdplane/debug.h>
#include <sdplane/termio.h>
#include <sdplane/vector.h>
#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>
#include <sdplane/debug_cmd.h>

#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "l3fwd.h"
#include "l2fwd_export.h"

#include "sdplane.h"
#include "stat_collector.h"
#include "tap_handler.h"

#include "snprintf_flags.h"

#include "rib.h"
#include "internal_message.h"

// clang-format off

struct flag_name link_speeds[] = {
  { "Fix", RTE_ETH_LINK_SPEED_FIXED },
  { "10M-hd", RTE_ETH_LINK_SPEED_10M_HD },
  { "10M", RTE_ETH_LINK_SPEED_10M },
  { "100M-hd", RTE_ETH_LINK_SPEED_10M_HD },
  { "100M", RTE_ETH_LINK_SPEED_10M },
  { "1G", RTE_ETH_LINK_SPEED_1G },
  { "2.5G", RTE_ETH_LINK_SPEED_2_5G },
  { "5G", RTE_ETH_LINK_SPEED_5G },
  { "10G", RTE_ETH_LINK_SPEED_10G },
  { "20G", RTE_ETH_LINK_SPEED_20G },
  { "25G", RTE_ETH_LINK_SPEED_25G },
  { "40G", RTE_ETH_LINK_SPEED_40G },
  { "50G", RTE_ETH_LINK_SPEED_50G },
  { "56G", RTE_ETH_LINK_SPEED_56G },
  { "100G", RTE_ETH_LINK_SPEED_100G },
  { "200G", RTE_ETH_LINK_SPEED_200G },
  { "400G", RTE_ETH_LINK_SPEED_400G },
};

struct flag_name rx_offload_capa[] = {
  { "VLAN_STRIP", RTE_ETH_RX_OFFLOAD_VLAN_STRIP },
  { "IPV4_CKSUM", RTE_ETH_RX_OFFLOAD_IPV4_CKSUM },
  { "UDP_CKSUM", RTE_ETH_RX_OFFLOAD_UDP_CKSUM },
  { "TCP_CKSUM", RTE_ETH_RX_OFFLOAD_TCP_CKSUM },
  { "TCP_LRO", RTE_ETH_RX_OFFLOAD_TCP_LRO },
  { "QINQ_STRIP", RTE_ETH_RX_OFFLOAD_QINQ_STRIP },
  { "OUTER_IPV4_CKSUM", RTE_ETH_RX_OFFLOAD_OUTER_IPV4_CKSUM },
  { "MACSEC_STRIP", RTE_ETH_RX_OFFLOAD_MACSEC_STRIP },
  { "VLAN_FILTER", RTE_ETH_RX_OFFLOAD_VLAN_FILTER },
  { "VLAN_EXTEND", RTE_ETH_RX_OFFLOAD_VLAN_EXTEND },
  { "SCATTER", RTE_ETH_RX_OFFLOAD_SCATTER },

  { "TIMESTAMP", RTE_ETH_RX_OFFLOAD_TIMESTAMP },
  { "SECURITY", RTE_ETH_RX_OFFLOAD_SECURITY },
  { "KEEP_CRC", RTE_ETH_RX_OFFLOAD_KEEP_CRC },
  { "SCTP_CKSUM", RTE_ETH_RX_OFFLOAD_SCTP_CKSUM },
  { "OUTER_UDP_CKSUM", RTE_ETH_RX_OFFLOAD_OUTER_UDP_CKSUM },
  { "RSS_HASH", RTE_ETH_RX_OFFLOAD_RSS_HASH },
  { "BUFFER_SPLIT", RTE_ETH_RX_OFFLOAD_BUFFER_SPLIT },
};

struct flag_name tx_offload_capa[] = {
  { "VLAN_INSERT", RTE_ETH_TX_OFFLOAD_VLAN_INSERT },
  { "IPV4_CKSUM", RTE_ETH_TX_OFFLOAD_IPV4_CKSUM },
  { "UDP_CKSUM", RTE_ETH_TX_OFFLOAD_UDP_CKSUM },
  { "TCP_CKSUM", RTE_ETH_TX_OFFLOAD_TCP_CKSUM },
  { "SCTP_CKSUM", RTE_ETH_TX_OFFLOAD_SCTP_CKSUM },
  { "TCP_TSO", RTE_ETH_TX_OFFLOAD_TCP_TSO },
  { "UDP_TSO", RTE_ETH_TX_OFFLOAD_UDP_TSO },
  { "OUTER_IPV4_CKSUM", RTE_ETH_TX_OFFLOAD_OUTER_IPV4_CKSUM },
  { "QINQ_INSERT", RTE_ETH_TX_OFFLOAD_QINQ_INSERT },
  { "VXLAN_TNL_TSO", RTE_ETH_TX_OFFLOAD_VXLAN_TNL_TSO },
  { "GRE_TNL_TSO", RTE_ETH_TX_OFFLOAD_GRE_TNL_TSO },
  { "IPIP_TNL_TSO", RTE_ETH_TX_OFFLOAD_IPIP_TNL_TSO },
  { "GENEVE_TNL_TSO", RTE_ETH_TX_OFFLOAD_GENEVE_TNL_TSO },
  { "MACSEC_INSERT", RTE_ETH_TX_OFFLOAD_MACSEC_INSERT },

  { "MT_LOCKFREE", RTE_ETH_TX_OFFLOAD_MT_LOCKFREE },
  { "MULTI_SEGS", RTE_ETH_TX_OFFLOAD_MULTI_SEGS },
  { "MBUF_FAST_FREE", RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE },
  { "SECURITY", RTE_ETH_TX_OFFLOAD_SECURITY },
  { "UDP_TNL_TSO", RTE_ETH_TX_OFFLOAD_UDP_TNL_TSO },
  { "IP_TNL_TSO", RTE_ETH_TX_OFFLOAD_IP_TNL_TSO },
  { "OUTER_UDP_CKSUM", RTE_ETH_TX_OFFLOAD_OUTER_UDP_CKSUM },
  { "SEND_ON_TIMESTAMP", RTE_ETH_TX_OFFLOAD_SEND_ON_TIMESTAMP },
};

CLI_COMMAND2 (start_stop_port, "(start|stop|reset) port (|<0-16>|all)",
              START_HELP, STOP_HELP, RESET_HELP, PORT_HELP, PORT_NUMBER_HELP,
              PORT_ALL_HELP)
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
      printf ("rte_eth_dev_%s (): ret: %d port: %u\n", argv[0], ret, port_id);
    }
  return 0;
}

CLI_COMMAND2 (show_port, "show port (|<0-16>|all)", SHOW_HELP, PORT_HELP,
              PORT_NUMBER_HELP, PORT_ALL_HELP)
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
  char link_capa2[48];
  bool brief = false;
  char devname[32];
  char *devname2;
  const char *businfo;
  struct rte_eth_link link;
  char drivername[32];
  char *drivername2;
  char *status;

  if (argc == 2)
    brief = true;
  else if (strcmp (argv[2], "all"))
    port_spec = strtol (argv[2], NULL, 0);

  if (brief)
    fprintf (t, "%-8s %-7s %6s %7s %-9s %-24s%s", "port:", "device", "status",
             "speed", "driver", "<capability>", shell->NL);

  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      if (port_spec != -1 && port_spec != port_id)
        continue;

      ret = rte_eth_dev_info_get (port_id, &dev_info);
      if (ret != 0)
        {
          fprintf (t, "rte_eth_dev_info_get() returned %d.%s", ret, shell->NL);
          continue;
        }

      memset (&link, 0, sizeof (link));
      ret = rte_eth_link_get_nowait (port_id, &link);
      if (ret < 0)
        {
          fprintf (t, "port[%d]: rte_eth_link_get_nowait() failed: %d.%s",
                   port_id, ret, shell->NL);
        }
      status = (link.link_status ? "up" : "down");

      snprintf (devname, sizeof (devname), "%s", rte_dev_name (dev->device));
      devname2 = NULL;
      if (! strncmp (devname, "0000:", 5))
        devname2 = &devname[5];
      businfo = rte_dev_bus_info (dev->device);
      snprintf_flags (link_capa, sizeof (link_capa), dev_info.speed_capa,
                      link_speeds, "|",
                      sizeof (link_speeds) / sizeof (struct flag_name));
      snprintf (link_capa2, sizeof (link_capa2), "<%s>", link_capa);
      snprintf (drivername, sizeof (drivername), "%s", dev->driver_name);
      drivername2 = NULL;
      if (! strncmp (drivername, "net_", 4))
        drivername2 = &drivername[4];

      if (brief)
        {
          char port_name[16];
          snprintf (port_name, sizeof (port_name), "port[%d]:", port_id);
          fprintf (t, "%-8s %-7s %6s %'7d %-9s %-24s%s", port_name,
                   (devname2 ? devname2 : devname), status, link.link_speed,
                   (drivername2 ? drivername2 : drivername), link_capa2,
                   shell->NL);
        }
      else
        {
          fprintf (t, "port[%d]:%s", port_id, shell->NL);
          fprintf (t, "  link speed: %'d%s", link.link_speed, shell->NL);
          fprintf (t, "  link duplex: %d%s", link.link_duplex, shell->NL);
          fprintf (t, "  link autoneg: %d%s", link.link_autoneg, shell->NL);
          fprintf (t, "  link status: %d%s", link.link_status, shell->NL);
          fprintf (t, "  device name: %s%s", devname, shell->NL);
          fprintf (t, "  bus info: %s%s", businfo, shell->NL);
          fprintf (t, "  driver_name: %s%s", dev->driver_name, shell->NL);
          fprintf (t, "  if_index: %'u%s", dev->if_index, shell->NL);
          fprintf (t, "  min_mtu: %'u max_mtu: %'u%s", dev->min_mtu,
                   dev->max_mtu, shell->NL);
          fprintf (t, "  min_rx_bufsize: %'u max_rx_bufsize: %'u%s",
                   dev->min_rx_bufsize, dev->max_rx_bufsize, shell->NL);
          fprintf (t, "  max_rx_pktlen: %'u max_lro_pkt_size: %'u%s",
                   dev->max_rx_pktlen, dev->max_lro_pkt_size, shell->NL);
          fprintf (t, "  max_rx_queues: %'u max_tx_queues: %'u%s",
                   dev->max_rx_queues, dev->max_tx_queues, shell->NL);
          fprintf (t, "  speed_capa: %s%s", link_capa2, shell->NL);
          fprintf (t, "  nb_rx_queues: %'u nb_tx_queues: %'u%s",
                   dev->nb_rx_queues, dev->nb_tx_queues, shell->NL);

          char rx_offload_str[128];
          char tx_offload_str[128];
          snprintf_flags (rx_offload_str, sizeof (rx_offload_str),
                          dev_info.rx_offload_capa, rx_offload_capa, "|",
                          sizeof (rx_offload_capa) /
                              sizeof (struct flag_name));
          snprintf_flags (tx_offload_str, sizeof (tx_offload_str),
                          dev_info.tx_offload_capa, tx_offload_capa, "|",
                          sizeof (tx_offload_capa) /
                              sizeof (struct flag_name));
          fprintf (t, "  rx_offload_capa: <%s>%s", rx_offload_str, shell->NL);
          fprintf (t, "  tx_offload_capa: <%s>%s", tx_offload_str, shell->NL);

          char rx_conf_str[128];
          char tx_conf_str[128];
          memset (rx_conf_str, 0, sizeof (rx_conf_str));
          memset (tx_conf_str, 0, sizeof (tx_conf_str));
          snprintf_flags (
              rx_conf_str, sizeof (rx_conf_str),
              dev_info.default_rxconf.offloads, rx_offload_capa, "|",
              sizeof (rx_offload_capa) / sizeof (struct flag_name));
          snprintf_flags (
              tx_conf_str, sizeof (tx_conf_str),
              dev_info.default_txconf.offloads, tx_offload_capa, "|",
              sizeof (tx_offload_capa) / sizeof (struct flag_name));
          fprintf (t, "  default_rxconf.offloads: <%s>%s", rx_conf_str,
                   shell->NL);
          fprintf (t, "  default_txconf.offloads: <%s>%s", tx_conf_str,
                   shell->NL);

          int i;
          uint32_t ptypes[32];
          char ptypes_name[32];
          ret = rte_eth_dev_get_supported_ptypes (port_id, RTE_PTYPE_ALL_MASK,
                                                  ptypes, 32);
          for (i = 0; i < ret && i < 32; i++)
            {
              rte_get_ptype_name (ptypes[i], ptypes_name,
                                  sizeof (ptypes_name));
              fprintf (t, "  ptypes[%d]: %s%s", i, ptypes_name, shell->NL);
            }
        }
    }
  return 0;
}

CLI_COMMAND2 (show_port_statistics,
              "show port statistics (|pps|total|bps|Bps|total-bytes)",
              SHOW_HELP,
              PORT_HELP,
              "show port statistics\n",
              "packets per second.\n",
              "total packets.\n",
              "bits per second.\n",
              "Bytes per second.\n",
              "total bytes.\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  int i, port_id;
  uint16_t nb_ports;
  char name[16];
  bool packets = false;
  bool bytes = false;
  bool total = false;
  struct rte_eth_stats *stats, *stats_array;

  /* default is to show "pps" */
  packets = true;
  total = false;
  stats_array = stats_per_sec;

  if (argc > 3)
    {
  if (! strcmp (argv[3], "pps"))
    {
      packets = true;
      total = false;
      stats_array = stats_per_sec;
    }
  else if (! strcmp (argv[3], "total"))
    {
      packets = true;
      total = true;
      stats_array = stats_current;
    }
  else if (! strcmp (argv[3], "bps"))
    {
      packets = false;
      total = false;
      stats_array = stats_per_sec;
    }
  else if (! strcmp (argv[3], "Bps"))
    {
      packets = false;
      bytes = true;
      total = false;
      stats_array = stats_per_sec;
    }
  else if (! strcmp (argv[3], "total-bytes"))
    {
      packets = false;
      bytes = true;
      total = true;
      stats_array = stats_current;
    }
    }

  /* 100Gbps: 148.8Mpps = 148,800,000 1Tbps: 1.488Gpps = 1,488,000,000: 13 */
  if (packets)
    fprintf (t, "%16s %13s %13s %8s %8s%s", "port name:", "rx", "tx", "ierrors",
             "oerrors", shell->NL);
  else if (bytes)
    fprintf (t, "%16s %15s %15s%s", "port name:", "bytes-in", "bytes-out",
             shell->NL);
  else
    fprintf (t, "%16s %15s %15s%s", "port name:", "bits-in", "bits-out",
             shell->NL);

  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      stats = &stats_array[port_id];
      snprintf (name, sizeof (name), "port[%d]:", port_id);
      if (packets)
        fprintf (t, "%16s %'13lu %'13lu %'8lu %'8lu%s", name, stats->ipackets,
                 stats->opackets, stats->ierrors, stats->oerrors, shell->NL);
      else if (bytes)
        fprintf (t, "%16s %'15lu %'15lu%s", name, stats->ibytes, stats->obytes,
                 shell->NL);
      else
        fprintf (t, "%16s %'15lu %'15lu%s", name,
                 stats->ibytes * 8, stats->obytes * 8,
                 shell->NL);
    }
  return 0;
}

CLI_COMMAND2 (show_port_promiscuous, "show port (<0-16>|all) promiscuous",
              SHOW_HELP PORT_HELP PORT_NUMBER_HELP ALL_HELP "promiscuous\n")
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
        fprintf (shell->terminal, "get promiscuous error: ret: %d%s", ret,
                 shell->NL);
      else if (ret == 1)
        fprintf (shell->terminal, "port[%d]: promiscuous: enabled.%s", port_id,
                 shell->NL);
      else
        fprintf (shell->terminal, "port[%d]: promiscuous: disabled.%s",
                 port_id, shell->NL);
    }
  return 0;
}

CLI_COMMAND2 (show_port_flowcontrol, "show port (<0-16>|all) flowcontrol",
              SHOW_HELP PORT_HELP PORT_NUMBER_HELP ALL_HELP "flowcontrol\n")
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

      fprintf (shell->terminal, "port[%d]: flow control:%s", port_id,
               shell->NL);
      fprintf (shell->terminal, "rx pause: %s%s", (rx_enabled ? "on" : "off"),
               shell->NL);
      fprintf (shell->terminal, "tx pause: %s%s", (tx_enabled ? "on" : "off"),
               shell->NL);
      fprintf (shell->terminal, "autoneg: %s%s",
               (fc_conf.autoneg ? "on" : "off"), shell->NL);
      fprintf (shell->terminal, "pause time: %'u%s", fc_conf.pause_time,
               shell->NL);
      fprintf (shell->terminal, "high waterline: %'u%s", fc_conf.high_water,
               shell->NL);
      fprintf (shell->terminal, "low waterline: %'u%s", fc_conf.low_water,
               shell->NL);
      fprintf (shell->terminal, "send xon: %s%s",
               (fc_conf.send_xon ? "on" : "off"), shell->NL);
      fprintf (shell->terminal, "forward mac control frames: %s%s",
               (fc_conf.mac_ctrl_frame_fwd ? "on" : "off"), shell->NL);
    }
  return 0;
}

CLI_COMMAND2 (set_port_promiscuous,
              "set port (<0-16>|all) promiscuous (enable|disable)",
              SET_HELP PORT_HELP PORT_NUMBER_HELP ALL_HELP
              "promiscuous\n" ENABLE_HELP DISABLE_HELP)
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
  return 0;
}

CLI_COMMAND2 (
    set_port_flowcontrol,
    "set port (<0-16>|all) flowcontrol (rx|tx|autoneg|send-xon|fwd-mac-ctrl) (on|off)",
    SET_HELP PORT_HELP PORT_NUMBER_HELP ALL_HELP
    "flowcontrol\n"
    "flowcontrol rx\n"
    "flowcontrol tx\n"
    "flowcontrol autoneg\n"
    "flowcontrol send-xon\n"
    "flowcontrol forward mac control frames\n"
    "flowcontrol on\n"
    "flowcontrol off\n")
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
      fprintf (shell->terminal, "autoneg: %s\n",
               (fc_conf.autoneg ? "on" : "off"));
      fprintf (shell->terminal, "send-xon: %s\n",
               (fc_conf.send_xon ? "on" : "off"));
      fprintf (shell->terminal, "fwd-mac-ctrl: %s\n",
               (fc_conf.send_xon ? "on" : "off"));

      ret = rte_eth_dev_flow_ctrl_set (port_id, &fc_conf);
      fprintf (shell->terminal, "set flow_ctrl error: ret: %d\n", ret);
    }
  return 0;
}

extern struct rte_eth_dev_tx_buffer
    *tx_buffer_per_q[RTE_MAX_ETHPORTS][RTE_MAX_LCORE];

static int
lsi_event_callback(uint16_t port_id, enum rte_eth_event_type type, void *param,
		    void *ret_param)
{
  void *msgp;
  msgp = internal_msg_create (INTERNAL_MSG_TYPE_PORT_STATUS, NULL, 0);
  internal_msg_send_to (msg_queue_rib, msgp, NULL);
}

CLI_COMMAND2 (set_port_dev_configure,
              "set port (<0-16>|all) dev-configure <0-64> <0-64>", SHOW_HELP,
              PORT_HELP, PORT_NUMBER_HELP, ALL_HELP,
              "rte_eth_dev_configure.\n", "nb_rx_queue.\n", "nb_tx_queue.\n")
{
  struct shell *shell = (struct shell *) context;
  int port_spec = -1;
  uint16_t port_id, nb_ports;

  int ret;
  struct rte_eth_dev_info dev_info;
  struct rte_eth_conf port_conf =
    { .txmode = { .mq_mode = RTE_ETH_MQ_TX_NONE, },
      .intr_conf = { .lsc = 1 },
    };

  uint16_t nb_rx_queue = 1;
  uint16_t nb_tx_queue = 1;

  int i;
  struct rte_eth_rxconf rxq_conf;
  struct rte_eth_txconf txq_conf;

  const uint16_t nb_rxd = RX_DESC_DEFAULT;
  const uint16_t nb_txd = TX_DESC_DEFAULT;

  if (strcmp (argv[2], "all"))
    port_spec = strtol (argv[2], NULL, 0);

  nb_rx_queue = strtol (argv[4], NULL, 0);
  nb_tx_queue = strtol (argv[5], NULL, 0);

  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      if (port_spec != -1 && port_spec != port_id)
        continue;

      ret = rte_eth_dev_info_get (port_id, &dev_info);
      if (ret != 0)
        {
          fprintf (shell->terminal,
                   "rte_eth_dev_info_get(): port: %d failed: %s%s", port_id,
                   strerror (-ret), shell->NL);
          continue;
        }
      if (dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE)
        port_conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE;

      ret = rte_eth_dev_configure (port_id, nb_rx_queue, nb_tx_queue,
                                   &port_conf);
      if (ret < 0)
        {
          fprintf (shell->terminal,
                   "rte_eth_dev_configure(): port: %d failed: %d%s", port_id,
                   ret, shell->NL);
        }
      
      ret = rte_eth_dev_callback_register (
          port_id, RTE_ETH_EVENT_INTR_LSC, lsi_event_callback, NULL);
      if (ret < 0)
        {
          fprintf (shell->terminal,
                    "rte_eth_dev_callback_register(): port: %d failed: %d%s",
                    port_id, ret, shell->NL);
        }

      rxq_conf = dev_info.default_rxconf;
      rxq_conf.offloads = port_conf.rxmode.offloads;
      for (i = 0; i < nb_rx_queue; i++)
        {
          ret = rte_eth_rx_queue_setup (port_id, i, nb_rxd,
                                        rte_eth_dev_socket_id (port_id),
                                        &rxq_conf, l2fwd_pktmbuf_pool);
          if (ret < 0)
            {
              fprintf (shell->terminal,
                       "rte_eth_rx_queue_setup(): "
                       "port: %d queue: %d failed: %d%s",
                       port_id, i, ret, shell->NL);
            }
        }

      txq_conf = dev_info.default_txconf;
      txq_conf.offloads = port_conf.txmode.offloads;
      for (i = 0; i < nb_tx_queue; i++)
        {
          ret = rte_eth_tx_queue_setup (
              port_id, i, nb_txd, rte_eth_dev_socket_id (port_id), &txq_conf);
          if (ret < 0)
            {
              fprintf (shell->terminal,
                       "rte_eth_tx_queue_setup(): "
                       "port: %d queue: %d failed: %d%s",
                       port_id, i, ret, shell->NL);
            }

          if (tx_buffer_per_q[port_id][i])
            continue;

          DEBUG_SDPLANE_LOG (L2_REPEATER, "tx_buffer_init: port: %d queue: %d",
                             port_id, i);
          tx_buffer_per_q[port_id][i] = rte_zmalloc_socket (
              "tx_buffer", RTE_ETH_TX_BUFFER_SIZE (MAX_PKT_BURST), 0,
              rte_eth_dev_socket_id (port_id));
          rte_eth_tx_buffer_init (tx_buffer_per_q[port_id][i], MAX_PKT_BURST);
        }
    }
  return 0;
}

CLI_COMMAND2 (set_port_txrx_desc,
              "set port (<0-16>|all) (nrxdesc|ntxdesc) <0-16384>", SET_HELP,
              PORT_HELP, PORT_NUMBER_HELP, ALL_HELP,
              "set the number of rx descriptor for the port\n",
              "set the number of tx descriptor for the port\n",
              "Specify the descriptor number.\n")
{
  struct shell *shell = (struct shell *) context;
  int i, port_spec = -1;
  uint16_t port_id, nb_ports;
  int ret;
  bool rx_spec, tx_spec;
  uint16_t nb_rx_desc, nb_tx_desc;
  uint16_t desc_val;
  struct rib *rib = rib_tlocal;

  if (strcmp (argv[2], "all"))
    port_spec = strtol (argv[2], NULL, 0);

  rx_spec = false;
  if (! strcmp (argv[3], "nrxdesc"))
    rx_spec = true;
  tx_spec = false;
  if (! strcmp (argv[3], "ntxdesc"))
    tx_spec = true;

  desc_val = strtol (argv[4], NULL, 0);

  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      if (port_spec != -1 && port_spec != port_id)
        continue;

      nb_rx_desc = RX_DESC_DEFAULT;
      if (rib && rib->rib_info && rib->rib_info->port[port_id].nb_rxd)
        nb_rx_desc = rib->rib_info->port[port_id].nb_rxd;
      if (rx_spec)
        nb_rx_desc = desc_val;

      nb_tx_desc = TX_DESC_DEFAULT;
      if (rib && rib->rib_info && rib->rib_info->port[port_id].nb_txd)
        nb_tx_desc = rib->rib_info->port[port_id].nb_txd;
      if (tx_spec)
        nb_tx_desc = desc_val;

      fprintf (shell->terminal, "port: %d nb_rxd: %hu nb_txd: %hu%s", port_id,
               nb_rx_desc, nb_tx_desc, shell->NL);

      ret =
          rte_eth_dev_adjust_nb_rx_tx_desc (port_id, &nb_rx_desc, &nb_tx_desc);
      if (ret < 0)
        {
          fprintf (shell->terminal,
                   "rte_eth_dev_adjust_nb_rx_tx_desc(): error: ret: %d%s", ret,
                   shell->NL);
          continue;
        }
      else
        {
          fprintf (shell->terminal,
                   "rte_eth_dev_adjust_nb_rx_tx_desc(): success: ret: %d%s",
                   ret, shell->NL);
        }

      void *msgp;
      struct internal_msg_txrx_desc txrx_desc;
      txrx_desc.portid = port_id;
      txrx_desc.nb_rxd = nb_rx_desc;
      txrx_desc.nb_txd = nb_tx_desc;
      msgp = internal_msg_create (INTERNAL_MSG_TYPE_TXRX_DESC, &txrx_desc,
                                  sizeof (txrx_desc));
      ret = internal_msg_send_to (msg_queue_rib, msgp, shell);
      if (ret < 0)
        {
          return CMD_FAILURE;
        }

      fprintf (shell->terminal, "send internal msg: %p%s", msgp, shell->NL);
    }

  return CMD_SUCCESS;
}

CLI_COMMAND2 (set_port_link_updown,
              "set port (<0-16>|all) link (up|down)",
              SET_HELP, PORT_HELP, PORT_NUMBER_HELP, ALL_HELP,
              "set port link up/down status\n",
              "set port the link status up\n",
              "set port the link status down\n")
{
  struct shell *shell = (struct shell *) context;
  int i, port_spec = -1;
  uint16_t port_id, nb_ports;
  int ret;
  int negate = 0;

  if (strcmp (argv[2], "all"))
    port_spec = strtol (argv[2], NULL, 0);

  if (! strcmp (argv[4], "down"))
    negate = 1;

  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      if (port_spec != -1 && port_spec != port_id)
        continue;
      if (negate)
        rte_eth_dev_set_link_down (port_id);
      else
        rte_eth_dev_set_link_up (port_id);
    }

  return 0;
}


void
dpdk_port_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, start_stop_port);
  INSTALL_COMMAND2 (cmdset, show_port);
  INSTALL_COMMAND2 (cmdset, show_port_statistics);
  INSTALL_COMMAND2 (cmdset, show_port_promiscuous);
  INSTALL_COMMAND2 (cmdset, show_port_flowcontrol);
  INSTALL_COMMAND2 (cmdset, set_port_promiscuous);
  INSTALL_COMMAND2 (cmdset, set_port_flowcontrol);
  INSTALL_COMMAND2 (cmdset, set_port_dev_configure);
  INSTALL_COMMAND2 (cmdset, set_port_txrx_desc);
  INSTALL_COMMAND2 (cmdset, set_port_link_updown);
}
