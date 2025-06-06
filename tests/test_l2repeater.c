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

  struct rte_ring *ring[4];
  int port0, port1;

  ring[0] = rte_ring_create ("R0", 256, 0, RING_F_SP_ENQ | RING_F_SC_DEQ);
  ring[1] = rte_ring_create ("R1", 256, 0, RING_F_SP_ENQ | RING_F_SC_DEQ);
  ring[2] = rte_ring_create ("R2", 256, 0, RING_F_SP_ENQ | RING_F_SC_DEQ);
  ring[3] = rte_ring_create ("R3", 256, 0, RING_F_SP_ENQ | RING_F_SC_DEQ);

  port0 = rte_eth_from_rings ("net_ring0", &ring[0], 1, &ring[1], 1, 0);
  port1 = rte_eth_from_rings ("net_ring1", &ring[2], 1, &ring[3], 1, 0);

  count_dev = rte_eth_dev_count_avail ();
  printf ("Available Ethernet devices after creating rings: %d\n", count_dev);
  printf ("Added port: %d, %d\n", port0, port1);

  debug_log_init ("sdplane");
  sdplane_init ();
  debug_zcmdsh_cmd_init ();
  command_shell_init ();

  printf ("EAL initialized, ports: %d, %d\n", port0, port1);
  apply_config ();
  printf ("applied config");

  lthread_sleep (500);

  printf ("Enqueuing message to R0...\n");
  ret = rte_ring_enqueue_burst (ring[0], (void *) "Hello from R0", 32, NULL);
  if (ret < 0)
    {
      printf ("Failed to enqueue message to R0: %s\n", rte_strerror (-ret));
      return;
    }
  printf ("Message enqueued to R0.\n");

  lthread_sleep (500);
  printf ("Dequeuing message from R3...\n");
  ret = rte_ring_dequeue_burst (ring[3], (void **) &msg, 32, NULL);
  if (ret == 0)
    {
      printf ("R3 is empty, no message dequeued.\n");
    }
  else
    {
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
