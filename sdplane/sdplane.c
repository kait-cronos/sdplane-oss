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

#include <locale.h>

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

DEFINE_COMMAND (show_version,
                "show version",
                SHOW_HELP
                "version\n"
                )
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  fprintf (t, "sdplane version: %s%s", sdplane_version, shell->NL);
}

DEFINE_COMMAND (set_locale,
                //"set locale (LC_ALL|LC_NUMERIC) (C|C.utf8|en_US.utf8|POSIX)",
                "set locale (C|C.utf8|en_US.utf8|POSIX)",
                SET_HELP
                "locale information\n"
                "C\n"
                "C.utf8\n"
                "en_US.utf8\n"
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

DEFINE_COMMAND (set_l3fwd_argv,
                "set l3fwd argv <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>",
                SET_HELP
                "set l3fwd-related information.\n"
                "set command-line arguments.\n"
                "arbitrary word\n")
{
  struct shell *shell = (struct shell *) context;
  int i;

#if 0
  for (i = 0; i < argc; i++)
    fprintf (shell->terminal, "argv[%d]: %s\n", i, argv[i]);
#endif

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

#if 0
  fprintf (shell->terminal, "l3fwd_argv[%d]:", l3fwd_argc);
  for (i = 0; i < l3fwd_argc; i++)
    {
      fprintf (shell->terminal, " %s", l3fwd_argv[i]);
    }
  fprintf (shell->terminal, "\n");
#endif

  for (i = 0; i < l3fwd_argc; i++)
    fprintf (shell->terminal, "l3fwd_argv[%d]: %s\n", i, l3fwd_argv[i]);
}

DEFINE_COMMAND (show_loop_count,
                "show loop-count (console|l2fwd) (pps|total)",
                SHOW_HELP
                "loop count\n"
                "console shell\n"
                "l2fwd loop\n"
                "pps\n"
                "total count\n"
                )
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

void dpdk_lcore_cmd_init (struct command_set *cmdset);
void dpdk_port_cmd_init (struct command_set *cmdset);

void
soft_dplane_cmd_init (struct command_set *cmdset)
{
  setlocale (LC_ALL, "en_US.utf8");
  dpdk_lcore_cmd_init (cmdset);
  dpdk_port_cmd_init (cmdset);
  INSTALL_COMMAND2 (cmdset, set_l3fwd_argv);
  INSTALL_COMMAND2 (cmdset, show_loop_count);
  INSTALL_COMMAND2 (cmdset, show_version);
}

extern struct rte_ring *tap_ring_by_lcore[RTE_MAX_LCORE];

void
soft_dplane_init ()
{
  int lcore_id;
  for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++)
    tap_ring_by_lcore[lcore_id] = NULL;
  debug_sdplane_cmd_init ();
}

