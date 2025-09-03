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
#include <rte_malloc.h>

#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>

#include <sdplane/debug.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "rib_manager.h"
#include "sdplane.h"
#include "thread_info.h"

#include "l2fwd_export.h"

#include "internal_message.h"

// clang-format off

CLI_COMMAND2 (show_rib,
              "show rib",
              SHOW_HELP,
              "show rib information.\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;
  int i, j;
  int nb_ports;

  if (! rib || ! rib->rib_info)
    {
      fprintf (shell->terminal, "no rib information.%s", shell->NL);
      return 0;
    }

#if 0
  nb_ports = rte_eth_dev_count_avail ();

  fprintf (shell->terminal, "rib: ver: %llu%s", rib->ver, shell->NL);
  for (i = 0; i < nb_ports; i++)
    fprintf (shell->terminal, "link[%d]: speed: %lu "
             "duplex: %d autoneg: %d status: %d%s", i,
             rib->link[i].link_speed, rib->link[i].link_duplex,
             rib->link[i].link_autoneg, rib->link[i].link_status,
             shell->NL);

  for (i = 0; i < RTE_MAX_LCORE; i++)
    {
      struct sdplane_queue_conf *qconf;
      qconf = &rib->qconf[i];
      for (j = 0; j < qconf->nrxq; j++)
        {
          struct port_queue_conf *rx_queue;
          rx_queue = &qconf->rx_queue_list[j];
          fprintf (shell->terminal,
                   "lcore[%d]: rx_queue[%d] { port_id: %d queue_id: %d }%s",
                   i, j, rx_queue->port_id, rx_queue->queue_id,
                   shell->NL);
        }
    }
#endif

  // show rib information version
  fprintf (shell->terminal, "rib information version: %lu (%p)%s",
           rib->rib_info->ver,
           rib->rib_info, shell->NL);

#if 0
  fprintf (shell->terminal, "rib_info: tapif_size: %d%s",
           rib->rib_info->tapif_size, shell->NL);
  for (i = 0; i < rib->rib_info->tapif_size; i++)
    {
      struct tap_if_conf *tapconf;
      tapconf = &rib->rib_info->tap_if[i];
      fprintf (shell->terminal, "rib_info: tapif[%d]: sockfd: %d%s",
               i, tapconf->sockfd, shell->NL);
    }
#endif

  // show vswitches
  fprintf (shell->terminal, "vswitches: %s", shell->NL);
  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vswitch = &rib->rib_info->vswitch[i];

      // show vswitch links
      fprintf (shell->terminal, "  vswitch[%d]: %s", i, shell->NL);
      for (j = 0; j < vswitch->vswitch_port_size; j++)
        {
          uint16_t vswitch_link_id = vswitch->vswitch_link_id[j];
          struct vswitch_link *link = &rib->rib_info->vswitch_link[vswitch_link_id];

          fprintf (shell->terminal,
                   "    vswitch_link[%d]: "
                   "vswitch_link_id=%d, dpdk_port[%u], vlan=%u, tag=%u%s",
                   j, vswitch_link_id,
                   link->port_id, link->vlan_id, link->tag_id, shell->NL);
        }
    }

  // show dpdk port information
  fprintf (shell->terminal, "dpdk ports: %s", shell->NL);
  for (i = 0; i < rib->rib_info->port_size; i++)
    {
      struct port_conf *port = &rib->rib_info->port[i];

      fprintf (shell->terminal, "  dpdk_port[%d]: %s",
               port->dpdk_port_id, shell->NL);
      fprintf (shell->terminal,
              "    link: speed=%luMbps duplex=%s autoneg=%s status=%s%s",
              port->link.link_speed,
              ETH_LINK_DUPLEX_STR(port->link.link_duplex),
              ETH_LINK_AUTONEG_STR(port->link.link_autoneg),
              ETH_LINK_STATUS_STR(port->link.link_status),
              shell->NL);
      fprintf (shell->terminal,
              "    nb_rxd=%hu nb_txd=%hu%s",
              port->nb_rxd, port->nb_txd,
              shell->NL);
      fprintf (shell->terminal,
              "    queues: nrxq=%d ntxq=%d%s",
              port->dev_info.nb_rx_queues,
              port->dev_info.nb_tx_queues,
              shell->NL);
    }

  // show lcore informartion
  fprintf (shell->terminal, "lcores: %s", shell->NL);
  for (i = 0; i < rib->rib_info->lcore_size; i++)
    {
      struct lcore_qconf *qconf = &rib->rib_info->lcore_qconf[i];

      // show number of RX queues in each lcore
      fprintf (shell->terminal, "  lcore[%d]: nrxq=%d%s",
               i, qconf->nrxq, shell->NL);
      for (j = 0; j < qconf->nrxq; j++)
        {
          struct port_queue_conf *rx_queue = &qconf->rx_queue_list[j];

          // show RX queue information
          fprintf (shell->terminal,
                   "    rxq[%d]: "
                   "dpdk_port[%d], queue_id=%d%s",
                   j, rx_queue->port_id, rx_queue->queue_id, shell->NL);
        }
    }

  return 0;
}

/* 🤖 生成AI (CLAUDE) */
CLI_COMMAND2 (show_rib_vswitch,
              "show rib vswitch",
              SHOW_HELP,
              "show rib information.\n"
              "show vswitch information.\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;
  int i, j;

  if (! rib || ! rib->rib_info)
    {
      fprintf (shell->terminal, "no rib information.%s", shell->NL);
      return 0;
    }

  fprintf (shell->terminal, "total vswitches: %d%s",
           rib->rib_info->vswitch_size, shell->NL);

  // show vswitches
  fprintf (shell->terminal, "vswitches: %s", shell->NL);
  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vswitch = &rib->rib_info->vswitch[i];

      // show vswitch links attached to vswitch[i]
      fprintf (shell->terminal, "  vswitch[%d]: %s", i, shell->NL);
      for (j = 0; j < vswitch->vswitch_port_size; j++)
        {
          uint16_t vswitch_link_id = vswitch->vswitch_link_id[j];
          struct vswitch_link *link = &rib->rib_info->vswitch_link[vswitch_link_id];

          fprintf (shell->terminal,
                   "    vswitch_link[%d]: "
                   "vswitch_link_id=%d, dpdk_port[%u], vlan=%u, tag=%u%s",
                   j, vswitch_link_id,
                   link->port_id, link->vlan_id, link->tag_id, shell->NL);
        }
    }

  return 0;
}

/* 🤖 生成AI (CLAUDE) */
CLI_COMMAND2 (show_rib_vswitch_link,
              "show rib vswitch-link",
              SHOW_HELP,
              "show rib information.\n"
              "show vswitch link information.\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;
  int i;

  if (! rib || ! rib->rib_info)
    {
      fprintf (shell->terminal, "no rib information.%s", shell->NL);
      return 0;
    }

  fprintf (shell->terminal, "total vswitch links: %d%s",
           rib->rib_info->vswitch_link_size, shell->NL);

  // show vswitch links
  fprintf (shell->terminal, "vswitch links: %s", shell->NL);
  for (i = 0; i < rib->rib_info->vswitch_link_size; i++)
    {
      struct vswitch_link *link = &rib->rib_info->vswitch_link[i];

      fprintf (shell->terminal,
               "  vswitch_link[%d]: "
               "id=%d, dpdk_port[%u] <-> vswitch[%u]%s",
               i, link->vswitch_link_id,
               link->port_id, link->vswitch_id, shell->NL);
    }

  return 0;
}
/* End of 🤖 生成AI (CLAUDE) */

CLI_COMMAND2 (show_rib_router_if,
              "show rib router-if",
              SHOW_HELP,
              "show rib information.\n"
              "show router interface information.\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;
  int i;
  char mac_str[18], ipv4_str[16], ipv6_str[40];

  if (! rib || ! rib->rib_info)
    {
      fprintf (shell->terminal, "no rib information.%s", shell->NL);
      return 0;
    }

  fprintf (shell->terminal, "router interface configurations:%s", shell->NL);

  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct router_if *rif = &rib->rib_info->vswitch[i].router_if;
      if (rif->sockfd < 0)
        continue;

      rte_ether_format_addr (mac_str, sizeof (mac_str), &rif->mac_addr);
      inet_ntop (AF_INET, &rif->ipv4_addr, ipv4_str, sizeof (ipv4_str));
      inet_ntop (AF_INET6, &rif->ipv6_addr, ipv6_str, sizeof (ipv6_str));

      fprintf (shell->terminal, "vswitch[%d]: router interface configured%s",
               rib->rib_info->vswitch[i].vswitch_id, shell->NL);
      fprintf (shell->terminal, "  tap_name: %s%s", rif->tap_name, shell->NL);
      fprintf (shell->terminal, "  MAC: %s, IPv4: %s, IPv6: %s%s", mac_str,
               ipv4_str, ipv6_str, shell->NL);
      fprintf (shell->terminal, "  sockfd: %d, tap_ring_id: %u%s", rif->sockfd,
               rif->tap_ring_id, shell->NL);
      fprintf (shell->terminal, "  ring_up: %p, ring_dn: %p%s", rif->ring_up,
               rif->ring_dn, shell->NL);
    }

  return 0;
}

CLI_COMMAND2 (show_rib_capture_if,
              "show rib capture-if",
              SHOW_HELP,
              "show rib information.\n"
              "show capture interface information.\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;
  int i;

  if (! rib || ! rib->rib_info)
    {
      fprintf (shell->terminal, "no rib information available%s", shell->NL);
      return 0;
    }

  fprintf (shell->terminal, "capture interface configurations:%s", shell->NL);

  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct capture_if *cif = &rib->rib_info->vswitch[i].capture_if;
      if (cif->sockfd < 0)
        continue;

      fprintf (shell->terminal, "vswitch[%d]: capture interface configured%s",
               rib->rib_info->vswitch[i].vswitch_id, shell->NL);
      fprintf (shell->terminal, "  sockfd: %d, tap_ring_id: %u%s", cif->sockfd,
               cif->tap_ring_id, shell->NL);
      fprintf (shell->terminal, "  ring_up: %p, ring_dn: %p%s", cif->ring_up,
               cif->ring_dn, shell->NL);
    }

  return 0;
}

CLI_COMMAND2 (set_vswitch,
              "set vswitch <1-4094> vlan <1-4094>",
              SET_HELP,
              "vswitch\n",
              "vswitch id\n",
              "vlan\n",
              "vlan id\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_vswitch vswitch_set;
  struct internal_msg_header *msgp;

  uint16_t vswitch_id = atoi (argv[2]);
  uint16_t vlan_id = atoi (argv[4]);

  vswitch_set.vswitch_id = vswitch_id;
  vswitch_set.vlan_id = vlan_id;
  msgp = internal_msg_create (INTERNAL_MSG_TYPE_VSWITCH_SET, &vswitch_set,
                              sizeof (vswitch_set));
  rib_manager_send_message (msgp, shell);

  return 0;
}

CLI_COMMAND2 (set_vswitch_port,
              "set vswitch <1-4094> port <0-7> (tagged|untag)",
              SET_HELP,
              "vswitch\n",
              "vswitch id\n",
              "port\n",
              "dpdk port id\n",
              "tagged\n",
              "untag\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_vswitch_port vswitch_port_set;
  struct internal_msg_header *msgp;

  uint16_t vswitch_id = atoi (argv[2]);
  uint16_t port_id = atoi (argv[4]);

  vswitch_port_set.vswitch_id = vswitch_id;
  vswitch_port_set.port_id = port_id;
  vswitch_port_set.tag_id = 0; // unused
  if (! strcmp (argv[5], "tagged"))
    vswitch_port_set.is_tagged = true;
  else if (! strcmp (argv[5], "untag"))
    vswitch_port_set.is_tagged = false;
  else
    {
      fprintf (shell->terminal, "usage: tagged or untag%s", shell->NL);
      return 0;
    }

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_VSWITCH_PORT_SET,
                              &vswitch_port_set, sizeof (vswitch_port_set));
  rib_manager_send_message (msgp, shell);

  return 0;
}

CLI_COMMAND2 (set_vswitch_port_tag_swap,
              "set vswitch <1-4094> port <0-7> tag swap <1-4094>",
              SET_HELP,
              "vswitch\n",
              "vswitch id\n",
              "port\n",
              "dpdk port id\n",
              "tag\n",
              "tag id\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_vswitch_port vswitch_port_set_tag_swap;
  struct internal_msg_header *msgp;

  uint16_t vswitch_id = atoi (argv[2]);
  uint16_t port_id = atoi (argv[4]);

  vswitch_port_set_tag_swap.vswitch_id = vswitch_id;
  vswitch_port_set_tag_swap.port_id = port_id;
  if (! strcmp (argv[5], "tag"))
    {
      uint16_t tag_id = atoi (argv[7]);
      vswitch_port_set_tag_swap.is_tagged = true;
      vswitch_port_set_tag_swap.tag_id = tag_id;
    }
  else
    {
      fprintf (shell->terminal, "usage: tag swap <vlan id>%s", shell->NL);
      return 0;
    }

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_VSWITCH_PORT_SET,
                              &vswitch_port_set_tag_swap,
                              sizeof (vswitch_port_set_tag_swap));
  rib_manager_send_message (msgp, shell);

  return 0;
}

CLI_COMMAND2 (set_router_if,
              "set vswitch <1-4094> router-if <WORD>",
              SET_HELP,
              "vswitch\n",
              "vswitch id\n",
              "router interface\n",
              "tap name\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_tap_dev router_if_set;
  struct internal_msg_header *msgp;
  int vswitch_id;
  char *tap_name;

  vswitch_id = atoi (argv[2]);
  tap_name = argv[4];

  router_if_set.vswitch_id = vswitch_id;
  snprintf (router_if_set.tap_name, sizeof (router_if_set.tap_name), "%s",
            tap_name);

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_ROUTER_IF_SET, &router_if_set,
                              sizeof (router_if_set));
  rib_manager_send_message (msgp, shell);

  return 0;
}

CLI_COMMAND2 (set_capture_if,
              "set vswitch <1-4094> capture-if <WORD>",
              SET_HELP,
              "vswitch\n",
              "vswitch id\n",
              "capture interface\n",
              "tap name\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_tap_dev capture_if_set;
  struct internal_msg_header *msgp;
  int vswitch_id;
  char *tap_name;

  vswitch_id = atoi (argv[2]);
  tap_name = argv[4];

  capture_if_set.vswitch_id = vswitch_id;
  snprintf (capture_if_set.tap_name, sizeof (capture_if_set.tap_name), "%s",
            tap_name);

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_CAPTURE_IF_SET,
                              &capture_if_set, sizeof (capture_if_set));
  rib_manager_send_message (msgp, shell);

  return 0;
}

CLI_COMMAND2 (no_set_vswitch,
              "no set vswitch <1-4094>",
              NO_HELP,
              SET_HELP,
              "vswitch\n",
              "vswitch id\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_vswitch vswitch_no_set;
  struct internal_msg_header *msgp;

  uint16_t vswitch_id = atoi (argv[3]);

  vswitch_no_set.vswitch_id = vswitch_id;
  msgp = internal_msg_create (INTERNAL_MSG_TYPE_VSWITCH_NO_SET,
                              &vswitch_no_set, sizeof (vswitch_no_set));
  rib_manager_send_message (msgp, shell);

  return 0;
}

CLI_COMMAND2 (no_set_vswitch_port,
              "no set vswitch <1-4094> port <0-7>",
              NO_HELP,
              SET_HELP,
              "vswitch\n",
              "vswitch id\n",
              "port\n",
              "dpdk port id\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_vswitch_port vswitch_port_no_set;
  struct internal_msg_header *msgp;
  uint16_t vswitch_id = atoi (argv[3]);
  uint16_t port_id = atoi (argv[5]);

  vswitch_port_no_set.vswitch_id = vswitch_id;
  vswitch_port_no_set.port_id = port_id;
  msgp =
      internal_msg_create (INTERNAL_MSG_TYPE_VSWITCH_PORT_NO_SET,
                           &vswitch_port_no_set, sizeof (vswitch_port_no_set));
  rib_manager_send_message (msgp, shell);

  return 0;
}

CLI_COMMAND2 (no_set_router_if,
              "no set router-if <WORD>",
              NO_HELP,
              SET_HELP,
              "router interface\n",
              "tap name\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_tap_dev router_if_no_set;
  struct internal_msg_header *msgp;
  char *tap_name;

  tap_name = argv[3];
  snprintf (router_if_no_set.tap_name, sizeof (router_if_no_set.tap_name),
            "%s", tap_name);

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_ROUTER_IF_NO_SET,
                              &router_if_no_set, sizeof (router_if_no_set));
  rib_manager_send_message (msgp, shell);

  return 0;
}

CLI_COMMAND2 (no_set_capture_if,
              "no set capture-if <WORD>",
              NO_HELP,
              SET_HELP,
              "capture interface\n",
              "tap name\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_tap_dev capture_if_no_set;
  struct internal_msg_header *msgp;
  char *tap_name;

  tap_name = argv[3];
  snprintf (capture_if_no_set.tap_name, sizeof (capture_if_no_set.tap_name),
            "%s", tap_name);

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_CAPTURE_IF_NO_SET,
                              &capture_if_no_set, sizeof (capture_if_no_set));
  rib_manager_send_message (msgp, shell);

  return 0;
}

void
rib_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, show_rib);
  INSTALL_COMMAND2 (cmdset, show_rib_vswitch);
  INSTALL_COMMAND2 (cmdset, show_rib_vswitch_link);
  INSTALL_COMMAND2 (cmdset, show_rib_router_if);
  INSTALL_COMMAND2 (cmdset, show_rib_capture_if);
  INSTALL_COMMAND2 (cmdset, set_vswitch);
  INSTALL_COMMAND2 (cmdset, set_vswitch_port);
  INSTALL_COMMAND2 (cmdset, set_vswitch_port_tag_swap);
  INSTALL_COMMAND2 (cmdset, set_router_if);
  INSTALL_COMMAND2 (cmdset, set_capture_if);
  INSTALL_COMMAND2 (cmdset, no_set_vswitch);
  INSTALL_COMMAND2 (cmdset, no_set_vswitch_port);
  INSTALL_COMMAND2 (cmdset, no_set_router_if);
  INSTALL_COMMAND2 (cmdset, no_set_capture_if);
}
