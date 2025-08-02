/*
 * Copyright (C) 2007-2023 Yasuhiro Ohara. All rights reserved.
 */

#include <includes.h>

#include <signal.h>

#include "opensh_ver.h"

#include "sdplane/debug.h"
#include "sdplane/termio.h"
#include "sdplane/vector.h"
#include "sdplane/shell.h"
#include "sdplane/command.h"
#include "sdplane/command_shell.h"
#include "sdplane/debug_cmd.h"

#include "sdplane/shell_keyfunc.h"
#include "sdplane/shell_fselect.h"

#include "sdplane/debug_log.h"
#include "sdplane/debug_category.h"
#include "sdplane/debug_zcmdsh.h"
#include "sdplane/log_cmd.h"

/* global shell */
struct shell *shell = NULL;

void
get_winsize (struct shell *shell)
{
  ioctl (shell->writefd, TIOCGWINSZ, &shell->winsize);
  fprintf (shell->terminal, "row: %d col: %d\n",
           shell->winsize.ws_row, shell->winsize.ws_col);
}

void
winch_handler (int signal)
{
  if (shell)
    get_winsize (shell);
}

CLI_COMMAND2 (show_version,
                "show version",
                SHOW_HELP,
                "show version information.\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "opensh version: %s\n", opensh_version);
}

void
opensh_shell_keyfunc_ctrl_d (struct shell *shell)
{
  if (shell->cursor == 0 && shell->end == 0)
    {
      (*exit_cmd.cmdfunc) ((void *)shell, 1, (char **)"exit");
      return;
    }

  /* Delete one character */
  if (shell->cursor < shell->end)
    shell_delete_string (shell, shell->cursor, shell->cursor + 1);
}

void
shell_set_prompt_cwd (struct shell *shell)
{
  char buf[MAXPATHLEN];
  char prompt[MAXPATHLEN * 2];
  char *p;

  p = getcwd (buf, sizeof (buf));
  if (! p)
    DEBUG_ZCMDSH_LOG (SHELL, "getcwd() failed: %s",
                      strerror (errno));
  snprintf (prompt, sizeof (prompt), "[%s]> ", buf);

  shell_set_prompt (shell, prompt);
}

CLI_COMMAND2 (chdir,
               "(cd|chdir) (|<FILENAME>)",
               "change current working directory.\n",
               "change current working directory.\n",
               "filename\n")
{
  int ret = 0;
  struct shell *shell = (struct shell *) context;
  char *target;

  if (argc > 1)
    {
      target = argv[1];
      fprintf (shell->terminal, "chdir: %s\n", target);
    }
  else
    {
      target = getenv ("HOME");
      fprintf (shell->terminal, "chdir: home: %s\n", target);
    }

  ret = chdir (target);
  if (ret)
    fprintf (shell->terminal, "chdir failed: %s\n", strerror (errno));

  shell_set_prompt_cwd (shell);
}

CLI_COMMAND2 (pwd,
               "pwd",
               "print current working directory.\n")
{
  struct shell *shell = (struct shell *) context;
  char buf[MAXPATHLEN];
  char *p;
  p = getcwd (buf, sizeof (buf));
  if (! p)
    DEBUG_ZCMDSH_LOG (SHELL, "getcwd() failed: %s",
                      strerror (errno));
  fprintf (shell->terminal, "cwd: %s\n", buf);
}

CLI_COMMAND2 (list,
               "(ls|list) (|<FILENAME>)",
               "list files in the directory.\n",
               "list files in the directory.\n",
               "directory name\n")
{
  struct shell *shell = (struct shell *) context;
  char buf[MAXPATHLEN], dir[MAXPATHLEN + 8];
  if (argc > 1)
    snprintf (buf, sizeof (buf), "%s", argv[1]);
  else
    {
      char *p;
      p = getcwd (buf, sizeof (buf));
      if (! p)
        DEBUG_ZCMDSH_LOG (SHELL, "getcwd() failed: %s",
                          strerror (errno));
    }
  snprintf (dir, sizeof (dir), "%s/", buf);
  fprintf (shell->terminal, "dir: %s\n", dir);
  file_ls_candidate (shell, dir);
}

CLI_COMMAND2 (open,
               "open <FILENAME>",
               "open.\n",
               "filename\n")
{
  struct shell *shell = (struct shell *) context;
  char command[1024];
  char *filename = argv[1];
  int ret;

  snprintf (command, sizeof (command), "open \"%s\"", filename);
  ret = system (command);
  if (ret < 0)
    fprintf (shell->terminal, "system() failed: %s",
             strerror (errno));
}

CLI_COMMAND2 (launch_terminal,
               "launch terminal",
               "Launch application.\n",
               "open a new terminal window.\n")
{
  struct shell *shell = (struct shell *) context;
  char command[1024];
  int ret;

  snprintf (command, sizeof (command),
            "osascript -e \"tell app \\\"Terminal\\\""
            " to do script \\\"cd $PWD\\\"\"");
  fprintf (shell->terminal, "system: %s\n", command);
  ret = system (command);
  if (ret < 0)
    fprintf (shell->terminal, "system() failed: %s",
             strerror (errno));
}

CLI_COMMAND2 (launch_shell,
               "launch (sh|bash|csh|dash|ksh|tcsh|zsh)",
               "Launch application.\n",
               "launch a system shell: check /private/var/select/sh.\n",
               "launch a bash shell.\n",
               "launch a csh shell.\n",
               "launch a dash shell.\n",
               "launch a ksh shell.\n",
               "launch a tcsh shell.\n",
               "launch a zsh shell.\n"
               )
{
  struct shell *shell = (struct shell *) context;
  char command[1024];
  int ret;

  if (argc > 2)
    snprintf (command, sizeof (command), "%s", argv[2]);
  else
    snprintf (command, sizeof (command), "%s", getenv ("SHELL"));

  termio_reset ();
  fprintf (shell->terminal, "system: %s\n", command);
  ret = system (command);
  if (ret < 0)
    fprintf (shell->terminal, "system() failed: %s",
             strerror (errno));
  fprintf (shell->terminal, "system: done: %s\n", command);
  termio_start ();
}

CLI_COMMAND2 (launch_vi,
               "(vi|vim) <FILENAME>",
               "edit a file with vi.\n",
               "edit a file with vim.\n",
               "filename\n")
{
  struct shell *shell = (struct shell *) context;
  char command[1024];
  char *filename = argv[1];
  int ret;

  snprintf (command, sizeof (command), "%s \"%s\"", argv[0], filename);

  termio_reset ();
  fprintf (shell->terminal, "system: %s\n", command);
  ret = system (command);
  if (ret < 0)
    fprintf (shell->terminal, "system() failed: %s",
             strerror (errno));
  fprintf (shell->terminal, "system: %s: done.\n", command);
  termio_start ();
}

extern char *optarg;
extern int optind, opterr, optopt;

char *progname;

struct option longopts[] =
{
  { "debug",          no_argument,       NULL, 'd'},
  { "help",           no_argument,       NULL, 'h'},
  { "version",        no_argument,       NULL, 'v'},
  { 0 }
};

void
print_version ()
{
  printf ("%s version: %s\n", progname, opensh_version);
}

void
print_usage ()
{
  printf ("Usage : %s [-d|-h|-v]\n\n", progname);
}

int
main (int argc, char **argv)
{
  struct sigaction action;
  char *p;
  int ret;

  /* Preserve name of myself. */
  progname = ((p = strrchr (argv[0], '/')) ? ++p : argv[0]);
  debug_log_init (progname);

  /* Command line argument treatment. */
  while (1)
    {
      ret = getopt_long (argc, argv, "dhv", longopts, 0);

      if (ret == EOF)
        break;

      switch (ret)
        {
        case 0:
          break;

        case 'd':
          FLAG_SET (debug_config, DEBUG_SHELL);
          break;
        case 'v':
          print_version ();
          exit (0);
          break;
        case 'h':
          print_usage ();
          exit (0);
          break;
        default:
          print_usage ();
          exit (1);
          break;
        }
    }

  /* timer set */
  //timer_init (3600, "2024/9/30 23:59:59");
  //timer_init (0, NULL);

  /* signal related initialization. */
  /* SIGWINCH */
  action.sa_handler = winch_handler;
  sigemptyset (&action.sa_mask);
  action.sa_flags = SA_RESTART;
  sigaction (SIGWINCH, &action, NULL);

  /* SIGINT */
  action.sa_handler = SIG_IGN;
  sigemptyset (&action.sa_mask);
  action.sa_flags = SA_RESTART;
  sigaction (SIGINT, &action, NULL);

  /* library initialization. */
  debug_zcmdsh_cmd_init ();
  command_shell_init ();

  shell = command_shell_create ();
  shell_set_prompt_cwd (shell);
  shell_set_terminal (shell, 0, 1);
  get_winsize (shell);

  log_cmd_init (shell->cmdset);

  INSTALL_COMMAND2 (shell->cmdset, show_version);

  INSTALL_COMMAND2 (shell->cmdset, chdir);
  INSTALL_COMMAND2 (shell->cmdset, list);

  INSTALL_COMMAND2 (shell->cmdset, debug_zcmdsh);
  INSTALL_COMMAND2 (shell->cmdset, show_debug_zcmdsh);

  INSTALL_COMMAND (shell->cmdset, pwd);
  INSTALL_COMMAND (shell->cmdset, open);
  INSTALL_COMMAND2 (shell->cmdset, launch_terminal);
  INSTALL_COMMAND2 (shell->cmdset, launch_shell);
  INSTALL_COMMAND2 (shell->cmdset, launch_vi);

  shell_install (shell, '>', (shell_keyfunc_t) fselect_keyfunc_start);
  shell_install (shell, CONTROL ('D'), (shell_keyfunc_t) opensh_shell_keyfunc_ctrl_d);
  shell_install (shell, 0x7f, shell_keyfunc_delete_char_advanced);
  FUNC_STR_REGISTER (fselect_keyfunc_start);
  FUNC_STR_REGISTER (opensh_shell_keyfunc_ctrl_d);
  FUNC_STR_REGISTER (shell_keyfunc_delete_char_advanced);

  shell_escape_keyfunc_init (shell);

  shell_fselect_init ();

  termio_init ();

  shell_clear (shell);
  shell_prompt (shell);
  while (shell_running (shell))
    {
      if (shell->is_paging)
        shell_read_nowait_paging (shell);
      else
        shell_read (shell);
    }

  termio_finish ();

  shell_close (shell);
  shell_delete (shell);

  return 0;
}

