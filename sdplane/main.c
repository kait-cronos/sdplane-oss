#include <stdlib.h>
#include <lthread.h>

#include <rte_common.h>

#include "module/l3fwd/l3fwd.h"
#include "sdplane.h"

int
main (int argc, char **argv)
{
  lthread_t *lt = NULL;

  soft_dplane_init ();

  //lthread_main (NULL);
  lthread_create (&lt, (lthread_func) lthread_main, NULL);
  lthread_run ();

#if 0
  if (l3fwd_argc)
    {
      int i;
      printf ("l3fwd_argv:");
      for (i = 0; i < l3fwd_argc; i++)
        printf (" %s", l3fwd_argv[i]);
      printf ("\n");
      l3fwd_init (l3fwd_argc, l3fwd_argv);
    }
  else
    l3fwd_init (argc, argv);
#endif
  //l3fwd_terminate (argc, argv);
  return EXIT_SUCCESS;
}

