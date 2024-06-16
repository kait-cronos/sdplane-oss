#include <stdlib.h>

#include <rte_common.h>

#include "module/l3fwd/l3fwd.h"

void soft_dplane_init ();
int lthread_main (__rte_unused void *dummy);

int
main (int argc, char **argv)
{
  soft_dplane_init ();
  l3fwd_init (argc, argv);
  lthread_main (NULL);
  l3fwd_terminate (argc, argv);
  return EXIT_SUCCESS;
}

