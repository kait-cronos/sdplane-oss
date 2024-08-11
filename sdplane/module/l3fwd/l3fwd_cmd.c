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

#include <zcmdsh/debug.h>
#include <zcmdsh/termio.h>
#include <zcmdsh/vector.h>
#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>
#include <zcmdsh/command_shell.h>
#include <zcmdsh/debug_cmd.h>

#include "l2fwd_export.h"
#include "l2fwd_cmd.h"

#include "l3fwd.h"
#include "sdplane.h"

#include "tap_handler.h"

DEFINE_COMMAND (l3fwd_init,
               "l3fwd init",
               "l3fwd\n"
               "init\n")
{
  struct shell *shell = (struct shell *) context;
  int i;
  for (i = 0; i < l3fwd_argc; i++)
    fprintf (shell->terminal, "l3fwd_argv[%d]: %s\n", i, l3fwd_argv[i]);
  l3fwd_init (l3fwd_argc, l3fwd_argv, NULL);
}

void
l3fwd_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, l3fwd_init);
}

