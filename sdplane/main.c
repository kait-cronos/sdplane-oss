#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <lthread.h>

#include <rte_common.h>

#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>
#include <zcmdsh/debug_log.h>
#include <zcmdsh/debug_category.h>
#include <zcmdsh/debug_zcmdsh.h>

#include "l3fwd.h"

#include "sdplane.h"
#include "thread_info.h"

void
signal_handler (int signum)
{
  printf ("%s:%d: %s: Signal %d received.\n", __FILE__, __LINE__, __func__,
          signum);

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

  sdplane_init ();

  lthread_create (&lt, (lthread_func) lthread_main, NULL);
  thread_register (-1, lt, lthread_main, "lthread_main", NULL);
  lthread_run ();

  //l3fwd_terminate (argc, argv);
  return EXIT_SUCCESS;
}
