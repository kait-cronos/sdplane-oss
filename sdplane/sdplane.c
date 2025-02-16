#include "include.h"

#include <rte_ethdev.h>
#include <rte_bus_pci.h>

#include <zcmdsh/debug.h>
#include <zcmdsh/termio.h>
#include <zcmdsh/vector.h>
#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>
#include <zcmdsh/command_shell.h>
#include <zcmdsh/debug_cmd.h>

#include "l3fwd.h"
#include "l2fwd_export.h"

#include "sdplane.h"
#include "sdplane_version.h"
#include "stat_collector.h"
#include "tap_handler.h"
#include "debug_sdplane.h"

#include <lthread.h>
#include "thread_info.h"
#include "queue_config.h"

#include "rib.h"
#include "tap_cmd.h"

CLI_COMMAND2 (show_version, "show version", SHOW_HELP, "version\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  fprintf (t, "sdplane version: %s%s", sdplane_version, shell->NL);
}

CLI_COMMAND2 (set_locale,
              //"set locale (LC_ALL|LC_NUMERIC) (C|C.utf8|en_US.utf8|POSIX)",
              "set locale (C|C.utf8|en_US.utf8|POSIX)", SET_HELP,
              "locale information\n", "C\n", "C.utf8\n", "en_US.utf8\n",
              "POSIX")
{
  struct shell *shell = (struct shell *) context;
  char *ret;
  ret = setlocale (LC_ALL, argv[2]);
  if (! ret)
    fprintf (shell->terminal, "setlocale(): failed.\n");
  else
    fprintf (shell->terminal, "setlocale(): %s.\n", ret);
}

char *l3fwd_argv[L3FWD_ARGV_MAX];
int l3fwd_argc = 0;

CLI_COMMAND2 (set_l3fwd_argv,
              "set l3fwd argv <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>",
              SET_HELP, "set l3fwd-related information.\n",
              "set command-line arguments.\n", "arbitrary word\n")
{
  struct shell *shell = (struct shell *) context;
  int i;

  if (argc - 2 >= L3FWD_ARGV_MAX - 2)
    {
      fprintf (shell->terminal, "too many arguments: %d.\n", argc);
      return;
    }

  l3fwd_argc = 0;
  memset (l3fwd_argv, 0, sizeof (l3fwd_argv));
  l3fwd_argv[l3fwd_argc++] = "sdplane";

  for (i = 3; i < argc; i++)
    {
      l3fwd_argv[l3fwd_argc++] = strdup (argv[i]);
    }

  for (i = 0; i < l3fwd_argc; i++)
    fprintf (shell->terminal, "l3fwd_argv[%d]: %s\n", i, l3fwd_argv[i]);
}

CLI_COMMAND2 (show_loop_count,
              "show loop-count (console|vty-shell|l2fwd) (pps|total)",
              SHOW_HELP, "loop count\n", "console shell\n", "vty shell\n",
              "l2fwd loop\n", "pps\n", "total count\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;

  char name[32];

  if (! strcmp (argv[2], "console"))
    {
      snprintf (name, sizeof (name), "console:");
      if (! strcmp (argv[3], "pps"))
        {
          fprintf (t, "%16s %'8lu\n", name, loop_console_pps);
        }
      else if (! strcmp (argv[3], "total"))
        {
          fprintf (t, "%16s %'8lu\n", name, loop_console_current);
        }
    }
  else if (! strcmp (argv[2], "vty-shell"))
    {
      snprintf (name, sizeof (name), "vty-shell:");
      if (! strcmp (argv[3], "pps"))
        {
          fprintf (t, "%16s %'8lu\n", name, loop_vty_shell_pps);
        }
      else if (! strcmp (argv[3], "total"))
        {
          fprintf (t, "%16s %'8lu\n", name, loop_vty_shell_current);
        }
    }
  else if (! strcmp (argv[2], "l2fwd"))
    {
      int i;
      for (i = 0; i < RTE_MAX_LCORE; i++)
        {
          snprintf (name, sizeof (name), "l2fwd: core[%d]:", i);
          if (loop_l2fwd_current[i])
            {
              if (! strcmp (argv[3], "pps"))
                {
                  fprintf (t, "%24s %'8lu\n", name, loop_l2fwd_pps[i]);
                }
              else if (! strcmp (argv[3], "total"))
                {
                  fprintf (t, "%24s %'8lu\n", name, loop_l2fwd_current[i]);
                }
            }
        }
    }
}

extern uint64_t rib_rcu_replace;
CLI_COMMAND2 (show_rcu, "show rcu",
              SHOW_HELP, "show rcu-information.\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  fprintf (t, "rib_rcu_replace: %'lu%s",
           rib_rcu_replace, shell->NL);
}

CLI_COMMAND2 (show_fdb, "show fdb",
              SHOW_HELP, "show fdb-information.\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  int i;
  char buf[32];
  for (i = 0; i < FDB_SIZE; i++)
    {
      rte_ether_format_addr (buf, sizeof (buf), &fdb[i].l2addr);
      if (! rte_is_zero_ether_addr (&fdb[i].l2addr))
        fprintf (t, "fdb[%d]: %s port %d%s",
                 i, buf, fdb[i].port, shell->NL);
    }
}

CLI_COMMAND2 (show_vswitch,
              "show vswitch",
              SHOW_HELP,
              "show vswitch-information.\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  struct vswitch *vswitch = &vswitch0;
  struct vswitch_port *vport;
  char *type_str;
  int i;
  for (i = 0; i < vswitch->size; i++)
    {
      vport = &vswitch->port[i];
      type_str = NULL;
      switch (vport->type)
        {
        case VSWITCH_PORT_TYPE_NONE:
          type_str = "none";
          break;
        case VSWITCH_PORT_TYPE_DPDK_LCORE:
          type_str = "dpdk-port";
          break;
        case VSWITCH_PORT_TYPE_LINUX_TAP:
          type_str = "linux-tap";
          break;
        default:
          type_str = "unknown";
          break;
        }
      fprintf (t, "vswport[%d]: [%d] %9s %s sock: %d lcore: %d ring[%p/%p]%s",
               i, vport->id, type_str, vport->name, vport->sockfd,
               vport->lcore_id,
               vport->ring[TAPDIR_UP], vport->ring[TAPDIR_DOWN],
               shell->NL);
    }
}

CLI_COMMAND2 (sleep_cmd, "sleep <0-300>",
              "sleep command\n",
              "specify seconds to sleep.\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  int sec;
  sec = strtol (argv[1], NULL, 0);
  while (sec > 0)
    {
      fprintf (t, " %d", sec);
      fflush (t);
      //sleep (1);
      lthread_sleep (1000);
      sec--;
    }
  fprintf (t, " 0.%s", shell->NL);
  fflush (t);
}

void dpdk_lcore_cmd_init (struct command_set *cmdset);
void dpdk_port_cmd_init (struct command_set *cmdset);
void lthread_cmd_init (struct command_set *cmdset);
void queue_config_cmd_init (struct command_set *cmdset);
void nettlp_cmd_init (struct command_set *cmdset);

void
sdplane_cmd_init (struct command_set *cmdset)
{
  setlocale (LC_ALL, "en_US.utf8");
  dpdk_lcore_cmd_init (cmdset);
  dpdk_port_cmd_init (cmdset);
  INSTALL_COMMAND2 (cmdset, set_l3fwd_argv);
  INSTALL_COMMAND2 (cmdset, show_loop_count);
  INSTALL_COMMAND2 (cmdset, show_version);
  INSTALL_COMMAND2 (cmdset, show_rcu);
  INSTALL_COMMAND2 (cmdset, show_fdb);
  INSTALL_COMMAND2 (cmdset, show_rib);
  INSTALL_COMMAND2 (cmdset, show_vswitch);
  INSTALL_COMMAND2 (cmdset, sleep_cmd);
  thread_info_cmd_init (cmdset);
  queue_config_cmd_init (cmdset);
  lthread_cmd_init (cmdset);
  tap_cmd_init (cmdset);

  nettlp_cmd_init (cmdset);
}

void
sdplane_init ()
{
  int lcore_id;
  debug_sdplane_cmd_init ();
  thread_info_init ();
}

