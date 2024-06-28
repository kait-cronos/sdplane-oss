#include <stdio.h>
#include <stdlib.h>
#include <lthread.h>
#include <signal.h>

#include <rte_common.h>

#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>

#include "module/l3fwd/l3fwd.h"

#include "sdplane.h"

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

  soft_dplane_init ();

  lthread_create (&lt, (lthread_func) lthread_main, NULL);
  lthread_run ();

  l3fwd_terminate (argc, argv);
  return EXIT_SUCCESS;
}
