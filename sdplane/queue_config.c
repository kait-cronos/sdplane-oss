#include "include.h"

#include <rte_ethdev.h>
#include <rte_bus_pci.h>

#include <zcmdsh/debug.h>
#include <zcmdsh/termio.h>
#include <zcmdsh/vector.h>
#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>
#include <zcmdsh/command_shell.h>
#include <zcmdsh/debug_cmd.h>

#include "sdplane.h"

CLI_COMMAND2 (set_thread_lcore_port_queue,
              "set thread <0-128> port <0-128> queue <0-128>",
              SET_HELP,
              "thread-information\n",
              "thread-id (lcore-id)\n",
              PORT_HELP,
              PORT_NUMBER_HELP,
              QUEUE_HELP,
              QUEUE_NUMBER_HELP)
{
  struct shell *shell = (struct shell *) context;
  uint16_t lcore_id, port_id, queue_id;
  lcore_id = (uint16_t) strtol (argv[2], NULL, 0);
  port_id = (uint16_t) strtol (argv[4], NULL, 0);
  queue_id = (uint16_t) strtol (argv[6], NULL, 0);
  fprintf (shell->terminal, "lcore: %d port: %d rx-queue: %d%s",
           lcore_id, port_id, queue_id, shell->NL);
}

void
queue_config_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, set_thread_lcore_port_queue);
}

