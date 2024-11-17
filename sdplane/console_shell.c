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

#include <zcmdsh/log.h>
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
// #include <zcmdsh/shell_fselect.h>
#include <zcmdsh/debug_zcmdsh.h>

#include "l3fwd.h"
#include "l3fwd_event.h"
#include "l3fwd_route.h"
#include "l3fwd_cmd.h"

#include "l2fwd_export.h"
#include "l2fwd_cmd.h"

#include "sdplane.h"
#include "tap_handler.h"

#include "debug_sdplane.h"
#include "vty_shell.h"

#include "thread_info.h"

extern int lthread_core;

CLI_COMMAND2 (exit_cmd, "(exit|quit|logout)", "exit\n", "quite\n", "logout\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "console exit !\n");
  FLAG_SET (shell->flag, SHELL_FLAG_EXIT);
  /* don't shell_close(): this closes stdout. */
  // shell_close (shell);

  int nb_lcores = rte_lcore_count ();
  for (int lcore_id = 0; lcore_id < nb_lcores; lcore_id++)
    stop_lcore (shell, lcore_id);
}

bool reboot = false;

CLI_COMMAND2 (reboot_cmd, "reboot", "reboot\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "reboot !\n");
  reboot = true;
  FLAG_SET (shell->flag, SHELL_FLAG_EXIT);
  /* don't shell_close(): this closes stdout. */
  // shell_close (shell);

  int nb_lcores = rte_lcore_count ();
  for (int lcore_id = 0; lcore_id < nb_lcores; lcore_id++)
    stop_lcore (shell, lcore_id);
}

void
get_winsize (struct shell *shell)
{
  ioctl (shell->writefd, TIOCGWINSZ, &shell->winsize);
  fprintf (shell->terminal, "row: %d col: %d\n", shell->winsize.ws_row,
           shell->winsize.ws_col);
}

uint64_t loop_console = 0;

void
console_shell (void *arg)
{
  struct shell *shell = NULL;

  printf ("%s[%d]: %s: enter.\n", __FILE__, __LINE__, __func__);

  shell = command_shell_create ();
  shell_set_terminal (shell, 0, 1);
  shell_set_prompt (shell, "console> ");
  get_winsize (shell);

  INSTALL_COMMAND2 (shell->cmdset, exit_cmd);

  INSTALL_COMMAND2 (shell->cmdset, show_worker);
  INSTALL_COMMAND2 (shell->cmdset, set_worker);
  INSTALL_COMMAND2 (shell->cmdset, start_stop_worker);

  INSTALL_COMMAND2 (shell->cmdset, debug_zcmdsh);
  INSTALL_COMMAND2 (shell->cmdset, show_debug_zcmdsh);

  INSTALL_COMMAND2 (shell->cmdset, debug_sdplane);
  INSTALL_COMMAND2 (shell->cmdset, show_debug_sdplane);

  INSTALL_COMMAND2 (shell->cmdset, clear_cmd);
  shell_install (shell, CONTROL ('L'), shell_keyfunc_clear_terminal);

  log_cmd_init (shell->cmdset);
  l2fwd_cmd_init (shell->cmdset);
  l3fwd_cmd_init (shell->cmdset);
  sdplane_cmd_init (shell->cmdset);

  termio_init ();

  shell_clear (shell);
  shell_prompt (shell);

  int thread_id;
  thread_id = thread_lookup (console_shell);
  thread_register_loop_counter (thread_id, &loop_console);

  while (! force_quit && ! force_stop[lthread_core] && shell_running (shell))
    {
      loop_console++;
      lthread_sleep (100); // yield.
      shell_read_nowait (shell);
    }

  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);

  termio_finish ();
}
