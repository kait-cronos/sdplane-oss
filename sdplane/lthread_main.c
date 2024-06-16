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

#include <unistd.h>
#include <sys/ioctl.h>
#include <lthread.h>

#include <zcmdsh/debug.h>
#include <zcmdsh/termio.h>
#include <zcmdsh/vector.h>
#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>
#include <zcmdsh/command_shell.h>
#include <zcmdsh/debug_cmd.h>
#include <zcmdsh/debug_module.h>
#include <zcmdsh/debug_module_cmd.h>
//#include <zcmdsh/shell_fselect.h>

#include "l3fwd.h"
#include "l3fwd_event.h"
#include "l3fwd_route.h"

#include "l2fwd_export.h"
#include "l2fwd_cmd.h"

#include "sdplane.h"
#include "tap_handler.h"

#include "debug_sdplane.h"

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

  int nb_lcores = rte_lcore_count ();
  for (int lcore_id = 0; lcore_id < nb_lcores; lcore_id++)
    stop_lcore (shell, lcore_id);
}

bool reboot = false;

DEFINE_COMMAND (reboot_cmd,
                "reboot",
                "reboot\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "reboot !\n");
  reboot = true;
  FLAG_SET (shell->flag, SHELL_FLAG_EXIT);
  /* don't shell_close(): this closes stdout. */
  //shell_close (shell);

  int nb_lcores = rte_lcore_count ();
  for (int lcore_id = 0; lcore_id < nb_lcores; lcore_id++)
    stop_lcore (shell, lcore_id);
}

void
get_winsize (struct shell *shell)
{
  ioctl (shell->writefd, TIOCGWINSZ, &shell->winsize);
  fprintf (shell->terminal, "row: %d col: %d\n",
           shell->winsize.ws_row, shell->winsize.ws_col);
}

void
shell_keyfunc_clear_terminal (struct shell *shell)
{
  const char clr[] = { 27, '[', '2', 'J', '\0' };
  const char topLeft[] = { 27, '[', '1', ';', '1', 'H', '\0' };
  /* Clear screen and move to top left */
  fprintf (shell->terminal, "%s%s", clr, topLeft);
  fflush (shell->terminal);
}

DEFINE_COMMAND (clear_cmd,
                "clear",
                CLEAR_HELP)
{
  struct shell *shell = (struct shell *) context;
  shell_keyfunc_clear_terminal (shell);
}

void
lthread_shell (void *arg)
{
  struct shell *shell = NULL;

  printf ("%s[%d]: %s: enter.\n", __FILE__, __LINE__, __func__);

  shell = command_shell_create ();
  //shell_set_prompt_cwd (shell);
  shell_set_terminal (shell, 0, 1);
  get_winsize (shell);

  INSTALL_COMMAND2 (shell->cmdset, exit_cmd);

  INSTALL_COMMAND2 (shell->cmdset, show_worker);
  INSTALL_COMMAND2 (shell->cmdset, set_worker);
  INSTALL_COMMAND2 (shell->cmdset, start_stop_worker);

  //INSTALL_COMMAND2 (shell->cmdset, show_version);

  //INSTALL_COMMAND2 (shell->cmdset, chdir);
  //INSTALL_COMMAND2 (shell->cmdset, list);

  INSTALL_COMMAND2 (shell->cmdset, debug);
  INSTALL_COMMAND2 (shell->cmdset, show_debug);

  INSTALL_COMMAND3 (shell->cmdset, debug_module, debug_module_sdplane);
  INSTALL_COMMAND2 (shell->cmdset, show_debug_module);

  //INSTALL_COMMAND (shell->cmdset, pwd);
  //INSTALL_COMMAND (shell->cmdset, open);
  //INSTALL_COMMAND2 (shell->cmdset, terminal);
  //INSTALL_COMMAND2 (shell->cmdset, launch_shell);
  //INSTALL_COMMAND2 (shell->cmdset, edit_vi);

  //shell_install (shell, '>', fselect_keyfunc_start);
  //shell_install (shell, CONTROL ('D'), opensh_shell_keyfunc_ctrl_d);

  INSTALL_COMMAND2 (shell->cmdset, clear_cmd);
  shell_install (shell, CONTROL ('L'), shell_keyfunc_clear_terminal);

  l2fwd_cmd_init (shell->cmdset);
  soft_dplane_cmd_init (shell->cmdset);

  termio_init ();
  //shell_fselect_init ();

  shell_clear (shell);
  shell_prompt (shell);

  while (shell_running (shell))
    {
      lthread_sleep (0); // yield.
      //printf ("%s: schedule.\n", __func__);
      shell_read_nowait (shell);
    }

  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);

  termio_finish ();
}

int
lthread_main (__rte_unused void *dummy)
{
  lthread_t *lt = NULL;

  /* timer set */
  timer_init (60 * 60, "2024/12/31 23:59:59");

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

  /* library initialization. */
  debug_cmd_init ();
  command_shell_init ();

  debug_module_cmd_init ();

  lthread_create (&lt, (lthread_func) load_startup_config, NULL);
  lthread_create (&lt, (lthread_func) lthread_shell, NULL);
  //lthread_create (&lt, (lthread_func) tap_handler, NULL);
  lthread_create (&lt, (lthread_func) stat_collector, NULL);
  lthread_run ();
}

