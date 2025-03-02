#include "include.h"

#include <lthread.h>

#include <rte_common.h>
#include <rte_launch.h>
#include <rte_ether.h>
#include <rte_malloc.h>

#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>
#include <zcmdsh/command_shell.h>

#include <zcmdsh/debug.h>
#include <zcmdsh/debug_cmd.h>
#include <zcmdsh/debug_log.h>
#include <zcmdsh/debug_category.h>
#include <zcmdsh/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "rib_manager.h"
#include "sdplane.h"
#include "thread_info.h"

#include "l2fwd_export.h"

#include "internal_message.h"

static __thread struct rib *rib = NULL;

#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

CLI_COMMAND2 (show_rib,
              "show rib",
              SHOW_HELP,
              "rib information\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib;
  int i, j;
  int nb_ports;
  rib = rcu_dereference (rcu_global_ptr_rib);

  if (! rib)
    return;

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
      return;
    }

  fprintf (shell->terminal, "rib_info: ver: %lu (%p)%s",
           rib->rib_info->ver, rib->rib_info, shell->NL);

  fprintf (shell->terminal, "rib_info: tapif_size: %d%s",
           rib->rib_info->tapif_size, shell->NL);
  for (i = 0; i < rib->rib_info->tapif_size; i++)
    {
      struct tapif_conf *tapconf;
      tapconf = &rib->rib_info->tapif[i];
      fprintf (shell->terminal, "rib_info: tapif[%d]: sockfd: %d%s",
               i, tapconf->sockfd, shell->NL);
    }

  fprintf (shell->terminal, "rib_info: vswitch_size: %d%s",
           rib->rib_info->vswitch_size, shell->NL);
  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vswitch;
      vswitch = &rib->rib_info->vswitch[i];
      fprintf (shell->terminal, "rib_info: vswitch[%d]: port_size: %d%s",
               i, vswitch->port_size, shell->NL);
      for (j = 0; j < vswitch->port_size; j++)
        {
          struct switch_port *port;
          port = &vswitch->port[j];
          fprintf (shell->terminal,
                   "rib_info: vswitch[%d]: port[%d]: type: %d portid: %d%s",
                   i, j, port->type, port->portid, shell->NL);
        }
    }

  fprintf (shell->terminal, "rib_info: port_size: %d%s",
           rib->rib_info->port_size, shell->NL);
  for (i = 0; i < rib->rib_info->port_size; i++)
    {
      struct port_conf *port;
      port = &rib->rib_info->port[i];
      fprintf (shell->terminal, "rib_info: port[%d]: "
             "nb_rxd: %hu nb_txd: %hu%s",
             i, port->nb_rxd, port->nb_txd, shell->NL);
      fprintf (shell->terminal, "rib_info: port[%d]: "
             "link: speed: %lu duplex: %d autoneg: %d status: %d%s",
             i, port->link.link_speed, port->link.link_duplex,
             port->link.link_autoneg, port->link.link_status,
             shell->NL);
      fprintf (shell->terminal, "rib_info: port[%d]: nrxq: %d ntxq: %d%s",
               i, port->dev_info.nb_rx_queues, port->dev_info.nb_tx_queues,
               shell->NL);
    }

  fprintf (shell->terminal, "rib_info: lcore_size: %d%s",
           rib->rib_info->lcore_size, shell->NL);
  for (i = 0; i < rib->rib_info->lcore_size; i++)
    {
      struct lcore_qconf *qconf;
      qconf = &rib->rib_info->lcore_qconf[i];
      fprintf (shell->terminal, "rib_info: lcore[%d]: nrxq: %d%s",
               i, qconf->nrxq, shell->NL);
      for (j = 0; j < qconf->nrxq; j++)
        {
          struct port_queue_conf *rx_queue;
          rx_queue = &qconf->rx_queue_list[j];
          fprintf (shell->terminal, "rib_info: lcore[%d]: rxq[%d]: "
                   "port_id: %d queue_id: %d%s",
                   i, j, rx_queue->port_id, rx_queue->queue_id,
                   shell->NL);
        }
    }
}

