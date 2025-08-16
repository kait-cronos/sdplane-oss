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
#include "common/test_assert.h"

char msg[256];

#define RX_PORT  0
#define TX_PORT  1
#define LCORE_ID 2

int
test_l2_repeater (void *arg)
{
  const char *payload = "Hello, R0!";
  size_t payload_len = strlen (payload);

  SEND_TO_PORT (payload, payload_len, RX_PORT, 0);

  EXPECT_FROM_PORT (payload, payload_len, TX_PORT, LCORE_ID);

  return 0;
}

int
main (void)
{
  struct test_config configs[] = { {
      .name = "l2_repeater repeats any data to other ports",
      .test_func = test_l2_repeater,
      .config_path = "config/l2_repeater.conf",
  } };
  return run_tests (configs, sizeof (configs) / sizeof (configs[0]));
}
