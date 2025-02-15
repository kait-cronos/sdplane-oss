#include "include.h"

#include <lthread.h>

#include <rte_ether.h>
#include <rte_ethdev.h>

#include <zcmdsh/debug.h>
#include <zcmdsh/termio.h>
#include <zcmdsh/vector.h>
#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>
#include <zcmdsh/command_shell.h>
#include <zcmdsh/debug_cmd.h>
#include <zcmdsh/debug_zcmdsh.h>
// #include <zcmdsh/shell_fselect.h>
#include <zcmdsh/log_cmd.h>

#include "l3fwd.h"
#include "l3fwd_cmd.h"
#include "l2fwd_cmd.h"
#include "sdplane.h"
#include "debug_sdplane.h"

int
startup_config (__rte_unused void *dummy)
{
  struct shell *shell = NULL;

  shell = command_shell_create ();
  shell_set_prompt (shell, "startup-config> ");
  shell->pager = false;

  // INSTALL_COMMAND2 (shell->cmdset, show_worker);
  INSTALL_COMMAND2 (shell->cmdset, set_worker);
  INSTALL_COMMAND2 (shell->cmdset, start_stop_worker);

  INSTALL_COMMAND2 (shell->cmdset, debug_zcmdsh);
  // INSTALL_COMMAND2 (shell->cmdset, show_debug_zcmdsh);

  INSTALL_COMMAND2 (shell->cmdset, debug_sdplane);
  // INSTALL_COMMAND2 (shell->cmdset, show_debug_sdplane);

  INSTALL_COMMAND2 (shell->cmdset, l2fwd_init);

  log_cmd_init (shell->cmdset);
  l2fwd_cmd_init (shell->cmdset);
  l3fwd_cmd_init (shell->cmdset);
  sdplane_cmd_init (shell->cmdset);

  // termio_init ();

  shell_clear (shell);
  shell_prompt (shell);

  char *config_file = "/etc/sdplane/sdplane.conf";
  printf ("%s[%d]: %s: opening %s.\n", __FILE__, __LINE__, __func__,
          config_file);
  int fd;
  fd = open (config_file, O_RDONLY);
  if (fd >= 0)
    {
      shell_set_terminal (shell, fd, 1);
      while (shell_running (shell))
        {
          lthread_sleep (10); // yield.

          shell_read_nowait (shell);
        }
    }
  else
    printf ("%s[%d]: %s: opening %s: failed: %s.\n", __FILE__, __LINE__,
            __func__, config_file, strerror (errno));

  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);
  fflush (stdout);

  // termio_finish ();
  return 0;
}
