// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2023-2026 Yasuhiro Ohara

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

#ifdef ENABLE_L3FWD
#include "l3fwd.h"
#endif
#include "l2fwd_export.h"
#include "l2_repeater.h"
#include "l2_switch.h"
#include "sdplane.h"
#include "tap_handler.h"
#include "enhanced_repeater.h"

#include "argv_list.h"

int router (__rte_unused void *dummy);
int srv6_router (__rte_unused void *dummy);
int srv6_encap_router (__rte_unused void *dummy);
int srv6_dx_router (__rte_unused void *dummy);


#include "thread_info.h"

// clang-format off

bool rte_eal_init_done = false;

volatile bool force_quit;
volatile bool force_stop[RTE_MAX_LCORE];
volatile bool startup_config_completed = false;

struct lcore_worker lcore_workers[RTE_MAX_LCORE];

extern int lthread_core;

int nettlp_thread (void *arg);
int vlan_switch (void *arg);
#ifdef ENABLE_PKTGEN
int pktgen_launch_one_lcore(void *arg __rte_unused);
#endif

void
start_lcore (struct shell *shell, int lcore_id)
{
  if (! rte_eal_init_done)
    {
      fprintf (shell->terminal,
               "rte_eal_init() is not done: "
               "can't start worker on lcore: %d\n", lcore_id);
      return;
    }
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

  int ret;
  ret = thread_lookup_by_lcore (lcore_workers[lcore_id].func, lcore_id);
  if (ret < 0)
    thread_register (lcore_id, NULL,
                     (lthread_func) lcore_workers[lcore_id].func,
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
      return;
    }

  enum rte_lcore_state_t state;
  state = rte_eal_get_lcore_state (lcore_id);
  if (state != RUNNING)
    {
      fprintf (shell->terminal, "lcore is not running: %d\n", lcore_id);
      return;
    }

  rte_eal_wait_lcore (lcore_id);
  fprintf (shell->terminal, "stopped worker on lcore: %d\n", lcore_id);
}

/* rte_eal_init() makes the calling thread the DPDK main lcore
   (the lowest lcore in the coremask, unless --main-lcore is given).
   the lthread environment runs on that thread, so lthread_core
   must follow.  the lthread_main marker in lcore_workers[] must
   also move so that set/start/stop worker keep protecting the
   lthread lcore. */
void
update_lthread_core (struct shell *shell)
{
  unsigned int current_lcore;

  current_lcore = rte_lcore_id ();
  if (current_lcore == LCORE_ID_ANY)
    return;
  if (lthread_core == (int) current_lcore)
    return;

  DEBUG_NEW (WARNING, "lthread lcore changed: %d -> %d.",
             lthread_core, current_lcore);
  fprintf (shell->terminal,
           "lthread lcore changed: core[%d] -> core[%d]%s",
           lthread_core, current_lcore, shell->NL);

  if (lcore_workers[lthread_core].func == lthread_main)
    {
      lcore_workers[current_lcore] = lcore_workers[lthread_core];
      lcore_workers[lthread_core].func = NULL;
      lcore_workers[lthread_core].arg = NULL;
      lcore_workers[lthread_core].func_name = NULL;
    }

  lthread_core = current_lcore;

  lthread_t *lt = NULL;
  int thread_id;
  thread_id = thread_lookup (lthread_main);
  if (thread_id >= 0)
    thread_update (thread_id, lthread_core, lt, (lthread_func) lthread_main,
                   "lthread_main", NULL);
}

int dhcp_server (__rte_unused void *dummy);
int rib_manager (__rte_unused void *dummy);
int netlink_thread (__rte_unused void *dummy);
int igmp_snooping (__rte_unused void *dummy);

CLI_COMMAND2 (set_worker,
    "(set|reset|start|restart) worker lcore "LCORE_ID_SPEC" "
    "(|none|l2fwd"
#ifdef ENABLE_L3FWD
    "|l3fwd|l3fwd-lpm"
#endif
    "|tap-handler|l3-tap-handler"
    "|l2-repeater|vlan-switch|enhanced-repeater|l2-switch"
    "|router|srv6-router|srv6-encap-router|srv6-dx-router"
    "|nettlp-thread"
#ifdef ENABLE_PKTGEN
    "|pktgen"
#endif
    "|dhcp-server|igmp-snooping"
    "|rib-manager"
    "|netlink-thread"
    ")",
    SET_HELP, RESET_HELP, START_HELP, RESTART_HELP,
    WORKER_HELP, LCORE_HELP, LCORE_NUMBER_HELP,
    "set lcore not to launch anything\n",
    "set lcore to launch l2fwd\n",
#ifdef ENABLE_L3FWD
    "set lcore to launch l3fwd (default: lpm)\n",
    "set lcore to launch l3fwd-lpm\n",
#endif
    "set lcore to launch tap-handler\n",
    "set lcore to launch l3-tap-handler\n"
    "set lcore to launch l2-repeater\n",
    "set lcore to launch vlan-switch\n"
    "set lcore to launch enhanced-repeater\n"
    "set lcore to launch l2-switch\n"
    "set lcore to launch router\n"
    "set lcore to launch srv6-router\n"
    "set lcore to launch srv6-encap-router\n"
    "set lcore to launch srv6-dx-router\n"
    "set lcore to launch nettlp-thread\n"
#ifdef ENABLE_PKTGEN
    "set lcore to launch pktgen\n"
#endif
    "set lcore to launch dhcp-server\n"
    "set lcore to launch igmp-snooping\n"
    "set lcore to launch rib-manager\n"
    "set lcore to launch netlink-thread\n"
    )
{
  struct shell *shell = (struct shell *) context;
  int lcore_id;
  lcore_id = strtol (argv[3], NULL, 0);
  lcore_function_t *func;
  void *arg = NULL;

  if (argc == 4)
    {
      func = lcore_workers[lcore_id].func;
      fprintf (shell->terminal, "resetting lcore-id: %d to func %s%s",
               lcore_id, lcore_workers[lcore_id].func_name, shell->NL);
    }
  else if (! strcmp (argv[4], "none"))
    func = NULL;
  else if (! strcmp (argv[4], "l2fwd"))
    func = l2fwd_launch_one_lcore;
  else if (! strcmp (argv[4], "tap-handler"))
    func = tap_handler;
  else if (! strcmp (argv[4], "l2-repeater"))
    func = l2_repeater;
  else if (! strcmp (argv[4], "vlan-switch"))
    func = vlan_switch;
  else if (! strcmp (argv[4], "l3-tap-handler"))
    func = l3_tap_handler;
  else if (! strcmp (argv[4], "enhanced-repeater"))
    func = enhanced_repeater;
  else if (! strcmp (argv[4], "l2-switch"))
    func = l2_switch;
  else if (! strcmp (argv[4], "router"))
    func = router;
  else if (! strcmp (argv[4], "srv6-router"))
    func = srv6_router;
  else if (! strcmp (argv[4], "srv6-encap-router"))
    func = srv6_encap_router;
  else if (! strcmp (argv[4], "srv6-dx-router"))
    func = srv6_dx_router;
#ifdef ENABLE_PKTGEN
  else if (! strcmp (argv[4], "pktgen"))
    func = pktgen_launch_one_lcore;
#endif
  else if (! strcmp (argv[4], "dhcp-server"))
    func = dhcp_server;
  else if (! strcmp (argv[4], "igmp-snooping"))
    func = igmp_snooping;
  else if (! strcmp (argv[4], "rib-manager"))
    func = rib_manager;
  else if (! strcmp (argv[4], "netlink-thread"))
    func = netlink_thread;
#ifdef ENABLE_L3FWD
  else if (! strcmp (argv[4], "l3fwd-lpm"))
    func = lpm_main_loop;
  else /* if (! strcmp (argv[4], "l3fwd")) */
    func = lpm_main_loop;
#endif

  if (lcore_workers[lcore_id].func == lthread_main)
    {
      fprintf (shell->terminal, "cannot override lthread: lcore[%d].%s",
               lcore_id, shell->NL);
      return -1;
    }

  char *func_name;
  if (func == l2fwd_launch_one_lcore)
    func_name = "l2fwd";
  else if (func == lthread_main)
    func_name = "lthread_main";
  else if (func == tap_handler)
    func_name = "tap-handler";
  else if (func == l2_repeater)
    func_name = "l2-repeater";
  else if (func == vlan_switch)
    func_name = "vlan-switch";
  else if (func == l3_tap_handler)
    func_name = "l3-tap-handler";
  else if (func == enhanced_repeater)
    func_name = "enhanced-repeater";
  else if (func == l2_switch)
    func_name = "l2-switch";
  else if (func == router)
    func_name = "router";
  else if (func == srv6_router)
    func_name = "srv6-router";
  else if (func == srv6_encap_router)
    func_name = "srv6-encap-router";
  else if (func == srv6_dx_router)
    func_name = "srv6-dx-router";
#ifdef ENABLE_PKTGEN
  else if (func == pktgen_launch_one_lcore)
    func_name = "pktgen";
#endif
  else if (func == dhcp_server)
    func_name = "dhcp-server";
  else if (func == igmp_snooping)
    func_name = "igmp-snooping";
  else if (func == rib_manager)
    func_name = "rib-manager";
  else if (func == netlink_thread)
    func_name = "netlink-thread";
#ifdef ENABLE_L3FWD
  else if (func == lpm_main_loop)
    func_name = "l3fwd-lpm";
#endif
  else
    func_name = "none";

  lcore_workers[lcore_id].func = func;
  lcore_workers[lcore_id].arg = arg;
  lcore_workers[lcore_id].func_name = func_name;

  fprintf (shell->terminal, "worker set to lcore[%d]: func: %s%s",
           lcore_id, func_name, shell->NL);

  if (! strcmp (argv[0], "reset") || ! strcmp (argv[0], "start") ||
      ! strcmp (argv[0], "restart"))
    {
      stop_lcore (shell, lcore_id);
      start_lcore (shell, lcore_id);
      fprintf (shell->terminal, "worker[%d]: restarted.%s",
               lcore_id, shell->NL);
    }
  else if (! strcmp (argv[0], "set") && argc == 4)
    fprintf (shell->terminal, "nothing changed.%s", shell->NL);
  else
    fprintf (shell->terminal,
             "workers need to be restarted for changes to take effect.%s",
             shell->NL);
  return 0;
}

CLI_COMMAND2 (start_stop_worker,
              "(start|stop|reset|restart) worker lcore ("LCORE_ID_SPEC"|all)",
              START_HELP, STOP_HELP, RESET_HELP, RESTART_HELP,
              WORKER_HELP,
              LCORE_HELP,
              LCORE_NUMBER_HELP, LCORE_ALL_HELP)
{
  struct shell *shell = (struct shell *) context;
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

  /* before rte_eal_init(), RTE_LCORE_FOREACH() would iterate
     all RTE_MAX_LCORE lcores because the lcore roles are not
     initialized yet. */
  if (! rte_eal_init_done)
    {
      fprintf (shell->terminal, "rte_eal_init() is not done.%s", shell->NL);
      return 0;
    }

  RTE_LCORE_FOREACH (lcore_id)
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
  return 0;
}

CLI_COMMAND2 (show_worker,
              "show worker",
              SHOW_HELP, WORKER_HELP)
{
  struct shell *shell = (struct shell *) context;
  unsigned int lcore_id;
  unsigned int main_lcore_id;
  char *state;
  char flags[16];
  char lcore_name[16];

  if (! rte_eal_init_done)
    {
      fprintf (shell->terminal, "rte_eal_init() is not done.%s", shell->NL);
      return 0;
    }

  main_lcore_id = rte_get_main_lcore ();
  fprintf (shell->terminal, "main: %d lthread: %d%s",
           main_lcore_id, lthread_core, shell->NL);

  fprintf (shell->terminal, "%-9s: %-12s %-8s %s%s",
           "lcore", "flags", "state", "func_name", shell->NL);
  RTE_LCORE_FOREACH (lcore_id)
    {
      snprintf (flags, sizeof (flags), "%s%s",
                (rte_lcore_is_enabled (lcore_id) ? "enabled" : "disabled"),
                (lcore_id == main_lcore_id ? ",main" : ""));
      state =
          (rte_eal_get_lcore_state (lcore_id) == RUNNING ? "running" : "wait");
      snprintf (lcore_name, sizeof (lcore_name), "lcore[%d]", lcore_id);
      fprintf (shell->terminal, "%-9s: %-12s %-8s %s%s", lcore_name, flags,
               state,
               (lcore_workers[lcore_id].func_name
                ? lcore_workers[lcore_id].func_name : "none"),
               shell->NL);
    }
  return 0;
}

CLI_COMMAND2 (set_mempool,
              "set mempool (|auto|<1-1048575>)",
              SET_HELP,
              "mempool\n")
{
  struct shell *shell = (struct shell *) context;
  uint16_t nb_ports;
  unsigned int nb_mbufs;
  unsigned int nb_mbufs_auto;
  unsigned int nb_mbufs_requested = 0;

#define MAX_PKT_BURST       32
#define MEMPOOL_CACHE_SIZE  256

#define DEFAULT_NB_MBUFS    131071U

  nb_ports = rte_eth_dev_count_avail ();
  nb_mbufs = DEFAULT_NB_MBUFS;

  unsigned int nb_lcores;
  uint16_t nb_rxd = 1024;
  uint16_t nb_txd = 1024;
  nb_lcores = sysconf (_SC_NPROCESSORS_ONLN);
  nb_mbufs_auto = RTE_MAX (nb_ports * (nb_rxd + nb_txd + MAX_PKT_BURST +
                                       nb_lcores * MEMPOOL_CACHE_SIZE),
                           8192U);

  if (argc > 2)
    {
      if (! strcmp (argv[2], "auto"))
        {
          nb_mbufs = nb_mbufs_auto;
        }
      else
        {
          unsigned long requested = strtoul (argv[2], NULL, 0);
          nb_mbufs_requested = (unsigned int) requested;
          nb_mbufs = nb_mbufs_requested;
        }
    }
  else
    {
      nb_mbufs = nb_mbufs_auto;
    }

  fprintf (shell->terminal,
           "nb_ports: %u nb_lcores: %u cache: %u data_room: %u%s",
           nb_ports, nb_lcores, MEMPOOL_CACHE_SIZE, 9500U, shell->NL);
  fprintf (shell->terminal, "nb_mbufs_auto: %u nb_mbuf_requested: %u%s",
           nb_mbufs_auto, nb_mbufs_requested, shell->NL);
  fprintf (shell->terminal, "nb_mbufs: %u%s", nb_mbufs, shell->NL);

  if (l2fwd_pktmbuf_pool != NULL)
    {
#if 1
      /* if mempool is already existing, the workers may be
         (highly-likely) depending on it. */
      fprintf (shell->terminal, "mempool already exists.%s", shell->NL);
      return 0;
#else
      rte_mempool_free (l2fwd_pktmbuf_pool);
      l2fwd_pktmbuf_pool = NULL;
#endif
    }

  l2fwd_pktmbuf_pool =
      rte_pktmbuf_pool_create ("mbuf_pool", nb_mbufs, MEMPOOL_CACHE_SIZE,
                               0, 9500U, rte_socket_id ());

  if (l2fwd_pktmbuf_pool == NULL)
    {
      fprintf (shell->terminal, "Cannot init mbuf pool: %s%s",
               rte_strerror (rte_errno), shell->NL);
      return -1;
    }

  return 0;
}

#define RTE_EAL_ARGV_MAX 8
char *rte_eal_argv[RTE_EAL_ARGV_MAX];
int rte_eal_argc = 0;

CLI_COMMAND2 (set_rte_eal_argv,
              "set rte_eal argv "
              "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>",
              SET_HELP, "set rte_eal related information.\n",
              "set command-line arguments.\n",
              "arbitrary word\n", "arbitrary word\n",
              "arbitrary word\n", "arbitrary word\n",
              "arbitrary word\n", "arbitrary word\n",
              "arbitrary word\n", "arbitrary word\n")
{
  struct shell *shell = (struct shell *) context;
  int i;

  if (argc - 3 > RTE_EAL_ARGV_MAX - 1)
    {
      fprintf (shell->terminal, "too many arguments: %d.%s", argc, shell->NL);
      return -1;
    }

  rte_eal_argc = 0;
  memset (rte_eal_argv, 0, sizeof (rte_eal_argv));
  rte_eal_argv[rte_eal_argc++] = "sdplane";

  for (i = 3; i < argc; i++)
    {
      rte_eal_argv[rte_eal_argc++] = strdup (argv[i]);
    }

  for (i = 0; i < rte_eal_argc; i++)
    fprintf (shell->terminal, "rte_eal_argv[%d]: %s%s", i, rte_eal_argv[i],
             shell->NL);

  return 0;
}

ALIAS_COMMAND (set_rte_eal_argv_1,
               set_rte_eal_argv,
               "set rte_eal argv <WORD>",
               SET_HELP
               "set rte_eal related information.\n"
               "set command-line arguments.\n"
               "arbitrary word\n");

ALIAS_COMMAND (set_rte_eal_argv_2,
               set_rte_eal_argv,
               "set rte_eal argv <WORD> <WORD>",
               SET_HELP
               "set rte_eal related information.\n"
               "set command-line arguments.\n"
               "arbitrary word\n"
               "arbitrary word\n");

ALIAS_COMMAND (set_rte_eal_argv_3,
               set_rte_eal_argv,
               "set rte_eal argv <WORD> <WORD> <WORD>",
               SET_HELP
               "set rte_eal related information.\n"
               "set command-line arguments.\n"
               "arbitrary word\n"
               "arbitrary word\n"
               "arbitrary word\n");

ALIAS_COMMAND (set_rte_eal_argv_4,
               set_rte_eal_argv,
               "set rte_eal argv <WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set rte_eal related information.\n"
               "set command-line arguments.\n"
               "arbitrary word\n"
               "arbitrary word\n"
               "arbitrary word\n"
               "arbitrary word\n");

ALIAS_COMMAND (set_rte_eal_argv_5,
               set_rte_eal_argv,
               "set rte_eal argv <WORD> <WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set rte_eal related information.\n"
               "set command-line arguments.\n"
               "arbitrary word\n"
               "arbitrary word\n"
               "arbitrary word\n"
               "arbitrary word\n"
               "arbitrary word\n");

ALIAS_COMMAND (set_rte_eal_argv_6,
               set_rte_eal_argv,
               "set rte_eal argv <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set rte_eal related information.\n"
               "set command-line arguments.\n"
               "arbitrary word\n"
               "arbitrary word\n"
               "arbitrary word\n"
               "arbitrary word\n"
               "arbitrary word\n"
               "arbitrary word\n");

CLI_COMMAND2 (rte_eal_init, "rte_eal_init", "rte_eal_init command\n")
{
  struct shell *shell = (struct shell *) context;
  int ret;
  int i;

  fprintf (shell->terminal, "rte_eal_init (");
  for (i = 0; i < rte_eal_argc; i++)
    {
      if (i > 0)
        fprintf (shell->terminal, " ");
      fprintf (shell->terminal, "%s", rte_eal_argv[i]);
      if (i < rte_eal_argc - 1)
        fprintf (shell->terminal, ",");
    }
  fprintf (shell->terminal, ");%s", shell->NL);

  ret = rte_eal_init (rte_eal_argc, rte_eal_argv);
  if (ret < 0)
    {
      fprintf (shell->terminal, "Invalid EAL parameters.%s", shell->NL);
      return -1;
    }
  rte_eal_init_done = true;
  DEBUG_NEW (RTE_ETHDEV, "rte_eal_init(): done.");
  update_lthread_core (shell);
  return 0;
}

CLI_COMMAND2 (rte_eal_init_argv_list,
              "rte_eal_init argv-list <0-7>",
              "rte_eal_init command\n",
              "specify argv-list\n",
              "specify argv-list number\n")
{
  struct shell *shell = (struct shell *) context;
  int ret;

  int index;
  index = strtol (argv[2], NULL, 0);

  int i;
  char rte_argv_array[32][64];
  char *rte_argv[32];
  int rte_argc = 0;
  memset (rte_argv_array, 0, sizeof (rte_argv_array));
  memset (rte_argv, 0, sizeof (rte_argv));

  int *argcp = &argv_list_argc[index];
  char **argvp = argv_list[index];

  rte_argc = *argcp;

  /* rte_eal_init() may break the argv array,
   * so copy in a safer place, in advance. */
  pthread_mutex_lock (&argv_list_mutex);
  for (i = 0; i < rte_argc; i++)
    {
      rte_argv[i] = rte_argv_array[i];
      snprintf (rte_argv_array[i], sizeof (rte_argv_array[i]),
                "%s", argvp[i]);
    }
  pthread_mutex_unlock (&argv_list_mutex);

  ret = rte_eal_init (rte_argc, rte_argv);
  if (ret < 0)
    {
      fprintf (shell->terminal, "Invalid EAL parameters.%s", shell->NL);
      return -1;
    }
  rte_eal_init_done = true;
  DEBUG_NEW (RTE_ETHDEV, "rte_eal_init(): done.");
  update_lthread_core (shell);
  return 0;
}

#ifdef REGVAR_SUPPORT
int
regvar_lcoreid_is_spec (char *spec)
{
  return ! strcmp (spec, "<LCORE-ID>");
}

int
regvar_lcoreid_is_match (char *spec, char *word)
{
  int lcore_id;
  int lcore_max;
  char *endptr;
  lcore_max = sysconf (_SC_NPROCESSORS_ONLN);
  lcore_id = strtol (word, &endptr, 0);
  if (endptr[0] != '\0')
    return 0;
  if (0 <= lcore_id && lcore_id < lcore_max)
    return 1;
  return 0;
}

char *
regvar_lcoreid_convert_cmdstr (char *spec, char *word)
{
  char cmdstr[32];
  int lcore_max;
  lcore_max = sysconf (_SC_NPROCESSORS_ONLN);
  snprintf (cmdstr, sizeof (cmdstr), "<0-%d>", lcore_max - 1);
  return strdup (cmdstr);
}

char *
regvar_lcoreid_convert_helpstr (char *spec, char *orig_helpstr, char *word)
{
  char helpstr[32];
  int lcore_max;
  lcore_max = sysconf (_SC_NPROCESSORS_ONLN);
  snprintf (helpstr, sizeof (helpstr),
            "specify LCORE-ID. (<0-%d>)", lcore_max - 1);
  return strdup (helpstr);
}

struct regvar regvar_lcoreid;
#endif /*REGVAR_SUPPORT*/

void
dpdk_lcore_cmd_init (struct command_set *cmdset)
{
#ifdef REGVAR_SUPPORT
  struct regvar *regvar;
  regvar = &regvar_lcoreid;
  regvar->spec = "<LCORE-ID>";
  regvar->is_spec = regvar_lcoreid_is_spec;
  regvar->is_match = regvar_lcoreid_is_match;
  regvar->convert_cmdstr = regvar_lcoreid_convert_cmdstr;
  regvar->convert_helpstr = regvar_lcoreid_convert_helpstr;

  if (! cmdset->regvars)
    cmdset->regvars = vector_create ();
  vector_add (regvar, cmdset->regvars);
#endif /*REGVAR_SUPPORT*/

  INSTALL_COMMAND2 (cmdset, set_worker);
  INSTALL_COMMAND2 (cmdset, start_stop_worker);
  INSTALL_COMMAND2 (cmdset, show_worker);
  INSTALL_COMMAND2 (cmdset, set_mempool);
  INSTALL_COMMAND2 (cmdset, set_rte_eal_argv);
  INSTALL_COMMAND2 (cmdset, set_rte_eal_argv_1);
  INSTALL_COMMAND2 (cmdset, set_rte_eal_argv_2);
  INSTALL_COMMAND2 (cmdset, set_rte_eal_argv_3);
  INSTALL_COMMAND2 (cmdset, set_rte_eal_argv_4);
  INSTALL_COMMAND2 (cmdset, set_rte_eal_argv_5);
  INSTALL_COMMAND2 (cmdset, set_rte_eal_argv_6);
  INSTALL_COMMAND2 (cmdset, rte_eal_init);
  INSTALL_COMMAND2 (cmdset, rte_eal_init_argv_list);
}
