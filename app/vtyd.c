#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>

#include <lthread.h>

#include "sdplane/shell.h"
#include "sdplane/command.h"
#include "sdplane/command_shell.h"

#include "sdplane/debug.h"
#include "sdplane/debug_cmd.h"

#include "vtyd.h"
#include "vty_server.h"

#include "sdplane/log.h"
#include "sdplane/debug_log.h"
#include "sdplane/debug_category.h"
#include "sdplane/debug_zcmdsh.h"

const int lthread_core = 0;

bool force_quit = false;
bool force_stop[1] = { false };

void
signal_handler (int signum)
{
  printf ("%s:%d: %s: Signal %d received.\n",
          __FILE__, __LINE__, __func__, signum);

  if (signum == SIGINT)
    {
      /* do nothing. */
      printf ("Signal %d received.\n", signum);
    }
  else if (signum == SIGTERM)
    {
      printf ("Signal %d received, preparing to exit...\n", signum);
      force_quit = true;
    }
}

int
main (int argc, char **argv)
{
  lthread_t *lt = NULL;

  signal (SIGINT, signal_handler);
  signal (SIGTERM, signal_handler);
  signal (SIGHUP, signal_handler);

  /* Preserve name of myself. */
  char *progname, *p;
  progname = ((p = strrchr (argv[0], '/')) ? ++p : argv[0]);
  debug_log_init (progname);
  debug_output |= DEBUG_OUTPUT_STDOUT;
  DEBUG_SET (ZCMDSH, PAGER);
  DEBUG_SET (ZCMDSH, TELNET);

  debug_zcmdsh_cmd_init ();
  command_shell_init ();

  lthread_create (&lt, (lthread_func) vty_server, NULL);
  lthread_run ();

  return EXIT_SUCCESS;
}
