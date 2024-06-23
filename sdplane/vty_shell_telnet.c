#include <stdio.h>

#include <arpa/telnet.h>

#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>
#include <zcmdsh/command_shell.h>


/* Send WILL TELOPT_ECHO to remote server. */
void
vty_will_echo (struct shell *shell)
{
  char cmd[] = { IAC, WILL, TELOPT_ECHO, '\0' };
  fprintf (shell->terminal, "%s", cmd);
}

/* Make suppress Go-Ahead telnet option. */
void
vty_will_suppress_go_ahead (struct shell *shell)
{
  char cmd[] = { IAC, WILL, TELOPT_SGA, '\0' };
  fprintf (shell->terminal, "%s", cmd);
}

/* Make don't use linemode over telnet. */
void
vty_dont_linemode (struct shell *shell)
{
  char cmd[] = { IAC, DONT, TELOPT_LINEMODE, '\0' };
  fprintf (shell->terminal, "%s", cmd);
}

/* Use window size. */
void
vty_do_window_size (struct shell *shell)
{
  char cmd[] = { IAC, DO, TELOPT_NAWS, '\0' };
  fprintf (shell->terminal, "%s", cmd);
}


