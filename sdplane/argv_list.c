#include "include.h"

#include <sdplane/vector.h>
#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>

#include <pthread.h>

#include "argv_list.h"
#include "sdplane.h"

pthread_mutex_t argv_list_mutex;

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

  pthread_mutex_lock (&argv_list_mutex);
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
  pthread_mutex_unlock (&argv_list_mutex);

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

CLI_COMMAND2 (show_argv_list,
              "show argv-list (|<0-7>)",
              SHOW_HELP,
              "show argv-list.\n",
              "specify argv-list index.\n")
{
  struct shell *shell = (struct shell *) context;
  int i, j;

  int index = -1;
  if (argc > 2)
    index = strtol (argv[2], NULL, 0);

  pthread_mutex_lock (&argv_list_mutex);
  for (i = 0; i < ARGV_LIST_MAX; i++)
    {
      int *argcp = &argv_list_argc[i];
      char **argvp = argv_list[i];

      if (index >= 0 && i != index)
        continue;

      for (j = 0; j < *argcp; j++)
        fprintf (shell->terminal, "argv_list[%d][%d]: %s%s",
                 i, j, argv_list[i][j], shell->NL);
    }
  pthread_mutex_unlock (&argv_list_mutex);

  return 0;
}

void
argv_list_cmd_init (struct command_set *cmdset)
{
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
}

void
argv_list_init ()
{
  pthread_mutex_init (&argv_list_mutex, NULL);
}
