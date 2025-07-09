#include "include.h"

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

#include "test_util.h"

char msg[256];

#define RX_PORT 0
#define TX_PORT 1
#define LCORE_ID 2

int
test_l2_repeater (void *arg)
{
  int ret;

  struct rte_mbuf *mbuf = rte_pktmbuf_alloc (test_mbuf_pool);
  if (! mbuf)
    {
      printf ("Failed to allocate mbuf from pool: %s\n",
              rte_strerror (rte_errno));
      return -1;
    }

  mbuf->data_len = 13;
  mbuf->pkt_len = mbuf->data_len;
  memcpy (rte_pktmbuf_mtod (mbuf, void *), "Hello, R0!", 13);
  ret = rte_ring_enqueue (test_rings[RXQ_TO_RING_IDX (RX_PORT, 0)], mbuf);
  if (ret < 0)
    {
      printf ("Failed to enqueue message to R0: %s\n", rte_strerror (-ret));
      return -1;
    }
  printf ("Message enqueued to R0.\n");

  lthread_sleep (1000);
  printf ("Dequeuing message from R3...\n");
  struct rte_mbuf *received_mbuf;
  ret = rte_ring_dequeue (test_rings[TXQ_TO_RING_IDX (TX_PORT, LCORE_ID)], &received_mbuf);
  if (ret < 0)
    {
      printf ("Failed to dequeue message from R3: %s\n", rte_strerror (-ret));
      return -1;
    }
  else
    {
      memcpy (msg, rte_pktmbuf_mtod (received_mbuf, void *),
              received_mbuf->data_len);
      printf ("Message dequeued from R3: %s\n", msg);

      if (! strcmp (msg, "Hello, R0!"))
        {
          printf ("Message content matches expected: %s\n", msg);
          return 0;
        }
      else
        {
          printf ("Message content does not match expected: %s\n", msg);
          return -1;
        }
    }
}

int
main ()
{
  struct test_config config = {
    .name = "test_l2_repeater",
    .test_f = test_l2_repeater,
    .config_path = "test_l2repeater.conf",
  };
  run_test (&config);
}
