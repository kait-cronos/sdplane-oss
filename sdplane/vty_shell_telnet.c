#include <stdio.h>

#include <arpa/telnet.h>

#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>
#include <zcmdsh/command_shell.h>

#include <zcmdsh/debug.h>
#include <zcmdsh/debug_cmd.h>
#include <zcmdsh/debug_module.h>
#include <zcmdsh/debug_module_cmd.h>
#include "debug_sdplane.h"

/* Send WILL TELOPT_ECHO to remote server. */
void
vty_will_echo (struct shell *shell)
{
  char cmd[] = { IAC, WILL, TELOPT_ECHO, '\0' };
  fprintf (shell->terminal, "%s", cmd);
  if (FLAG_CHECK (debug_module_config[debug_module_sdplane],
                  DEBUG_SDPLANE_TELNET_OPT))
    fprintf (shell->terminal, "IAC WILL TELOPT_ECHO%s", shell->LF);
  fflush (shell->terminal);
}

/* Make suppress Go-Ahead telnet option. */
void
vty_will_suppress_go_ahead (struct shell *shell)
{
  char cmd[] = { IAC, WILL, TELOPT_SGA, '\0' };
  fprintf (shell->terminal, "%s", cmd);
  if (FLAG_CHECK (debug_module_config[debug_module_sdplane],
                  DEBUG_SDPLANE_TELNET_OPT))
    fprintf (shell->terminal, "IAC WILL TELOPT_SGA%s", shell->LF);
  fflush (shell->terminal);
}

/* Make don't use linemode over telnet. */
void
vty_dont_linemode (struct shell *shell)
{
  char cmd[] = { IAC, DONT, TELOPT_LINEMODE, '\0' };
  fprintf (shell->terminal, "%s", cmd);
  if (FLAG_CHECK (debug_module_config[debug_module_sdplane],
                  DEBUG_SDPLANE_TELNET_OPT))
    fprintf (shell->terminal, "IAC DONT TELOPT_LINEMODE%s", shell->LF);
  fflush (shell->terminal);
}

/* Use window size. */
void
vty_do_window_size (struct shell *shell)
{
  char cmd[] = { IAC, DO, TELOPT_NAWS, '\0' };
  fprintf (shell->terminal, "%s", cmd);
  if (FLAG_CHECK (debug_module_config[debug_module_sdplane],
                  DEBUG_SDPLANE_TELNET_OPT))
    fprintf (shell->terminal, "IAC DO TELOPT_NAWS%s", shell->LF);
  fflush (shell->terminal);
}


