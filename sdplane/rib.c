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

#include "rib_manager.h"
#include "neigh_manager.h"
#include "fdb.h"
#include "mdb.h"
#include "sdplane.h"
#include "thread_info.h"
#include "internal_message.h"
#include "tap.h"
#include "fib.h"
#include "rib.h"
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
  RTE_LCORE_FOREACH (i)
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

  // show vswitches
  fprintf (shell->terminal, "#vswitches: %d%s",
           rib->rib_info->vswitch_size, shell->NL);

  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vswitch = &rib->rib_info->vswitch[i];

      fprintf (shell->terminal, "  vswitch[%d]: id: %d vlan: %d%s",
               i, vswitch->vswitch_id, vswitch->vlan_id, shell->NL);

      if (vswitch->is_deleted)
        {
          fprintf (shell->terminal, "    is_deleted: true%s",
                   shell->NL);
        }

      // show vswitch links attached to vswitch[i]
      fprintf (shell->terminal, "    vswitch_port_size: %d%s",
               vswitch->vswitch_port_size, shell->NL);
      for (j = 0; j < vswitch->vswitch_port_size; j++)
        {
          uint16_t vswlink_id = vswitch->vswitch_link_id[j];
          struct vswitch_link *link = &rib->rib_info->vswitch_link[vswlink_id];

          fprintf (shell->terminal, "    vswitch_link[%d]:"
                   " vswitch_link_id: %d port: %u vlan: %u tag: %u%s",
                   j, vswlink_id, link->port_id, link->vlan_id,
                   link->tag_id, shell->NL);
        }

      struct router_if *rif = &vswitch->router_if;
      fprintf (shell->terminal,
               "    router-if: %s ifindex: %d%s"
               "        sockfd: %d tap_ring_id: %d ring(up/down): %p/%p%s",
               rif->tap_name, rif->ifindex, shell->NL,
               rif->sockfd, rif->tap_ring_id, rif->ring_up, rif->ring_dn,
               shell->NL);

      struct capture_if *cif = &vswitch->capture_if;
      fprintf (shell->terminal,
               "    capture-if: %s%s"
               "        sockfd: %d tap_ring_id: %d ring(up/down): %p/%p%s",
               cif->tap_name, shell->NL,
               cif->sockfd, cif->tap_ring_id, cif->ring_up, cif->ring_dn,
               shell->NL);

    }

  // show vswitch-links
  fprintf (shell->terminal, "#vswitch-links: %d%s",
           rib->rib_info->vswitch_link_size, shell->NL);
  for (i = 0; i < rib->rib_info->vswitch_link_size; i++)
    {
      struct vswitch_link *link = &rib->rib_info->vswitch_link[i];
      uint16_t vswlink_id = link->vswitch_link_id;
      fprintf (shell->terminal, "  vswitch_link[%d]:"
               " vswitch_link_id: %d port: %u vlan: %u tag: %u"
               " vswitch[%d]: port[%d] (is_deleted: %d)%s",
               i, vswlink_id, link->port_id, link->vlan_id, link->tag_id,
               link->vswitch_id, link->vswitch_port, link->is_deleted,
               shell->NL);
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
  char mac_str[18], ipv4_str[16], ipv6_str[40], ll_addr_str[40];

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
      inet_ntop (AF_INET6, &rif->ll_addr, ll_addr_str, sizeof (ll_addr_str));

      fprintf (shell->terminal, "vswitch[%d]: router interface configured%s",
               rib->rib_info->vswitch[i].vswitch_id, shell->NL);
      fprintf (shell->terminal, "  tap_name: %s%s", rif->tap_name, shell->NL);
      fprintf (shell->terminal, "  vlan-id: %hu%s", rif->vlan_id, shell->NL);
      fprintf (shell->terminal, "  ifindex: %d%s", rif->ifindex, shell->NL);
      fprintf (shell->terminal, "  MAC: %s, IPv4: %s, IPv6: %s%s", mac_str,
               ipv4_str, ipv6_str, shell->NL);
      fprintf (shell->terminal, "  link local: %s%s", ll_addr_str, shell->NL);
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

CLI_COMMAND2 (set_fdb_entry,
              "set fdb port <0-8> vlan <0-4096> lladdr "ETHER_ADDR_SPEC,
              SET_HELP,
              "set fdb entry\n",
              "set fdb entry port\n",
              "set fdb entry port\n",
              "set fdb entry vlan\n",
              "set fdb entry vlan\n",
              "set fdb entry lladdr\n",
              "set fdb entry lladdr\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_fdb_entry fdb_entry_add;
  struct internal_msg_header *msgp;

  rte_ether_unformat_addr (argv[7], &fdb_entry_add.mac_addr);
  fdb_entry_add.port = strtol (argv[3], NULL, 0);
  fdb_entry_add.vlan_id = strtol (argv[5], NULL, 0);

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_FDB_ENTRY_ADD, &fdb_entry_add,
                              sizeof (fdb_entry_add));
  shell_rib_send_message (msgp, shell);
  return CMD_SUCCESS;
}

CLI_COMMAND2 (show_fdb,
              "show fdb",
              SHOW_HELP,
              "show fdb information.\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;

  fprintf (shell->terminal, "fdb:%s", shell->NL);
  hash_table_foreach (&rib->rib_info->fdb_table, show_fdb_entry_callback, shell);
  fprintf (shell->terminal, "total fdb entries: %d%s",
           rib->rib_info->fdb_table.count, shell->NL);
  return 0;
}

static inline __attribute__ ((always_inline)) struct vswitch_conf *
vswitch_lookup (struct rib_info *rib_info, uint16_t vswitch_id)
{
  int i;
  for (i = 0; i < rib_info->vswitch_size; i++)
    {
      if (rib_info->vswitch[i].vswitch_id == vswitch_id &&
          ! rib_info->vswitch[i].is_deleted)
        return &rib_info->vswitch[i];
    }
  return NULL;
}

CLI_COMMAND2 (set_mdb_entry,
              "set igmp-snooping group "IPV4_ADDR_SPEC" vswitch <1-4094> port "PORT_ID_SPEC,
              SET_HELP,
              "IGMP snooping information\n",
              "multicast group information\n",
              "IPv4 multicast group address\n",
              "vswitch information\n",
              "vswitch ID\n",
              PORT_HELP,
              PORT_NUMBER_HELP)
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_mdb_entry mdb_entry_add;
  struct internal_msg_header *msgp;
  struct in_addr addr;

  if (inet_pton (AF_INET, argv[3], &addr) != 1
      || ! IN_MULTICAST (rte_be_to_cpu_32 (addr.s_addr)))
    {
      fprintf (shell->terminal, "invalid IPv4 multicast group: %s%s",
               argv[3], shell->NL);
      return CMD_FAILURE;
    }

  long port = strtol (argv[7], NULL, 0);
  if (port < 0 || port >= MAX_ETH_PORTS)
    {
      fprintf (shell->terminal, "invalid MDB port: %s%s",
               argv[7], shell->NL);
      return CMD_FAILURE;
    }

  mdb_entry_add.group_addr = addr.s_addr;
  mdb_entry_add.vswitch_id = strtol (argv[5], NULL, 0);
  mdb_entry_add.port = port;

  if (startup_config_completed && rib_tlocal && rib_tlocal->rib_info
      && ! vswitch_lookup (rib_tlocal->rib_info, mdb_entry_add.vswitch_id))
    {
      fprintf (shell->terminal, "vswitch %u does not exist%s",
               (unsigned int) mdb_entry_add.vswitch_id, shell->NL);
      return CMD_FAILURE;
    }

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_MDB_ENTRY_ADD, &mdb_entry_add,
                              sizeof (mdb_entry_add));
  shell_rib_send_message (msgp, shell);
  return CMD_SUCCESS;
}

CLI_COMMAND2 (no_set_mdb_entry,
              "no set igmp-snooping group "IPV4_ADDR_SPEC" vswitch <1-4094> port "PORT_ID_SPEC,
              NO_HELP,
              SET_HELP,
              "IGMP snooping information\n",
              "multicast group information\n",
              "IPv4 multicast group address\n",
              "vswitch information\n",
              "vswitch ID\n",
              PORT_HELP,
              PORT_NUMBER_HELP)
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_mdb_entry msg;
  struct internal_msg_header *msgp;
  struct in_addr addr;

  if (inet_pton (AF_INET, argv[4], &addr) != 1
      || ! IN_MULTICAST (rte_be_to_cpu_32 (addr.s_addr)))
    {
      fprintf (shell->terminal, "invalid IPv4 multicast group: %s%s",
               argv[4], shell->NL);
      return CMD_FAILURE;
    }

  long port = strtol (argv[8], NULL, 0);

  if (port < 0 || port >= MAX_ETH_PORTS)
    {
      fprintf (shell->terminal, "invalid MDB port: %s%s",
               argv[8], shell->NL);
      return CMD_FAILURE;
    }

  msg.group_addr = addr.s_addr;
  msg.vswitch_id = strtol (argv[6], NULL, 0);
  msg.port = port;

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_MDB_ENTRY_DEL, &msg,
                              sizeof (msg));
  shell_rib_send_message (msgp, shell);
  return CMD_SUCCESS;
}

CLI_COMMAND2 (show_mdb,
              "show igmp-snooping group",
              SHOW_HELP,
              "IGMP snooping information\n",
              "multicast group information\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;

  if (! rib || ! rib->rib_info)
    {
      fprintf (shell->terminal, "no rib information%s", shell->NL);
      return CMD_FAILURE;
    }

  fprintf (shell->terminal, "mdb:%s", shell->NL);
  hash_table_foreach (&rib->rib_info->mdb_table,
                      show_mdb_entry_callback, shell);
  fprintf (shell->terminal, "total mdb entries: %d%s",
           rib->rib_info->mdb_table.count, shell->NL);
  return CMD_SUCCESS;
}

CLI_COMMAND2 (show_neighbor,
              "show neighbor (|ipv4|ipv6) (|"IPV4_ADDR_SPEC"|"IPV6_ADDR_SPEC")",
              SHOW_HELP,
              "show neighbor table.\n",
              "show IPv4 ARP table.\n",
              "show IPv6 ND table.\n",
              "IPv4 address as key.\n",
              "IPv6 address as key.\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  int thread_id;
  bool spec_ipv4 = false;
  bool spec_ipv6 = false;
  bool spec_prefix = false;
  char *prefix;
  int ret;
  int family = 0;
  struct rib *rib = rib_tlocal;

  thread_id = thread_lookup (neigh_manager);
  if (thread_id < 0)
    {
      fprintf (t, "neigh_manager thread not found.%s", shell->NL);
      return -1;
    }

  if (argc > 2)
    {
      if (! strcmp (argv[2], "ipv4"))
        {
          spec_ipv4 = true;
          family = AF_INET;
        }
      else if (! strcmp (argv[2], "ipv6"))
        {
          spec_ipv6 = true;
          family = AF_INET6;
        }
    }

  if (argc > 3)
    {
      spec_prefix = true;
      prefix = argv[3];
    }
  else if (argc > 2 && ! spec_ipv4 && ! spec_ipv6)
    {
      spec_prefix = true;
      prefix = argv[2];
    }
  else if (argc == 2)
    {
      spec_ipv4 = spec_ipv6 = true;
    }

  if (spec_prefix)
    {
      struct in_addr ipv4_addr;
      struct in6_addr ipv6_addr;

      ret = inet_pton (AF_INET, prefix, &ipv4_addr);
      if (ret == 1)
        {
          fprintf (shell->terminal, "ipv4 addr: %s%s",
                   prefix, shell->NL);
          family = AF_INET;
        }
      else
        {
          ret = inet_pton (AF_INET6, prefix, &ipv6_addr);
          if (ret == 1)
            {
              fprintf (shell->terminal, "ipv6 addr: %s%s",
                       prefix, shell->NL);
              family = AF_INET6;
            }
          else
            {
              fprintf (shell->terminal, "unknown addr: %s%s",
                       prefix, shell->NL);
            }
        }

      int neigh_table_type;
      struct neigh_entry *neigh_entry = NULL;
      struct rte_ether_addr *dst_mac;
      char neigh_mac_str[RTE_ETHER_ADDR_FMT_SIZE];
      neigh_table_type = AF_TO_NEIGH_TABLE (family);
      switch (family)
        {
        case AF_INET:
          neigh_entry =
              neigh_lookup (&rib->rib_info->neigh_tables[neigh_table_type],
                            neigh_table_type, &ipv4_addr);
          break;
        case AF_INET6:
          neigh_entry =
              neigh_lookup (&rib->rib_info->neigh_tables[neigh_table_type],
                            neigh_table_type, &ipv6_addr);
          break;
        default:
          break;
        }
      if (neigh_entry)
        {
          dst_mac = &neigh_entry->mac_addr;
          rte_ether_format_addr (neigh_mac_str, sizeof (neigh_mac_str),
                                 dst_mac);
          fprintf (shell->terminal,
                   "neighbor lookup succeeded: addr: %s mac: %s%s",
                   prefix, neigh_mac_str, shell->NL);
          return CMD_SUCCESS;
        }
      else
        {
          fprintf (shell->terminal,
                   "neighbor lookup failed: addr: %s%s",
                   prefix, shell->NL);
          return CMD_FAILURE;
        }
    }

  //if (! spec_prefix)
    {
      if (spec_ipv4)
        neigh_manager_show_table (NEIGH_ARP_TABLE, shell);
      if (spec_ipv6)
        neigh_manager_show_table (NEIGH_ND_TABLE, shell);
    }

  return CMD_SUCCESS;
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
  struct show_route_arg show_arg = { .family = AF_UNSPEC };
  int i;

  if (! rib || ! rib->rib_info)
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

  fprintf (shell->terminal, "    %-*s %-*s %-8s %-10s %-12s %-20s%s",
           show_arg.family == AF_INET ? 22 : 47, "Destination",
           show_arg.family == AF_INET ? 20 : 45, "Nexthop",
           "NH-ID", "Table", "Type", "Interface", shell->NL);

  bool found = false;
  for (i = 0; i < ROUTE_TREE_SIZE; i++)
    {
      struct fib_tree *fib_tree = rib->rib_info->fib_tree[i];
      if (! fib_tree || fib_tree->family != show_arg.family)
        continue;

      fprintf (shell->terminal, "fib_tree[%d] (table %d):%p%s",
               i, fib_tree->table_id, fib_tree, shell->NL);
      fib_traverse (fib_tree, fib_show_route, &show_arg);
      found = true;
    }

  if (! found)
    {
      const char *family_str = (show_arg.family == AF_INET) ? "IPv4" : "IPv6";
      fprintf (shell->terminal, "no %s fib routes%s",
               family_str, shell->NL);
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

  if (! rib || ! rib->rib_info)
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

  fprintf (shell->terminal, "%-30s  %-30s  %-6s  Interface%s",
           "Destination", "Nexthop", "Table", shell->NL);

  bool found = false;
  for (i = 0; i < ROUTE_TREE_SIZE; i++)
    {
      struct rib_tree *rib_tree = rib_tree_master[i];
      if (! rib_tree || rib_tree->family != show_arg.family)
        continue;

      fprintf (shell->terminal, "rib_tree_master[%d] (table %d):%p%s",
               i, rib_tree->table_id, rib_tree, shell->NL);
      rib_traverse (rib_tree, rib_show_route, &show_arg);
      found = true;
    }

  if (! found)
    {
      const char *family_str = (show_arg.family == AF_INET) ? "IPv4" : "IPv6";
      fprintf (shell->terminal, "no %s rib routes%s",
               family_str, shell->NL);
    }

  return 0;
}

CLI_COMMAND2 (show_rib_nexthop_group,
              "show rib nexthop-group",
              SHOW_HELP,
              "rib_tree\n",
              "nexthop group table\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;

  if (! rib || ! rib->rib_info)
    {
      fprintf (shell->terminal, "no routing information%s", shell->NL);
      return 0;
    }

  nexthop_show_group_table (shell, rib->rib_info);

  return 0;
}

CLI_COMMAND2 (show_rib_nexthop_pool,
              "show rib nexthop-pool",
              SHOW_HELP,
              "rib_tree\n",
              "nexthop info pool\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;

  if (! rib || ! rib->rib_info)
    {
      fprintf (shell->terminal, "no routing information%s", shell->NL);
      return 0;
    }

  nexthop_show_pool (shell, rib->rib_info);

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
              "set vswitch <1-4094> port "PORT_ID_SPEC" (tagged|untag)",
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
              "set vswitch <1-4094> port "PORT_ID_SPEC" tag swap <1-4094>",
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
  struct rib *rib = rib_tlocal;

  memset (&router_if_set, 0, sizeof (router_if_set));
  vswitch_id = atoi (argv[2]);
  tap_name = argv[4];

  router_if_set.vswitch_id = vswitch_id;
  snprintf (router_if_set.tap_name, sizeof (router_if_set.tap_name), "%s",
            tap_name);

  if (argc > 5)
    {
      if (! strcmp (argv[5], "vlan-id"))
        {
          uint16_t vlan_id = strtol (argv[6], NULL, 0);
          router_if_set.vlan_id = vlan_id;
        }
    }
  else
    {
#if 0
      if (rib && rib->rib_info)
        {
          struct vswitch_conf *vswitch;
          vswitch = vswitch_lookup (rib->rib_info, vswitch_id);
          if (vswitch)
            router_if_set.vlan_id = vswitch->vlan_id;
        }
#endif
#define NO_VLAN_SPECIFIED 65535
      router_if_set.vlan_id = NO_VLAN_SPECIFIED;
    }

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_ROUTER_IF_SET, &router_if_set,
                              sizeof (router_if_set));
  shell_rib_send_message (msgp, shell);

  return 0;
}

ALIAS_COMMAND (set_router_if_vlan,
              set_router_if,
              "set vswitch <1-4094> router-if <WORD> vlan-id <0-4094>",
              SET_HELP
              "vswitch\n"
              "vswitch id\n"
              "router interface\n"
              "tap name\n");

CLI_COMMAND2 (set_router_if_hwaddr,
              "set vswitch <1-4094> router-if <WORD> hwaddr "ETHER_ADDR_SPEC,
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
  INSTALL_COMMAND2 (cmdset, set_fdb_entry);
  INSTALL_COMMAND2 (cmdset, show_fdb);
  INSTALL_COMMAND2 (cmdset, set_mdb_entry);
  INSTALL_COMMAND2 (cmdset, no_set_mdb_entry);
  INSTALL_COMMAND2 (cmdset, show_mdb);
  INSTALL_COMMAND2 (cmdset, show_neighbor);
  INSTALL_COMMAND2 (cmdset, show_vswitch);
  INSTALL_COMMAND2 (cmdset, show_fib_ip_route);
  INSTALL_COMMAND2 (cmdset, show_rib_ip_route);
  INSTALL_COMMAND2 (cmdset, show_rib_nexthop_group);
  INSTALL_COMMAND2 (cmdset, show_rib_nexthop_pool);
  INSTALL_COMMAND2 (cmdset, set_vswitch);
  INSTALL_COMMAND2 (cmdset, set_vswitch_port);
  INSTALL_COMMAND2 (cmdset, set_vswitch_port_tag_swap);
  INSTALL_COMMAND2 (cmdset, set_router_if);
  INSTALL_COMMAND2 (cmdset, set_router_if_vlan);
  INSTALL_COMMAND2 (cmdset, set_router_if_hwaddr);
  INSTALL_COMMAND2 (cmdset, set_capture_if);
  INSTALL_COMMAND2 (cmdset, no_set_vswitch);
  INSTALL_COMMAND2 (cmdset, no_set_vswitch_port);
  INSTALL_COMMAND2 (cmdset, no_set_router_if);
  INSTALL_COMMAND2 (cmdset, no_set_capture_if);
}
