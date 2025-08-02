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

#include "l2fwd_export.h"
#include "l2fwd_cmd.h"

#include "tap_handler.h"

DEFINE_COMMAND (set_l2fwd_vars_mask,
                "set l2fwd l2fwd_enabled_port_mask <0x0-0xffffffff>",
                SET_HELP
                "l2fwd\n"
                "l2fwd_enabled_port_mask\n"
                "mask\n")
{
  struct shell *shell = (struct shell *) context;
  uint32_t mask;
  mask = strtoul (argv[3], NULL, 0);
  l2fwd_enabled_port_mask = mask;
  return 0;
}

DEFINE_COMMAND (set_l2fwd_vars_integer,
                "set l2fwd l2fwd_rx_queue_per_lcore <0-32764>",
                SET_HELP
                "l2fwd\n"
                "l2fwd_rx_queue_per_lcore\n"
                "integer\n")
{
  struct shell *shell = (struct shell *) context;
  uint32_t val;
  val = strtoul (argv[3], NULL, 0);
  l2fwd_rx_queue_per_lcore = val;
  return 0;
}

DEFINE_COMMAND (show_l2fwd_vars,
                "show l2fwd vars "
                "(|all|l2fwd_enabled_port_mask|l2fwd_rx_queue_per_lcore)",
                SHOW_HELP
                "l2fwd\n"
                VARS_HELP
                ALL_HELP
                "l2fwd_enabled_port_mask\n"
                "l2fwd_rx_queue_per_lcore\n"
               )
{
  struct shell *shell = (struct shell *) context;
  bool brief = false;
  bool all = false;
  if (argc == 3)
    brief = true;
  else if (! strcmp (argv[3], "all"))
    all = true;
  if (brief || all || ! strcmp (argv[3], "l2fwd_enabled_port_mask"))
    fprintf (shell->terminal, "l2fwd_enabled_port_mask = %#x%s",
             l2fwd_enabled_port_mask, shell->NL);
  if (brief || all || ! strcmp (argv[3], "l2fwd_enabled_port_mask"))
    fprintf (shell->terminal, "l2fwd_rx_queue_per_lcore = %d%s",
             l2fwd_rx_queue_per_lcore, shell->NL);
  return 0;
}

DEFINE_COMMAND (show_l2fwd_all,
                "show l2fwd (|all)",
                SHOW_HELP
                "l2fwd\n"
                ALL_HELP
               )
{
  struct shell *shell = (struct shell *) context;
  bool brief = false;
  bool all = false;
  if (argc == 2)
    brief = true;
  else if (! strcmp (argv[2], "all"))
    all = true;

  fprintf (shell->terminal, "l2fwd_enabled_port_mask = %#x%s",
           l2fwd_enabled_port_mask, shell->NL);
  fprintf (shell->terminal, "l2fwd_rx_queue_per_lcore = %d%s",
           l2fwd_rx_queue_per_lcore, shell->NL);
  show_l2fwd_lcore_by_mask (shell, true, true, 0);
  return 0;
}

void
show_l2fwd_lcore_one (struct shell *shell, unsigned int rx_lcore_id)
{
  struct lcore_queue_conf *qconf;
  int i;
  int portid;

  qconf = &lcore_queue_conf[rx_lcore_id];
  fprintf (shell->terminal, "l2fwd lcore[%d]: lcore_queue_conf:%s",
           rx_lcore_id, shell->NL);

  fprintf (shell->terminal, "    n_rx_port: %d:%s", qconf->n_rx_port, shell->NL);
  for (i = 0; i < qconf->n_rx_port; i++)
    {
      portid = qconf->rx_port_list[i];
      fprintf (shell->terminal,
               "    rx_port_list[%d]: rxport %d txport: %d%s",
               i, portid, l2fwd_dst_ports[portid], shell->NL);
    }
}

void
show_l2fwd_lcore_by_mask (struct shell *shell,
                          bool brief, bool all, uint64_t mask)
{
  uint32_t nb_lcores;
  unsigned int rx_lcore_id;

  nb_lcores = rte_lcore_count ();
  for (rx_lcore_id = 0; rx_lcore_id < nb_lcores; rx_lcore_id++)
    {
      if (! all && ! (mask & (1ULL << rx_lcore_id)))
        continue;

      show_l2fwd_lcore_one (shell, rx_lcore_id);
    }
}

DEFINE_COMMAND (show_l2fwd_lcore,
                "show l2fwd lcore (|<0-16>|all)",
                SHOW_HELP
                "l2fwd\n"
                LCORE_HELP
                LCORE_NUMBER_HELP
                ALL_HELP
               )
{
  struct shell *shell = (struct shell *) context;
  unsigned int lcore_spec = -1;
  bool brief = false;
  bool all = false;
  uint64_t mask = 0xffffffffffffffff;

  if (argc == 3)
    brief = true;
  else if (! strcmp (argv[3], "all"))
    all = true;
  else
    {
      lcore_spec = strtol (argv[3], NULL, 0);
      mask = (1ULL << lcore_spec);
    }

  show_l2fwd_lcore_by_mask (shell, brief, all, mask);
  return 0;
}

DEFINE_COMMAND (show_l2fwd_stats,
                "show l2fwd stats",
                SHOW_HELP
                "l2fwd\n"
                "stats\n"
               )
{
  struct shell *shell = (struct shell *) context;
  print_stats ();
  return 0;
}

DEFINE_COMMAND (l2fwd_init,
               "l2fwd init",
               "l2fwd\n"
               "init\n")
{
  struct shell *shell = (struct shell *) context;
  l2fwd_init (0, NULL);
  return 0;
}

void
l2fwd_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, l2fwd_init);

  INSTALL_COMMAND2 (cmdset, show_l2fwd_lcore);
  INSTALL_COMMAND2 (cmdset, show_l2fwd_vars);
  INSTALL_COMMAND2 (cmdset, show_l2fwd_all);
  INSTALL_COMMAND2 (cmdset, set_l2fwd_vars_mask);
  INSTALL_COMMAND2 (cmdset, set_l2fwd_vars_integer);
  INSTALL_COMMAND2 (cmdset, show_l2fwd_stats);
}

