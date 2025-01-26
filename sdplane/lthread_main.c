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

#include <zcmdsh/debug.h>
#include <zcmdsh/termio.h>
#include <zcmdsh/vector.h>
#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>
#include <zcmdsh/command_shell.h>
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

int lthread_core = 0;

int startup_config (__rte_unused void *dummy);
void console_shell (void *arg);
void vty_server (void *arg);

int stat_collector (__rte_unused void *dummy);
void rib_manager (void *arg);

CLI_COMMAND2 (set_worker_lthread_stat_collector,
              "set worker lthread stat-collector",
              SET_HELP,
              WORKER_HELP,
              "lthread information\n",
              "stat-collector\n"
              )
{
  struct shell *shell = (struct shell *) context;
  lthread_t *lt = NULL;

  lthread_create (&lt, (lthread_func) stat_collector, NULL);
  thread_register (lthread_core, lt, (lthread_func) stat_collector, "stat_collector", NULL);
  lthread_detach2 (lt);
}

CLI_COMMAND2 (set_worker_lthread_rib_manager,
              "set worker lthread rib-manager",
              SET_HELP,
              WORKER_HELP,
              "lthread information\n",
              "rib-manager\n"
              )
{
  struct shell *shell = (struct shell *) context;
  lthread_t *lt = NULL;

  lthread_create (&lt, (lthread_func) rib_manager, NULL);
  thread_register (lthread_core, lt, (lthread_func) rib_manager, "rib_manager", NULL);
  lthread_detach2 (lt);
}

void
lthread_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, set_worker_lthread_stat_collector);
  INSTALL_COMMAND2 (cmdset, set_worker_lthread_rib_manager);
}

int
lthread_main (__rte_unused void *dummy)
{
  lthread_t *lt = NULL;

  /* timer set */
  // timer_init (60 * 60, "2024/12/31 23:59:59");
  timer_init (0, NULL);

  /* initialize workers */
  int lcore_id;
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

  /* library initialization. */
  debug_zcmdsh_cmd_init ();
  command_shell_init ();

  // lthread_create (&lt, (lthread_func) tap_handler, NULL);
  lthread_create (&lt, (lthread_func) vty_server, NULL);
  thread_register (lthread_core, lt, vty_server, "vty_server", NULL);
  lthread_detach2 (lt);

  lthread_create (&lt, (lthread_func) startup_config, NULL);
  thread_register (lthread_core, lt, (lthread_func) startup_config,
                   "startup_config", NULL);
  lthread_join (lt, NULL, 0);

  lthread_create (&lt, (lthread_func) console_shell, NULL);
  thread_register (lthread_core, lt, console_shell, "console_shell", NULL);
  lthread_detach2 (lt);
}
