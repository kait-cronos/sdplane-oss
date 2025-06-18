#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <string.h>
#include <sys/queue.h>
#include <stdarg.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>

#include <rte_common.h>
#include <rte_vect.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_random.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_udp.h>
#include <rte_string_fns.h>
#include <rte_cpuflags.h>
#include <rte_ethdev.h>

#include <sdplane/debug.h>
#include <sdplane/termio.h>
#include <sdplane/vector.h>
#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>
#include <sdplane/debug_cmd.h>

#include "l3fwd_cmd.h"
#include "l3fwd.h"
#include "sdplane.h"
#include "tap_handler.h"

DEFINE_COMMAND (set_l3fwd_vars_mask,
                "set l3fwd l3fwd_enabled_port_mask <0x0-0xffffffff>",
                SET_HELP
                "l3fwd\n"
                "l3fwd_enabled_port_mask\n"
                "mask\n")
{
  struct shell *shell = (struct shell *) context;
  uint32_t mask;
  mask = strtoul (argv[3], NULL, 0);
  enabled_port_mask = mask;
  return 0;
}

DEFINE_COMMAND (set_l3fwd_vars_config,
                "set l3fwd l3fwd_mapping_port_queue_lcore <WORD>",
                SET_HELP
                "l3fwd\n"
                "l3fwd_mapping_port_queue_lcore\n"
                "config_string .ex) (0,0,1),(1,0,2)\n")
{
  struct shell *shell = (struct shell *) context;
  int ret;

  ret = parse_config (argv[3]);
  if (ret < 0)
    {
      fprintf (shell->terminal, "Invalid config string: %s%s", argv[3],
               shell->NL);
      return -1;
    }

  return 0;
}

DEFINE_COMMAND (set_l3fwd_lookup_mode,
                "set l3fwd l3fwd_lookup_mode (em|lpm|fib|acl)",
                SET_HELP
                "l3fwd\n"
                "l3fwd_lookup_mode\n"
                "lookup_mode (em)\n"
                "lookup_mode (lpm)\n"
                "lookup_mode (fib)\n"
                "lookup_mode (acl)\n")
{
  struct shell *shell = (struct shell *) context;
  int ret;

  ret = parse_lookup (argv[3]);
  if (ret < 0)
    {
      fprintf (shell->terminal, "Invalid lookup mode: %s%s", argv[3],
               shell->NL);
      return -1;
    }
  return 0;
}

DEFINE_COMMAND (set_l3fwd_rule_ipv4,
                "set l3fwd l3fwd_rule_ipv4 <FILE>",
                SET_HELP
                "l3fwd\n"
                "l3fwd_rule_ipv4\n"
                "rule_name\n")
{
  struct shell *shell = (struct shell *) context;
  l3fwd_set_rule_ipv4_name (strdup (argv[3]));

  return 0;
}

DEFINE_COMMAND (set_l3fwd_rule_ipv6,
                "set l3fwd l3fwd_rule_ipv6 <FILE>",
                SET_HELP
                "l3fwd\n"
                "l3fwd_rule_ipv6\n"
                "rule_name\n")
{
  struct shell *shell = (struct shell *) context;
  l3fwd_set_rule_ipv6_name (strdup (argv[3]));

  return 0;
}

DEFINE_COMMAND (set_l3fwd_eth_dest,
                "set l3fwd l3fwd_eth_dest <WORD>",
                SET_HELP
                "l3fwd\n"
                "l3fwd_eth_dest\n"
                "eth_dest (ex) 0,00:11:22:33:44:55)\n")
{
  struct shell *shell = (struct shell *) context;
  parse_eth_dest (argv[3]);

  return 0;
}

DEFINE_COMMAND (l3fwd_init,
                "l3fwd init",
                "l3fwd\n"
                "init\n")
{
  struct shell *shell = (struct shell *) context;
  l3fwd_init (0, NULL, NULL);
  return 0;
}

void
l3fwd_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, set_l3fwd_vars_mask);
  INSTALL_COMMAND2 (cmdset, set_l3fwd_vars_config);
  INSTALL_COMMAND2 (cmdset, set_l3fwd_lookup_mode);
  INSTALL_COMMAND2 (cmdset, set_l3fwd_rule_ipv4);
  INSTALL_COMMAND2 (cmdset, set_l3fwd_rule_ipv6);
  INSTALL_COMMAND2 (cmdset, set_l3fwd_eth_dest);
  INSTALL_COMMAND2 (cmdset, l3fwd_init);
}
