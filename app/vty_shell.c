#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/param.h>
#include <limits.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <arpa/telnet.h>

#include <ifaddrs.h>

#include <lthread.h>

#include "sdplane/shell.h"
#include "sdplane/shell_keyfunc.h"
#include "sdplane/command.h"
#include "sdplane/command_shell.h"
#include "sdplane/debug.h"
#include "sdplane/debug_cmd.h"
#include "sdplane/log_cmd.h"

#include "sdplane/debug_log.h"
#include "sdplane/debug_category.h"
#include "sdplane/debug_zcmdsh.h"

#include "vtyd.h"
#include "vty_server.h"
#include "vty_shell.h"
#include "sdplane/shell_telnet.h"

int
shell_keyfunc_clear_terminal (struct shell *shell)
{
  const char clr[] = { 27, '[', '2', 'J', '\0' };
  const char topLeft[] = { 27, '[', '1', ';', '1', 'H', '\0' };
  /* Clear screen and move to top left */
  fprintf (shell->terminal, "%s%s", clr, topLeft);
  fflush (shell->terminal);
  shell_refresh (shell);
  return 0;
}

DEFINE_COMMAND (clear_cmd,
                "clear",
                "clear terminal\n")
{
  struct shell *shell = (struct shell *) context;
  shell_keyfunc_clear_terminal (shell);
  return CMD_SUCCESS;
}

#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/utsname.h>

void
vty_banner (struct shell *shell)
{
  fprintf (shell->terminal, "welcome to vty_shell.%s", shell->NL);
  fflush (shell->terminal);
}

DEFINE_COMMAND (vty_exit_cmd,
                "(exit|quit|logout)",
                "exit\n"
                "quite\n"
                "logout\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "vty exit !%s", shell->NL);
  FLAG_SET (shell->flag, SHELL_FLAG_EXIT);
  /* don't shell_close(): this closes stdout. */
  //shell_close (shell);
}

DEFINE_COMMAND (list,
               "(ls|list) (|<FILENAME>)",
               "list files in the directory.\n"
               "list files in the directory.\n"
               "directory name\n")
{
  struct shell *shell = (struct shell *) context;
  char buf[MAXPATHLEN], dir[MAXPATHLEN + 8];
  char *p;
  if (argc > 1)
    snprintf (buf, sizeof (buf), "%s", argv[1]);
  else
    {
      p = getcwd (buf, sizeof (buf));
      if (! p)
        {
          fprintf (shell->terminal, "getcwd() failed: %s\n",
                   strerror (errno));
          return CMD_FAILURE;
        }
    }
  snprintf (dir, sizeof (dir), "%s/", buf);
  fprintf (shell->terminal, "dir: %s\n", dir);
  file_ls_candidate (shell, dir);
  return CMD_SUCCESS;
}

DEFINE_COMMAND (shutdown_cmd,
                "shutdown",
                "shutdown\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "shutdown !%s", shell->NL);
  FLAG_SET (shell->flag, SHELL_FLAG_EXIT);
  force_quit = true;
}

void
vty_shell (void *arg)
{
  vty_client_t *client = (vty_client_t *) arg;
  struct shell *shell;

  lthread_detach ();

  char client_addr_str[128];
  inet_ntop (AF_INET, &client->peer_addr.sin_addr,
             client_addr_str, sizeof (client_addr_str));
  printf ("%s[%d]: client[%d]: %s.\n",
          __func__, client->id, client->id, client_addr_str);

  char prompt[64];
  snprintf (prompt, sizeof (prompt), "vty[%d]> ", client->id);

  shell = command_shell_create ();
  shell_set_terminal (shell, client->fd, client->fd);
  shell_set_prompt (shell, prompt);
  //get_winsize (shell);
  shell->NL = "\r\n";

  shell_escape_keyfunc_init (shell);
  shell_telnet_keyfunc_init (shell);

  log_cmd_init (shell->cmdset);
  INSTALL_COMMAND2 (shell->cmdset, vty_exit_cmd);

  INSTALL_COMMAND2 (shell->cmdset, debug_zcmdsh);
  INSTALL_COMMAND2 (shell->cmdset, show_debug_zcmdsh);

  //INSTALL_COMMAND2 (shell->cmdset, clear_cmd);
  shell_install (shell, CONTROL ('L'), shell_keyfunc_clear_terminal);
  FUNC_STR_REGISTER (shell_keyfunc_clear_terminal);

  INSTALL_COMMAND2 (shell->cmdset, list);
  INSTALL_COMMAND2 (shell->cmdset, shutdown_cmd);

  FLAG_SET (shell->debug_zcmdsh, DEBUG_TYPE (ZCMDSH, TELNET));

  vty_will_echo (shell);
  vty_will_suppress_go_ahead (shell);
  vty_dont_linemode (shell);
  vty_do_window_size (shell);

  shell_clear (shell);
  vty_banner (shell);
  shell_prompt (shell);

  while (! force_quit && ! force_stop[lthread_core] &&
         shell_running (shell))
    {
      lthread_sleep (100); // yield.
      if (shell->is_paging)
        shell_read_nowait_paging (shell);
      else
        shell_read_nowait (shell);
    }

  printf ("%s[%d]: %s: terminating for client[%d]: %s.\n",
          __FILE__, __LINE__, __func__,
          client->id, client_addr_str);

  lthread_close (client->fd);
  client->fd = -1;

  return;
}

