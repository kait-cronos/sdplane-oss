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
}

