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

#include "l3fwd.h"
#include "l2fwd_export.h"

#include "sdplane.h"
#include "tap_handler.h"
#include "l2_repeater.h"

#include "thread_info.h"

volatile bool force_stop[RTE_MAX_LCORE];

struct lcore_worker lcore_workers[RTE_MAX_LCORE];

extern int lthread_core;

int nettlp_thread (void *arg);

void
start_lcore (struct shell *shell, int lcore_id)
{
  fprintf (shell->terminal, "starting worker on lcore: %d\n", lcore_id);
  if (lcore_workers[lcore_id].func == NULL)
    {
      fprintf (shell->terminal, "can't start a null worker on lcore: %d\n",
               lcore_id);
      return;
    }
  force_stop[lcore_id] = false;
  if (lcore_id == lthread_core)
    {
      fprintf (shell->terminal, "skip for lthread lcore: %d\n", lcore_id);
      return;
    }

  thread_register (lcore_id, NULL, (lthread_func) lcore_workers[lcore_id].func,
                   lcore_workers[lcore_id].func_name, NULL);

  rte_eal_remote_launch (lcore_workers[lcore_id].func,
                         lcore_workers[lcore_id].arg, lcore_id);
  DEBUG_SDPLANE_LOG (THREAD, "started worker on lcore: %d.", lcore_id);
  fprintf (shell->terminal, "started worker on lcore: %d\n", lcore_id);
}

void
stop_lcore (struct shell *shell, int lcore_id)
{
  fprintf (shell->terminal, "stopping worker on lcore: %d\n", lcore_id);
  DEBUG_SDPLANE_LOG (THREAD, "stopping worker on lcore: %d.", lcore_id);
  force_stop[lcore_id] = true;

  if (lcore_id == rte_lcore_id ())
    {
      fprintf (shell->terminal, "can't stop lthread lcore: %d\n", lcore_id);
    }
  else
    {
      rte_eal_wait_lcore (lcore_id);
      fprintf (shell->terminal, "stopped worker on lcore: %d\n", lcore_id);
    }
}

CLI_COMMAND2 (set_worker,
              "(set|reset|start|restart) worker lcore <0-16> "
              "(|none|l2fwd|l3fwd|l3fwd-lpm|tap-handler|l2-repeater|nettlp-thread)",
              SET_HELP, RESET_HELP, START_HELP, RESTART_HELP, WORKER_HELP,
              LCORE_HELP, LCORE_NUMBER_HELP,
              "set lcore not to launch anything\n",
              "set lcore to launch l2fwd\n",
              "set lcore to launch l3fwd (default: lpm)\n",
              "set lcore to launch l3fwd-lpm\n",
              "set lcore to launch tap-handler\n",
              "set lcore to launch l2-repeater\n",
              "set lcore to launch nettlp-thread\n"
              )
{
  struct shell *shell = (struct shell *) context;
  int lcore_id;
  lcore_id = strtol (argv[3], NULL, 0);
  lcore_function_t *func;
  void *arg = NULL;

  if (argc == 4)
    func = lcore_workers[lcore_id].func;
  else if (! strcmp (argv[4], "none"))
    func = NULL;
  else if (! strcmp (argv[4], "l2fwd"))
    func = l2fwd_launch_one_lcore;
  else if (! strcmp (argv[4], "tap-handler"))
    func = tap_handler;
  else if (! strcmp (argv[4], "l2-repeater"))
    func = l2_repeater;
  else if (! strcmp (argv[4], "nettlp-thread"))
    func = nettlp_thread;
  else /* if (! strcmp (argv[4], "l3fwd")) */
    func = lpm_main_loop;

  if (lcore_workers[lcore_id].func == lthread_main)
    {
      fprintf (shell->terminal, "cannot override lthread: lcore[%d].\n",
               lcore_id);
      return;
    }

  char *func_name;
  if (func == lpm_main_loop)
    func_name = "l3fwd-lpm";
  else if (func == l2fwd_launch_one_lcore)
    func_name = "l2fwd";
  else if (func == lthread_main)
    func_name = "lthread_main";
  else if (func == tap_handler)
    func_name = "tap-handler";
  else if (func == l2_repeater)
    func_name = "l2-repeater";
  else if (func == nettlp_thread)
    func_name = "nettlp-thread";
  else
    func_name = "none";

  lcore_workers[lcore_id].func = func;
  lcore_workers[lcore_id].arg = arg;
  lcore_workers[lcore_id].func_name = func_name;

  fprintf (shell->terminal, "worker set to lcore[%d]: func: %s\n", lcore_id,
           func_name);

  if (! strcmp (argv[0], "reset") || ! strcmp (argv[0], "start") ||
      ! strcmp (argv[0], "restart"))
    {
      stop_lcore (shell, lcore_id);
      start_lcore (shell, lcore_id);
      fprintf (shell->terminal, "worker[%d]: restarted.\n", lcore_id);
    }
  else if (! strcmp (argv[0], "set") && argc == 4)
    fprintf (shell->terminal, "nothing changed.\n");
  else
    fprintf (shell->terminal,
             "workers need to be restarted for changes to take effect.\n");
}

CLI_COMMAND2 (start_stop_worker,
              "(start|stop|reset|restart) worker lcore (<0-16>|all)",
              START_HELP, STOP_HELP, RESET_HELP, RESTART_HELP, WORKER_HELP,
              LCORE_HELP, LCORE_NUMBER_HELP, LCORE_ALL_HELP)
{
  struct shell *shell = (struct shell *) context;
  uint32_t nb_lcores;
  unsigned int lcore_id;
  unsigned int lcore_spec = -1;

  if (! strcmp (argv[3], "all"))
    {
      if (! strcmp (argv[0], "stop"))
        force_quit = true;
      else
        force_quit = false;
    }
  else
    lcore_spec = strtol (argv[3], NULL, 0);

  nb_lcores = rte_lcore_count ();
  for (lcore_id = 0; lcore_id < nb_lcores; lcore_id++)
    {
      if (lcore_spec != -1 && lcore_spec != lcore_id)
        continue;
      if (! strcmp (argv[0], "start"))
        start_lcore (shell, lcore_id);
      else if (! strcmp (argv[0], "stop"))
        stop_lcore (shell, lcore_id);
      else if (! strcmp (argv[0], "reset") || ! strcmp (argv[0], "restart"))
        {
          stop_lcore (shell, lcore_id);
          start_lcore (shell, lcore_id);
        }
    }
}

CLI_COMMAND2 (show_worker, "show worker", SHOW_HELP, WORKER_HELP)
{
  struct shell *shell = (struct shell *) context;
  unsigned int lcore_id;
  uint32_t nb_lcores;
  unsigned int main_lcore_id;
  char *state;
  char flags[16];
  char lcore_name[16];
  nb_lcores = rte_lcore_count ();
  main_lcore_id = rte_get_main_lcore ();
  fprintf (shell->terminal, "%-9s: %-12s %-8s %s%s", "lcore", "flags", "state",
           "func_name", shell->NL);
  for (lcore_id = 0; lcore_id < nb_lcores; lcore_id++)
    {
      snprintf (flags, sizeof (flags), "%s%s",
                (rte_lcore_is_enabled (lcore_id) ? "enabled" : "disabled"),
                (lcore_id == main_lcore_id ? ",main" : ""));
      state =
          (rte_eal_get_lcore_state (lcore_id) == RUNNING ? "running" : "wait");
      snprintf (lcore_name, sizeof (lcore_name), "lcore[%d]", lcore_id);
      fprintf (shell->terminal, "%-9s: %-12s %-8s %s%s", lcore_name, flags,
               state, lcore_workers[lcore_id].func_name, shell->NL);
    }
}

void
dpdk_lcore_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, set_worker);
  INSTALL_COMMAND2 (cmdset, start_stop_worker);
  INSTALL_COMMAND2 (cmdset, show_worker);
}
