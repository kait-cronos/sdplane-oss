#include "include.h"

#include <rte_ethdev.h>
#include <rte_bus_pci.h>

#include <sdplane/debug.h>
#include <sdplane/termio.h>
#include <sdplane/vector.h>
#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>
#include <sdplane/debug_cmd.h>

#include "l3fwd.h"
#include "l2fwd_export.h"

#include "sdplane.h"
#include "sdplane_version.h"
#include "stat_collector.h"
#include "tap_handler.h"
#include "debug_sdplane.h"
#include "neigh_manager.h"

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

CLI_COMMAND2 (set_locale, "set locale (C|C.utf8|en_US.utf8|POSIX)", SET_HELP,
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

#if 0
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
      fprintf (shell->terminal, "too many arguments: %d.%s", argc, shell->NL);
      return -1;
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

  return 0;
}
#endif

char *argv_list[ARGV_LIST_MAX][ARGV_LIST_ARGV_MAX];
int argv_list_argc[ARGV_LIST_MAX];

CLI_COMMAND2 (set_argv_list_1,
              "set argv-list <0-7> <WORD>",
              SET_HELP,
              "set argv-list.\n",
              "specify argv-list index.\n",
              "set command-line arguments.\n")
{
  struct shell *shell = (struct shell *) context;
  int i;

  if (argc >= ARGV_LIST_ARGV_MAX)
    {
      fprintf (shell->terminal, "too many arguments: %d.%s", argc, shell->NL);
      return -1;
    }

  int index;
  index = strtol (argv[2], NULL, 0);

  int *argcp = &argv_list_argc[index];
  char **argvp = argv_list[index];

  for (i = 0; i < ARGV_LIST_ARGV_MAX; i++)
    {
      if (argvp[i])
        free (argvp[i]);
      argvp[i] = NULL;
    }

  *argcp = 0;
  for (i = 3; i < argc; i++)
    {
      argvp[(*argcp)++] = strdup (argv[i]);
    }

  for (i = 0; i < *argcp; i++)
    fprintf (shell->terminal, "argv_list[%d][%d]: %s%s",
             index, i, argv_list[index][i], shell->NL);

  return 0;
}

ALIAS_COMMAND (set_argv_list_2,
               set_argv_list_1,
               "set argv-list <0-7> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_3,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_4,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_5,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_6,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_7,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_8,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_9,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_10,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_11,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_12,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_13,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_14,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_15,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_16,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_17,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_18,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_19,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_20,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_21,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_22,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_23,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

ALIAS_COMMAND (set_argv_list_24,
               set_argv_list_1,
               "set argv-list <0-7> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> "
               "<WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD> <WORD>",
               SET_HELP
               "set argv-list.\n"
               "specify argv-list index.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n"
               "set command-line arguments.\n");

CLI_COMMAND2 (show_argv_list, "show argv-list (|<0-7>)", SHOW_HELP,
              "show argv-list.\n", "specify argv-list index.\n")
{
  struct shell *shell = (struct shell *) context;
  int i, j;

  int index = -1;
  if (argc > 2)
    index = strtol (argv[2], NULL, 0);

  for (i = 0; i < ARGV_LIST_MAX; i++)
    {
      int *argcp = &argv_list_argc[i];
      char **argvp = argv_list[i];

      if (index >= 0 && i != index)
        continue;
      for (j = 0; j < *argcp; j++)
        fprintf (shell->terminal, "argv_list[%d][%d]: %s\n", i, j,
                 argv_list[i][j]);
    }

  return 0;
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
  return 0;
}

extern uint64_t rib_rcu_replace;
CLI_COMMAND2 (show_rcu, "show rcu", SHOW_HELP, "show rcu-information.\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  fprintf (t, "rib_rcu_replace: %'lu%s", rib_rcu_replace, shell->NL);
  return 0;
}

CLI_COMMAND2 (show_fdb, "show fdb", SHOW_HELP, "show fdb-information.\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  int i;
  char buf[32];
  for (i = 0; i < FDB_SIZE; i++)
    {
      rte_ether_format_addr (buf, sizeof (buf), &fdb[i].l2addr);
      if (! rte_is_zero_ether_addr (&fdb[i].l2addr))
        fprintf (t, "fdb[%d]: %s port %d%s", i, buf, fdb[i].port, shell->NL);
    }
  return 0;
}

CLI_COMMAND2 (show_vswitch, "show vswitch", SHOW_HELP,
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
               vport->lcore_id, vport->ring[TAPDIR_UP],
               vport->ring[TAPDIR_DOWN], shell->NL);
    }
  return 0;
}

CLI_COMMAND2 (sleep_cmd, "sleep <0-300>", "sleep command\n",
              "specify seconds to sleep.\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  int sec;
  sec = strtol (argv[1], NULL, 0);
  if (sec > 0)
    {
      while (sec > 0)
        {
          fprintf (t, " %d", sec);
          fflush (t);
          lthread_sleep (1000);
          sec--;
        }
    }
  else
    lthread_sleep (0);
  fprintf (t, " 0.%s", shell->NL);
  fflush (t);
  return 0;
}

CLI_COMMAND2 (show_mempool, "show mempool", SHOW_HELP, "show mempool.\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  struct rte_mempool *mp;
  unsigned int count;
  int is_full;

  mp = l2fwd_pktmbuf_pool;
  count = rte_mempool_avail_count (mp);
  is_full = rte_mempool_full (mp);

  fprintf (t, "mempool: size: %u count: %u is_full: %d%s", mp->size, count,
           is_full, shell->NL);

  return 0;
}

CLI_COMMAND2 (show_neighbor, "show neighbor (ipv4|ipv6)", SHOW_HELP,
              "show neighbor table.\n", "show ARP table.\n",
              "show ND table.\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  int thread_id;

  thread_id = thread_lookup (neigh_manager);
  if (thread_id < 0)
    {
      fprintf (t, "neigh_manager thread not found.%s", shell->NL);
      return -1;
    }

  if (argc < 3)
    {
      fprintf (t, "Usage: %s\n", argv[0]);
      return -1;
    }
  if (! strcmp (argv[2], "ipv4"))
    neigh_manager_show_table (NEIGH_ARP_TABLE, shell);
  else if (! strcmp (argv[2], "ipv6"))
    neigh_manager_show_table (NEIGH_ND_TABLE, shell);
  else
    {
      fprintf (t, "Unknown neighbor type: %s\n", argv[2]);
      return -1;
    }

  return 0;
}

void dpdk_lcore_cmd_init (struct command_set *cmdset);
void dpdk_port_cmd_init (struct command_set *cmdset);
void lthread_cmd_init (struct command_set *cmdset);
void queue_config_cmd_init (struct command_set *cmdset);
void nettlp_cmd_init (struct command_set *cmdset);
void dpdk_devbind_cmd_init (struct command_set *cmdset);
void pktgen_cmd_init (struct command_set *cmdset);

void
sdplane_cmd_init (struct command_set *cmdset)
{
  setlocale (LC_ALL, "en_US.utf8");
  dpdk_lcore_cmd_init (cmdset);
  dpdk_port_cmd_init (cmdset);
#if 0
  INSTALL_COMMAND2 (cmdset, set_l3fwd_argv);
#endif
  INSTALL_COMMAND2 (cmdset, set_argv_list_1);
  INSTALL_COMMAND2 (cmdset, set_argv_list_2);
  INSTALL_COMMAND2 (cmdset, set_argv_list_3);
  INSTALL_COMMAND2 (cmdset, set_argv_list_4);
  INSTALL_COMMAND2 (cmdset, set_argv_list_5);
  INSTALL_COMMAND2 (cmdset, set_argv_list_6);
  INSTALL_COMMAND2 (cmdset, set_argv_list_7);
  INSTALL_COMMAND2 (cmdset, set_argv_list_8);
  INSTALL_COMMAND2 (cmdset, set_argv_list_9);
  INSTALL_COMMAND2 (cmdset, set_argv_list_10);
  INSTALL_COMMAND2 (cmdset, set_argv_list_11);
  INSTALL_COMMAND2 (cmdset, set_argv_list_12);
  INSTALL_COMMAND2 (cmdset, set_argv_list_13);
  INSTALL_COMMAND2 (cmdset, set_argv_list_14);
  INSTALL_COMMAND2 (cmdset, set_argv_list_15);
  INSTALL_COMMAND2 (cmdset, set_argv_list_16);
  INSTALL_COMMAND2 (cmdset, set_argv_list_17);
  INSTALL_COMMAND2 (cmdset, set_argv_list_18);
  INSTALL_COMMAND2 (cmdset, set_argv_list_19);
  INSTALL_COMMAND2 (cmdset, set_argv_list_20);
  INSTALL_COMMAND2 (cmdset, set_argv_list_21);
  INSTALL_COMMAND2 (cmdset, set_argv_list_22);
  INSTALL_COMMAND2 (cmdset, set_argv_list_23);
  INSTALL_COMMAND2 (cmdset, set_argv_list_24);
  INSTALL_COMMAND2 (cmdset, show_argv_list);

  INSTALL_COMMAND2 (cmdset, show_loop_count);
  INSTALL_COMMAND2 (cmdset, show_version);
  INSTALL_COMMAND2 (cmdset, show_rcu);
  INSTALL_COMMAND2 (cmdset, show_fdb);
  INSTALL_COMMAND2 (cmdset, show_vswitch);
  INSTALL_COMMAND2 (cmdset, sleep_cmd);
  INSTALL_COMMAND2 (cmdset, set_locale);
  INSTALL_COMMAND2 (cmdset, show_mempool);
  INSTALL_COMMAND2 (cmdset, show_neighbor);
  thread_info_cmd_init (cmdset);
  queue_config_cmd_init (cmdset);
  lthread_cmd_init (cmdset);
  tap_cmd_init (cmdset);
  rib_cmd_init (cmdset);
  dpdk_devbind_cmd_init (cmdset);
#ifdef ENABLE_PKTGEN
  pktgen_cmd_init (cmdset);
#endif

  nettlp_cmd_init (cmdset);
}

void
sdplane_init ()
{
  int lcore_id;
  debug_sdplane_cmd_init ();
  thread_info_init ();
}
