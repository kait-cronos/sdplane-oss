#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <lthread.h>

#include <rte_common.h>

#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>

#include "l3fwd.h"

#include "sdplane.h"
#include "thread_info.h"

#include <unistd.h>
#include <getopt.h>

extern char *optarg;
extern int optind, opterr, optopt;

struct option longopts[] =
{
  { "config-file", required_argument, NULL, 'f' },
  { NULL,          no_argument,       NULL,  0  },
};
char *optstring = "f:";

int opt;
int longindex;

extern char *config_file;

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

  while ((opt = getopt_long (argc, argv, optstring,
                             longopts, &longindex)) != -1)
    {
      switch (opt)
        {
        case 'f':
          config_file = optarg;
          break;
        default:
          fprintf (stderr, "unknown option: \'%c\' optopt: \'%c\'\n",
                   opt, optopt);
          exit (-1);
          break;
        }
    }

  debug_log_init (progname);
  sdplane_init ();

  lthread_create (&lt, (lthread_func) lthread_main, NULL);
  thread_register (-1, lt, (lthread_func) lthread_main, "lthread_main", NULL);
  lthread_run ();

  // l3fwd_terminate (argc, argv);
  return EXIT_SUCCESS;
}
