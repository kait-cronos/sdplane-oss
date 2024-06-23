#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <arpa/telnet.h>

#include <rte_common.h>
#include <rte_launch.h>

#include <lthread.h>

#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>
#include <zcmdsh/command_shell.h>
#include <zcmdsh/debug_cmd.h>
#include <zcmdsh/debug_module.h>
#include <zcmdsh/debug_module_cmd.h>

#include "sdplane.h"
#include "l2fwd_cmd.h"
#include "l3fwd_cmd.h"

#include "vty_server.h"
#include "vty_shell.h"
#include "vty_shell_telnet.h"

#include "debug_sdplane.h"

shell_keyfunc_t key_func_escape_1[256];
shell_keyfunc_t key_func_escape_2[256];
shell_keyfunc_t key_func_iac[256];
shell_keyfunc_t key_func_subnego[256];

shell_keyfunc_t *key_func_orig;

void
vty_shell_keyfunc_escape_1 (struct shell *shell)
{
  shell->key_func = key_func_escape_1;
}

void
vty_shell_keyfunc_escape_2 (struct shell *shell)
{
  shell->key_func = key_func_escape_2;
}

void
vty_shell_keyfunc_normal (struct shell *shell)
{
  shell->key_func = key_func_orig;
}

void
vty_shell_keyfunc_none (struct shell *shell)
{
}

void
vty_shell_move_word_backward (struct shell *shell)
{
  shell_move_word_backward (shell);
  vty_shell_keyfunc_normal (shell);
}

void
vty_shell_move_word_forward (struct shell *shell)
{
  shell_move_word_forward (shell);
  vty_shell_keyfunc_normal (shell);
}

void
vty_shell_delete_word_backward (struct shell *shell)
{
  shell_delete_word_backward (shell);
  vty_shell_keyfunc_normal (shell);
}

void
vty_shell_keyfunc_iac_start (struct shell *shell)
{
  shell->key_func = key_func_iac;
  fprintf (shell->terminal, "%s: IAC received.%s", __func__, shell->LF);
  fflush (shell->terminal);
}

void
vty_shell_keyfunc_iac_end (struct shell *shell)
{
  shell->key_func = key_func_orig;
  fprintf (shell->terminal, "%s: IAC %#hhx.%s",
           __func__, shell->inputch, shell->LF);
  fflush (shell->terminal);
}

void
vty_shell_keyfunc_subnego (struct shell *shell)
{
  fprintf (shell->terminal, "%s: subnego[%d] %#hhx.%s",
           __func__, shell->subnego_size, shell->inputch, shell->LF);
  if (shell->subnego_size < sizeof (shell->subnego_buf))
    {
      shell->subnego_buf[shell->subnego_size] = shell->inputch;
      shell->subnego_size++;
    }
  fflush (shell->terminal);
}

void
vty_shell_keyfunc_sb_start (struct shell *shell)
{
  shell->key_func = key_func_subnego;
  shell->subnego_size = 0;
  fprintf (shell->terminal, "%s: IAC SB: %#hhx.%s",
           __func__, shell->inputch, shell->LF);
  fflush (shell->terminal);
}

void
vty_shell_keyfunc_sb_end (struct shell *shell)
{
  shell->key_func = key_func_orig;
  fprintf (shell->terminal, "%s: IAC SE: %#hhx.%s",
           __func__, shell->inputch, shell->LF);
  fflush (shell->terminal);

  int i;
  fprintf (shell->terminal, "telnet_sb: len: %d [", shell->subnego_size);
  for (i = 0; i < shell->subnego_size; i++)
    {
      fprintf (shell->terminal, " %#hhx", shell->subnego_buf[i]);
    }
  fprintf (shell->terminal, "]%s", shell->LF);
  fflush (shell->terminal);
}

void
vty_shell_keyfunc_init (struct shell *shell)
{
  int i;
  key_func_orig = shell->key_func;
  memset (key_func_escape_1, 0, sizeof (key_func_escape_1));
  memset (key_func_escape_2, 0, sizeof (key_func_escape_2));
  memset (key_func_iac, 0, sizeof (key_func_iac));
  memset (key_func_subnego, 0, sizeof (key_func_subnego));
  for (i = 0; i < 256; i++)
    {
      key_func_escape_1[i] = vty_shell_keyfunc_normal;
      key_func_escape_2[i] = vty_shell_keyfunc_normal;
      key_func_iac[i] = vty_shell_keyfunc_iac_end;
      key_func_subnego[i] = vty_shell_keyfunc_subnego;
    }

  key_func_escape_1['b'] = vty_shell_move_word_backward;
  key_func_escape_1['f'] = vty_shell_move_word_forward;
  key_func_escape_1[CONTROL('H')] = vty_shell_delete_word_backward;
  key_func_escape_1[0x7f] = vty_shell_delete_word_backward;

  key_func_escape_1['['] = vty_shell_keyfunc_escape_2;

  key_func_iac[SB] = vty_shell_keyfunc_sb_start;
  key_func_iac[SE] = vty_shell_keyfunc_sb_end;

  key_func_subnego[IAC] = vty_shell_keyfunc_iac_start;

  shell->key_func[IAC] = vty_shell_keyfunc_iac_start;
  shell->key_func[CONTROL('[')] = vty_shell_keyfunc_escape_1;
}

void
shell_keyfunc_clear_terminal (struct shell *shell)
{
  const char clr[] = { 27, '[', '2', 'J', '\0' };
  const char topLeft[] = { 27, '[', '1', ';', '1', 'H', '\0' };
  /* Clear screen and move to top left */
  fprintf (shell->terminal, "%s%s", clr, topLeft);
  fflush (shell->terminal);
}

DEFINE_COMMAND (clear_cmd,
                "clear",
                CLEAR_HELP)
{
  struct shell *shell = (struct shell *) context;
  shell_keyfunc_clear_terminal (shell);
}

extern int lthread_core;
extern volatile bool force_stop[RTE_MAX_LCORE];

DEFINE_COMMAND (vty_exit_cmd,
                "(exit|quit|logout)",
                "exit\n"
                "quite\n"
                "logout\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "vty exit !%s", shell->LF);
  FLAG_SET (shell->flag, SHELL_FLAG_EXIT);
  /* don't shell_close(): this closes stdout. */
  //shell_close (shell);

  //int nb_lcores = rte_lcore_count ();
  //for (int lcore_id = 0; lcore_id < nb_lcores; lcore_id++)
  //  stop_lcore (shell, lcore_id);
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
  shell->LF = "\r\n";

  vty_shell_keyfunc_init (shell);

  INSTALL_COMMAND2 (shell->cmdset, vty_exit_cmd);

  INSTALL_COMMAND2 (shell->cmdset, show_worker);
  INSTALL_COMMAND2 (shell->cmdset, set_worker);
  INSTALL_COMMAND2 (shell->cmdset, start_stop_worker);

  INSTALL_COMMAND2 (shell->cmdset, debug);
  INSTALL_COMMAND2 (shell->cmdset, show_debug);

  INSTALL_COMMAND3 (shell->cmdset, debug_module, debug_module_sdplane);
  INSTALL_COMMAND2 (shell->cmdset, show_debug_module);

  INSTALL_COMMAND2 (shell->cmdset, clear_cmd);
  shell_install (shell, CONTROL ('L'), shell_keyfunc_clear_terminal);

  l2fwd_cmd_init (shell->cmdset);
  l3fwd_cmd_init (shell->cmdset);
  soft_dplane_cmd_init (shell->cmdset);

  //termio_init ();

  vty_will_echo (shell);
  vty_will_suppress_go_ahead (shell);
  vty_dont_linemode (shell);
  vty_do_window_size (shell);

  shell_clear (shell);
  shell_prompt (shell);

  while (shell_running (shell) && ! force_stop[lthread_core])
    {
      lthread_sleep (100); // yield.

      if (FLAG_CHECK (debug_module_config[debug_module_sdplane],
                      DEBUG_SDPLANE_LTHREAD))
        printf ("%s: schedule.\n", prompt);

      shell_read_nowait (shell);
    }

#if 0
  while (! force_stop[lthread_core])
    {
      lthread_sleep (100); // yield.

      if (FLAG_CHECK (debug_module_config[debug_module_sdplane],
                      DEBUG_SDPLANE_LTHREAD))
        printf ("%s: schedule.\n", __func__);

      char send_buf[1024];
      char buf[1024];
      int ret;
      ret = lthread_recv (client->fd, buf, sizeof (buf), 0, 1000);
      if (ret == -2)
        {
          printf ("%s[%d]: timeout.\n", __func__, client->id);
          continue;
        }
      else if (ret > 0)
        {
          snprintf (send_buf, sizeof (send_buf), "your message: %s\n", buf);
          lthread_send (client->fd, send_buf, strlen (send_buf), 0);
          printf ("%s[%d]: returned: %s.\n",
                  __func__, client->id, send_buf);
        }
      else if (ret < 0)
	{
          printf ("%s[%d]: lthread_recv(): error: %d. exiting.\n",
                  __func__, client->id, ret);
	  break;
        }
      else
	{
          printf ("%s[%d]: lthread_recv(): returns 0. exiting.\n",
                  __func__, client->id);
	  break;
        }
    }
#endif

  lthread_close (client->fd);
  printf ("%s[%d] finished for client[%d]: %s.\n",
          __func__, client->id, client->id, client_addr_str);
  client->fd = -1;

  return;
}


