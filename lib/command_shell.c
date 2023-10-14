/*
 * Copyright (C) 2007  Yasuhiro Ohara
 * 
 * This file is part of SimRouting.
 * 
 * SimRouting is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * SimRouting is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <includes.h>

#include "file.h"
#include "vector.h"
#include "shell.h"
#include "command.h"
#include "command_shell.h"

char *prompt_default = NULL;
struct command_set *cmdset_default = NULL;

DEFINE_COMMAND (exit,
                "exit",
                "exit\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "exit !\n");
  FLAG_SET (shell->flag, SHELL_FLAG_EXIT);
  shell_close (shell);
}

ALIAS_COMMAND (logout, exit, "logout", "logout\n");
ALIAS_COMMAND (quit, exit, "quit", "quit\n");

DEFINE_COMMAND (enable_shell_debugging,
                "enable shell debugging",
                "enable features\n"
                "enable shell settings\n"
                "enable shell debugging\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "enable shell debugging.\n");
  FLAG_SET (shell->flag, SHELL_FLAG_DEBUG);
}

DEFINE_COMMAND (disable_shell_debugging,
                "disable shell debugging",
                "disable features\n"
                "disable shell settings\n"
                "disable shell debugging\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "disable shell debugging.\n");
  FLAG_CLEAR (shell->flag, SHELL_FLAG_DEBUG);
}

void
command_history_delete (struct command_history *history)
{
  int i;
  for (i = 0; i < HISTORY_SIZE; i++)
    free (history->array[i]);
  free (history);
}

void
command_history_add (char *command_line,
                     struct command_history *history, struct shell *shell)
{
  if (! history)
    {
      history = malloc (sizeof (struct command_history));
      memset (history, 0, sizeof (struct command_history));
      shell->history = history;
    }

  if (history->array[history->last])
    history->current = HISTORY_NEXT (history->last);
  assert (! history->array[history->current]);
  history->array[history->current] = strdup (command_line);
  history->last = history->current;
  history->current = HISTORY_NEXT (history->current);
  if (history->array[history->current])
    free (history->array[history->current]);
  history->array[history->current] = NULL;
}

void
command_history_prev (struct shell *shell)
{
  int ceil, start, prev;
  struct command_history *history = shell->history;

  if (! history)
    return;

  ceil = HISTORY_NEXT (history->last);
  start = HISTORY_NEXT (ceil);

#if 0
  printf ("last: %d ceil: %d start: %d current: %d\n",
          history->last, ceil, start, history->current);
#endif

  /* wrapping */
  if (history->current == start)
    return;

  prev = HISTORY_PREV (history->current);
  if (history->array[prev])
    {
      shell_delete_string (shell, 0, shell->end);
      shell_insert (shell, history->array[prev]);
      history->current = prev;
    }
}

void
command_history_next (struct shell *shell)
{
  int floor, start, next;
  struct command_history *history = shell->history;

  if (! history)
    return;

  floor = HISTORY_NEXT (history->last);
  start = HISTORY_NEXT (floor);

#if 0
  printf ("last: %d floor: %d start: %d current: %d\n",
          history->last, floor, start, history->current);
#endif

  /* wrapping */
  if (history->current == floor)
    return;

  next = HISTORY_NEXT (history->current);
  if (history->array[next])
    {
      shell_delete_string (shell, 0, shell->end);
      shell_insert (shell, history->array[next]);
      history->current = next;
    }
}

DEFINE_COMMAND (show_history,
                "show history",
                "display information\n"
                "command history\n")
{
  struct shell *shell = (struct shell *) context;
  struct command_history *history = shell->history;
  int floor, start, i;

  if (! history)
    return;

  floor = HISTORY_NEXT (history->last);
  start = HISTORY_NEXT (floor);

  for (i = start; i != floor; i = HISTORY_NEXT (i))
    if (history->array[i])
      fprintf (shell->terminal, "[%3d] %s\n", i, history->array[i]);
}

void
command_shell_execute (struct shell *shell)
{
  int ret = 0;
  char *comment;

  shell_linefeed (shell);

  /* comment handling */
  comment = strpbrk (shell->command_line, "#!");
  if (comment)
    {
      shell->end = comment - shell->command_line;
      shell->cursor = (shell->cursor > shell->end ?
                       shell->end : shell->cursor);
      shell_terminate (shell);
    }

  shell_format (shell);

  if (! strlen (shell->command_line))
    {
      shell_clear (shell);
      shell_prompt (shell);
      return;
    }

  ret = command_execute (shell->command_line, shell->cmdset, shell);
  if (ret < 0)
    fprintf (shell->terminal, "no such command: %s\n", shell->command_line);
  command_history_add (shell->command_line, shell->history, shell);

  /* FILE buffer must be flushed before raw-writing the same file */
  fflush (shell->terminal);

  shell_clear (shell);
  shell_prompt (shell);
}

void
command_shell_completion (struct shell *shell)
{
  char *completion = NULL;

  shell_moveto (shell, shell_word_end (shell, shell->cursor));
  completion = command_complete (shell->command_line, shell->cursor,
                                 shell->cmdset);
  if (completion)
    shell_insert (shell, completion);
}

void
command_shell_ls_candidate (struct shell *shell)
{
  char *cmd_dup;
  char *last;
  int last_head;
  struct command_node *match;
  struct command_node *node;
  struct vector_node *vn;

  if (shell->cursor != shell->end)
    {
      shell->cursor = shell->end;
      shell_linefeed (shell);
      shell_format (shell);
      shell_refresh (shell);
      return;
    }

  last_head = shell_word_head (shell, shell->cursor);
  cmd_dup = strdup (shell->command_line);
  cmd_dup[last_head] = '\0';
  last = strdup (&shell->command_line[last_head]);

  shell_linefeed (shell);

  match = command_match_node (cmd_dup, shell->cmdset);
  if (match)
    {
      if (last_head == shell->cursor && match->func)
        fprintf (shell->terminal, "  %-16s %s\n",
                 "<cr>", match->helpstr);

      for (vn = vector_head (match->cmdvec); vn; vn = vector_next (vn))
        {
          node = (struct command_node *) vn->data;
          if (is_command_match (node->cmdstr, last))
            fprintf (shell->terminal, "  %-16s %s\n",
                     node->cmdstr, node->helpstr);

          if (file_spec (node->cmdstr))
            {
              char *path = strdup (last);
              char *dirname;
              char *filename;
              int num = 0;
              DIR *dir;
              struct dirent *dirent;

              fprintf (shell->terminal, "\n");
              path_disassemble (path, &dirname, &filename);

              dir = opendir (dirname);
              if (dir == NULL)
                {
                  free (path);
                  continue;
                }

              while ((dirent = readdir (dir)) != NULL)
                if (! strncmp (dirent->d_name, filename, strlen (filename)))
                  {
                    if (num % 4 == 0)
                      fprintf (shell->terminal, "  ");
                    fprintf (shell->terminal, "%-16s", dirent->d_name);
                    if (num % 4 == 3)
                      fprintf (shell->terminal, "\n");
                    num++;
                  }
              fprintf (shell->terminal, "\n");
              free (path);
            }
        }
    }

  free (last);
  free (cmd_dup);

  shell_format (shell);
  shell_refresh (shell);
}

DEFINE_COMMAND (redirect_stderr_file,
                "redirect stderr <FILENAME>",
                "redirect (FILE *)s\n"
                "redirect stderr\n"
                "File name\n")
{
  FILE *fp;
  fp = fopen_create (argv[2], "w+");
  redirect_stdio (stderr, fp);
}

DEFINE_COMMAND (restore_stderr,
                "restore stderr",
                "restore (FILE *)s\n"
                "restore stderr\n")
{
  restore_stdio ();
}

FILE *save_terminal;

DEFINE_COMMAND (redirect_terminal_file,
                "redirect terminal <FILENAME>",
                "redirect (FILE *)s\n"
                "redirect shell terminal\n"
                "File name\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *fp = fopen_create (argv[2], "w+");
  save_terminal = shell->terminal;
  shell->terminal = fp;
}

DEFINE_COMMAND (restore_terminal,
                "restore terminal",
                "restore (FILE *)s\n"
                "restore shell terminal\n")
{
  struct shell *shell = (struct shell *) context;
  shell->terminal = save_terminal;
}

void
default_install_command (struct command_set *cmdset)
{
  INSTALL_COMMAND (cmdset, enable_shell_debugging);
  INSTALL_COMMAND (cmdset, disable_shell_debugging);
  INSTALL_COMMAND (cmdset, show_history);

  INSTALL_COMMAND (cmdset, redirect_stderr_file);
  INSTALL_COMMAND (cmdset, restore_stderr);
  INSTALL_COMMAND (cmdset, redirect_terminal_file);
  INSTALL_COMMAND (cmdset, restore_terminal);
}

struct shell *
command_shell_create ()
{
  struct shell *shell;

  shell = shell_create ();
  shell->cmdset = cmdset_default;

  shell_install (shell, CONTROL('J'), command_shell_execute);
  shell_install (shell, CONTROL('M'), command_shell_execute);
  shell_install (shell, CONTROL('I'), command_shell_completion);
  shell_install (shell, '?', command_shell_ls_candidate);
  shell_install (shell, CONTROL('P'), command_history_prev);
  shell_install (shell, CONTROL('N'), command_history_next);

  return shell;
}

void
command_shell_delete (struct shell *shell)
{
  command_set_delete (shell->cmdset);
  command_history_delete (shell->history);
  shell_delete (shell);
}

void
command_shell_init ()
{
  cmdset_default = command_set_create ();
  default_install_command (cmdset_default);
}

void
command_shell_finish ()
{
  command_set_delete (cmdset_default);
}



