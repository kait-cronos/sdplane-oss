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

#include <ifaddrs.h>

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

#include "sdplane_version.h"

shell_keyfunc_t key_func_escape_1[256];
shell_keyfunc_t key_func_escape_2[256];
shell_keyfunc_t key_func_iac_1[256];
shell_keyfunc_t key_func_iac_2[256];
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
  shell->key_func = key_func_iac_1;
  shell->telnet_cmd = 0;
  if (FLAG_CHECK (debug_config, DEBUG_SHELL) ||
      FLAG_CHECK (debug_module_config[debug_module_sdplane],
                  DEBUG_SDPLANE_TELNET_OPT))
    fprintf (shell->terminal, "%s: IAC received.%s", __func__, shell->LF);
  fflush (shell->terminal);
}

void
vty_shell_keyfunc_telnet_opt (struct shell *shell)
{
  shell->key_func = key_func_orig;
  shell->telnet_opt = (u_char) shell->inputch;
  if (FLAG_CHECK (debug_config, DEBUG_SHELL) ||
      FLAG_CHECK (debug_module_config[debug_module_sdplane],
                  DEBUG_SDPLANE_TELNET_OPT))
    fprintf (shell->terminal, "%s: IAC %#02x %#02x.%s",
             __func__, shell->telnet_cmd, (u_char)shell->telnet_opt, shell->LF);
  fflush (shell->terminal);
}

void
vty_shell_keyfunc_telnet_cmd (struct shell *shell)
{
  char *telnet_cmd_str;
  shell->key_func = key_func_iac_2;
  shell->telnet_cmd = (u_char) shell->inputch;
  if (FLAG_CHECK (debug_config, DEBUG_SHELL) ||
      FLAG_CHECK (debug_module_config[debug_module_sdplane],
                  DEBUG_SDPLANE_TELNET_OPT))
    {
      switch (shell->telnet_cmd)
        {
          case DO: telnet_cmd_str = "DO"; break;
          case WILL: telnet_cmd_str = "WILL"; break;
          case DONT: telnet_cmd_str = "DONT"; break;
          case WONT: telnet_cmd_str = "WONT"; break;
          default: telnet_cmd_str = NULL; break;
        }
      if (telnet_cmd_str)
        fprintf (shell->terminal, "%s: IAC %s.%s",
                 __func__, telnet_cmd_str, shell->LF);
      else
        fprintf (shell->terminal, "%s: IAC %d(%#02x).%s",
                 __func__, (u_char)shell->inputch,
                 (u_char)shell->inputch, shell->LF);
    }
  fflush (shell->terminal);
}

void
vty_shell_keyfunc_subnego (struct shell *shell)
{
  if (FLAG_CHECK (debug_config, DEBUG_SHELL))
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
  if (FLAG_CHECK (debug_config, DEBUG_SHELL))
    fprintf (shell->terminal, "%s: IAC SB: %#hhx.%s",
             __func__, shell->inputch, shell->LF);
  fflush (shell->terminal);
}

void
vty_shell_keyfunc_sb_end (struct shell *shell)
{
  shell->key_func = key_func_orig;
  if (FLAG_CHECK (debug_config, DEBUG_SHELL))
    fprintf (shell->terminal, "%s: IAC SE: %#hhx.%s",
             __func__, shell->inputch, shell->LF);
  fflush (shell->terminal);

  int i;
  if (FLAG_CHECK (debug_config, DEBUG_SHELL))
    {
      fprintf (shell->terminal, "telnet_sb: len: %d [", shell->subnego_size);
      for (i = 0; i < shell->subnego_size; i++)
        {
          fprintf (shell->terminal, " %#hhx", shell->subnego_buf[i]);
        }
      fprintf (shell->terminal, "]%s", shell->LF);
      fflush (shell->terminal);
    }
}

void
vty_shell_keyfunc_init (struct shell *shell)
{
  int i;
  key_func_orig = shell->key_func;
  memset (key_func_escape_1, 0, sizeof (key_func_escape_1));
  memset (key_func_escape_2, 0, sizeof (key_func_escape_2));
  memset (key_func_iac_1, 0, sizeof (key_func_iac_1));
  memset (key_func_iac_2, 0, sizeof (key_func_iac_2));
  memset (key_func_subnego, 0, sizeof (key_func_subnego));
  for (i = 0; i < 256; i++)
    {
      key_func_escape_1[i] = vty_shell_keyfunc_normal;
      key_func_escape_2[i] = vty_shell_keyfunc_normal;
      key_func_iac_1[i] = vty_shell_keyfunc_telnet_opt;
      key_func_iac_2[i] = vty_shell_keyfunc_telnet_opt;
      key_func_subnego[i] = vty_shell_keyfunc_subnego;
    }

  key_func_escape_1['b'] = vty_shell_move_word_backward;
  key_func_escape_1['f'] = vty_shell_move_word_forward;
  key_func_escape_1[CONTROL('H')] = vty_shell_delete_word_backward;
  key_func_escape_1[0x7f] = vty_shell_delete_word_backward;

  key_func_escape_1['['] = vty_shell_keyfunc_escape_2;

  key_func_iac_1[SB] = vty_shell_keyfunc_sb_start;
  key_func_iac_1[SE] = vty_shell_keyfunc_sb_end;

  key_func_iac_1[DO] = vty_shell_keyfunc_telnet_cmd;
  key_func_iac_1[WILL] = vty_shell_keyfunc_telnet_cmd;
  key_func_iac_1[DONT] = vty_shell_keyfunc_telnet_cmd;
  key_func_iac_1[WONT] = vty_shell_keyfunc_telnet_cmd;

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

#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/utsname.h>

int
snprintf_signature (char *buf, int size, char *ifname)
{
  int ret;

  struct utsname utsname;
  ret = uname (&utsname);

  int sockfd;
  struct ifreq ifr;
  memset (&ifr, 0, sizeof (ifr));
  sockfd = socket (AF_PACKET, SOCK_RAW, IPPROTO_RAW);
  if (sockfd < 0)
    {
      printf ("socket (AF_PACKET) failed: %s\n", strerror (errno));
      return -1;
    }

  snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", ifname);
  ret = ioctl (sockfd, SIOCGIFHWADDR, &ifr);
  if (ret < 0)
    {
      printf ("ioctl (SIOCGIFHWADDR) failed: %s\n", strerror (errno));
      return ret;
    }

  ret = snprintf (buf, size,
                  "%s %s %s: %02x:%02x:%02x:%02x:%02x:%02x",
                  utsname.nodename, utsname.machine, ifname,
                  ((uint8_t *)(&ifr.ifr_hwaddr.sa_data))[0],
                  ((uint8_t *)(&ifr.ifr_hwaddr.sa_data))[1],
                  ((uint8_t *)(&ifr.ifr_hwaddr.sa_data))[2],
                  ((uint8_t *)(&ifr.ifr_hwaddr.sa_data))[3],
                  ((uint8_t *)(&ifr.ifr_hwaddr.sa_data))[4],
                  ((uint8_t *)(&ifr.ifr_hwaddr.sa_data))[5]);

  return ret;
}

void
vty_banner (struct shell *shell)
{
  int ret;
  char signature[1024];

  snprintf_signature (signature, sizeof (signature), "enp1s0");

  fprintf (shell->terminal, "welcome to sdplane vty_shell.%s", shell->LF);
  fprintf (shell->terminal, "sdplane version: %s%s", SDPLANE_VERSION, shell->LF);
  fprintf (shell->terminal, "signature: %s%s", signature, shell->LF);
  fflush (shell->terminal);
}

DEFINE_COMMAND (vty_exit_cmd,
                "(exit|quit|logout)",
                "exit\n"
                "quite\n"
                "logout\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "vty exit !%s", shell->LF);
  FLAG_SET (shell->flag, SHELL_FLAG_EXIT);
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

  if (FLAG_CHECK (debug_module_config[debug_module_sdplane],
                  DEBUG_SDPLANE_VTY_SHELL))
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
  vty_banner (shell);
  shell_prompt (shell);

  while (! force_quit && ! force_stop[lthread_core] &&
         shell_running (shell))
    {
      lthread_sleep (100); // yield.

#if 0
      if (FLAG_CHECK (debug_module_config[debug_module_sdplane],
                      DEBUG_SDPLANE_VTY_SHELL))
        printf ("%s[%d]: %s: schedule %s.\n",
                __FILE__, __LINE__, __func__, prompt);
#endif

      shell_read_nowait (shell);
    }

  if (FLAG_CHECK (debug_module_config[debug_module_sdplane],
                  DEBUG_SDPLANE_VTY_SHELL))
    printf ("%s[%d]: %s: terminating for client[%d]: %s.\n",
            __FILE__, __LINE__, __func__,
            client->id, client_addr_str);

  lthread_close (client->fd);
  client->fd = -1;

  return;
}

