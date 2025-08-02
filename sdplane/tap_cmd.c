#include "include.h"

#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>

#include "sdplane.h"
#include "tap_handler.h"

CLI_COMMAND2 (set_tap_capture_ifname, "set tap capture ifname <WORD>",
              SET_HELP, "tun/tap I/F information.\n",
              "tun/tap I/F capture information.\n", "set tun/tap I/F name.\n",
              "Specify ifname string.\n")
{
  struct shell *shell = (struct shell *) context;
  memset (capture_ifname, 0, sizeof (capture_ifname));
  snprintf (capture_ifname, sizeof (capture_ifname), "%s", argv[4]);
  return 0;
}

CLI_COMMAND2 (set_tap_capture_persistent,
              "(set|no|unset) tap capture persistent", SET_HELP, NO_HELP,
              NO_HELP, "tun/tap I/F information.\n",
              "tun/tap I/F capture information.\n",
              "set tun/tap capture I/F persistent.\n", )
{
  struct shell *shell = (struct shell *) context;
  capture_if_persistent = 1;
  return 0;
}

void
tap_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, set_tap_capture_ifname);
  INSTALL_COMMAND2 (cmdset, set_tap_capture_persistent);
}
