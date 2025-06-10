#include "include.h"
#include "sdplane_version_for_test.h"

#include <lthread.h>

#include <rte_common.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include <rte_mempool.h>
#include <rte_eal.h>
#include <rte_eth_ring.h>
#include <rte_mempool.h>

#include <sdplane/debug.h>
#include <sdplane/termio.h>
#include <sdplane/vector.h>
#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include <sdplane/log_cmd.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>

#include "l2_repeater.h"
#include "l3fwd.h"
#include "l3fwd_cmd.h"
#include "l2fwd_cmd.h"
#include "sdplane.h"
#include "debug_sdplane.h"

char msg[256];

void
test_lthread_main (void *arg)
{
  printf ("Hello, World!\n");

  char *rte_eal_argv[3] = { "sdplane", "-c", "0xf" };
  int rte_eal_argc = 3;

  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);

  int ret = rte_eal_init (rte_eal_argc, rte_eal_argv);
  if (ret < 0)
    rte_panic ("Cannot init EAL\n");

  int count_dev = rte_eth_dev_count_avail ();
  printf ("Available Ethernet devices: %d\n", count_dev);

  struct rte_ring *ring[18];
  int port0, port1;
  for (int i = 0; i < 18; i++)
    {
      char ring_name[8];
      snprintf (ring_name, sizeof (ring_name), "R%d", i);
      ring[i] = rte_ring_create (ring_name, 256, SOCKET_ID_ANY,
                                 RING_F_SP_ENQ | RING_F_SC_DEQ);
      if (! ring[i])
        {
          printf ("Failed to create ring %s: %s\n", ring_name,
                  rte_strerror (rte_errno));
          return;
        }
    }

  port0 = rte_eth_from_rings ("net_ring0", &ring[0], 1, &ring[1], 8, 0);
  port1 = rte_eth_from_rings ("net_ring1", &ring[9], 1, &ring[10], 8, 0);

  count_dev = rte_eth_dev_count_avail ();
  printf ("Available Ethernet devices after creating rings: %d\n", count_dev);
  printf ("Added port: %d, %d\n", port0, port1);

  debug_log_init ("sdplane");
  sdplane_init ();
  debug_zcmdsh_cmd_init ();
  command_shell_init ();

  printf ("EAL initialized, ports: %d, %d\n", port0, port1);
  apply_config ();
  printf ("applied config\n");

  lthread_sleep (500);
  printf ("Creating mbuf pool...\n");
  struct rte_mempool *mbuf_pool =
      rte_pktmbuf_pool_create ("test_mbuf_pool", 1024 * 8, 32, 0,
                               RTE_MBUF_DEFAULT_BUF_SIZE, SOCKET_ID_ANY);
  if (! mbuf_pool)
    {
      printf ("Failed to create mbuf pool: %s\n", rte_strerror (rte_errno));
      return;
    }
  struct rte_mbuf *mbuf = rte_pktmbuf_alloc (mbuf_pool);
  if (! mbuf)
    {
      printf ("Failed to allocate mbuf from pool: %s\n",
              rte_strerror (rte_errno));
      return;
    }


  printf ("Enqueuing message to R0...\n");
  mbuf->data_len = 13;
  mbuf->pkt_len = mbuf->data_len;
  memcpy (rte_pktmbuf_mtod (mbuf, void *), "Hello, R0!", 13);
  ret = rte_ring_enqueue (ring[0], mbuf);
  if (ret < 0)
    {
      printf ("Failed to enqueue message to R0: %s\n", rte_strerror (-ret));
      return;
    }
  printf ("Message enqueued to R0.\n");

  lthread_sleep (1000);
  printf ("Dequeuing message from R3...\n");
  struct rte_mbuf *received_mbuf;
  ret = rte_ring_dequeue (ring[12], &received_mbuf);
  if (ret < 0)
    {
      printf ("Failed to dequeue message from R3: %s\n", rte_strerror (-ret));
    }
  else
    {
      memcpy (msg, rte_pktmbuf_mtod (received_mbuf, void *),
              received_mbuf->data_len);
      printf ("Message dequeued from R3: %s\n", msg);
    }
}

int
apply_config ()
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

  char *config_file = "test_l2repeater.conf";
  printf ("%s[%d]: %s: opening %s.\n", __FILE__, __LINE__, __func__,
          config_file);
  int fd;
  int ret = 0;
  fd = open (config_file, O_RDONLY);
  printf ("%s[%d]: %s: opened %s.\n", __FILE__, __LINE__, __func__,
          config_file);
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
            __func__, config_file, strerror (errno));

  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);
  fflush (stdout);

  // termio_finish ();
  if (ret < 0)
    return ret;
  return 0;
}
