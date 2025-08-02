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
              "rib information\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;
  int i, j;
  int nb_ports;

  if (! rib)
    return 0;

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

  if (! rib->rib_info)
    {
      fprintf (shell->terminal, "no rib-info.%s", shell->NL);
      return 0;
    }

  fprintf (shell->terminal, "rib_info: ver: %lu (%p)%s", rib->rib_info->ver,
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

  fprintf (shell->terminal, "rib_info: vswitch_size: %d%s",
           rib->rib_info->vswitch_size, shell->NL);
  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vswitch;
      vswitch = &rib->rib_info->vswitch[i];
      if (vswitch->is_deleted)
        {
          fprintf (shell->terminal, "rib_info: vswitch[%d]: deleted%s", i,
                  shell->NL);
          continue;
        }
      fprintf (shell->terminal, "rib_info: vswitch[%d]: port_size: %d%s", i,
               vswitch->vswitch_port_size, shell->NL);
      for (j = 0; j < vswitch->vswitch_port_size; j++)
        {
          uint16_t vswitch_link_id = vswitch->vswitch_link_id[j];
          struct vswitch_link *link;
          link = &rib->rib_info->vswitch_link[vswitch_link_id];
          fprintf (shell->terminal,
                   "rib_info: vswitch[%d]: vswport[%d]: "
                   "vswitch_link: %d port_id: %u vlan: %u tag: %u "
                   "(vswitch%u[%u])%s",
                   i, j, vswitch_link_id, link->port_id, link->vlan_id,
                   link->tag_id, link->vswitch_id, link->vswitch_port,
                   shell->NL);
        }
    }

  fprintf (shell->terminal, "rib_info: port_size: %d%s",
           rib->rib_info->port_size, shell->NL);
  for (i = 0; i < rib->rib_info->port_size; i++)
    {
      struct port_conf *port;
      port = &rib->rib_info->port[i];
      fprintf (shell->terminal,
               "rib_info: port[%d]: "
               "nb_rxd: %hu nb_txd: %hu%s",
               i, port->nb_rxd, port->nb_txd, shell->NL);
      fprintf (shell->terminal,
               "rib_info: port[%d]: "
               "link: speed: %lu duplex: %d autoneg: %d status: %d%s",
               i, port->link.link_speed, port->link.link_duplex,
               port->link.link_autoneg, port->link.link_status, shell->NL);
      fprintf (shell->terminal, "rib_info: port[%d]: nrxq: %d ntxq: %d%s", i,
               port->dev_info.nb_rx_queues, port->dev_info.nb_tx_queues,
               shell->NL);
    }

  fprintf (shell->terminal, "rib_info: lcore_size: %d%s",
           rib->rib_info->lcore_size, shell->NL);
  for (i = 0; i < rib->rib_info->lcore_size; i++)
    {
      struct lcore_qconf *qconf;
      qconf = &rib->rib_info->lcore_qconf[i];
      fprintf (shell->terminal, "rib_info: lcore[%d]: nrxq: %d%s", i,
               qconf->nrxq, shell->NL);
      for (j = 0; j < qconf->nrxq; j++)
        {
          struct port_queue_conf *rx_queue;
          rx_queue = &qconf->rx_queue_list[j];
          fprintf (shell->terminal,
                   "rib_info: lcore[%d]: rxq[%d]: "
                   "port_id: %d queue_id: %d%s",
                   i, j, rx_queue->port_id, rx_queue->queue_id, shell->NL);
        }
    }

  return 0;
}

CLI_COMMAND2 (set_vswitch,
              "set vswitch <1-4094>",
              SET_HELP,
              "vswitch\n",
              "vlan id\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_vswitch_create vswitch_create;
  struct internal_msg_header *msgp;

  uint16_t vlan_id = atoi (argv[2]);

  vswitch_create.vlan_id = vlan_id;
  msgp = internal_msg_create (INTERNAL_MSG_TYPE_VSWITCH_CREATE,
                              &vswitch_create, sizeof (vswitch_create));
  rib_manager_send_message (msgp, shell);

  return 0;
}

CLI_COMMAND2 (delete_vswitch,
              "delete vswitch <0-3>",
              DELETE_HELP,
              "vswitch\n",
              "vswitch id\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_vswitch_delete vswitch_delete;
  struct internal_msg_header *msgp;

  uint16_t vswitch_id = atoi (argv[2]);

  vswitch_delete.vswitch_id = vswitch_id;
  msgp = internal_msg_create (INTERNAL_MSG_TYPE_VSWITCH_DELETE,
                              &vswitch_delete, sizeof (vswitch_delete));
  rib_manager_send_message (msgp, shell);

  return 0;
}

/* 🤖 生成AI (CLAUDE) */
CLI_COMMAND2 (show_vswitch_rib,
              "show vswitch_rib",
              SHOW_HELP,
              "show vswitch rib information\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;
  int i, j;

  if (! rib || ! rib->rib_info)
    {
      fprintf (shell->terminal, "no rib information available%s", shell->NL);
      return 0;
    }

  fprintf (shell->terminal, "vswitch configurations:%s", shell->NL);
  fprintf (shell->terminal, "total vswitches: %d%s",
           rib->rib_info->vswitch_size, shell->NL);

  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vswitch = &rib->rib_info->vswitch[i];
      if (vswitch->is_deleted)
        {
          fprintf (shell->terminal, "vswitch[%d]: deleted%s", i, shell->NL);
          continue;
        }
      fprintf (shell->terminal, "vswitch[%d]: vlan %u, ports: %u%s",
               vswitch->vswitch_id, vswitch->vlan_id,
               vswitch->vswitch_port_size, shell->NL);

      for (j = 0; j < vswitch->vswitch_port_size; j++)
        {
          uint16_t link_id = vswitch->vswitch_link_id[j];
          struct vswitch_link *link = &rib->rib_info->vswitch_link[link_id];
          fprintf (shell->terminal,
                   "  port[%d]: link %u -> port %u (tag:%u)%s", j, link_id,
                   link->port_id, link->tag_id, shell->NL);
        }
    }

  return 0;
}

CLI_COMMAND2 (set_vswitch_link,
              "set vswitch-link vswitch <0-3> port <0-7> tag <0-4094>",
              SET_HELP,
              "vswitch-link\n",
              "vswitch\n",
              "vswitch id\n",
              "port\n",
              "dpdk port id\n",
              "tag\n",
              "tag id (0: native, 1-4094: tagged)\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_vswitch_link_create vswitch_link_create;
  struct internal_msg_header *msgp;

  uint16_t vswitch_id = atoi (argv[3]);
  uint16_t port_id = atoi (argv[5]);
  uint16_t tag_id = atoi (argv[7]);

  vswitch_link_create.vswitch_id = vswitch_id;
  vswitch_link_create.port_id = port_id;
  vswitch_link_create.tag_id = tag_id;

  msgp =
      internal_msg_create (INTERNAL_MSG_TYPE_VSWITCH_LINK_CREATE,
                           &vswitch_link_create, sizeof (vswitch_link_create));
  rib_manager_send_message (msgp, shell);

  return 0;
}

CLI_COMMAND2 (delete_vswitch_link,
              "delete vswitch-link <0-7>",
              DELETE_HELP,
              "vswitch-link\n",
              "vswitch link id\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_vswitch_link_delete vswitch_link_delete;
  struct internal_msg_header *msgp;

  uint16_t vswitch_link_id = atoi (argv[2]);

  vswitch_link_delete.vswitch_link_id = vswitch_link_id;
  msgp =
      internal_msg_create (INTERNAL_MSG_TYPE_VSWITCH_LINK_DELETE,
                           &vswitch_link_delete, sizeof (vswitch_link_delete));
  rib_manager_send_message (msgp, shell);

  return 0;
}

/* 🤖 生成AI (CLAUDE) */
CLI_COMMAND2 (show_vswitch_link,
              "show vswitch-link",
              SHOW_HELP,
              "show vswitch link information\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;
  int i;

  if (! rib || ! rib->rib_info)
    {
      fprintf (shell->terminal, "no rib information available%s", shell->NL);
      return 0;
    }

  fprintf (shell->terminal, "vswitch link configurations:%s", shell->NL);
  fprintf (shell->terminal, "total vswitch links: %d%s",
           rib->rib_info->vswitch_link_size, shell->NL);

  for (i = 0; i < rib->rib_info->vswitch_link_size; i++)
    {
      struct vswitch_link *link = &rib->rib_info->vswitch_link[i];
      if (link->is_deleted)
        {
          fprintf (shell->terminal, "link[%d]: deleted%s", i, shell->NL);
          continue;
        }
      fprintf (
          shell->terminal,
          "link[%d]: port %u -> vswitch %u (vlan:%u, tag:%u, vswport:%u)%s",
          link->vswitch_link_id, link->port_id, link->vswitch_id,
          link->vlan_id, link->tag_id, link->vswitch_port, shell->NL);
    }

  return 0;
}
/* End of 🤖 生成AI (CLAUDE) */

CLI_COMMAND2 (set_router_if,
              "set router-if <0-3> <WORD>",
              SET_HELP,
              "router interface\n",
              "vswitch id\n",
              "tap name\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_router_if_create router_if_create;
  struct internal_msg_header *msgp;
  int vswitch_id;
  char *tap_name;

  vswitch_id = atoi (argv[2]);
  tap_name = argv[3];

  router_if_create.vswitch_id = vswitch_id;
  snprintf (router_if_create.tap_name, sizeof (router_if_create.tap_name),
            "%s", tap_name);

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_ROUTER_IF_CREATE,
                              &router_if_create, sizeof (router_if_create));
  rib_manager_send_message (msgp, shell);

  return 0;
}

CLI_COMMAND2 (delete_router_if,
              "delete router-if <0-3>",
              DELETE_HELP,
              "router interface\n",
              "vswitch id\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_router_if_delete router_if_delete;
  struct internal_msg_header *msgp;
  int vswitch_id;

  vswitch_id = atoi (argv[2]);

  router_if_delete.vswitch_id = vswitch_id;

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_ROUTER_IF_DELETE,
                              &router_if_delete, sizeof (router_if_delete));
  rib_manager_send_message (msgp, shell);

  return 0;
}

CLI_COMMAND2 (show_router_if,
              "show router-if",
              SHOW_HELP,
              "show router interface information\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;
  int i;
  char mac_str[18], ipv4_str[16], ipv6_str[40];

  if (! rib || ! rib->rib_info)
    {
      fprintf (shell->terminal, "no rib information available%s", shell->NL);
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
               i, shell->NL);
      fprintf (shell->terminal, "  MAC: %s, IPv4: %s, IPv6: %s%s", mac_str,
               ipv4_str, ipv6_str, shell->NL);
      fprintf (shell->terminal, "  sockfd: %d, tap_ring_id: %u%s", rif->sockfd,
               rif->tap_ring_id, shell->NL);
      fprintf (shell->terminal, "  ring_up: %p, ring_dn: %p%s", rif->ring_up,
               rif->ring_dn, shell->NL);
    }

  return 0;
}

CLI_COMMAND2 (set_capture_if,
              "set capture-if <0-3> <WORD>",
              SET_HELP,
              "capture interface\n",
              "vswitch id\n",
              "tap name\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_capture_if_create capture_if_create;
  struct internal_msg_header *msgp;
  int vswitch_id;
  char *tap_name = NULL;

  vswitch_id = atoi (argv[2]);
  tap_name = argv[3];

  capture_if_create.vswitch_id = vswitch_id;
  snprintf (capture_if_create.tap_name, sizeof (capture_if_create.tap_name),
            "%s", tap_name);

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_CAPTURE_IF_CREATE,
                              &capture_if_create, sizeof (capture_if_create));
  rib_manager_send_message (msgp, shell);

  return 0;
}

CLI_COMMAND2 (delete_capture_if,
              "delete capture-if <0-3>",
              DELETE_HELP,
              "capture interface\n",
              "vswitch id\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_capture_if_delete capture_if_delete;
  struct internal_msg_header *msgp;
  int vswitch_id;

  vswitch_id = atoi (argv[2]);

  capture_if_delete.vswitch_id = vswitch_id;

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_CAPTURE_IF_DELETE,
                              &capture_if_delete, sizeof (capture_if_delete));
  rib_manager_send_message (msgp, shell);

  return 0;
}

CLI_COMMAND2 (show_capture_if,
              "show capture-if",
              SHOW_HELP,
              "show capture interface information\n")
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
               i, shell->NL);
      fprintf (shell->terminal, "  sockfd: %d, tap_ring_id: %u%s", cif->sockfd,
               cif->tap_ring_id, shell->NL);
      fprintf (shell->terminal, "  ring_up: %p, ring_dn: %p%s", cif->ring_up,
               cif->ring_dn, shell->NL);
    }

  return 0;
}

void
rib_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, show_rib);
  INSTALL_COMMAND2 (cmdset, set_vswitch);
  INSTALL_COMMAND2 (cmdset, delete_vswitch);
  INSTALL_COMMAND2 (cmdset, show_vswitch_rib);
  INSTALL_COMMAND2 (cmdset, set_vswitch_link);
  INSTALL_COMMAND2 (cmdset, delete_vswitch_link);
  INSTALL_COMMAND2 (cmdset, show_vswitch_link);
  INSTALL_COMMAND2 (cmdset, set_router_if);
  INSTALL_COMMAND2 (cmdset, delete_router_if);
  INSTALL_COMMAND2 (cmdset, show_router_if);
  INSTALL_COMMAND2 (cmdset, set_capture_if);
  INSTALL_COMMAND2 (cmdset, delete_capture_if);
  INSTALL_COMMAND2 (cmdset, show_capture_if);
}
