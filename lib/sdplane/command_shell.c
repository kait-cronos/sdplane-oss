/*
 * Copyright (C) 2007-2023 Yasuhiro Ohara. All rights reserved.
 */

#include <includes.h>
#include <poll.h>
#include <stdbool.h>

#if HAVE_UTIL_H
#include <util.h> // for openpty()
#endif /*HAVE_UTIL_H*/

#include "debug.h"
#include "log.h"

#include "command.h"
#include "command_shell.h"
#include "file.h"
#include "shell.h"
#include "shell_keyfunc.h"
#include "termio.h"
#include "vector.h"

#include "debug_category.h"
#include "debug_log.h"
#include "debug_zcmdsh.h"

char *prompt_default = NULL;
struct command_set *cmdset_default = NULL;

DEFINE_COMMAND (exit, "exit", "exit\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "exit !%s", shell->NL);
  FLAG_SET (shell->flag, SHELL_FLAG_EXIT);
  /* don't do shell_close() here: it closes stdout,
     and breaks safe termination. */
  // shell_close (shell);
  return CMD_SUCCESS;
}

ALIAS_COMMAND (logout, exit, "logout", "logout\n");
ALIAS_COMMAND (quit, exit, "quit", "quit\n");

DEFINE_COMMAND (enable_shell_debugging, "enable shell debugging",
                "enable features\n"
                "enable shell settings\n"
                "enable shell debugging\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "enable shell debugging.%s", shell->NL);
  FLAG_SET (shell->flag, SHELL_FLAG_DEBUG);
  return CMD_SUCCESS;
}

DEFINE_COMMAND (disable_shell_debugging, "disable shell debugging",
                "disable features\n"
                "disable shell settings\n"
                "disable shell debugging\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "disable shell debugging.%s", shell->NL);
  FLAG_CLEAR (shell->flag, SHELL_FLAG_DEBUG);
  return CMD_SUCCESS;
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
command_history_add (char *command_line, struct command_history *history,
                     struct shell *shell)
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

int
command_history_prev (struct shell *shell)
{
  int ceil, start, prev;
  struct command_history *history = shell->history;

  if (! history)
    return 0;

  ceil = HISTORY_NEXT (history->last);
  start = HISTORY_NEXT (ceil);

#if 0
  printf ("last: %d ceil: %d start: %d current: %d\n",
          history->last, ceil, start, history->current);
#endif

  /* wrapping */
  if (history->current == start)
    return 0;

  prev = HISTORY_PREV (history->current);
  if (history->array[prev])
    {
      shell_delete_string (shell, 0, shell->end);
      shell_insert (shell, history->array[prev]);
      history->current = prev;
    }
  return 0;
}

int
command_history_next (struct shell *shell)
{
  int floor, start, next;
  struct command_history *history = shell->history;

  if (! history)
    return 0;

  floor = HISTORY_NEXT (history->last);
  start = HISTORY_NEXT (floor);

#if 0
  printf ("last: %d floor: %d start: %d current: %d\n",
          history->last, floor, start, history->current);
#endif

  /* wrapping */
  if (history->current == floor)
    return 0;

  next = HISTORY_NEXT (history->current);
  if (history->array[next])
    {
      shell_delete_string (shell, 0, shell->end);
      shell_insert (shell, history->array[next]);
      history->current = next;
    }
  return 0;
}

DEFINE_COMMAND (show_history, "show history",
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
  return 0;
}

#include <poll.h>
#if HAVE_PTY_H
#include <pty.h>
#endif /*HAVE_PTY_H*/
#include <signal.h>

#define DEFAULT_PAGER_ARG0 "/usr/bin/less"
#define DEFAULT_PAGER_ARG1 "-FX"
#define DEFAULT_PAGER      (DEFAULT_PAGER_ARG0 " " DEFAULT_PAGER_ARG1)

#define PAGER_USE_POPEN 0
void
pager_start (struct shell *shell)
{
  shell->is_paging = false;
#if PAGER_USE_POPEN
  FILE *pager_fp = NULL;

  DEBUG_ZCMDSH_LOG (PAGER, "pager: use popen.");

  if (shell->pager_command)
    {
      DEBUG_ZCMDSH_LOG (PAGER, "pager: %s", shell->pager_command);
      pager_fp = popen (shell->pager_command, "w");
    }

  if (! pager_fp)
    {
      DEBUG_ZCMDSH_LOG (PAGER, "default pager: %s", DEFAULT_PAGER);
      pager_fp = popen (DEFAULT_PAGER, "w");
    }

  if (! pager_fp)
    {
      DEBUG_ZCMDSH_LOG (PAGER, "can't open pager: give up paging.");
      return;
    }

  DEBUG_ZCMDSH_LOG (PAGER, "pager start: terminal: %p <- %p.", pager_fp,
                    shell->terminal);
  shell->pager_saved_terminal = shell->terminal;
  shell->pager_saved_writefd = shell->writefd;
  shell->terminal = pager_fp;
  shell->writefd = fileno (pager_fp);
#else
  int ret;
  pid_t process_id;

  int pty_master, pty_slave;

  openpty (&shell->pty_master, &shell->pty_slave, NULL, NULL, NULL);
  DEBUG_ZCMDSH_LOG (PAGER, "pty(master): fd %d, %s.", shell->pty_master,
                    ttyname (shell->pty_master));

  if (shell->winsize.ws_col && shell->winsize.ws_row)
    {
      ioctl (shell->pty_master, TIOCSWINSZ, &shell->winsize);
      DEBUG_ZCMDSH_LOG (PAGER, "pager: TIOCSWINSZ: row: %d col: %d.",
                        shell->winsize.ws_row, shell->winsize.ws_col);
    }

  DEBUG_ZCMDSH_LOG (PAGER, "pager: use fork/execvp.");
  ret = pipe (shell->pipefd);
  if (ret < 0)
    {
      DEBUG_ZCMDSH_LOG (PAGER, "pipe() failed: %s.", strerror (errno));
      return;
    }
#if 0
  else
    DEBUG_ZCMDSH_LOG (PAGER, "pipe() success: pipefd[] = {%d, %d}",
                      shell->pipefd[0], shell->pipefd[1]);
#endif

  process_id = fork ();
  if (process_id < 0)
    {
      DEBUG_ZCMDSH_LOG (PAGER, "fork() failed: close pipefd and pty");
      close (shell->pipefd[0]);
      close (shell->pipefd[1]);
      close (shell->pty_master);
      close (shell->pty_slave);
      return;
    }
  else if (process_id == 0)
    {
      DEBUG_ZCMDSH_LOG (PAGER, "child ok: pid: %d.", getpid ());

      /* child */
      close (shell->pipefd[1]);
      close (shell->pty_master);

      DEBUG_ZCMDSH_LOG (PAGER, "pty(slave): fd %d, %s.", shell->pty_slave,
                        ttyname (shell->pty_slave));

      char *lessargs[3];
      lessargs[0] = DEFAULT_PAGER_ARG0;
      lessargs[1] = DEFAULT_PAGER_ARG1;
      lessargs[2] = NULL;
      char *path = lessargs[0];

      DEBUG_ZCMDSH_LOG (PAGER, "execvp: %s %s", lessargs[0], lessargs[1]);

      dup2 (shell->pipefd[0], 0);
      dup2 (shell->pty_slave, 1);
      dup2 (shell->pty_slave, 2);
      stdin = fdopen (0, "r");
      stdout = fdopen (1, "w");
      stderr = fdopen (2, "r+");

      ret = execvp (path, lessargs);
      if (ret < 0)
        {
          DEBUG_ZCMDSH_LOG (PAGER, "execvp() failed: ret: %d: %s.", ret,
                            strerror (errno));
          exit (-1);
        }
    }
  else if (process_id > 0)
    {
      DEBUG_ZCMDSH_LOG (PAGER, "parent ok: pid: %d.", getpid ());
      shell->pager_pid = process_id;

      /* parent */
      close (shell->pipefd[0]);
      close (shell->pty_slave);

      shell->pager_saved_terminal = shell->terminal;
      shell->pager_saved_writefd = shell->writefd;

      shell->terminal = fdopen (shell->pipefd[1], "a");
      shell->writefd = shell->pipefd[1];

      DEBUG_ZCMDSH_LOG (PAGER, "pager start: terminal: %p <- %p.",
                        shell->terminal, shell->pager_saved_terminal);
    }
#endif
  shell->is_paging = true;
}

void
pager_end (struct shell *shell)
{
#if ! PAGER_USE_POPEN
  int wstatus;

  // DEBUG_ZCMDSH_LOG (PAGER, "pager: close forked child.");

  if (shell->pager_pid == 0)
    {
      DEBUG_ZCMDSH_LOG (PAGER, "should not be reached.");
      exit (-1);
    }

  close (shell->pipefd[1]);
#endif

  if (shell->pager_saved_terminal)
    {
      DEBUG_ZCMDSH_LOG (PAGER, "pager end: terminal: %p -> %p.",
                        shell->terminal, shell->pager_saved_terminal);
      pclose (shell->terminal);
      shell->terminal = shell->pager_saved_terminal;
      shell->writefd = shell->pager_saved_writefd;
      shell->pager_saved_terminal = NULL;
      shell->pager_saved_writefd = -1;
    }

#if ! PAGER_USE_POPEN
  //DEBUG_ZCMDSH_LOG (PAGER, "pager: bridging sockets and pty...");
  int ret, nwrite;
  struct pollfd fds[2];
  nfds_t nfds = 2;
  char buf[1024];
  int closed = 0;
  int i, j;
  while (1)
    {
      // DEBUG_ZCMDSH_LOG (PAGER, "pager: reset pollfds");
      memset (fds, 0, sizeof (fds));
      fds[0].fd = shell->readfd;
      fds[0].events |= POLLIN;
      fds[1].fd = shell->pty_master;
      fds[1].events |= POLLIN;
      ret = poll (fds, nfds, 0);
      if (ret < 0)
        {
          DEBUG_ZCMDSH_LOG (PAGER, "pager: ppoll() returned: %d: %s", ret,
                            strerror (errno));
          break;
        }

      for (i = 0; i < nfds; i++)
        {
          int readfd, writefd;

#if 0
          if (fds[i].revents & POLLHUP)
            closed++;
#endif

          if (! (fds[i].revents & (POLLIN | POLLHUP)))
            continue;

          DEBUG_ZCMDSH_LOG (
              PAGER, "pager: fds[%d]: fd: %d revents:%s%s%s %#x", i, fds[i].fd,
              (fds[i].revents & POLLIN ? " POLLIN" : ""),
              (fds[i].revents & POLLHUP ? " POLLHUP" : ""),
              (fds[i].revents & POLLERR ? " POLLERR" : ""), fds[i].revents);

          readfd = fds[i].fd;
          j = (i & 0x01) ^ 0x01;
          writefd = fds[j].fd;
          // DEBUG_ZCMDSH_LOG (PAGER, "pager: i: %d j: %d", i, j);

          ret = read (readfd, buf, sizeof (buf));
          if (ret < 0)
            {
              DEBUG_ZCMDSH_LOG (PAGER,
                                "pager: read() from fd: %d returned %d: %s",
                                readfd, ret, strerror (errno));
              closed++;
              continue;
            }
          else if (ret == 0)
            {
              DEBUG_ZCMDSH_LOG (PAGER, "pager: fds[%d]: fd: %d: closed, break",
                                i, fds[i].fd);
              closed++;
            }
          else
            {
              nwrite = write (writefd, buf, ret);
              if (nwrite < 0)
                DEBUG_ZCMDSH_LOG (PAGER, "write() failed: %s",
                                  strerror (errno));
              DEBUG_ZCMDSH_LOG (PAGER, "pager: fd: %d -> fd: %d, %d bytes",
                                readfd, writefd, ret);
            }
        }

      if (closed)
        break;
    }

  DEBUG_ZCMDSH_LOG (PAGER, "pager: waiting child: process %d...",
                    shell->pager_pid);
  waitpid (shell->pager_pid, &wstatus, 0);
  if (WIFEXITED (wstatus))
    {
      DEBUG_ZCMDSH_LOG (PAGER, "pager: process %d exited normally.",
                        shell->pager_pid);
    }
  else
    {
      DEBUG_ZCMDSH_LOG (PAGER, "pager: process %d failed.", shell->pager_pid);
    }
#endif

  shell->is_paging = false;
}

void
shell_read_nowait_paging (struct shell *shell)
{
  int wstatus;

  close (shell->pipefd[1]);

  if (shell->pager_saved_terminal)
    {
      DEBUG_ZCMDSH_LOG (PAGER, "pager end: terminal: %p -> %p.",
                        shell->terminal, shell->pager_saved_terminal);
      pclose (shell->terminal);
      shell->terminal = shell->pager_saved_terminal;
      shell->writefd = shell->pager_saved_writefd;
      shell->pager_saved_terminal = NULL;
      shell->pager_saved_writefd = -1;
    }

  //DEBUG_ZCMDSH_LOG (PAGER, "pager: bridging sockets and pty...");

  int ret, nwrite;
  struct pollfd fds[2];
  nfds_t nfds = 2;
  char buf[1024];
  int closed = 0;
  int i, j;

  //while (1)
    {
      // DEBUG_ZCMDSH_LOG (PAGER, "pager: reset pollfds");
      memset (fds, 0, sizeof (fds));
      fds[0].fd = shell->readfd;
      fds[0].events |= POLLIN;
      fds[1].fd = shell->pty_master;
      fds[1].events |= POLLIN;
      ret = poll (fds, nfds, 0);
      if (ret < 0)
        {
          DEBUG_ZCMDSH_LOG (PAGER, "pager: ppoll() returned: %d: %s", ret,
                            strerror (errno));
          return;
        }

      for (i = 0; i < nfds; i++)
        {
          int readfd, writefd;

          if (! (fds[i].revents & (POLLIN | POLLHUP)))
            continue;

          if (fds[i].revents & POLLHUP)
            closed++;

          DEBUG_ZCMDSH_LOG (
              PAGER, "pager: fds[%d]: fd: %d revents:%s%s%s %#x", i, fds[i].fd,
              (fds[i].revents & POLLIN ? " POLLIN" : ""),
              (fds[i].revents & POLLHUP ? " POLLHUP" : ""),
              (fds[i].revents & POLLERR ? " POLLERR" : ""), fds[i].revents);

          readfd = fds[i].fd;
          j = (i & 0x01) ^ 0x01;
          writefd = fds[j].fd;
          // DEBUG_ZCMDSH_LOG (PAGER, "pager: i: %d j: %d", i, j);

          ret = read (readfd, buf, sizeof (buf));
          if (ret < 0)
            {
              DEBUG_ZCMDSH_LOG (PAGER,
                                "pager: read() from fd: %d returned %d: %s",
                                readfd, ret, strerror (errno));
              closed++;
              continue;
            }
          else if (ret == 0)
            {
              DEBUG_ZCMDSH_LOG (PAGER, "pager: fds[%d]: fd: %d: closed, break",
                                i, fds[i].fd);
              closed++;
            }
          else
            {
              nwrite = write (writefd, buf, ret);
              if (nwrite < 0)
                DEBUG_ZCMDSH_LOG (PAGER, "write() failed: %s",
                                  strerror (errno));
              DEBUG_ZCMDSH_LOG (PAGER, "pager: fd: %d -> fd: %d, %d bytes",
                                readfd, writefd, ret);
              char str_buf[1024];
              char *curr = str_buf;
              char *end = str_buf + 1024;
              int ret2;
              for (i = 0; i < ret; i++)
                {
                  ret2 = 0;
                  if (isascii ((int) buf[i]))
                    ret2 = snprintf (curr, end - curr, "%c", buf[i]);
                  else
                    ret2 = snprintf (curr, end - curr, "%#x", buf[i]);
                  curr += ret2;
                }
              DEBUG_ZCMDSH_LOG (PAGER, "pager: buf: %s", str_buf);
            }
        }
    }

  if (! closed)
    return;

  DEBUG_ZCMDSH_LOG (PAGER, "pager: waiting child: process %d...",
                    shell->pager_pid);
  waitpid (shell->pager_pid, &wstatus, 0);
  if (WIFEXITED (wstatus))
    {
      DEBUG_ZCMDSH_LOG (PAGER, "pager: process %d exited normally.",
                        shell->pager_pid);
    }
  else
    {
      DEBUG_ZCMDSH_LOG (PAGER, "pager: process %d failed.", shell->pager_pid);
    }

  shell->is_paging = false;

  shell_clear (shell);
  shell_prompt (shell);
  shell_refresh (shell);
}

int
command_shell_execute (struct shell *shell)
{
  int ret = 0;
  char *comment;

  /* send a line-feed to terinate the comand line. */
  shell_linefeed (shell);

  /* remove the comment part, until the line-end. */
  comment = strpbrk (shell->command_line, "#!");
  if (comment)
    {
      shell->end = comment - shell->command_line;
      shell->cursor =
          (shell->cursor > shell->end ? shell->end : shell->cursor);
      shell_terminate (shell);
    }

  if (strlen (shell->command_line))
    {
      /* remove unnecessary spaces. */
      shell_format2 (shell);

      if (shell->pager)
        pager_start (shell);

      shell_linefeed (shell);

      if (FLAG_CHECK (shell->flag, SHELL_FLAG_DEBUG))
        fprintf (shell->terminal, "command started: %s%s",
                 shell->command_line, shell->NL);

      ret = command_execute (shell->command_line, shell->cmdset, shell);

      if (FLAG_CHECK (shell->flag, SHELL_FLAG_DEBUG))
        fprintf (shell->terminal, "command finished: %s%s",
                 shell->command_line, shell->NL);

      /* record the execution of the command. */
      DEBUG_ZCMDSH_LOG (COMMAND_LOG, "command-log: %s",
                        shell->command_line);

      shell->cmd_status = ret;
      if (ret == CMD_NOT_FOUND)
        {
          DEBUG_ZCMDSH_LOG (COMMAND_LOG, "command-log: not found");
          fprintf (shell->terminal, "no such command: %s%s",
                   shell->command_line, shell->NL);
          ret = -1;
        }
      if (ret == CMD_FAILURE)
        {
          DEBUG_ZCMDSH_LOG (COMMAND_LOG, "command-log: failed");
          fprintf (shell->terminal, "command failed: %s%s",
                   shell->command_line, shell->NL);
          ret = -1;
        }

      command_history_add (shell->command_line, shell->history, shell);

      if (shell->is_paging)
        {
          fprintf (shell->terminal, "read_nowait_paging from comand_shell_execute.%s",
                   shell->NL);

          /* fflush() is mandatory before closing the pager.*/
          fflush (shell->terminal);

          shell_read_nowait_paging (shell);
        }
    }

  if (! shell_running (shell))
    return ret;

  if (! shell->is_paging)
    {
      shell_clear (shell);
      shell_prompt (shell);
      if (FLAG_CHECK (shell->flag, SHELL_FLAG_INTERACTIVE))
        shell_refresh (shell);
    }

  return ret;
}

int
command_shell_completion (struct shell *shell)
{
  char *completion = NULL;

  shell_moveto (shell, shell_word_end (shell, shell->cursor));
  completion =
      command_complete (shell->command_line, shell->cursor, shell->cmdset);
  if (completion)
    shell_insert (shell, completion);

  fflush (shell->terminal);
  return 0;
}

static void
print_dirent (struct shell *shell, struct dirent *dirent, int num, int ncolumn,
              int print_width)
{
  char printname[1024];
  char *suffix;

  suffix = "";
  if (dirent->d_type == DT_DIR)
    suffix = "/";
  snprintf (printname, sizeof (printname), "%s%s", dirent->d_name, suffix);

  if (FLAG_CHECK (debug_config, DEBUG_SHELL))
    fprintf (shell->terminal, "%snum: %d ptr: %p ncolumn: %d %s dirent: %s%s",
             shell->NL, num, dirent, ncolumn, printname, dirent->d_name,
             shell->NL);

  if (num % ncolumn == 0)
    fprintf (shell->terminal, "  ");

  if (ncolumn != 1)
    fprintf (shell->terminal, "%-*s", print_width, printname);
  else
    fprintf (shell->terminal, "%s", printname);

  if (num % ncolumn == ncolumn - 1)
    fprintf (shell->terminal, "%s", shell->NL);
}

void
file_ls_candidate (struct shell *shell, char *file_path)
{
  char *path = strdup (file_path);
  char *dirname;
  char *filename;
  DIR *dir;
  struct dirent *dirent;
  int i;

  path_disassemble (path, &dirname, &filename);
  if (FLAG_CHECK (debug_config, DEBUG_SHELL))
    {
      fprintf (shell->terminal, "  path: %s dir: %s filename: %s%s", file_path,
               dirname, filename, shell->NL);
    }

  dir = opendir (dirname);
  if (dir == NULL)
    {
      fprintf (shell->terminal, "opendir() failed: %s dir: %s%s", file_path,
               dirname, shell->NL);
      free (path);
      return;
    }

  int maxlen = 0;
  int nentry = 0;
  while ((dirent = readdir (dir)) != NULL)
    {
      /* everything starts with '.' are hidden. */
      if (dirent->d_name[0] == '.')
        continue;

      /* filter by the pattern */
      if (strncmp (dirent->d_name, filename, strlen (filename)))
        continue;

      /* calculate the maxmum entry name length. */
      maxlen = (maxlen < strlen (dirent->d_name) ? strlen (dirent->d_name)
                                                 : maxlen);

      nentry++;
    }
  rewinddir (dir);

  int sort_size;
  struct dirent **sort_vector;

  sort_size = nentry;
  sort_vector = malloc (sizeof (struct dirent *) * sort_size);

  int num;
  int ncolumn;
  ncolumn = (shell->winsize.ws_col - 2) / (maxlen + 2);
  if (ncolumn == 0)
    ncolumn = 1;

  if (FLAG_CHECK (debug_config, DEBUG_SHELL))
    {
      fprintf (shell->terminal,
               "  %s: nentry: %d maxlen: %d ncol: %d "
               "sort_vector: %p%s",
               __func__, nentry, maxlen, ncolumn, sort_vector, shell->NL);
    }

  fprintf (shell->terminal, "%s", shell->NL);

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
        {
          sort_vector[num] = dirent_copy (dirent);
#if 0
          fprintf (shell->terminal, "sort_vec[%d]: %p: %s%s",
                   num, dirent, dirent->d_name, shell->NL);
#endif
        }
      else
        print_dirent (shell, dirent, num, ncolumn, maxlen + 2);

      num++;
    }
  closedir (dir);

  if (sort_vector)
    {
      qsort ((void *) sort_vector, sort_size, sizeof (struct dirent *),
             dirent_cmp);

      for (i = 0; i < sort_size; i++)
        print_dirent (shell, sort_vector[i], i, ncolumn, maxlen + 2);
    }

  fprintf (shell->terminal, "%s", shell->NL);

  for (i = 0; i < sort_size; i++)
    free (sort_vector[i]);
  free (sort_vector);

  free (path);
}

int
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
      shell_refresh (shell);
      shell_format (shell);
      shell_linefeed (shell);
      shell_refresh (shell);
      // return;
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
        fprintf (shell->terminal, "  %-16s %s%s", "<cr>", match->helpstr,
                 shell->NL);

      for (vn = vector_head (match->cmdvec); vn; vn = vector_next (vn))
        {
          node = (struct command_node *) vn->data;
          if (is_command_match (node->cmdstr, last))
            fprintf (shell->terminal, "  %-16s %s%s", node->cmdstr,
                     node->helpstr, shell->NL);

          if (file_spec (node->cmdstr))
            file_ls_candidate (shell, last);
        }
    }

  free (last);
  free (cmd_dup);

  // shell_format (shell);
  shell_linefeed (shell);
  shell_refresh (shell);
  return 0;
}

DEFINE_COMMAND (list_func_table, "list func-table",
                "list.\n"
                "list func-table.\n")
{
  struct shell *shell = (struct shell *) context;
  int i;
  for (i = 0; i < FUNC_TABLE_SIZE; i++)
    {
      if (func2str[i].ptr)
        fprintf (shell->terminal, "  func2str[%d]: %p: %s%s", i,
                 func2str[i].ptr, func2str[i].str, shell->NL);
    }
  return 0;
}

int
func_table_lookup (shell_keyfunc_t ptr)
{
  int i;
  for (i = 0; i < FUNC_TABLE_SIZE; i++)
    {
      if (func2str[i].ptr == ptr)
        return i;
    }
  return -1;
}

#define FUNC_STR_MAP(x)                                                       \
  {                                                                           \
    x, #x                                                            \
  }
struct funcp_str_map func2str[FUNC_TABLE_SIZE] = {
  FUNC_STR_MAP (shell_terminate),
  FUNC_STR_MAP (shell_format),
  FUNC_STR_MAP (shell_linefeed),
  FUNC_STR_MAP (shell_clear),
  FUNC_STR_MAP (shell_delete_word_backward),
  FUNC_STR_MAP (shell_move_word_backward),
  FUNC_STR_MAP (shell_move_word_forward),
  FUNC_STR_MAP (shell_refresh),
  FUNC_STR_MAP (shell_read),
  FUNC_STR_MAP (shell_read_nowait),
  FUNC_STR_MAP (shell_keyfunc_forward_char),
  FUNC_STR_MAP (shell_keyfunc_backward_char),
  FUNC_STR_MAP (shell_keyfunc_move_to_begin),
  FUNC_STR_MAP (shell_keyfunc_move_to_end),
  FUNC_STR_MAP (shell_keyfunc_delete_char),
  FUNC_STR_MAP (shell_keyfunc_backspace),
  FUNC_STR_MAP (shell_keyfunc_kill_line),
  FUNC_STR_MAP (shell_keyfunc_kill_all),
  FUNC_STR_MAP (shell_keyfunc_yank),
  FUNC_STR_MAP (shell_keyfunc_clear_screen),
  FUNC_STR_MAP (shell_keyfunc_refresh),
  FUNC_STR_MAP (shell_keyfunc_empty_enter),
  FUNC_STR_MAP (shell_keyfunc_insert_tab),
  FUNC_STR_MAP (shell_keyfunc_escape),
  FUNC_STR_MAP (shell_keyfunc_delete_word_backward),
  FUNC_STR_MAP (shell_input_char),

  FUNC_STR_MAP (command_shell_execute),
  FUNC_STR_MAP (command_shell_execute),
  FUNC_STR_MAP (command_shell_completion),
  FUNC_STR_MAP (command_shell_ls_candidate),
  FUNC_STR_MAP (command_history_prev),
  FUNC_STR_MAP (command_history_next),
};

DEFINE_COMMAND (list_keymaps, "list keymaps",
                "list.\n"
                "list keymaps.\n")
{
  struct shell *shell = (struct shell *) context;
  int i;
  //void *ptr;
  shell_keyfunc_t ptr;
  char *name;
  int index = -1;
  int start = -1;
  shell_keyfunc_t next_ptr = NULL;
  char strname[32];

  bool summary = true;

  for (i = 0; i <= 0xff; i++)
    {
      ptr = shell->keymap[i];
      if (! ptr)
        continue;

      name = "(name unknown)";
      index = func_table_lookup (ptr);
      if (index >= 0)
        name = func2str[index].str;

      if (summary)
        {
          if (i + 1 <= 0xff)
            {
              next_ptr = shell->keymap[i + 1];
              if (next_ptr && next_ptr == ptr)
                {
                  if (start < 0)
                    start = i;
                  continue;
                }
            }

          if (start >= 0)
            {
              shell_keyfunc_t prev_ptr;
              int prev_index;
              char *prev_name;
              prev_ptr = shell->keymap[start];
              assert (prev_ptr);
              prev_name = "(name unknown)";
              prev_index = func_table_lookup (prev_ptr);
              if (prev_index >= 0)
                prev_name = func2str[prev_index].str;

              if (isalnum (start + '@'))
                snprintf (strname, sizeof (strname), " CONTROL('%c')",
                          start + '@');
              else if (isascii (start) && ! iscntrl (start))
                snprintf (strname, sizeof (strname), " ('%c')", start);
              else
                memset (strname, 0, sizeof (strname));

              fprintf (shell->terminal, "  key: 0x%02x (%3d)%-14s%s", start,
                       start, strname, shell->NL);

              fprintf (shell->terminal, "%-31s %p: %s%s", "          :",
                       (void *) prev_ptr, prev_name, shell->NL);
            }
        }

      if (isalnum (i + '@'))
        snprintf (strname, sizeof (strname), " CONTROL('%c')", i + '@');
      else if (isascii (i) && ! iscntrl (i))
        snprintf (strname, sizeof (strname), " ('%c')", i);
      else
        memset (strname, 0, sizeof (strname));

      if (start >= 0)
        fprintf (shell->terminal, "  key: 0x%02x (%3d)%-14s%s", i, i, strname,
                 shell->NL);
      else
        fprintf (shell->terminal, "  key: 0x%02x (%3d)%-14s %p: %s%s", i, i,
                 strname, (void *) ptr, name, shell->NL);

      start = -1;
    }
  return 0;
}

DEFINE_COMMAND (set_pager, "(set|no|) pager",
                "set command.\n"
                "no command.\n"
                "pager command.\n")
{
  struct shell *shell = (struct shell *) context;
  bool value = false;

  if (! strcmp (argv[0], "set"))
    value = true;
  else if (! strcmp (argv[0], "no"))
    {
      value = false;
    }
  else if (argc == 1)
    value = true;
  DEBUG_ZCMDSH_LOG (PAGER, "pager: %d", (int) value);
  shell->pager = value;
  if (value == true)
    fprintf (shell->terminal, "pager enabled.%s", shell->NL);
  else
    fprintf (shell->terminal, "pager disabled.%s", shell->NL);
  return 0;
}

DEFINE_COMMAND (set_pager_command, "set pager <FILE> (|<LINE>)",
                "set.\n"
                "pager.\n"
                "pager command.\n"
                "pager command option.\n")
{
  struct shell *shell = (struct shell *) context;

  int i;
  int ret;
  char line[64];
  char *p = line;

  if (shell->pager_command)
    {
      free (shell->pager_command);
      shell->pager_command = NULL;
    }

  for (i = 2; i < argc; i++)
    {
      ret = snprintf (p, sizeof (line) - (p - line), (i == 2 ? "%s" : " %s"),
                      argv[i]);
      p += ret;
    }

  shell->pager_command = strdup (line);
  shell->pager = true;
  return 0;
}

DEFINE_COMMAND (set_pager_default, "set pager default",
                "set.\n"
                "pager.\n"
                "pager default.\n")
{
  struct shell *shell = (struct shell *) context;
  if (shell->pager_command)
    {
      free (shell->pager_command);
      shell->pager_command = NULL;
    }
  return 0;
}

void
default_install_command (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, exit);
  INSTALL_COMMAND2 (cmdset, quit);
  INSTALL_COMMAND2 (cmdset, logout);
  INSTALL_COMMAND2 (cmdset, list_func_table);
  INSTALL_COMMAND2 (cmdset, list_keymaps);
  INSTALL_COMMAND2 (cmdset, show_history);
  INSTALL_COMMAND2 (cmdset, set_pager);
  INSTALL_COMMAND2 (cmdset, set_pager_command);
  INSTALL_COMMAND2 (cmdset, set_pager_default);

  INSTALL_COMMAND (cmdset, enable_shell_debugging);
  INSTALL_COMMAND (cmdset, disable_shell_debugging);
}

struct shell *
command_shell_create ()
{
  struct shell *shell;

  shell = shell_create ();
  shell->cmdset = command_set_create ();
  default_install_command (shell->cmdset);

  shell_install (shell, CONTROL ('J'), command_shell_execute);
  shell_install (shell, CONTROL ('M'), command_shell_execute);
  shell_install (shell, CONTROL ('I'), command_shell_completion);
  shell_install (shell, '?', command_shell_ls_candidate);
  shell_install (shell, CONTROL ('P'), command_history_prev);
  shell_install (shell, CONTROL ('N'), command_history_next);

  shell->pager = true;

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
