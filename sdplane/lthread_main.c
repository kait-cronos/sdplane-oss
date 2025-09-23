/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#include "include.h"

#include <rte_common.h>
#include <rte_vect.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_random.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_udp.h>
#include <rte_string_fns.h>
#include <rte_cpuflags.h>

#include <cmdline_parse.h>
#include <cmdline_parse_etheraddr.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <lthread.h>

#include <sdplane/debug.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include <sdplane/termio.h>
#include <sdplane/vector.h>
#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>

#include "l3fwd.h"
#include "l3fwd_event.h"
#include "l3fwd_route.h"
#include "l3fwd_cmd.h"

#include "l2fwd_export.h"
#include "l2fwd_cmd.h"

#include "sdplane.h"
#include "tap_handler.h"
#include "neigh_manager.h"

#include "vty_shell.h"
#include "thread_info.h"

#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

__thread struct rib *rib_tlocal = NULL;

static __thread uint64_t loop_counter = 0;

int lthread_core = 0;

int startup_config (__rte_unused void *dummy);
void console_shell (void *arg);
void vty_server (void *arg);

int stat_collector (__rte_unused void *dummy);
void rib_manager (void *arg);
void netlink_thread (void *arg);

CLI_COMMAND2 (set_worker_lthread_stat_collector,
              "set worker lthread stat-collector", SET_HELP, WORKER_HELP,
              "lthread information\n", "stat-collector\n")
{
  struct shell *shell = (struct shell *) context;
  lthread_t *lt = NULL;

  lthread_create (&lt, (lthread_func) stat_collector, NULL);
  thread_register (lthread_core, lt, (lthread_func) stat_collector,
                   "stat_collector", NULL);
  lthread_detach2 (lt);
  return 0;
}

CLI_COMMAND2 (set_worker_lthread_rib_manager, "set worker lthread rib-manager",
              SET_HELP, WORKER_HELP, "lthread information\n", "rib-manager\n")
{
  struct shell *shell = (struct shell *) context;
  lthread_t *lt = NULL;

  lthread_create (&lt, (lthread_func) rib_manager, NULL);
  thread_register (lthread_core, lt, (lthread_func) rib_manager, "rib_manager",
                   NULL);
  lthread_detach2 (lt);

  /* rib_manager is an important worker, and it needs to be
     started immediately.
     If it is a lthread, lthread_sleep() is necessary. */
  lthread_sleep (0);

  /* check whether the rib_manager is actually started. */
  if (! msg_queue_rib)
    {
      fprintf (shell->terminal, "Can't start rib_manager.%s", shell->NL);
      return CMD_FAILURE;
    }

  return CMD_SUCCESS;
}

CLI_COMMAND2 (set_worker_lthread_netlink_thread,
              "set worker lthread netlink-thread", SET_HELP, WORKER_HELP,
              "lthread information\n", "netlink-thread\n")
{
  struct shell *shell = (struct shell *) context;
  lthread_t *lt = NULL;

  lthread_create (&lt, (lthread_func) netlink_thread, NULL);
  thread_register (lthread_core, lt, (lthread_func) netlink_thread,
                   "netlink_thread", NULL);
  lthread_detach2 (lt);
  return 0;
}

CLI_COMMAND2 (set_worker_lthread_neigh_manager,
              "set worker lthread neigh-manager", SET_HELP, WORKER_HELP,
              "lthread information\n", "neigh_manager\n")
{
  struct shell *shell = (struct shell *) context;
  lthread_t *lt = NULL;

  lthread_create (&lt, (lthread_func) neigh_manager, NULL);
  thread_register (lthread_core, lt, (lthread_func) neigh_manager,
                   "neigh_manager", NULL);
  lthread_detach2 (lt);
  return 0;
}

void
lthread_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, set_worker_lthread_stat_collector);
  INSTALL_COMMAND2 (cmdset, set_worker_lthread_rib_manager);
  INSTALL_COMMAND2 (cmdset, set_worker_lthread_netlink_thread);
  INSTALL_COMMAND2 (cmdset, set_worker_lthread_neigh_manager);
}

int
lthread_main (__rte_unused void *dummy)
{
  lthread_t *lt = NULL;
  int ret;
  int lcore_id;
  int thread_id;

  /* timer set */
  // timer_init (60 * 60, "2024/12/31 23:59:59");
  //timer_init (0, NULL);

  /* initialize workers */
  for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++)
    {
      lcore_workers[lcore_id].func = NULL;
      lcore_workers[lcore_id].arg = NULL;
      lcore_workers[lcore_id].func_name = NULL;
    }

  lcore_id = rte_lcore_id ();
  if (lcore_id < 0)
    lcore_id = 0;
  lthread_core = lcore_id;
  lcore_workers[lcore_id].func = lthread_main;
  lcore_workers[lcore_id].func_name = "lthread_main";

  printf ("%s[%d]: %s: enter at core[%d].\n", __FILE__, __LINE__, __func__,
          lthread_core);

  thread_id = thread_lookup (lthread_main);
  if (thread_id < 0)
    thread_id =
        thread_register (lthread_core, lt, (lthread_func)lthread_main, "lthread_main", NULL);
  else
    thread_update (thread_id, lthread_core, lt, (lthread_func)lthread_main, "lthread_main",
                   NULL);
  thread_register_loop_counter (thread_id, &loop_counter);

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_register_thread ();
#endif /*HAVE_LIBURCU_QSBR*/

  /* library initialization. */
  debug_zcmdsh_cmd_init ();
  command_shell_init ();

  lthread_create (&lt, (lthread_func) vty_server, NULL);
  thread_register (lthread_core, lt, vty_server, "vty_server", NULL);
  lthread_detach2 (lt);

  ret = startup_config (NULL);
  if (ret < 0)
    {
      printf ("%s[%d]: %s: error in startup_config.\n", __FILE__, __LINE__,
              __func__);
      unlink (pid_path);
      exit (-1);
    }

  lthread_create (&lt, (lthread_func) console_shell, NULL);
  thread_id =
      thread_register (lthread_core, lt, console_shell, "console_shell", NULL);

  /* quietly wait/check for the application process quit status. */
  while (! force_quit && ! force_stop[lthread_core])
    {
      lthread_sleep (1000); // yield.
      loop_counter++;
    }

  /* join the console lthread. */
  //lthread_join_all ();
  ret = lthread_join (threads[thread_id].lthread, NULL, 0);
  switch (ret)
    {
    case 0:
      //DEBUG_SDPLANE_LOG (LTHREAD, "successfully joined.");
      printf ("%s[%d]: %s: lthread_join() on console_thread: %d: success.\n",
              __FILE__, __LINE__, __func__, ret);
      break;
    case -1:
      //DEBUG_SDPLANE_LOG (LTHREAD, "joined thread is canceled.");
      printf ("%s[%d]: %s: lthread_join() on console_thread: %d: canceled.\n",
              __FILE__, __LINE__, __func__, ret);
      break;
    case -2:
      //DEBUG_SDPLANE_LOG (LTHREAD, "join timeout.");
      printf ("%s[%d]: %s: lthread_join() on console_thread: %d: timeout.\n",
              __FILE__, __LINE__, __func__, ret);
      break;
    default:
      //DEBUG_SDPLANE_LOG (LTHREAD, "join ended in unknown status.");
      printf ("%s[%d]: %s: lthread_join() on console_thread: %d: timeout.\n",
              __FILE__, __LINE__, __func__, ret);
      break;
    }

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_unregister_thread ();
#endif /*HAVE_LIBURCU_QSBR*/
  return ret;
}
