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

  lthread_create (&lt, (lthread_func) lthread_main, NULL);
  lthread_run ();

  //l3fwd_terminate (argc, argv);
  return EXIT_SUCCESS;
}

