#include "include.h"

#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>

#include "sdplane.h"
//#include "tap_handler.h"

#include "../module/pktgen/app/pktgen.h"

extern pktgen_t pktgen;

CLI_COMMAND2 (show_pktgen,
              "show pktgen",
              SHOW_HELP,
              "pktgen information.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  int lcore_id;
  char *type;
  int running;

  if (! pktgen.l2p)
    {
      fprintf (t, "pktgen not configured.%s", shell->NL);
      return 0;
    }

  for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++)
    {
      switch (get_type (pktgen.l2p, lcore_id))
        {
        case RX_TYPE: type = "rx"; break;
        case TX_TYPE: type = "tx"; break;
        case (RX_TYPE | TX_TYPE): type = "rx/tx"; break;
        default: type = "none"; break;
        }

      running = pg_lcore_is_running (pktgen.l2p, lcore_id);
      fprintf (t, "lcore[%d]: pktgen type: %s running: %d%s",
               lcore_id, type, running, shell->NL);
    }
  return 0;
}

int _pktgen_main_init(int argc, char **argv);

CLI_COMMAND2 (pktgen_init,
              "pktgen init argv-list <0-7>",
              "pktgen\n",
              "init\n",
              "specify argv-list\n",
              "specify argv-list number\n")
{
  struct shell *shell = (struct shell *) context;

  int index;
  index = strtol (argv[3], NULL, 0);

  int *argcp = &argv_list_argc[index];
  char **argvp = argv_list[index];

  _pktgen_main_init (*argcp, argvp);
  return 0;
}

void
pktgen_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, show_pktgen);
  INSTALL_COMMAND2 (cmdset, pktgen_init);
}


