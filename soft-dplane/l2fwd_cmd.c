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

#include "debug.h"
#include "termio.h"
#include "vector.h"
#include "shell.h"
#include "command.h"
#include "command_shell.h"
#include "debug_cmd.h"

#include "l2fwd.h"

#include "soft_dplane.h"

DEFINE_COMMAND (show_l2fwd_lcore,
                "show l2fwd lcore (|<0-16>|all)",
                SHOW_HELP
                "l2fwd\n"
                LCORE_HELP
                LCORE_NUMBER_HELP
               )
{
  struct shell *shell = (struct shell *) context;
  uint32_t nb_lcores;
  unsigned int rx_lcore_id;
  unsigned int lcore_spec = -1;
  struct lcore_queue_conf *qconf;
  int i;
  int portid;
  bool brief = false;

  if (argc == 3)
    brief = true;
  else if (strcmp (argv[3], "all"))
    lcore_spec = strtol (argv[3], NULL, 0);

  nb_lcores = rte_lcore_count ();
  for (rx_lcore_id = 0; rx_lcore_id < nb_lcores; rx_lcore_id++)
    {
      qconf = &lcore_queue_conf[rx_lcore_id];
      fprintf (shell->terminal, "lcore_queue_conf[%d]:\n", rx_lcore_id);
      fprintf (shell->terminal, "n_rx_port: %d:\n", qconf->n_rx_port);

      for (i = 0; i < qconf->n_rx_port; i++)
        {
          portid = qconf->rx_port_list[i];
          fprintf (shell->terminal, 
                   "rx_port_list[%d]: rxport %d txport: %d\n",
                   i, portid, l2fwd_dst_ports[portid]);
        }
    }
}

void
l2fwd_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, show_l2fwd_lcore);
}

