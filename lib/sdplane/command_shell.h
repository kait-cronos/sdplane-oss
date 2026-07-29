/*
 * Copyright (C) 2007-2023 Yasuhiro Ohara. All rights reserved.
 */

#ifndef __COMMAND_SHELL_H__
#define __COMMAND_SHELL_H__

#include "command.h"
#include "shell.h"

extern char *prompt_default;
extern struct command_set *cmdset_default;

#define HISTORY_SIZE    128
#define HISTORY_PREV(x) ((x) == 0 ? HISTORY_SIZE - 1 : (x) - 1)
#define HISTORY_NEXT(x) ((x) + 1 == HISTORY_SIZE ? 0 : (x) + 1)
struct command_history
{
  char *array[HISTORY_SIZE];
  int last;
  int current;
  time_t clock[HISTORY_SIZE];
};

EXTERN_COMMAND (exit);
EXTERN_COMMAND (quit);
EXTERN_COMMAND (logout);
EXTERN_COMMAND (enable_shell_debugging);
EXTERN_COMMAND (disable_shell_debugging);
EXTERN_COMMAND (show_history);
EXTERN_COMMAND (redirect_stderr_file);
EXTERN_COMMAND (restore_stderr);
EXTERN_COMMAND (redirect_terminal_file);
EXTERN_COMMAND (restore_terminal);

struct shell *command_shell_create ();
void command_shell_delete (struct shell *shell);

void command_shell_install_default (struct shell *shell);

int shell_pager_bridge (struct shell *shell);
void shell_read_nowait_paging (struct shell *shell);

/* pager pipe stages: filter commands (such as moreutils sponge)
   inserted between the command output and the pager, in the
   index order: command output -> stage[0] -> ... -> pager.
   argv[] is deep-copied; the caller keeps the ownership. */
#define PAGER_PIPE_ARGV_MAX 32
int pager_pipe_stage_set (int index, int stage_argc, char **stage_argv);
int pager_pipe_stage_unset (int index);

/* the pager command itself (the last consumer on the pty).
   when unset, the built-in default (strace-wrapped less) is used.
   argv[] is deep-copied; the caller keeps the ownership. */
int pager_command_set (int cmd_argc, char **cmd_argv);
int pager_command_unset (void);

void command_shell_init ();
void command_shell_finish ();

void command_history_add (char *command_line, struct command_history *history,
                          struct shell *shell);

int dirent_cmp (const void *va, const void *vb);
void file_ls_candidate (struct shell *shell, char *file_path);

#endif /*__COMMAND_SHELL_H__*/
