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

#include <sdplane/debug.h>
#include <sdplane/termio.h>
#include <sdplane/vector.h>
#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>
#include <sdplane/log_cmd.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_zcmdsh.h>

#include "debug_sdplane.h"

#include "rib_manager.h"
#include "queue_config.h"

#include "internal_message.h"

#include "sdplane.h"

struct sdplane_queue_conf thread_qconf[RTE_MAX_LCORE];

struct port_queue_conf *
qconf_diff (struct sdplane_queue_conf *new, struct sdplane_queue_conf *old)
{
  int i, j;
  struct port_queue_conf *p, *q;
  for (i = 0; i < new->nrxq; i++)
    {
      p = &new->rx_queue_list[i];
      for (j = 0; j < old->nrxq; j++)
        {
          q = &old->rx_queue_list[j];
          if (p->port_id == q->port_id && p->queue_id == q->queue_id)
            return p;
        }
    }
  return NULL;
}

#if 0
CLI_COMMAND2 (update_port_link_status, "update port link-status",
              "update information\n", PORT_HELP, "link-status information\n")
{
  struct shell *shell = (struct shell *) context;

  void *msgp;
  struct internal_msg_eth_link *msg_eth_link;

  uint16_t nb_ports;
  int i;

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_ETH_LINK, NULL,
                              sizeof (struct internal_msg_eth_link));
  msg_eth_link = (struct internal_msg_eth_link *) internal_msg_body (msgp);

  nb_ports = rte_eth_dev_count_avail ();
  for (i = 0; i < nb_ports; i++)
    {
      rte_eth_link_get_nowait (i, &msg_eth_link->link[i]);
    }

  internal_msg_send_to (msg_queue_rib, msgp, shell);
}
#endif

CLI_COMMAND2 (update_port_status, "update port status", "update information\n",
              PORT_HELP, "port status information\n")
{
  struct shell *shell = (struct shell *) context;
  void *msgp;
  msgp = internal_msg_create (INTERNAL_MSG_TYPE_PORT_STATUS, NULL, 0);
  internal_msg_send_to (msg_queue_rib, msgp, shell);
}

CLI_COMMAND2 (set_thread_lcore_port_queue,
              "set thread <0-128> port <0-128> queue <0-128>", SET_HELP,
              "thread-information\n", "thread-id (lcore-id)\n", PORT_HELP,
              PORT_NUMBER_HELP, QUEUE_HELP, QUEUE_NUMBER_HELP)
{
  struct shell *shell = (struct shell *) context;
  uint16_t lcore_id, port_id, queue_id;
  lcore_id = (uint16_t) strtol (argv[2], NULL, 0);
  port_id = (uint16_t) strtol (argv[4], NULL, 0);
  queue_id = (uint16_t) strtol (argv[6], NULL, 0);
  fprintf (shell->terminal, "lcore: %d port: %d rx-queue: %d%s", lcore_id,
           port_id, queue_id, shell->NL);

  /* search existing */
  int i, j;
  struct sdplane_queue_conf *lcore_conf;
  struct port_queue_conf *port_qconf, *match;

  for (i = 0; i < RTE_MAX_LCORE; i++)
    {
      lcore_conf = &thread_qconf[i];
      for (j = 0; j < lcore_conf->nrxq; j++)
        {
          port_qconf = &lcore_conf->rx_queue_list[j];
          if (port_qconf->port_id == port_id &&
              port_qconf->queue_id == queue_id)
            {
              if (i != lcore_id)
                {
                  int num;
                  num = lcore_conf->nrxq - (j + 1);
                  if (num)
                    memmove (port_qconf, port_qconf + 1,
                             sizeof (struct port_queue_conf) * num);
                  lcore_conf->nrxq--;
                }
            }
        }
    }

  match = NULL;
  lcore_conf = &thread_qconf[lcore_id];
  for (j = 0; j < lcore_conf->nrxq; j++)
    {
      port_qconf = &lcore_conf->rx_queue_list[j];

      if (port_qconf->port_id == port_id && port_qconf->queue_id == queue_id)
        {
          match = port_qconf;
        }
    }

  if (! match)
    {
      j = lcore_conf->nrxq;
      if (j < MAX_RX_QUEUE_PER_LCORE)
        {
          port_qconf = &lcore_conf->rx_queue_list[j];
          port_qconf->port_id = port_id;
          port_qconf->queue_id = queue_id;
          lcore_conf->nrxq++;
        }
    }

  struct sdplane_queue_conf *qconf;
  for (i = 0; i < RTE_MAX_LCORE; i++)
    {
      qconf = &thread_qconf[i];
      for (j = 0; j < qconf->nrxq; j++)
        {
          fprintf (
              shell->terminal,
              "thread_qconf[%d]: lcore: %d rxq[%d/%d]: port: %d queue: %d%s",
              i, i, j, qconf->nrxq, qconf->rx_queue_list[j].port_id,
              qconf->rx_queue_list[j].queue_id, shell->NL);
        }
    }

  void *msgp;
  struct internal_msg_qconf *msg_qconf;
  char dummy[8];
  int ret;

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_QCONF, thread_qconf,
                              sizeof (thread_qconf));
  ret = internal_msg_send_to (msg_queue_rib, msgp, shell);
  if (ret < 0)
    {
      DEBUG_SDPLANE_LOG (RIB, "imsg fail.");
      return -1;
    }
  return 0;
}

CLI_COMMAND2 (show_thread_qconf, "show thread qconf", SHOW_HELP,
              "thread information.\n", "queue configuration.\n")
{
  struct shell *shell = (struct shell *) context;
  int i, j;
  struct sdplane_queue_conf *qconf;

  struct rib *rib = rib_tlocal;

  if (rib && rib->rib_info)
    {
      for (i = 0; i < rib->rib_info->lcore_size; i++)
        {
          int nrxq;
          nrxq = rib->rib_info->lcore_qconf[i].nrxq;
          for (j = 0; j < nrxq; j++)
            {
              struct port_queue_conf *rxq;
              rxq = &rib->rib_info->lcore_qconf[i].rx_queue_list[j];
              fprintf (
                  shell->terminal,
                  "rib->rib_info: lcore: %d rxq[%d/%d]: port: %d queue: %d%s",
                  i, j, nrxq, rxq->port_id, rxq->queue_id, shell->NL);
            }
        }
    }
  else
    {
      for (i = 0; i < RTE_MAX_LCORE; i++)
        {
          qconf = &thread_qconf[i];
          for (j = 0; j < qconf->nrxq; j++)
            {
              fprintf (
                  shell->terminal,
                  "thread_qconf[%d]: lcore: %d rxq[%d/%d]: port: %d queue: %d%s",
                  i, i, j, qconf->nrxq, qconf->rx_queue_list[j].port_id,
                  qconf->rx_queue_list[j].queue_id, shell->NL);
            }
        }
    }

  return 0;
}

void
queue_config_cmd_init (struct command_set *cmdset)
{
  //INSTALL_COMMAND2 (cmdset, update_port_link_status);
  INSTALL_COMMAND2 (cmdset, update_port_status);
  INSTALL_COMMAND2 (cmdset, set_thread_lcore_port_queue);
  INSTALL_COMMAND2 (cmdset, show_thread_qconf);
}
