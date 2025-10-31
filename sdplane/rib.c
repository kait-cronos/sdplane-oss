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

#include "tap.h"
#include "fib.h"
#include "radix.h"

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

  // show rib information version
  fprintf (shell->terminal, "rib information version: %lu (%p)%s",
           (unsigned long) rib->rib_info->ver,
           rib->rib_info, shell->NL);

  // show vswitches
  fprintf (shell->terminal, "vswitches: %s", shell->NL);
  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vswitch = &rib->rib_info->vswitch[i];

      if (vswitch->is_deleted)
        continue;

      // show vswitch links
      fprintf (shell->terminal, "  vswitch[%d]: id: %d vlan: %d%s",
               i, vswitch->vswitch_id, vswitch->vlan_id, shell->NL);
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
              (unsigned long) port->link.link_speed,
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

#if 0
  fprintf (shell->terminal, "total vswitches: %d%s",
           rib->rib_info->vswitch_size, shell->NL);
#endif

  // show vswitches
  fprintf (shell->terminal, "vswitches: %s", shell->NL);
  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vswitch = &rib->rib_info->vswitch[i];

      if (vswitch->is_deleted)
        continue;

      // show vswitch links attached to vswitch[i]
      fprintf (shell->terminal, "  vswitch[%d]: id: %d vlan: %d%s",
               i, vswitch->vswitch_id, vswitch->vlan_id, shell->NL);
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
/* End of 🤖 生成AI (CLAUDE) */

ALIAS_COMMAND (show_vswitch,
               show_rib_vswitch,
               "show vswitch",
               SHOW_HELP
               "show vswitch information.\n");

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

CLI_COMMAND2 (show_fdb,
              "show fdb",
              SHOW_HELP,
              "show fdb information.\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;
  int i, count = 0;
  char mac_str[32];

  fprintf (shell->terminal, "fdb:%s", shell->NL);
  for (i = 0; i < FDB_SIZE; i++)
    {
      if (rib->rib_info->fdb[i].state != FDB_STATE_NONE)
        {
          rte_ether_format_addr (mac_str, sizeof (mac_str),
                                 &rib->rib_info->fdb[i].l2addr);
          fprintf (shell->terminal,
                   "  fdb[%d]: %s vlan:%u port:%d last_seen:%lu%s",
                   i, mac_str, rib->rib_info->fdb[i].vlan_id,
                   rib->rib_info->fdb[i].port,
                   rib->rib_info->fdb[i].last_seen, shell->NL);
          count++;
        }
    }
  fprintf (shell->terminal, "total fdb entries: %d/%d%s", count, FDB_SIZE,
           shell->NL);
  return 0;
}

CLI_COMMAND2 (show_fib_ip_route,
              "show fib (ipv4|ipv6) route",
              SHOW_HELP,
              "fib_tree\n",
              "IP\n",
              "routing table\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;
  struct show_route_arg show_arg;
  int i;

  if (! rib || ! rib->rib_info || ! rib->rib_info->fib_tree)
    {
      fprintf (shell->terminal, "no routing information%s", shell->NL);
      return 0;
    }

  show_arg.shell = shell;
  show_arg.rib_info = rib->rib_info;
  if (! strcmp (argv[2], "ipv4"))
    show_arg.family = AF_INET;
  else if (! strcmp (argv[2], "ipv6"))
    show_arg.family = AF_INET6;

  fprintf (shell->terminal, "%-30s  %-30s  Interface%s",
           "Destination", "Nexthop", shell->NL);

  for (i = 0; i < ROUTE_TREE_SIZE; i++)
    {
      /* main table only for test */
      if (rib->rib_info->fib_tree[i]->table_id == 254 &&
          rib->rib_info->fib_tree[i]->family == show_arg.family)
        {
          fprintf (shell->terminal, "fib_tree[%d]:%p%s",
                   i, rib->rib_info->fib_tree[i], shell->NL);
          fib_traverse (rib->rib_info->fib_tree[i], fib_show_route,
                        &show_arg);
          break;
        }
    }

  return 0;
}

CLI_COMMAND2 (show_rib_ip_route,
              "show rib (ipv4|ipv6) route",
              SHOW_HELP,
              "rib_tree\n",
              "IP\n",
              "routing table\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;
  struct show_route_arg show_arg;
  int i;

  if (! rib || ! rib->rib_info || ! rib_tree_master)
    {
      fprintf (shell->terminal, "no routing information%s", shell->NL);
      return 0;
    }

  show_arg.shell = shell;
  show_arg.rib_info = rib->rib_info;
  if (! strcmp (argv[2], "ipv4"))
    show_arg.family = AF_INET;
  else if (! strcmp (argv[2], "ipv6"))
    show_arg.family = AF_INET6;

  fprintf (shell->terminal, "%-30s  %-30s  Interface%s",
           "Destination", "Nexthop", shell->NL);

  for (i = 0; i < ROUTE_TREE_SIZE; i++)
    {
      /* main table only for test */
      if (rib_tree_master[i]->table_id == 254 &&
          rib_tree_master[i]->family == show_arg.family)
        {
          fprintf (shell->terminal, "rib_tree_master[%d]:%p%s",
                   i, rib_tree_master[i], shell->NL);
          rib_traverse (rib_tree_master[i], rib_show_route,
                        &show_arg);
          break;
        }
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
  shell_rib_send_message (msgp, shell);

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
  shell_rib_send_message (msgp, shell);

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
  shell_rib_send_message (msgp, shell);

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
  shell_rib_send_message (msgp, shell);

  return 0;
}

CLI_COMMAND2 (set_router_if_hwaddr,
              "set vswitch <1-4094> router-if <WORD> hwaddr <WORD>",
              SET_HELP,
              "vswitch\n",
              "vswitch id\n",
              "router interface\n",
              "tap name\n",
              "set hardware address\n",
              "specify MAC address\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;
  int vswitch_id;
  char *tap_name;
  char *hwaddr_str;
  struct rte_ether_addr eth_addr;
  int i;
  struct vswitch_conf *matched = NULL;

  vswitch_id = atoi (argv[2]);
  tap_name = argv[4];
  hwaddr_str = argv[6];

  if (rib && rib->rib_info)
    {
      for (i = 0; i < rib->rib_info->vswitch_size; i++)
        {
          struct vswitch_conf *vswitch = &rib->rib_info->vswitch[i];
          if (vswitch->vswitch_id == vswitch_id)
            matched = vswitch;
        }

      if (! matched)
        return -1;
    }

  rte_ether_unformat_addr (hwaddr_str, &eth_addr);
  tap_set_hwaddr (tap_name, &eth_addr);

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
  shell_rib_send_message (msgp, shell);

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
  shell_rib_send_message (msgp, shell);

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
  shell_rib_send_message (msgp, shell);

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
  shell_rib_send_message (msgp, shell);

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
  shell_rib_send_message (msgp, shell);

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
  INSTALL_COMMAND2 (cmdset, show_fdb);
  INSTALL_COMMAND2 (cmdset, show_vswitch);
  INSTALL_COMMAND2 (cmdset, show_fib_ip_route);
  INSTALL_COMMAND2 (cmdset, show_rib_ip_route);
  INSTALL_COMMAND2 (cmdset, set_vswitch);
  INSTALL_COMMAND2 (cmdset, set_vswitch_port);
  INSTALL_COMMAND2 (cmdset, set_vswitch_port_tag_swap);
  INSTALL_COMMAND2 (cmdset, set_router_if);
  INSTALL_COMMAND2 (cmdset, set_router_if_hwaddr);
  INSTALL_COMMAND2 (cmdset, set_capture_if);
  INSTALL_COMMAND2 (cmdset, no_set_vswitch);
  INSTALL_COMMAND2 (cmdset, no_set_vswitch_port);
  INSTALL_COMMAND2 (cmdset, no_set_router_if);
  INSTALL_COMMAND2 (cmdset, no_set_capture_if);
}
