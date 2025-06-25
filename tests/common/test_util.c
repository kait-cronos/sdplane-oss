#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <lthread.h>

#include <rte_common.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include <rte_mempool.h>
#include <rte_eal.h>
#include <rte_eth_ring.h>
#include <rte_mempool.h>

#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>

#include "l3fwd.h"

#include "sdplane.h"
#include "thread_info.h"

#include "test_util.h"

int test_ports[TEST_PORT_NUM];
struct rte_ring *test_rings[TEST_RING_NUM];
char *test_rte_eal_argv[4] = { "sdplane", "-c", "0xf", "--no-pci" };
int test_rte_eal_argc = 4;

int (*test_func) ();
int test_ret;

int prepare_test ()
{
  int dev_count = rte_eth_dev_count_avail ();
  printf ("Available Ethernet devices: %d\n", dev_count);
    for (int i = 0; i < TEST_RING_NUM; i++)
    {
      char ring_name[4];
      snprintf (ring_name, sizeof (ring_name), "R%d", i);
      test_rings[i] = rte_ring_create (ring_name, 256, SOCKET_ID_ANY,
                                 RING_F_SP_ENQ | RING_F_SC_DEQ);
      if (! test_rings[i])
        {
          printf ("Failed to create ring %s: %s\n", ring_name,
                  rte_strerror (rte_errno));
          return 1;
        }
    }

    for (int i = 0; i < TEST_PORT_NUM; i++)
    {
      char port_name[2];
      snprintf (port_name, sizeof (port_name), "%d", i);
      test_ports[i] = rte_eth_from_rings(port_name, &test_rings[RXQ_TO_RING_IDX(i, 0)],
                                         TEST_NRXQ_NUM, &test_rings[TXQ_TO_RING_IDX(i, 0)],
                                         TEST_NTXQ_NUM, SOCKET_ID_ANY);
      if (test_ports[i] < 0)
        {
          printf ("Failed to create port %d: %s\n", i, rte_strerror (rte_errno));
          return 1;
        }
      printf ("Created port %d with rx_ring=%d and tx_ring=%d-%d\n",
        i, RXQ_TO_RING_IDX(i, 0), TXQ_TO_RING_IDX(i, 0), TXQ_TO_RING_IDX(i, TEST_NTXQ_NUM - 1));
    }
}

void
test_lthread_main (void *arg)
{
  prepare_test();
  test_ret = test_func ();
  force_quit = true;
}

void
signal_handler (int signum)
{
  force_quit = true;
}

int
run_test (struct test_config *config)
{
  lthread_t *lt = NULL;
  test_func = config->test_f;

  signal (SIGINT, signal_handler);
  signal (SIGTERM, signal_handler);
  signal (SIGHUP, signal_handler);

  debug_log_init (config->name);

  sdplane_init ();

  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);

  int ret = rte_eal_init (test_rte_eal_argc, test_rte_eal_argv);
  if (ret < 0)
    rte_panic ("Cannot init EAL\n");

  lthread_create (&lt, (lthread_func) test_lthread_main, NULL);
  thread_register (-1, lt, (lthread_func) test_lthread_main,
                   "test_lthread_main", NULL);
  lthread_run ();

  // l3fwd_terminate (argc, argv);
  exit (test_ret);
}
