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
#include <zcmdsh/shell_keyfunc.h>
#include <zcmdsh/shell_telnet.h>
#include <zcmdsh/command.h>
#include <zcmdsh/command_shell.h>
#include <zcmdsh/debug_cmd.h>
#include <zcmdsh/debug_module.h>
#include <zcmdsh/debug_module_cmd.h>
#include <zcmdsh/log.h>
#include <zcmdsh/log_cmd.h>

#include <zcmdsh/debug_log.h>
#include <zcmdsh/debug_category.h>
#include <zcmdsh/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "sdplane.h"
#include "l2fwd_cmd.h"
#include "l3fwd_cmd.h"

#include "vty_server.h"
#include "vty_shell.h"

#include "sdplane_version.h"

void
shell_keyfunc_clear_terminal (struct shell *shell)
{
  const char clr[] = { 27, '[', '2', 'J', '\0' };
  const char topLeft[] = { 27, '[', '1', ';', '1', 'H', '\0' };
  /* Clear screen and move to top left */
  fprintf (shell->terminal, "%s%s", clr, topLeft);
  fflush (shell->terminal);
  shell_refresh (shell);
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

  fprintf (shell->terminal, "welcome to sdplane vty_shell.%s", shell->NL);
  fprintf (shell->terminal, "sdplane version: %s%s", SDPLANE_VERSION, shell->NL);
  fprintf (shell->terminal, "signature: %s%s", signature, shell->NL);
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

#if 0
  if (FLAG_CHECK (debug_module_config[debug_module_sdplane],
                  DEBUG_SDPLANE_VTY_SHELL))
    printf ("%s[%d]: client[%d]: %s.\n",
            __func__, client->id, client->id, client_addr_str);
#else
  FLAG_SET (DEBUG_CONFIG(SDPLANE), DEBUG_SDPLANE_VTY);
  DEBUG_SDPLANE_LOG (VTY, "%s[%d]: client[%d]: %s.",
                     "vty", client->id, client->id, client_addr_str);
#endif

  char prompt[64];
  snprintf (prompt, sizeof (prompt), "vty[%d]> ", client->id);

  shell = command_shell_create ();
  shell_set_terminal (shell, client->fd, client->fd);
  shell_set_prompt (shell, prompt);
  //get_winsize (shell);

  DEBUG_SDPLANE_LOG (VTY, "%s[%d]: fd: %d terminal: %p.",
                     "vty", client->id, client->fd, shell->terminal);

  shell->NL = "\r\n";

  FLAG_SET (shell->debug_zcmdsh, DEBUG_TYPE (ZCMDSH, TELNET));

  shell_escape_keyfunc_init (shell);
  shell_telnet_keyfunc_init (shell);

  log_cmd_init (shell->cmdset);
  INSTALL_COMMAND2 (shell->cmdset, vty_exit_cmd);

  INSTALL_COMMAND2 (shell->cmdset, show_worker);
  INSTALL_COMMAND2 (shell->cmdset, set_worker);
  INSTALL_COMMAND2 (shell->cmdset, start_stop_worker);

  INSTALL_COMMAND2 (shell->cmdset, debug);
  INSTALL_COMMAND2 (shell->cmdset, show_debug);

  INSTALL_COMMAND3 (shell->cmdset, debug_module, debug_module_sdplane);
  INSTALL_COMMAND2 (shell->cmdset, show_debug_module);

  //INSTALL_COMMAND2 (shell->cmdset, clear_cmd);
  shell_install (shell, CONTROL ('L'), shell_keyfunc_clear_terminal);

  shell_install (shell, 0x7f, shell_keyfunc_delete_char_advanced);
  FUNC_STR_REGISTER (shell_keyfunc_delete_char_advanced);

  INSTALL_COMMAND2 (shell->cmdset, shutdown_cmd);

  log_cmd_init (shell->cmdset);
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

