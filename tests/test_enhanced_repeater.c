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
#define LCORE_ID 1

int
test_vswitch (void *arg)
{
  struct rte_ether_hdr eth_hdr;
  struct rte_vlan_hdr vlan_hdr;
  const char *payload = "Hello, R0!";

  rte_ether_unformat_addr (&eth_hdr.src_addr, "00:11:22:33:44:55");
  rte_ether_unformat_addr (&eth_hdr.dst_addr, "00:66:77:88:99:AA");
  eth_hdr.ether_type = rte_cpu_to_be_16 (RTE_ETHER_TYPE_VLAN);
  vlan_hdr.vlan_tci = rte_cpu_to_be_16 (1234);
  vlan_hdr.eth_proto = rte_cpu_to_be_16 (RTE_ETHER_TYPE_IPV4);

  size_t packet_len = sizeof (eth_hdr) + sizeof (vlan_hdr) + strlen (payload);
  char packet[packet_len];
  memcpy (packet, &eth_hdr, sizeof (eth_hdr));
  memcpy (packet + sizeof (eth_hdr), &vlan_hdr, sizeof (vlan_hdr));
  memcpy (packet + sizeof (eth_hdr) + sizeof (vlan_hdr), payload,
          strlen (payload));

  SEND_TO_PORT (packet, packet_len, RX_PORT, 0);

  EXPECT_FROM_PORT (packet, packet_len, TX_PORT, LCORE_ID);

  return 0;
}

int
main (void)
{
  struct test_config configs[] = { {
      .name = "vswitch broadcast",
      .test_func = test_vswitch,
      .config_path = "config/vswitch.conf",
  } };
  return run_tests (configs, sizeof (configs) / sizeof (configs[0]));
}
