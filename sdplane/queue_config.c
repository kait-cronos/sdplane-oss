#include "include.h"

#include <rte_ethdev.h>
#include <rte_bus_pci.h>

#include <zcmdsh/debug.h>
#include <zcmdsh/termio.h>
#include <zcmdsh/vector.h>
#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>
#include <zcmdsh/command_shell.h>
#include <zcmdsh/log_cmd.h>
#include <zcmdsh/debug_log.h>
#include <zcmdsh/debug_category.h>
#include <zcmdsh/debug_cmd.h>
#include <zcmdsh/debug_zcmdsh.h>

#include "debug_sdplane.h"

#include "queue_config.h"
#include "sdplane.h"

struct sdplane_queue_conf thread_qconf[RTE_MAX_LCORE];

CLI_COMMAND2 (set_thread_lcore_port_queue,
              "set thread <0-128> port <0-128> queue <0-128>",
              SET_HELP,
              "thread-information\n",
              "thread-id (lcore-id)\n",
              PORT_HELP,
              PORT_NUMBER_HELP,
              QUEUE_HELP,
              QUEUE_NUMBER_HELP)
{
  struct shell *shell = (struct shell *) context;
  uint16_t lcore_id, port_id, queue_id;
  lcore_id = (uint16_t) strtol (argv[2], NULL, 0);
  port_id = (uint16_t) strtol (argv[4], NULL, 0);
  queue_id = (uint16_t) strtol (argv[6], NULL, 0);
  fprintf (shell->terminal, "lcore: %d port: %d rx-queue: %d%s",
           lcore_id, port_id, queue_id, shell->NL);

  int i, j;
  for (i = 0; i < RTE_MAX_LCORE; i++)
    {
      if (thread_qconf[i].rx_queue_list[0].port_id == port_id)
        {
          if (i == lcore_id)
            {
              thread_qconf[i].rx_queue_list[0].port_id = port_id;
              thread_qconf[i].rx_queue_list[0].queue_id = 0;
              thread_qconf[i].nrxq = 1;
            }
          else
            {
              /* cancel */
              thread_qconf[i].rx_queue_list[0].port_id = 0;
              thread_qconf[i].rx_queue_list[0].queue_id = 0;
              thread_qconf[i].nrxq = 0;
            }
        }
      else if (i == lcore_id)
        {
          thread_qconf[i].rx_queue_list[0].port_id = port_id;
          thread_qconf[i].rx_queue_list[0].queue_id = queue_id;
          thread_qconf[i].nrxq = 1;
        }
    }

  struct sdplane_queue_conf *qconf;
  for (i = 0; i < RTE_MAX_LCORE; i++)
    {
      qconf = &thread_qconf[i];
      for (j = 0; j < qconf->nrxq; j++)
        {
          fprintf (shell->terminal,
                   "thread_qconf[%d]: rxq[%d/%d]: port: %d queue: %d%s",
                   i, j, qconf->nrxq,
                   qconf->rx_queue_list[j].port_id,
                   qconf->rx_queue_list[j].queue_id,
                   shell->NL);
        }
    }

  void *msgp;
  struct stream_msg_qconf *msg_qconf;
  msg_qconf = (struct stream_msg_qconf *)
    malloc (sizeof (struct stream_msg_qconf));
  memcpy (msg_qconf->qconf, thread_qconf,
          sizeof (struct sdplane_queue_conf) * RTE_MAX_LCORE);
  msgp = msg_qconf;

  if (msg_queue_rib)
    {
      DEBUG_SDPLANE_LOG (RIB, "%s: sending message %p.", __func__, msgp);
      rte_ring_enqueue (msg_queue_rib, msgp);
    }
  else
    {
      fprintf (shell->terminal, "can't send message to rib: queue: NULL.%s",
               shell->NL);
    }
}

CLI_COMMAND2 (show_thread_qconf,
              "show thread-qconf",
              SHOW_HELP,
              "thread-qconf\n")
{
  struct shell *shell = (struct shell *) context;
  int i, j;
  struct sdplane_queue_conf *qconf;
  for (i = 0; i < RTE_MAX_LCORE; i++)
    {
      qconf = &thread_qconf[i];
      for (j = 0; j < qconf->nrxq; j++)
        {
          fprintf (shell->terminal,
                   "thread_qconf[%d]: rxq[%d]: port: %d queue: %d%s",
                   i, j,
                   qconf->rx_queue_list[j].port_id,
                   qconf->rx_queue_list[j].queue_id,
                   shell->NL);
        }
    }
}

void
queue_config_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, set_thread_lcore_port_queue);
  INSTALL_COMMAND2 (cmdset, show_thread_qconf);
}

