#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*HAVE_CONFIG_H*/

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
#ifdef HAVE_SDPLANE_LIBSDPLANE_VERSION_H
#include <sdplane/libsdplane_version.h>
#endif

#include "l3fwd.h"

#include "sdplane.h"
#include "sdplane_version.h"
#include "thread_info.h"

#include <unistd.h>
#include <getopt.h>

extern char *optarg;
extern int optind, opterr, optopt;

struct option longopts[] =
{
  { "version",     no_argument,       NULL, 'v' },
  { "config-file", required_argument, NULL, 'f' },
  { NULL,          no_argument,       NULL,  0  },
};
char *optstring = "vf:";

int opt;
int longindex;

extern char *config_file;

void
print_version ()
{
  FILE *f = stdout;
  fprintf (f, "libsdplane version: %s\n", libsdplane_version);
  fprintf (f, "sdplane version: %s\n", sdplane_version);
}

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

void
pid_file_lock (char *path)
{
  pid_t pid;
  int fd;
  char buf[32];
  char *p;
  int ret;

  pid = getpid ();
  fd = open (path, O_RDWR | O_CREAT | O_EXCL, 0644);
  if (fd < 0)
    {
      fd = open (path, O_RDONLY);
      if (fd < 0)
        fprintf (stderr, "Can't create pid lock file, exit.\n");
      else
        {
          ret = read (fd, buf, sizeof (buf));
          p = index (buf, '\n');
          if (p)
            *p = '\0';
          fprintf (stderr, "Another process(%s) running, exit.\n", buf);
        }
      exit (-1);
    }

  snprintf (buf, sizeof (buf), "%d\n", (int) pid);
  ret = write (fd, buf, strlen (buf));
  close (fd);
}

void
pid_file_without_lock (char *path)
{
  pid_t pid;
  int fd;
  char buf[32];
  char *p;
  int ret;

  pid = getpid ();
  fd = open (path, O_RDWR | O_CREAT | O_EXCL, 0644);
  if (fd < 0)
    {
      fd = open (path, O_RDONLY);
      if (fd < 0)
        fprintf (stderr, "Can't create pid lock file.\n");
      else
        {
          ret = read (fd, buf, sizeof (buf));
          p = index (buf, '\n');
          if (p)
            *p = '\0';
          fprintf (stderr, "Overriding the pid-file by process(%s).\n", buf);
        }
    }

  if (fd < 0)
    {
      fd = open (path, O_RDWR, 0644);
      if (fd < 0)
        {
          fprintf (stderr, "Can't override the pid-file.\n");
          return;
        }
    }

  snprintf (buf, sizeof (buf), "%d\n", (int) pid);
  ret = write (fd, buf, strlen (buf));
  close (fd);
}

char *pid_path = "/var/run/sdplane.pid";

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
        case 'v':
          print_version ();
          exit (0);
          break;
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

  pid_file_without_lock (pid_path);

  debug_log_init (progname);
  sdplane_init ();

  lthread_create (&lt, (lthread_func) lthread_main, NULL);
  thread_register (-1, lt, (lthread_func) lthread_main, "lthread_main", NULL);
  lthread_run ();

  // l3fwd_terminate (argc, argv);
  unlink (pid_path);
  return EXIT_SUCCESS;
}
