#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <string.h>
#include <sys/queue.h>
#include <stdarg.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>

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

#include "l3fwd.h"
#include "l3fwd_event.h"
#include "l3fwd_route.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <lthread.h>

#include "debug.h"
#include "termio.h"
#include "vector.h"
#include "shell.h"
#include "command.h"
#include "command_shell.h"
#include "debug_cmd.h"
//#include "shell_fselect.h"


int lthread_main (__rte_unused void *dummy);

extern uint32_t l2fwd_dst_ports[RTE_MAX_ETHPORTS];
int l2fwd_launch_one_lcore (__rte_unused void *dummy);

struct lcore_worker
{
  lcore_function_t *func;
  void *arg;
  char *func_name;
};
struct lcore_worker lcore_workers[RTE_MAX_LCORE];

void
start_lcore (struct shell *shell, int lcore_id)
{
  fprintf (shell->terminal, "starting worker on lcore: %d\n", lcore_id);
  if (lcore_workers[lcore_id].func == NULL)
    {
      fprintf (shell->terminal,
               "can't start a null worker on lcore: %d\n", lcore_id);
      return;
    }
  force_stop[lcore_id] = false;
  rte_eal_remote_launch (lcore_workers[lcore_id].func,
                         lcore_workers[lcore_id].arg, lcore_id);
  fprintf (shell->terminal, "started worker on lcore: %d\n", lcore_id);
}

void
stop_lcore (struct shell *shell, int lcore_id)
{
  if (lcore_id == rte_lcore_id ())
    {
      fprintf (shell->terminal, "can't stop lthread lcore: %d\n", lcore_id);
      return;
    }
  fprintf (shell->terminal, "stopping worker on lcore: %d\n", lcore_id);
  force_stop[lcore_id] = true;
  rte_eal_wait_lcore (lcore_id);
  fprintf (shell->terminal, "stopped worker on lcore: %d\n", lcore_id);
}

//#define SHOW_HELP "show information\n"
#define SET_HELP "set information\n"
#define RESET_HELP "reset information\n"
#define START_HELP "start information\n"
#define STOP_HELP "stop information\n"
#define RESTART_HELP "restart information\n"
#define WORKER_HELP "worker information\n"
#define LCORE_HELP "lcore information\n"
#define LCORE_NUMBER_HELP "specify lcore number\n"

DEFINE_COMMAND (set_worker,
                "(set|reset|restart) worker lcore <0-16> "
                "(|none|l2fwd|l3fwd|l3fwd-lpm)",
                SET_HELP
                RESET_HELP
                RESTART_HELP
                WORKER_HELP
                LCORE_HELP
                LCORE_NUMBER_HELP
                "set lcore not to launch anything\n"
                "set lcore to launch l2fwd\n"
                "set lcore to launch l3fwd (default: lpm)\n"
                "set lcore to launch l3fwd-lpm\n"
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
  else
    func_name = "none";

  lcore_workers[lcore_id].func = func;
  lcore_workers[lcore_id].arg = arg;
  lcore_workers[lcore_id].func_name = func_name;

  fprintf (shell->terminal, "worker set to lcore[%d]: func: %s\n",
           lcore_id, func_name);

  if (! strcmp (argv[0], "reset") ||
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
             "workers need to be reset for changes to take effect.\n");
}

DEFINE_COMMAND (start_stop_worker,
                "(start|stop) worker lcore <0-16>",
                START_HELP
                STOP_HELP
                WORKER_HELP
                LCORE_HELP
                LCORE_NUMBER_HELP
               )
{
  struct shell *shell = (struct shell *) context;
  int lcore_id;
  lcore_id = strtol (argv[2], NULL, 0);
  if (! strcmp (argv[0], "start"))
    start_lcore (shell, lcore_id);
  else if (! strcmp (argv[0], "stop"))
    stop_lcore (shell, lcore_id);
}

DEFINE_COMMAND (start_stop_worker_all,
                "(start|stop) worker lcore all",
                START_HELP
                STOP_HELP
                WORKER_HELP
                LCORE_HELP
                LCORE_NUMBER_HELP
                "all lcore\n"
               )
{
  struct shell *shell = (struct shell *) context;
  unsigned int lcore_id;
  uint32_t nb_lcores;
  nb_lcores = rte_lcore_count ();
  for (lcore_id = 0; lcore_id < nb_lcores; lcore_id++)
    {
      if (! strcmp (argv[0], "start"))
        start_lcore (shell, lcore_id);
      else if (! strcmp (argv[0], "stop"))
        stop_lcore (shell, lcore_id);
    }
}

DEFINE_COMMAND (show_worker,
                "show worker",
                SHOW_HELP
                WORKER_HELP
               )
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
  fprintf (shell->terminal, "%-9s: %-12s %-8s %s\n",
           "lcore", "flags", "state", "func_name");
  for (lcore_id = 0; lcore_id < nb_lcores; lcore_id++)
    {
      snprintf (flags, sizeof (flags), "%s%s",
                (rte_lcore_is_enabled (lcore_id) ? "enabled" : "disabled"),
                (lcore_id == main_lcore_id ? ",main" : ""));
      state = (rte_eal_get_lcore_state (lcore_id) == RUNNING ?
               "running" : "wait");
      snprintf (lcore_name, sizeof (lcore_name),
                "lcore[%d]", lcore_id);
      fprintf (shell->terminal, "%-9s: %-12s %-8s %s\n",
               lcore_name, flags, state, lcore_workers[lcore_id].func_name);
    }
}

DEFINE_COMMAND (exit_cmd,
               "(exit|quit|logout)",
               "exit\n"
               "quite\n"
               "logout\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "exit !\n");
  FLAG_SET (shell->flag, SHELL_FLAG_EXIT);
  /* don't shell_close(): this closes stdout. */
  //shell_close (shell);
}

void
get_winsize (struct shell *shell)
{
  ioctl (shell->writefd, TIOCGWINSZ, &shell->winsize);
  fprintf (shell->terminal, "row: %d col: %d\n",
           shell->winsize.ws_row, shell->winsize.ws_col);
}

void
lthread_shell (void *arg)
{
  struct shell *shell = NULL;

  printf ("%s[%d]: %s: enter.\n", __FILE__, __LINE__, __func__);

  /* library initialization. */
  debug_cmd_init ();
  command_shell_init ();

  shell = command_shell_create ();
  //shell_set_prompt_cwd (shell);
  shell_set_terminal (shell, 0, 1);
  get_winsize (shell);

  INSTALL_COMMAND2 (shell->cmdset, exit_cmd);

  INSTALL_COMMAND2 (shell->cmdset, show_worker);
  INSTALL_COMMAND2 (shell->cmdset, set_worker);
  INSTALL_COMMAND2 (shell->cmdset, start_stop_worker);
  INSTALL_COMMAND2 (shell->cmdset, start_stop_worker_all);

  //INSTALL_COMMAND2 (shell->cmdset, show_version);

  //INSTALL_COMMAND2 (shell->cmdset, chdir);
  //INSTALL_COMMAND2 (shell->cmdset, list);

  INSTALL_COMMAND2 (shell->cmdset, debug);
  INSTALL_COMMAND2 (shell->cmdset, show_debug);

  //INSTALL_COMMAND (shell->cmdset, pwd);
  //INSTALL_COMMAND (shell->cmdset, open);
  //INSTALL_COMMAND2 (shell->cmdset, terminal);
  //INSTALL_COMMAND2 (shell->cmdset, launch_shell);
  //INSTALL_COMMAND2 (shell->cmdset, edit_vi);

  //shell_install (shell, '>', fselect_keyfunc_start);
  //shell_install (shell, CONTROL ('D'), opensh_shell_keyfunc_ctrl_d);

  termio_init ();
  //shell_fselect_init ();

  shell_clear (shell);
  shell_prompt (shell);

  while (shell_running (shell))
    shell_read (shell);

  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);

  termio_finish ();
}

int
lthread_main (__rte_unused void *dummy)
{
  lthread_t *lt = NULL;

  /* timer set */
  timer_init (60 * 60, "2025/03/31 23:59:59");

  /* initialize workers */
  int lcore_id;
  for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++)
    {
      lcore_workers[lcore_id].func = NULL;
      lcore_workers[lcore_id].arg = NULL;
      lcore_workers[lcore_id].func_name = NULL;
    }

  lcore_id = rte_lcore_id ();
  lcore_workers[lcore_id].func = lthread_main;
  lcore_workers[lcore_id].func_name = "lthread_main";

  printf ("%s[%d]: %s: enter.\n", __FILE__, __LINE__, __func__);
  lthread_create (&lt, lthread_shell, NULL);
  lthread_run ();
}

