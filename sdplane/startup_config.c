#include "include.h"

#include <lthread.h>

#include <rte_ether.h>
#include <rte_ethdev.h>

#include <sdplane/debug.h>
#include <sdplane/termio.h>
#include <sdplane/vector.h>
#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_zcmdsh.h>
// #include <sdplane/shell_fselect.h>
#include <sdplane/log_cmd.h>

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
  FLAG_UNSET (shell->flag, SHELL_FLAG_INTERACTIVE);

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
  int ret;
  fd = open (config_file, O_RDONLY);
  if (fd >= 0)
    {
      shell_set_terminal (shell, fd, 1);
      while (shell_running (shell))
        {
          lthread_sleep (10); // yield.

          ret = shell_read_nowait (shell);
	  if (ret < 0)
            FLAG_SET (shell->flag, SHELL_FLAG_EXIT);
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
