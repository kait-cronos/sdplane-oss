#include "include.h"

#include <rte_ethdev.h>
#include <rte_bus_pci.h>

#include <sdplane/debug.h>
#include <sdplane/termio.h>
#include <sdplane/vector.h>
#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>
#include <sdplane/debug_cmd.h>

#include "l3fwd.h"
#include "l2fwd_export.h"

#include "sdplane.h"
#include "sdplane_version.h"
#include "stat_collector.h"
#include "tap_handler.h"
#include "debug_sdplane.h"
#include "neigh_manager.h"

#include <lthread.h>
#include "thread_info.h"
#include "queue_config.h"

#include "rib.h"
#include "tap_cmd.h"

#include "dhcp_server.h"

CLI_COMMAND2 (init_dhcp_server,
              "init dhcp-server",
              "init\n",
              "dhcp-server.\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  if (! rte_eal_init_done)
    {
      fprintf (shell->terminal,
               "init dhcp-server must be called after rte_eal_init.\n");
      return -1;
    }
  dhcp_server_init ();
  return 0;
}

void
dhcp_cmd_init (struct command_set *cmdset)
{
  //INSTALL_COMMAND2 (cmdset, init_dhcp_server);
}
