#include <stdlib.h>

#include <rte_common.h>

void soft_dplane_init ();
int l3fwd_init (int argc, char **argv);
int lthread_main (__rte_unused void *dummy);
int l3fwd_terminate ();

int
main (int argc, char **argv)
{
  soft_dplane_init ();
  l3fwd_init (argc, argv);
  lthread_main (NULL);
  l3fwd_terminate ();
  return EXIT_SUCCESS;
}

