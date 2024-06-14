
int lthread_main (__rte_unused void *dummy);
void soft_dplane_init ();

int
main (int argc, char **argv)
{
  soft_dplane_init ();
  lthread_main (NULL);

  return EXIT_SUCCESS;
}

