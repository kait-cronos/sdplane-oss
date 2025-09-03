/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#include "include.h"

#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>

#include "sdplane.h"
// #include "tap_handler.h"

#ifdef ENABLE_PKTGEN

#include "../module/pktgen/app/pktgen.h"

// clang-format off

extern pktgen_t pktgen;

CLI_COMMAND2 (show_pktgen,
              "show pktgen",
              SHOW_HELP,
              "pktgen information.\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  int lcore_id;
  char *type;
  int running;

  if (! pktgen.l2p)
    {
      fprintf (t, "pktgen not configured.%s", shell->NL);
      return 0;
    }

  for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++)
    {
      if (! rte_lcore_is_enabled (lcore_id))
        continue;

      switch (get_type (pktgen.l2p, lcore_id))
        {
        case RX_TYPE:
          type = "rx";
          break;
        case TX_TYPE:
          type = "tx";
          break;
        case (RX_TYPE | TX_TYPE):
          type = "rx/tx";
          break;
        default:
          type = "none";
          break;
        }

      running = pg_lcore_is_running (pktgen.l2p, lcore_id);
      fprintf (t, "lcore[%d]: pktgen type: %s running: %d%s", lcore_id, type,
               running, shell->NL);
    }

  l2p_t *l2p = pktgen.l2p;

  uint16_t i, j;
  lobj_t *lobj; // lcore obj
  pobj_t *pobj; // port obj
  const char *lcore_type_str[] = { "unknown", "rx", "tx", "rxtx", NULL };

  for (i = 0; i < RTE_MAX_LCORE; i++)
    {
      if (! rte_lcore_is_enabled (i))
        continue;

      lobj = &l2p->lcores[i];
      fprintf (t, "lcore[%d]: type: %s private: %p%s",
               lobj->lid, lcore_type_str[lobj->type], lobj->private,
               shell->NL);

      for (j = 0; j < lobj->pids.rx_cnt; j++)
        fprintf (t, "    pids.rx[%d/%d]: port %d%s",
                 j, lobj->pids.rx_cnt, lobj->pids.rx[j], shell->NL);
      for (j = 0; j < lobj->qids.rx_cnt; j++)
        fprintf (t, "    qids.rx[%d/%d]: queue %d%s",
                 j, lobj->qids.rx_cnt, lobj->qids.rx[j], shell->NL);

      for (j = 0; j < lobj->pids.tx_cnt; j++)
        fprintf (t, "    pids.tx[%d/%d]: port %d%s",
                 j, lobj->pids.tx_cnt, lobj->pids.tx[j], shell->NL);
      for (j = 0; j < lobj->qids.tx_cnt; j++)
        fprintf (t, "    qids.tx[%d/%d]: queue %d%s",
                 j, lobj->qids.tx_cnt, lobj->qids.tx[j], shell->NL);
    }

  for (i = 0; i < RTE_MAX_ETHPORTS; i++)
    {
      pobj = &l2p->ports[i];
      if (pobj->nb_lids)
        fprintf (t, "port[%d]: rx_qids: %d tx_qids: %d private: %p%s",
                 pobj->pid, pobj->rx_qid, pobj->tx_qid, pobj->private,
                 shell->NL);
      for (j = 0; j < pobj->nb_lids; j++)
        fprintf (t, "    lcore_index[%d/%d]: %d%s",
                 j, pobj->nb_lids, pobj->lids[j], shell->NL);
    }

  return 0;
}

int _pktgen_main_init (int argc, char **argv);

CLI_COMMAND2 (pktgen_init,
              "pktgen init argv-list <0-7>",
              "pktgen\n",
              "init\n",
              "specify argv-list\n",
              "specify argv-list number\n")
{
  struct shell *shell = (struct shell *) context;

  int index;
  index = strtol (argv[3], NULL, 0);

  int *argcp = &argv_list_argc[index];
  char **argvp = argv_list[index];

  _pktgen_main_init (*argcp, argvp);
  return 0;
}

int start_stop_cmd(int argc, char **argv);

CLI_COMMAND2 (pktgen_do_start_stop,
              "pktgen do (start|stop) port (<0-7>|all)",
              "pktgen\n",
              "pktgen do\n",
              "pktgen start cmd\n",
              "pktgen stop cmd\n",
              "specify port.\n",
              "specify port.\n",
              "specify for all ports.\n"
             )
{
  struct shell *shell = (struct shell *) context;
  int ret;
  int pktgen_argc;
  char *pktgen_argv[16];
  pktgen_argv[0] = argv[2];
  pktgen_argv[1] = argv[4];
  pktgen_argc = 2;
  ret = start_stop_cmd (pktgen_argc, pktgen_argv);
  if (ret < 0)
    return CMD_FAILURE;
  return CMD_SUCCESS;
}

int set_cmd(int argc, char **argv);

CLI_COMMAND2 (pktgen_do_set_count,
              "pktgen do set port (<0-7>|all) count <0-4000000000>",
              "pktgen\n",
              "pktgen do\n",
              "pktgen set cmd\n",
              "specify port.\n",
              "specify port.\n",
              "specify for all ports.\n",
              "set packet count.\n",
              "set packet count.\n"
             )
{
  struct shell *shell = (struct shell *) context;
  int ret;
  int pktgen_argc;
  char *pktgen_argv[16];
  pktgen_argv[0] = "set";
  pktgen_argv[1] = argv[4];
  pktgen_argv[2] = "count";
  pktgen_argv[3] = argv[6];
  pktgen_argc = 4;
  ret = set_cmd (pktgen_argc, pktgen_argv);
  if (ret < 0)
    return CMD_FAILURE;
  return CMD_SUCCESS;
}

CLI_COMMAND2 (pktgen_do_set_size,
              "pktgen do set port (<0-7>|all) size <0-9999>",
              "pktgen\n",
              "pktgen do\n",
              "pktgen set cmd\n",
              "specify port.\n",
              "specify port.\n",
              "specify for all ports.\n",
              "set packet size.\n",
              "set packet size.\n"
             )
{
  struct shell *shell = (struct shell *) context;
  int ret;
  int pktgen_argc;
  char *pktgen_argv[16];
  pktgen_argv[0] = "set";
  pktgen_argv[1] = argv[4];
  pktgen_argv[2] = "size";
  pktgen_argv[3] = argv[6];
  pktgen_argc = 4;
  ret = set_cmd (pktgen_argc, pktgen_argv);
  if (ret < 0)
    return CMD_FAILURE;
  return CMD_SUCCESS;
}

CLI_COMMAND2 (pktgen_do_set_rate,
              "pktgen do set port (<0-7>|all) rate <0-100>",
              "pktgen\n",
              "pktgen do\n",
              "pktgen set cmd\n",
              "specify port.\n",
              "specify port.\n",
              "specify for all ports.\n",
              "set packet rate.\n",
              "set packet rate in percentage.\n"
             )
{
  struct shell *shell = (struct shell *) context;
  int ret;
  int pktgen_argc;
  char *pktgen_argv[16];
  pktgen_argv[0] = "set";
  pktgen_argv[1] = argv[4];
  pktgen_argv[2] = "rate";
  pktgen_argv[3] = argv[6];
  pktgen_argc = 4;
  ret = set_cmd (pktgen_argc, pktgen_argv);
  if (ret < 0)
    return CMD_FAILURE;
  return CMD_SUCCESS;
}

CLI_COMMAND2 (pktgen_do_set_tcp_port,
              "pktgen do set port (<0-7>|all) "
              "tcp (source-port|destination-port) <0-65535>",
              "pktgen\n",
              "pktgen do\n",
              "pktgen set cmd\n",
              "specify port.\n",
              "specify port.\n",
              "specify for all ports.\n",
              "set packet proto tcp.\n",
              "set packet tcp source-port.\n",
              "set packet tcp destination-port.\n",
              "specify the packet tcp port number.\n"
             )
{
  struct shell *shell = (struct shell *) context;
  int ret;
  int pktgen_argc;
  char *pktgen_argv[16];
  pktgen_argv[0] = "set";
  pktgen_argv[1] = argv[4];
  if (! strcmp (argv[6], "source-port"))
    pktgen_argv[2] = "sport";
  else
    pktgen_argv[2] = "dport";
  pktgen_argv[3] = argv[7];
  pktgen_argc = 4;
  ret = set_cmd (pktgen_argc, pktgen_argv);
  if (ret < 0)
    return CMD_FAILURE;
  return CMD_SUCCESS;
}

CLI_COMMAND2 (pktgen_do_set_ttl,
              "pktgen do set port (<0-7>|all) "
              "ttl <0-255>",
              "pktgen\n",
              "pktgen do\n",
              "pktgen set cmd\n",
              "specify port.\n",
              "specify port.\n",
              "specify for all ports.\n",
              "set packet ttl.\n",
              "specify the packet ttl number.\n"
             )
{
  struct shell *shell = (struct shell *) context;
  int ret;
  int pktgen_argc;
  char *pktgen_argv[16];
  pktgen_argv[0] = "set";
  pktgen_argv[1] = argv[4];
  pktgen_argv[2] = "ttl";
  pktgen_argv[3] = argv[6];
  pktgen_argc = 4;
  ret = set_cmd (pktgen_argc, pktgen_argv);
  if (ret < 0)
    return CMD_FAILURE;
  return CMD_SUCCESS;
}

CLI_COMMAND2 (pktgen_do_set_mac_addr,
              "pktgen do set port (<0-7>|all) "
              "mac (source|destination) <WORD>",
              "pktgen\n",
              "pktgen do\n",
              "pktgen set cmd\n",
              "specify port.\n",
              "specify port.\n",
              "specify for all ports.\n",
              "set packet mac address.\n",
              "set packet mac source address.\n",
              "set packet mac destination address.\n",
              "specify the packet mac address: e.g., aa:bb:cc:dd:ee:ff.\n"
             )
{
  struct shell *shell = (struct shell *) context;
  int ret;
  int pktgen_argc;
  char *pktgen_argv[16];
  struct rte_ether_addr ether_addr;
  ret = rte_ether_unformat_addr (argv[7], &ether_addr);
  if (ret < 0)
    {
      fprintf (shell->terminal, "invalid address: %s%s",
               argv[7], shell->NL);
      return CMD_FAILURE;
    }
  pktgen_argv[0] = "set";
  pktgen_argv[1] = argv[4];
  if (! strcmp (argv[6], "source"))
    pktgen_argv[2] = "src";
  else
    pktgen_argv[2] = "dst";
  pktgen_argv[3] = "mac";
  pktgen_argv[4] = argv[7];
  pktgen_argc = 5;
  ret = set_cmd (pktgen_argc, pktgen_argv);
  if (ret < 0)
    return CMD_FAILURE;
  return CMD_SUCCESS;
}

CLI_COMMAND2 (pktgen_do_set_ipv4_addr,
              "pktgen do set port (<0-7>|all) "
              "(ip|ipv4) (source|destination) A.B.C.D",
              "pktgen\n",
              "pktgen do\n",
              "pktgen set cmd\n",
              "specify port.\n",
              "specify port.\n",
              "specify for all ports.\n",
              "set packet IPv4 address.\n",
              "set packet IPv4 address.\n",
              "set packet IPv4 source address.\n",
              "set packet IPv4 destination address.\n",
              "specify the packet ipv4 address: e.g., 10.0.0.1.\n"
             )
{
  struct shell *shell = (struct shell *) context;
  int ret;
  int pktgen_argc;
  char *pktgen_argv[16];
  struct in_addr ipv4_addr;
  ret = inet_pton (AF_INET, argv[7], &ipv4_addr);
  if (ret != 1)
    {
      fprintf (shell->terminal, "invalid address: %s%s",
               argv[7], shell->NL);
      return CMD_FAILURE;
    }
  pktgen_argv[0] = "set";
  pktgen_argv[1] = argv[4];
  if (! strcmp (argv[6], "source"))
    pktgen_argv[2] = "src";
  else
    pktgen_argv[2] = "dst";
  pktgen_argv[3] = "ip";
  pktgen_argv[4] = argv[7];
  pktgen_argc = 5;
  ret = set_cmd (pktgen_argc, pktgen_argv);
  if (ret < 0)
    return CMD_FAILURE;
  return CMD_SUCCESS;
}

void
pktgen_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, show_pktgen);
  INSTALL_COMMAND2 (cmdset, pktgen_init);
  INSTALL_COMMAND2 (cmdset, pktgen_do_start_stop);
  INSTALL_COMMAND2 (cmdset, pktgen_do_set_count);
  INSTALL_COMMAND2 (cmdset, pktgen_do_set_size);
  INSTALL_COMMAND2 (cmdset, pktgen_do_set_rate);
  INSTALL_COMMAND2 (cmdset, pktgen_do_set_tcp_port);
  INSTALL_COMMAND2 (cmdset, pktgen_do_set_ttl);
  INSTALL_COMMAND2 (cmdset, pktgen_do_set_mac_addr);
  INSTALL_COMMAND2 (cmdset, pktgen_do_set_ipv4_addr);
}
#endif /*ENABLE_PKTGEN*/
