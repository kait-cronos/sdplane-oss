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
#include "tap_handler.h"

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

#define L3FWD_ARGV_MAX 16
char *l3fwd_argv[L3FWD_ARGV_MAX];
int l3fwd_argc;

DEFINE_COMMAND (set_l3fwd_argv,
                "set l3fwd argv <WORD>[14]",
                SET_HELP
                "set l3fwd-related information.\n"
                "set command-line arguments.\n"
                "arbitrary word\n")
{
  struct shell *shell = (struct shell *) context;
  int i;
  char **t_argv;
  int t_argc = 0;

  if (argc - 2 >= L3FWD_ARGV_MAX - 2)
    {
      fprintf (shell->terminal, "too many arguments: %d.\n", argc);
      return;
    }

  t_argc = argc - 2;
  t_argv = &argv[2];

  l3fwd_argc = 0;
  memset (l3fwd_argv, 0, sizeof (l3fwd_argv));
  l3fwd_argv[l3fwd_argc++] = "sdplane";

  while (t_argc)
    {
      l3fwd_argv[l3fwd_argc++] = *t_argv;
      t_argv++;
      t_argc--;
    }

  fprintf (shell->terminal, "l3fwd_argv[%d]:", l3fwd_argc);
  for (i = 0; i < l3fwd_argc; i++)
    {
      fprintf (shell->terminal, " %s", l3fwd_argv[i]);
    }
  fprintf (shell->terminal, "\n");
}

void dpdk_lcore_cmd_init (struct command_set *cmdset);
void dpdk_port_cmd_init (struct command_set *cmdset);

void
soft_dplane_cmd_init (struct command_set *cmdset)
{
  setlocale (LC_ALL, "en_US.utf8");
  dpdk_lcore_cmd_init (cmdset);
  dpdk_port_cmd_init (cmdset);
}

extern struct rte_ring *tap_ring_by_lcore[RTE_MAX_LCORE];

void
soft_dplane_init ()
{
  int lcore_id;
  for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++)
    tap_ring_by_lcore[lcore_id] = NULL;
}

