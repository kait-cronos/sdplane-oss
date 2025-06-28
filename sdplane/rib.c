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

CLI_COMMAND2 (show_rib, "show rib", SHOW_HELP, "rib information\n")
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

CLI_COMMAND2 (set_vswitch, "set vswitch <1-4094>", 
              SET_HELP,
              "vswitch\n",
              "vlan_id\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_vswitch_create vswitch_create;
  struct internal_msg_header *msgp;
  
  if (argc != 3) {
    fprintf (shell->terminal, "usage: set vswitch create vlan_id%s", shell->NL);
    return 0;
  }
  
  uint16_t vlan_id = atoi (argv[2]);
  if (vlan_id == 0 || vlan_id > 4094) {
    fprintf (shell->terminal, "invalid vlan_id: %u (must be 1-4094)%s", vlan_id, shell->NL);
    return 0;
  }
  
  vswitch_create.vlan_id = vlan_id;
  msgp = internal_msg_create (INTERNAL_MSG_TYPE_VSWITCH_CREATE, &vswitch_create, sizeof (vswitch_create));
  rib_manager_send_message (msgp, shell);
  
  fprintf (shell->terminal, "create vswitch with vlan_id %u%s", vlan_id, shell->NL);
  return 0;
}

CLI_COMMAND2 (delete_vswitch, "delete vswitch <0-16>",
              DELETE_HELP,
              "vswitch\n",
              "vswitch_id\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_vswitch_delete vswitch_delete;
  struct internal_msg_header *msgp;
  
  if (argc != 3) {
    fprintf (shell->terminal, "usage: delete vswitch vswitch_id%s", shell->NL);
    return 0;
  }
  
  uint16_t vswitch_id = atoi (argv[2]);
  
  vswitch_delete.vswitch_id = vswitch_id;
  msgp = internal_msg_create (INTERNAL_MSG_TYPE_VSWITCH_DELETE, &vswitch_delete, sizeof (vswitch_delete));
  rib_manager_send_message (msgp, shell);
  
  fprintf (shell->terminal, "delete vswitch_id %u%s", vswitch_id, shell->NL);
  return 0;
}

CLI_COMMAND2 (show_vswitch_rib, "show vswitch_rib",
              SHOW_HELP,
              "show vswitch_rib information\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib;
  int i, j;
  
  rib = rcu_dereference (rcu_global_ptr_rib);
  if (! rib || ! rib->rib_info) {
    fprintf (shell->terminal, "no rib information available%s", shell->NL);
    return 0;
  }
  
  fprintf (shell->terminal, "vswitch configurations:%s", shell->NL);
  fprintf (shell->terminal, "total vswitches: %d%s", rib->rib_info->vswitch_size, shell->NL);
  
  for (i = 0; i < rib->rib_info->vswitch_size; i++) {
    struct vswitch_conf *vswitch = &rib->rib_info->vswitch[i];
    fprintf (shell->terminal, "vswitch[%d]: vlan %u, ports: %u%s", 
             vswitch->vswitch_id, vswitch->vlan_id, vswitch->vswitch_port_size, shell->NL);
    
    for (j = 0; j < vswitch->vswitch_port_size; j++) {
      uint16_t link_id = vswitch->vswitch_link_id[j];
      struct vswitch_link *link = &rib->rib_info->vswitch_link[link_id];
      fprintf (shell->terminal, "  port[%d]: link %u -> port %u (tag:%u)%s",
               j, link_id, link->port_id, link->tag_id, shell->NL);
    }
  }
  
  return 0;
}

CLI_COMMAND2 (set_vswitch_link, "set vswitch-link <0-16> <0-16> <0-4094>",
              SET_HELP,
              "vswitch\n",
              "vswitch_id\n"
              "dpdk_port_id\n",
              "tag_id (0: native, 1-4094: tagged\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_vswitch_link_create vswitch_link_create;
  struct internal_msg_header *msgp;
  
  if (argc < 5) {
    fprintf (shell->terminal, "usage: set vswitch-link vswitch_id port_id tag_id%s", shell->NL);
    return 0;
  }
  
  uint16_t vswitch_id = atoi (argv[2]);
  uint16_t port_id = atoi (argv[3]);
  uint16_t tag_id = atoi (argv[4]);
  
  vswitch_link_create.vswitch_id = vswitch_id;
  vswitch_link_create.port_id = port_id;
  vswitch_link_create.tag_id = tag_id;
  
  msgp = internal_msg_create (INTERNAL_MSG_TYPE_VSWITCH_LINK_CREATE, &vswitch_link_create, sizeof (vswitch_link_create));
  rib_manager_send_message (msgp, shell);
  
  fprintf (shell->terminal, "create vswitch link: vswitch %u -> port %u tag:%u%s",
           vswitch_id, port_id, tag_id, shell->NL);
  return 0;
}

CLI_COMMAND2 (delete_vswitch_link, "delete vswitch-link <0-32>",
              DELETE_HELP,
              "vswitch-link\n",
              "vswitch_link_id\n")
{
  struct shell *shell = (struct shell *) context;
  struct internal_msg_vswitch_link_delete vswitch_link_delete;
  struct internal_msg_header *msgp;
  
  if (argc != 3) {
    fprintf (shell->terminal, "usage: delete vswitch-link vswitch_link_id%s", shell->NL);
    return 0;
  }
  
  uint16_t vswitch_link_id = atoi (argv[2]);
  
  vswitch_link_delete.vswitch_link_id = vswitch_link_id;
  msgp = internal_msg_create (INTERNAL_MSG_TYPE_VSWITCH_LINK_DELETE, &vswitch_link_delete, sizeof (vswitch_link_delete));
  rib_manager_send_message (msgp, shell);
  
  fprintf (shell->terminal, "delete vswitch_link_id %u%s", vswitch_link_id, shell->NL);
  return 0;
}

CLI_COMMAND2 (show_vswitch_link, "show vswitch-link",
              SHOW_HELP,
              "show vswitch link information\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib;
  int i;
  
  rib = rcu_dereference (rcu_global_ptr_rib);
  if (! rib || ! rib->rib_info) {
    fprintf (shell->terminal, "no rib information available%s", shell->NL);
    return 0;
  }
  
  fprintf (shell->terminal, "vswitch link configurations:%s", shell->NL);
  fprintf (shell->terminal, "total vswitch links: %d%s", rib->rib_info->vswitch_link_size, shell->NL);
  
  for (i = 0; i < rib->rib_info->vswitch_link_size; i++) {
    struct vswitch_link *link = &rib->rib_info->vswitch_link[i];
    fprintf (shell->terminal, "link[%d]: port %u -> vswitch %u (vlan:%u, tag:%u, vswport:%u)%s",
             link->vswitch_link_id, link->port_id, link->vswitch_id, 
             link->vlan_id, link->tag_id, link->vswitch_port, shell->NL);
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
}
