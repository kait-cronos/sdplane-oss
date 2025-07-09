#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

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
#include <sdplane/command_shell.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/log_cmd.h>

#include "l3fwd.h"
#include "l3fwd_cmd.h"
#include "l2fwd_cmd.h"

#include "sdplane.h"
#include "thread_info.h"
#include "debug_sdplane.h"

#include "test_util.h"
#include "sdplane_version_for_test.h"

struct rte_mempool *test_mbuf_pool = NULL;

int test_ports[TEST_PORT_NUM];
struct rte_ring *test_rings[TEST_RING_NUM];
char *test_rte_eal_argv[4] = { "sdplane", "-c", "0xf", "--no-pci" };
int test_rte_eal_argc = 4;

int test_ret;

int
prepare_test (struct test_config *config)
{
  int dev_count = rte_eth_dev_count_avail ();

  test_mbuf_pool =
      rte_pktmbuf_pool_create ("test_mbuf_pool", 1024 * 8, 32, 0,
                               RTE_MBUF_DEFAULT_BUF_SIZE, SOCKET_ID_ANY);
  if (! test_mbuf_pool)
    {
      printf ("Failed to create mbuf pool: %s\n", rte_strerror (rte_errno));
      return 1;
    }


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
      test_ports[i] = rte_eth_from_rings (
          port_name, &test_rings[RXQ_TO_RING_IDX (i, 0)], TEST_NRXQ_NUM,
          &test_rings[TXQ_TO_RING_IDX (i, 0)], TEST_NTXQ_NUM, SOCKET_ID_ANY);
      if (test_ports[i] < 0)
        {
          printf ("Failed to create port %d: %s\n", i,
                  rte_strerror (rte_errno));
          return 1;
        }
      printf ("Created port %d with rx_ring=%d and tx_ring=%d-%d\n", i,
              RXQ_TO_RING_IDX (i, 0), TXQ_TO_RING_IDX (i, 0),
              TXQ_TO_RING_IDX (i, TEST_NTXQ_NUM - 1));
    }

  if (config->config_path)
    apply_config (config->config_path);
}

void
test_lthread_main (void *arg)
{
  struct test_config *config = (struct test_config *) arg;
  prepare_test (config);
  test_ret = config->test_func ();
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

  signal (SIGINT, signal_handler);
  signal (SIGTERM, signal_handler);
  signal (SIGHUP, signal_handler);

  debug_log_init (config->name);
  debug_zcmdsh_cmd_init ();
  command_shell_init ();
  sdplane_init ();

  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);

  int ret = rte_eal_init (test_rte_eal_argc, test_rte_eal_argv);
  if (ret < 0)
    rte_panic ("Cannot init EAL\n");

  lthread_create (&lt, (lthread_func) test_lthread_main, config);
  thread_register (-1, lt, (lthread_func) test_lthread_main,
                   "test_lthread_main", config);
  lthread_run ();

  // l3fwd_terminate (argc, argv);
  exit (test_ret);
}

int
apply_config (const char *config_path)
{
  struct shell *shell = NULL;

  shell = command_shell_create ();
  shell_set_prompt (shell, "startup-config> ");
  shell->pager = false;
  FLAG_UNSET (shell->flag, SHELL_FLAG_INTERACTIVE);

  // INSTALL_COMMAND2 (shell->cmdset, show_worker);
  INSTALL_COMMAND2 (shell->cmdset, start_stop_worker);

  INSTALL_COMMAND2 (shell->cmdset, debug_zcmdsh);
  // INSTALL_COMMAND2 (shell->cmdset, show_debug_zcmdsh);

  INSTALL_COMMAND2 (shell->cmdset, debug_sdplane);
  // INSTALL_COMMAND2 (shell->cmdset, show_debug_sdplane);

  INSTALL_COMMAND2 (shell->cmdset, l2fwd_init);

  log_cmd_init (shell->cmdset);
  l2fwd_cmd_init (shell->cmdset);
  l3fwd_cmd_init (shell->cmdset);
  sdplane_cmd_init (shell->cmdset);

  printf ("%s[%d]: %s: command set initialized.\n", __FILE__, __LINE__,
          __func__);

  // termio_init ();

  shell_clear (shell);
  shell_prompt (shell);

  printf ("%s[%d]: %s: opening %s.\n", __FILE__, __LINE__, __func__,
          config_path);
  int fd;
  int ret = 0;
  fd = open (config_path, O_RDONLY);
  printf ("%s[%d]: %s: opened %s.\n", __FILE__, __LINE__, __func__,
          config_path);
  if (fd >= 0)
    {
      shell_set_terminal (shell, fd, 1);
      while (shell_running (shell))
        {
          lthread_sleep (10); // yield.

          ret = shell_read_nowait (shell);
          if (ret < 0)
            {
              FLAG_SET (shell->flag, SHELL_FLAG_EXIT);
              DEBUG_SDPLANE_LOG (RIB, "shell_read_nowait: %d", ret);
              printf ("shell_read_nowait: %d\n", ret);
            }
        }
    }
  else
    printf ("%s[%d]: %s: opening %s: failed: %s.\n", __FILE__, __LINE__,
            __func__, config_path, strerror (errno));

  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);
  fflush (stdout);

  // termio_finish ();
  if (ret < 0)
    return ret;
  return 0;
}
