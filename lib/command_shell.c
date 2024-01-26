/*
 * Copyright (C) 2007-2023 Yasuhiro Ohara. All rights reserved.
 */

#include <includes.h>

#include "log.h"
#include "debug.h"

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

int duration_limit;
time_t start, end, limit;
char start_str[64], end_str[64], limit_str[64];

struct tm start_tm_buf, end_tm_buf, limit_tm_buf;
struct tm *start_tm, *end_tm, *limit_tm;

void
timer_init (int dura_limit, char *date_limit)
{
  int ret;

  duration_limit = dura_limit;
  ret = sscanf (date_limit, "%d/%d/%d %d:%d:%d",
                &limit_tm_buf.tm_year,
                &limit_tm_buf.tm_mon,
                &limit_tm_buf.tm_mday,
                &limit_tm_buf.tm_hour,
                &limit_tm_buf.tm_min,
                &limit_tm_buf.tm_sec);
  assert (ret == 6);
  limit_tm = &limit_tm_buf;

  limit_tm->tm_year -= 1900;
  limit_tm->tm_mon -= 1;

  limit = mktime (limit_tm);
  strftime (limit_str, sizeof (limit_str),
            "%Y/%m/%d %H:%M:%S", limit_tm);

  time (&start);
  start_tm = localtime_r (&start, &start_tm_buf);
  strftime (start_str, sizeof (start_str),
            "%Y/%m/%d %H:%M:%S", start_tm);

  end = start + duration_limit;
  end_tm = localtime_r (&end, &end_tm_buf);
  strftime (end_str, sizeof (end_str),
            "%Y/%m/%d %H:%M:%S", end_tm);
}

void
timer_check ()
{
  time_t current;
  char current_str[64];
  struct tm *current_tm, current_tm_buf;

  time (&current);
  current_tm = localtime_r (&current, &current_tm_buf);
  strftime (current_str, sizeof (current_str),
            "%Y/%m/%d %H:%M:%S", current_tm);

  if (FLAG_CHECK (debug_config, DEBUG_TIMER))
    {
      printf ("%9s %d sec\n", "duration:", duration_limit);
      printf ("%9s %s\n", "start:", start_str);
      printf ("%9s %s\n", "current:", current_str);
      printf ("%9s %s\n", "end:", end_str);
      printf ("%9s %s\n", "limit:", limit_str);
    }

  double diff_end, diff_limit;

  diff_end = difftime (end, current);
  diff_limit = difftime (limit, current);

  if (FLAG_CHECK (debug_config, DEBUG_TIMER))
    {
      //printf ("end diff: %.1lf\n", diff_end);
      printf ("limit diff: %.1lf\n", diff_limit);
    }

#if 1
  if (diff_end < 0)
    {
      printf ("opensh: beta-version: duration-limit: %'d secs\n",
              duration_limit);
      printf ("opensh: shutdown.\n");
      exit (1);
    }
#endif

  if (diff_limit < 0)
    {
      printf ("opensh: beta-version: date-limit: %s\n", limit_str);
      printf ("opensh: shutdown.\n");
      exit (1);
    }
}

void
command_shell_execute (struct shell *shell)
{
  int ret = 0;
  char *comment;

  shell_linefeed (shell);

  timer_check ();

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

static void
print_dirent (struct shell *shell, struct dirent *dirent,
              int num, int ncolumn, int print_width)
{
  char printname[1024];
  char *suffix;

  if (FLAG_CHECK (debug_config, DEBUG_SHELL))
    {
      fprintf (shell->terminal, "%d: ncolumn: %d width: %d\n",
               num, ncolumn, print_width);
      fprintf (shell->terminal, "\"%s\" (%d)\n",
               dirent->d_name, strlen (dirent->d_name));
    }

  suffix = "";
  if (dirent->d_type == DT_DIR)
    suffix = "/";
  snprintf (printname, sizeof (printname),
            "%s%s", dirent->d_name, suffix);

  if (num % ncolumn == 0)
    fprintf (shell->terminal, "  ");

  if (ncolumn != 1)
    fprintf (shell->terminal, "%-*s", print_width, printname);
  else
    fprintf (shell->terminal, "%s", printname);

  if (num % ncolumn == ncolumn - 1)
    fprintf (shell->terminal, "\n");
}

int
dirent_cmp (const void *va, const void *vb)
{
  struct dirent *da = *(struct dirent **) va;
  struct dirent *db = *(struct dirent **) vb;
  return strcmp (da->d_name, db->d_name);
}

void
file_ls_candidate (struct shell *shell, char *file_path)
{
  char *path = strdup (file_path);
  char *dirname;
  char *filename;
  int num;
  DIR *dir;
  struct dirent *dirent;

  path_disassemble (path, &dirname, &filename);
  if (FLAG_CHECK (debug_config, DEBUG_SHELL))
    {
      fprintf (shell->terminal, "  path: %s dir: %s filename: %s\n",
               file_path, dirname, filename);
    }

  dir = opendir (dirname);
  if (dir == NULL)
    {
      free (path);
      return;
    }

  int maxlen = 0;
  num = 0;
  while ((dirent = readdir (dir)) != NULL)
    {
      /* everything starts with '.' are hidden. */
      if (dirent->d_name[0] == '.')
        continue;

      /* filter by the pattern */
      if (strncmp (dirent->d_name, filename, strlen (filename)))
        continue;

      /* calculate the maxmum entry name length. */
#if 0
      maxlen = (maxlen < dirent->d_namlen ? dirent->d_namlen : maxlen);
#else
      maxlen = (maxlen < strlen (dirent->d_name) ?
                strlen (dirent->d_name) : maxlen);
#endif

      num++;
    }
  rewinddir (dir);

  int sort_size;
  struct dirent **sort_vector;

  sort_size = num;
  sort_vector = malloc (sizeof (struct dirent *) * sort_size);

  int ncolumn;
  ncolumn = (shell->winsize.ws_col - 2) / (maxlen + 2);
  if (ncolumn == 0)
    ncolumn = 1;

  if (FLAG_CHECK (debug_config, DEBUG_SHELL))
    {
      fprintf (shell->terminal, "  maxlen: %d ncol: %d\n",
               maxlen, ncolumn);
    }

  fprintf (shell->terminal, "\n");

  num = 0;
  while ((dirent = readdir (dir)) != NULL)
    {
      /* everything starts with '.' are hidden. */
      if (dirent->d_name[0] == '.')
        continue;

      /* filter by the pattern */
      if (strncmp (dirent->d_name, filename, strlen (filename)))
        continue;

      if (sort_vector)
        sort_vector[num] = dirent;
      else
        print_dirent (shell, dirent, num, ncolumn, maxlen + 2);

      num++;
    }

  if (sort_vector)
    {
      qsort ((void *) sort_vector, sort_size,
             sizeof (struct dirent *), dirent_cmp);

      for (int i = 0; i < sort_size; i++)
        {
          print_dirent (shell, sort_vector[i], i, ncolumn, maxlen + 2);
        }
    }

  fprintf (shell->terminal, "\n");

  closedir (dir);
  free (path);
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
            file_ls_candidate (shell, last);
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
  INSTALL_COMMAND (cmdset, exit);
  INSTALL_COMMAND (cmdset, quit);
  INSTALL_COMMAND (cmdset, logout);

#if 0
  INSTALL_COMMAND (cmdset, enable_shell_debugging);
  INSTALL_COMMAND (cmdset, disable_shell_debugging);
  INSTALL_COMMAND (cmdset, show_history);

  INSTALL_COMMAND (cmdset, redirect_stderr_file);
  INSTALL_COMMAND (cmdset, restore_stderr);
  INSTALL_COMMAND (cmdset, redirect_terminal_file);
  INSTALL_COMMAND (cmdset, restore_terminal);
#endif
}

struct shell *
command_shell_create ()
{
  struct shell *shell;

  shell = shell_create ();
  shell->cmdset = cmdset_default;
  //shell->cmdset = command_set_copy (cmdset_default);

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

