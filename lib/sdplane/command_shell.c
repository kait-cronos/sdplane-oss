/*
 * Copyright (C) 2007-2023 Yasuhiro Ohara. All rights reserved.
 */

#include <poll.h>
#include <stdbool.h>

#if HAVE_UTIL_H
#include <util.h> // for openpty()
#endif /*HAVE_UTIL_H*/

#include "includes.h"
#include "command.h"
#include "command_shell.h"
#include "file.h"
#include "shell.h"
#include "shell_keyfunc.h"
#include "vector.h"
#include "debug_log.h"

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

  if (! history)
    return;
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

  /* update the time the command was added in the history */
  if (history->array[history->last] &&
      ! strcmp (history->array[history->last], command_line))
    {
      /* ommit adding identical command_line. */
      time (&history->clock[history->last]);
      return;
    }

  time (&history->clock[history->current]);
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
  if (next == floor)
    {
      shell_delete_string (shell, 0, shell->end);
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
  struct tm *tm;
#define TIMEBUFSIZ 32
  char timebuf[TIMEBUFSIZ];

  if (! history)
    return CMD_SUCCESS;

  floor = HISTORY_NEXT (history->last);
  start = HISTORY_NEXT (floor);

  for (i = start; i != floor; i = HISTORY_NEXT (i))
    if (history->array[i])
      {
        tm = localtime (&history->clock[i]);
        strftime (timebuf, sizeof (timebuf), "%Y/%m/%d %H:%M:%S", tm);
        fprintf (shell->terminal, "[%3d] %s %s\n",
                 i, timebuf, history->array[i]);
      }
  return CMD_SUCCESS;
}

#include <poll.h>
#if HAVE_PTY_H
#include <pty.h>
#endif /*HAVE_PTY_H*/
#include <signal.h>

/* pager pipe stages: filter commands (such as moreutils sponge)
   inserted between the command output and the pager, in the
   index order: command output -> stage[0] -> ... -> pager.
   an unset stage (argc == 0) is skipped. */
struct pager_pipe_stage
{
  int argc;
  char *argv[PAGER_PIPE_ARGV_MAX + 1]; /* NULL-terminated for execvp(). */
};

static struct pager_pipe_stage pager_pipe_stages[SHELL_PAGER_PIPE_MAX];

/* the default pager command: the strace-wrapped less,
   kept from the previous behavior. */
char *pager_command_default[] = {
  "strace", "-o", "/tmp/less.trace",
  "/usr/bin/less", "-FX",
  NULL
};

/* the pager command configured by 'set pager command <LINE>'.
   when unset (argc == 0), pager_command_default[] is used. */
static struct pager_pipe_stage pager_command_config;

static void
pager_stage_clear (struct pager_pipe_stage *stage)
{
  int i;

  for (i = 0; i < stage->argc; i++)
    {
      free (stage->argv[i]);
      stage->argv[i] = NULL;
    }
  stage->argc = 0;
}

static int
pager_stage_assign (struct pager_pipe_stage *stage, int stage_argc,
                    char **stage_argv)
{
  int i;

  if (stage_argc < 1 || stage_argc > PAGER_PIPE_ARGV_MAX)
    return -1;

  pager_stage_clear (stage);

  for (i = 0; i < stage_argc; i++)
    stage->argv[i] = strdup (stage_argv[i]);
  stage->argv[stage_argc] = NULL;
  stage->argc = stage_argc;

  return 0;
}

/* join the effective pager pipeline: the pipe stages and the pager
   command (the configured one, or the default), connected with '|',
   into buf[] as a single line, for popen()/display.
   e.g. "sponge | strace -o /tmp/less.trace /usr/bin/less -FX".
   returns buf (truncated when it does not fit). */
static char *
pager_command_line_join (char *buf, size_t size)
{
  char **pager_argv;
  char *p = buf;
  char *e = buf + size;
  int nwrite;
  int i, j;

  buf[0] = '\0';

  /* the pipe stages first: "stage0 | stage1 | ". */
  for (i = 0; i < SHELL_PAGER_PIPE_MAX; i++)
    {
      struct pager_pipe_stage *stage = &pager_pipe_stages[i];
      if (! stage->argc)
        continue;
      for (j = 0; j < stage->argc; j++)
        {
          nwrite = snprintf (p, e - p, (j == 0 ? "%s" : " %s"),
                             stage->argv[j]);
          if (nwrite < 0 || nwrite >= e - p)
            return buf; /* truncated: give up appending. */
          p += nwrite;
        }
      nwrite = snprintf (p, e - p, " | ");
      if (nwrite < 0 || nwrite >= e - p)
        return buf; /* truncated: give up appending. */
      p += nwrite;
    }

  /* then the pager command. */
  pager_argv = (pager_command_config.argc ? pager_command_config.argv
                                          : pager_command_default);
  for (i = 0; pager_argv[i]; i++)
    {
      nwrite = snprintf (p, e - p, (i == 0 ? "%s" : " %s"), pager_argv[i]);
      if (nwrite < 0 || nwrite >= e - p)
        return buf; /* truncated: give up appending. */
      p += nwrite;
    }

  return buf;
}

/* split a command line into words. the words point into line[]:
   the caller keeps the ownership of line[].
   returns the number of words, or -1 when more than max words. */
static int
pager_command_line_split (char *line, char **words, int max)
{
  char *stringp = line;
  char *word;
  int nwords = 0;

  while ((word = strsep (&stringp, COMMAND_WORD_DELIMITERS)) != NULL)
    {
      if (*word == '\0')
        continue;
      if (nwords >= max)
        return -1;
      words[nwords++] = word;
    }

  return nwords;
}

int
pager_pipe_stage_set (int index, int stage_argc, char **stage_argv)
{
  if (index < 0 || index >= SHELL_PAGER_PIPE_MAX)
    return -1;

  return pager_stage_assign (&pager_pipe_stages[index],
                             stage_argc, stage_argv);
}

int
pager_pipe_stage_unset (int index)
{
  if (index < 0 || index >= SHELL_PAGER_PIPE_MAX)
    return -1;

  pager_stage_clear (&pager_pipe_stages[index]);

  return 0;
}

int
pager_command_set (int cmd_argc, char **cmd_argv)
{
  return pager_stage_assign (&pager_command_config, cmd_argc, cmd_argv);
}

int
pager_command_unset (void)
{
  pager_stage_clear (&pager_command_config);
  return 0;
}

/* fork the configured filter stages, connecting them with pipes:
   source_fd -> stage[0] -> ... -> stage[N-1] -> (returned fd).
   returns the read-end fd the pager should read from.
   the forked pids are recorded in shell->pager_pipe_pid[]. */
static int
pager_pipe_stages_start (struct shell *shell, int source_fd)
{
  int ret;
  int i;
  int stagefd[2];
  pid_t process_id;

  shell->pager_pipe_nstages = 0;

  for (i = 0; i < SHELL_PAGER_PIPE_MAX; i++)
    {
      struct pager_pipe_stage *stage = &pager_pipe_stages[i];

      if (! stage->argc)
        continue;

      ret = pipe (stagefd);
      if (ret < 0)
        {
          DEBUG_NEW (PAGER, "stage[%d]: pipe() failed: %s.",
                     i, strerror (errno));
          break;
        }

      process_id = fork ();
      if (process_id < 0)
        {
          DEBUG_NEW (PAGER, "stage[%d]: fork() failed: %s.",
                     i, strerror (errno));
          close (stagefd[0]);
          close (stagefd[1]);
          break;
        }
      else if (process_id == 0)
        {
          /* stage child: source_fd -> argv[] -> stagefd[1]. */
          close (stagefd[0]);
          close (shell->pipefd[1]);
          close (shell->pty_master);

          dup2 (source_fd, 0);
          close (source_fd);
          dup2 (stagefd[1], 1);
          close (stagefd[1]);

          /* keep the error messages visible on the pager terminal. */
          dup2 (shell->pty_slave, 2);
          close (shell->pty_slave);

          execvp (stage->argv[0], stage->argv);

          DEBUG_NEW (PAGER, "stage[%d]: execvp() failed: %s",
                     i, strerror (errno));
          DEBUG_NEW (PAGER, "stage[%d]: execvp(): argc: %d argv[0]: %s",
                     i, stage->argc, stage->argv[0]);

          /* exec failed: fall back to a plain passthrough (like cat),
             so that the command output is not lost. */
          {
            char fbuf[1024];
            int nread;
            while ((nread = read (0, fbuf, sizeof (fbuf))) > 0)
              if (write (1, fbuf, nread) < 0)
                break;
          }
          _exit (127);
        }

      /* parent. */
      DEBUG_NEW (PAGER, "stage[%d]: %s: pid: %d.",
                 i, stage->argv[0], (int) process_id);
      close (stagefd[1]);
      close (source_fd);
      source_fd = stagefd[0];
      shell->pager_pipe_pid[shell->pager_pipe_nstages++] = process_id;
    }

  return source_fd;
}

/* reap the filter stage processes. called after the pager exited. */
static void
pager_pipe_stages_reap (struct shell *shell)
{
  int i;
  int wstatus;
  pid_t pid, ret;

  for (i = 0; i < shell->pager_pipe_nstages; i++)
    {
      pid = shell->pager_pipe_pid[i];
      if (pid <= 0)
        continue;

      do
        ret = waitpid (pid, &wstatus, WNOHANG);
      while (ret < 0 && errno == EINTR);
      if (ret == 0)
        {
          /* the stage may keep running (e.g. sponge feeding a
             pager that the user has quit): terminate it. */
          DEBUG_ZCMDSH_LOG (PAGER, "pager: stage process %d: SIGTERM.",
                            (int) pid);
          kill (pid, SIGTERM);
          do
            ret = waitpid (pid, &wstatus, 0);
          while (ret < 0 && errno == EINTR);
        }

      DEBUG_ZCMDSH_LOG (PAGER, "pager: stage process %d reaped.", (int) pid);
      shell->pager_pipe_pid[i] = 0;
    }

  shell->pager_pipe_nstages = 0;
}

static void
pager_process_cancel (pid_t *pid)
{
  pid_t ret;
  int status;

  if (*pid <= 0)
    return;
  do
    ret = waitpid (*pid, &status, WNOHANG);
  while (ret < 0 && errno == EINTR);
  if (ret == 0)
    {
      /* The pager creates a session; include children such as less. */
      if (getpgid (*pid) == *pid)
        kill (-*pid, SIGKILL);
      kill (*pid, SIGKILL);
      do
        ret = waitpid (*pid, &status, 0);
      while (ret < 0 && errno == EINTR);
    }
  *pid = 0;
}

static void
pager_output_close (struct shell *shell)
{
  if (shell->pager_saved_terminal)
    {
      fclose (shell->terminal);
      shell->pipefd[1] = -1;
      shell->terminal = shell->pager_saved_terminal;
      shell->writefd = shell->pager_saved_writefd;
      shell->pager_saved_terminal = NULL;
      shell->pager_saved_writefd = -1;
    }
  else if (shell->pipefd[1] >= 0)
    {
      close (shell->pipefd[1]);
      shell->pipefd[1] = -1;
    }
}

static void
pager_fds_close (struct shell *shell)
{
  if (shell->pipefd[0] >= 0)
    close (shell->pipefd[0]);
  shell->pipefd[0] = -1;
  if (shell->pty_master >= 0)
    close (shell->pty_master);
  shell->pty_master = -1;
  if (shell->pty_slave >= 0)
    close (shell->pty_slave);
  shell->pty_slave = -1;
}

static void
pager_cleanup (struct shell *shell)
{
  int i;

  pager_output_close (shell);
  pager_process_cancel (&shell->pager_pid);
  for (i = 0; i < shell->pager_pipe_nstages; i++)
    pager_process_cancel (&shell->pager_pipe_pid[i]);
  shell->pager_pipe_nstages = 0;

  pager_fds_close (shell);
  shell->is_paging = false;
}

#define PAGER_USE_POPEN 0
void
pager_start (struct shell *shell)
{
  if (shell->is_paging)
    return;
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
      char pager_line[256];
      pager_command_line_join (pager_line, sizeof (pager_line));
      DEBUG_ZCMDSH_LOG (PAGER, "default pager: %s", pager_line);
      pager_fp = popen (pager_line, "w");
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

  char slave_name[256];
  char *slave_name_ptr;

  ret = openpty (&shell->pty_master, &shell->pty_slave, slave_name, NULL, NULL);
  if (ret < 0)
    {
      DEBUG_NEW (PAGER, "openpty() failed: %s.", strerror (errno));
      pager_cleanup (shell);
      return;
    }
  slave_name_ptr = ptsname (shell->pty_slave);
  DEBUG_NEW (PAGER, "pty(master): fd %d, %s.", shell->pty_master,
                    ttyname (shell->pty_master));
  DEBUG_NEW (PAGER, "pty(slave): fd %d, ttyname: %s "
                    "slave_name: %s ptsname: %s.", shell->pty_slave,
                    ttyname (shell->pty_slave), slave_name, slave_name_ptr);

  if (shell->winsize.ws_col && shell->winsize.ws_row)
    {
      ioctl (shell->pty_master, TIOCSWINSZ, &shell->winsize);
      DEBUG_NEW (PAGER, "pager: TIOCSWINSZ: row: %d col: %d.",
                 shell->winsize.ws_row, shell->winsize.ws_col);
    }

  DEBUG_NEW (PAGER, "pager: use fork/execvp.");
  ret = pipe (shell->pipefd);
  if (ret < 0)
    {
      DEBUG_NEW (PAGER, "pipe() failed: %s.", strerror (errno));
      pager_cleanup (shell);
      return;
    }
#if 0
  else
    DEBUG_NEW (PAGER, "pipe() success: pipefd[] = {%d, %d}",
                      shell->pipefd[0], shell->pipefd[1]);
#endif

  /* insert the configured filter stages (e.g. moreutils sponge)
     between the command output and the pager. the pager reads
     from source_fd: the last stage's output, or the command
     output pipe itself when no stage is configured. */
  int source_fd;
  source_fd = pager_pipe_stages_start (shell, shell->pipefd[0]);
  shell->pipefd[0] = source_fd;

  process_id = fork ();
  if (process_id < 0)
    {
      DEBUG_ZCMDSH_LOG (PAGER, "fork() failed: close pipefd and pty");
      pager_cleanup (shell);
      return;
    }
  else if (process_id == 0)
    {
      DEBUG_NEW (PAGER, "child ok: pid: %d.", getpid ());

      ret = setsid ();
      if (ret < 0)
        DEBUG_NEW (PAGER, "setsid(): failed.");
#if 0
      else
        DEBUG_NEW (PAGER, "setsid(): succeeded.");
#endif

      /* child (pager) */
      close (shell->pipefd[1]);
      close (shell->pty_master);

      close (shell->pty_slave);
      shell->pty_slave = -1;

      if (slave_name_ptr)
        {
          DEBUG_NEW (PAGER, "opening slave_name_ptr: %s.", slave_name_ptr);
          shell->pty_slave = open (slave_name_ptr, O_RDWR);
        }
      if (shell->pty_slave < 0)
        {
          DEBUG_NEW (PAGER, "opening slave_name: %s", slave_name);
          shell->pty_slave = open (slave_name, O_RDWR);
        }

      DEBUG_NEW (PAGER, "pty(slave): fd %d, %s.", shell->pty_slave,
                        ttyname (shell->pty_slave));

      /* the pager command: the configured one, or the default. */
      char **pager_argv;
      pager_argv = (pager_command_config.argc ? pager_command_config.argv
                                             : pager_command_default);

      DEBUG_NEW (PAGER, "execvp: %s%s", pager_argv[0],
                 pager_argv[1] ? " ..." : "");

      ret = ioctl (shell->pty_slave, TIOCSCTTY, 0);
      if (ret < 0)
        {
          DEBUG_NEW (PAGER, "ioctl (%d, TIOCSCTTY) failed.", shell->pty_slave);
        }
#if 0
      else
        {
          DEBUG_NEW (PAGER, "ioctl (%d, TIOCSCTTY) succeeded.", shell->pty_slave);
        }
#endif

      ret = setenv ("TERM", "vt100", 1);
      if (ret < 0)
        DEBUG_NEW (PAGER, "setenv (TERM, vt100): failed: %s.",
                 strerror (errno));
#if 0
      else
        DEBUG_NEW (PAGER, "setenv (TERM, vt100): succeeded.");
#endif

#if 0
      int flags;
      flags = fcntl (shell->pty_slave, F_GETFL, 0);
      if (flags < 0)
        DEBUG_NEW (PAGER, "pty_slave: F_GETFL: failed. can't set O_NONBLOCK.");
      else
        {
          ret = fcntl (shell->pty_slave, F_SETFL, flags | O_NONBLOCK);
          if (ret < 0)
            DEBUG_NEW (PAGER, "pty_slave: F_SETFL: failed. can't set O_NONBLOCK.");
        }
#endif

      dup2 (source_fd, 0);
      dup2 (shell->pty_slave, 1);
      dup2 (shell->pty_slave, 2);
      stdin = fdopen (0, "r");
      stdout = fdopen (1, "w");
      stderr = fdopen (2, "r+");

      ret = execvp (pager_argv[0], pager_argv);
      if (ret < 0)
        {
          DEBUG_NEW (PAGER, "execvp() failed: ret: %d: %s.", ret,
                            strerror (errno));
          _exit (127);
        }
    }
  else if (process_id > 0)
    {
      DEBUG_NEW (PAGER, "parent ok: pid: %d.", getpid ());
      shell->pager_pid = process_id;

      /* parent */
      close (source_fd);
      shell->pipefd[0] = -1;
      close (shell->pty_slave);
      shell->pty_slave = -1;

#if 0
      int flags;
      flags = fcntl (shell->pipefd[1], F_GETFL, 0);
      if (flags < 0)
        DEBUG_NEW (PAGER, "F_GETFL: failed. can't set O_NONBLOCK.");
      else
        {
          ret = fcntl (shell->pipefd[1], F_SETFL, flags | O_NONBLOCK);
          if (ret < 0)
            DEBUG_NEW (PAGER, "F_SETFL: failed. can't set O_NONBLOCK.");
          else
            DEBUG_NEW (PAGER, "pipe to shell->terminal: "
                       "F_SETFL: O_NONBLOCK succeeded.");
        }
#else
      DEBUG_NEW (PAGER, "pipe to shell->terminal: "
                 "didn't set O_NONBLOCK.");
#endif

      FILE *pager_fp = fdopen (shell->pipefd[1], "a");
      if (! pager_fp)
        {
          DEBUG_NEW (PAGER, "fdopen() failed: %s.", strerror (errno));
          pager_cleanup (shell);
          return;
        }
      shell->pager_saved_terminal = shell->terminal;
      shell->pager_saved_writefd = shell->writefd;
      shell->terminal = pager_fp;
      shell->writefd = shell->pipefd[1];

      fcntl (shell->writefd, F_SETPIPE_SZ, 1048576);

      struct pollfd fds[2];
      fds[0].fd = shell->pty_master;
      fds[0].events = POLLIN;
      DEBUG_NEW (PAGER, "poll(): fds[0]: pty_master: %d", shell->pty_master);
      fds[1].fd = shell->readfd;
      fds[1].events = POLLIN;
      DEBUG_NEW (PAGER, "poll(): fds[1]: readfd: %d", shell->readfd);

      while (1)
        {
          fds[0].revents = 0;
          fds[1].revents = 0;

          int nbytes;
          char buf[256];

          DEBUG_NEW (PAGER, "poll(): fd: %d fd: %d", fds[0].fd, fds[1].fd);
          ret = poll (fds, 2, 0);
          if (ret < 0)
            {
              if (errno == EINTR)
                continue;
              DEBUG_NEW (PAGER, "poll() failed: %s.", strerror (errno));
              pager_cleanup (shell);
              return;
            }
          if (fds[0].revents & POLLIN)
            {
              DEBUG_NEW (PAGER, "poll(): pty_master: %d: POLLIN",
                         shell->pty_master);
              nbytes = read (shell->pty_master, buf, sizeof (buf) - 1);
              if (nbytes < 0 && (errno == EINTR || errno == EAGAIN))
                continue;
              DEBUG_NEW (PAGER, "read(): pty_master: %d: %d bytes",
                       shell->pty_master, nbytes);
              if (nbytes <= 0)
                {
                  pager_cleanup (shell);
                  return;
                }
              if (nbytes > 0)
                {
                  buf[nbytes] = '\0';
                  DEBUG_NEW (PAGER, "read(): pty_master: %d: %d bytes: %s",
                           shell->pty_master, nbytes, buf);
                  ret = write (shell->readfd, buf, nbytes);
                  DEBUG_NEW (PAGER, "write(): readfd: %d: %d bytes",
                           shell->readfd, ret);
                }
            }
          if (fds[1].revents & POLLIN)
            {
              DEBUG_NEW (PAGER, "poll(): readfd: %d: POLLIN", shell->readfd);
              nbytes = read (shell->readfd, buf, sizeof (buf) - 1);
              if (nbytes < 0 && (errno == EINTR || errno == EAGAIN))
                continue;
              DEBUG_NEW (PAGER, "read(): readfd: %d: %d bytes",
                       shell->readfd, nbytes);
              if (nbytes <= 0)
                {
                  FLAG_SET (shell->flag, SHELL_FLAG_CLOSE);
                  pager_cleanup (shell);
                  return;
                }
              if (nbytes > 0)
                {
                  buf[nbytes] = '\0';
                  DEBUG_NEW (PAGER, "read(): readfd: %d: %d bytes: %s",
                             shell->readfd, nbytes, buf);
                  ret = write (shell->pty_master, buf, nbytes);
                  DEBUG_NEW (PAGER, "write(): pty_master: %d: %d bytes",
                             shell->pty_master, ret);
                }
            }
          DEBUG_NEW (PAGER, "poll(): revents: %d revents: %d",
                     fds[0].revents, fds[1].revents);
          if ((fds[0].revents & POLLIN) == 0 &&
              (fds[1].revents & POLLIN) == 0)
            {
              DEBUG_NEW (PAGER, "poll(): break");
              break;
            }
        }

      DEBUG_NEW (PAGER, "pager start: terminal: %p <- %p.",
                 shell->terminal, shell->pager_saved_terminal);
    }
#endif
  shell->is_paging = true;
}

void
pager_end (struct shell *shell)
{
  if (! shell->is_paging)
    return;

  pager_output_close (shell);
#if ! PAGER_USE_POPEN
  int wstatus;
  pid_t wait_ret = 0;

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
          int poll_errno = errno;

          DEBUG_ZCMDSH_LOG (PAGER, "pager: ppoll() returned: %d: %s", ret,
                            strerror (poll_errno));
          if (poll_errno == EINTR)
            continue;
          pager_cleanup (shell);
          return;
        }

      for (i = 0; i < nfds; i++)
        {
          int readfd, writefd;

#if 0
          if (fds[i].revents & POLLHUP)
            closed++;
#endif

          if (! (fds[i].revents & (POLLIN | POLLHUP | POLLERR | POLLNVAL)))
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
              int read_errno = errno;

              DEBUG_ZCMDSH_LOG (PAGER,
                                "pager: read() from fd: %d returned %d: %s",
                                readfd, ret, strerror (read_errno));
              if (read_errno == EINTR || read_errno == EAGAIN)
                continue;
              if (i == 0)
                FLAG_SET (shell->flag, SHELL_FLAG_CLOSE);
              closed++;
              continue;
            }
          else if (ret == 0)
            {
              DEBUG_ZCMDSH_LOG (PAGER, "pager: fds[%d]: fd: %d: closed, break",
                                i, fds[i].fd);
              if (i == 0)
                FLAG_SET (shell->flag, SHELL_FLAG_CLOSE);
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

  if (FLAG_CHECK (shell->flag, SHELL_FLAG_CLOSE))
    {
      pager_cleanup (shell);
      return;
    }

  DEBUG_ZCMDSH_LOG (PAGER, "pager: waiting child: process %d...",
                    shell->pager_pid);
  if (shell->pager_pid > 0)
    {
      do
        wait_ret = waitpid (shell->pager_pid, &wstatus, 0);
      while (wait_ret < 0 && errno == EINTR);
    }
  if (wait_ret > 0 && WIFEXITED (wstatus))
    {
      DEBUG_ZCMDSH_LOG (PAGER, "pager: process %d exited normally.",
                        shell->pager_pid);
    }
  else
    {
      DEBUG_ZCMDSH_LOG (PAGER, "pager: process %d failed.", shell->pager_pid);
    }

  pager_pipe_stages_reap (shell);
  shell->pager_pid = 0;
#endif

  pager_fds_close (shell);
  shell->is_paging = false;
}

int
shell_pager_bridge (struct shell *shell)
{
  //DEBUG_NEW (PAGER, "pager: bridging sockets and pty...");

  int ret, nwrite;
  struct pollfd fds[2];
  nfds_t nfds = 2;
  char buf[1024];
  int closed = 0;
  int i, j;

  //while (1)
    {
      //DEBUG_NEW (PAGER, "pager: reset pollfds");
      memset (fds, 0, sizeof (fds));
      fds[0].fd = shell->readfd;
      fds[0].events |= POLLIN;
      fds[1].fd = shell->pty_master;
      fds[1].events |= POLLIN;

#if 0
      DEBUG_NEW (PAGER, "pager: bridge: poll: fds[%d]: %d fds[%d]: %d",
                 0, fds[0].fd, 1, fds[1].fd);
#endif

      ret = poll (fds, nfds, 0);
      if (ret < 0)
        {
          int poll_errno = errno;

          DEBUG_NEW (PAGER, "pager: ppoll() returned: %d: %s", ret,
                            strerror (poll_errno));
          return (poll_errno == EINTR ? 0 : -1);
        }

      for (i = 0; i < nfds; i++)
        {
          int readfd, writefd;

          if (! (fds[i].revents & (POLLIN | POLLHUP | POLLERR | POLLNVAL)))
            {
#if 0
            DEBUG_NEW (PAGER, "pager: bridge: no events: fds[%d]: %d",
                       i, fds[i].fd);
#endif
            continue;
            }

          if ((fds[i].revents & POLLHUP) && ! (fds[i].revents & POLLIN))
            closed++;

          DEBUG_NEW (
              PAGER, "pager: fds[%d]: fd: %d revents:%s%s%s %#x", i, fds[i].fd,
              (fds[i].revents & POLLIN ? " POLLIN" : ""),
              (fds[i].revents & POLLHUP ? " POLLHUP" : ""),
              (fds[i].revents & POLLERR ? " POLLERR" : ""), fds[i].revents);

          readfd = fds[i].fd;
          j = (i & 0x01) ^ 0x01;
          writefd = fds[j].fd;
          DEBUG_NEW (PAGER, "pager: bridge: fds[%d]: %d -> fds[%d]: %d",
                     i, fds[i].fd, j, fds[j].fd);

          ret = read (readfd, buf, sizeof (buf));
          if (ret < 0)
            {
              int read_errno = errno;

              DEBUG_ZCMDSH_LOG (PAGER,
                                "pager: read() from fd: %d returned %d: %s",
                                readfd, ret, strerror (read_errno));
              if (read_errno == EINTR || read_errno == EAGAIN)
                continue;
              if (i == 0)
                FLAG_SET (shell->flag, SHELL_FLAG_CLOSE);
              closed++;
              continue;
            }
          else if (ret == 0)
            {
              DEBUG_ZCMDSH_LOG (PAGER, "pager: fds[%d]: fd: %d: closed, break",
                                i, fds[i].fd);
              if (i == 0)
                FLAG_SET (shell->flag, SHELL_FLAG_CLOSE);
              closed++;
            }
          else
            {
              DEBUG_ZCMDSH_LOG (PAGER, "pager: read(): fd: %d, %d bytes",
                                readfd, ret);

              nwrite = write (writefd, buf, ret);
              if (nwrite < 0)
                DEBUG_ZCMDSH_LOG (PAGER, "write() failed: %s",
                                  strerror (errno));
              DEBUG_ZCMDSH_LOG (PAGER, "pager: fd: %d -> fd: %d, %d bytes",
                                readfd, writefd, ret);
#ifndef NO_PAGER_CONTENTS
              if (FLAG_CHECK (DEBUG_CONFIG (ZCMDSH),
                              DEBUG_TYPE (ZCMDSH, PAGER_CONTENTS)))
                {
                  char str_buf[1024];
                  char *curr = str_buf;
                  char *end = str_buf + sizeof (str_buf);
                  int nwrite;
                  for (i = 0; i < ret; i++)
                    {
                      nwrite = 0;
                      if (isascii ((int) buf[i]))
                        nwrite = snprintf (curr, end - curr, "%c", buf[i]);
                      else
                        nwrite = snprintf (curr, end - curr, "%#x", buf[i]);
                      curr += nwrite;
                    }
                  DEBUG_ZCMDSH_LOG (PAGER_CONTENTS,
                                    "pager: buf: \"%s\"", str_buf);
                }
#endif /*NO_PAGER_CONTENTS*/
            }
        }
    }

  return closed;
}

void
shell_read_nowait_paging (struct shell *shell)
{
  int wstatus;
  pid_t wait_ret = 0;
  int closed;

  if (! shell->is_paging)
    return;

  pager_output_close (shell);
  closed = shell_pager_bridge (shell);

  if (! closed)
    return;

  if (closed < 0 || FLAG_CHECK (shell->flag, SHELL_FLAG_CLOSE))
    pager_cleanup (shell);
  else
    {
      DEBUG_ZCMDSH_LOG (PAGER, "pager: waiting child: process %d...",
                        shell->pager_pid);
      if (shell->pager_pid > 0)
        {
          do
            wait_ret = waitpid (shell->pager_pid, &wstatus, 0);
          while (wait_ret < 0 && errno == EINTR);
        }
      if (wait_ret > 0 && WIFEXITED (wstatus))
        {
          DEBUG_ZCMDSH_LOG (PAGER, "pager: process %d exited normally.",
                            shell->pager_pid);
        }
      else
        {
          DEBUG_ZCMDSH_LOG (PAGER, "pager: process %d failed.", shell->pager_pid);
        }

      pager_pipe_stages_reap (shell);
      shell->pager_pid = 0;

      pager_fds_close (shell);
      shell->is_paging = false;

    }
  if (shell_running (shell))
    {
      shell_clear (shell);
      shell_prompt (shell);
      shell_refresh (shell);
    }
}

int
command_execute2 (int argc, char **argv, struct command_set *cmdset,
                  struct command_node **cmdnodes, struct shell *shell)
{
  int ret;

  struct command_node *node;
  struct vector_node *vn;
  struct command_node *match = NULL;
  struct command_node *parent = NULL;

  int index;
  int curr, upper;

  assert (argc > 0);
  index = argc - 1;

  if (index == 0)
    {
      if (cmdnodes[0])
        match = cmdnodes[0];
      parent = cmdset->root;
    }
  else
    {
      match = cmdnodes[index];
      parent = cmdnodes[index - 1];
    }

  if (! parent)
    return CMD_NOT_FOUND;

  /* treat cases when "show port" command is installed and the
     current command-line is "show port ?".
     In the case, the current argv is "" and the
     upper matched node has the execution function. */
  if (! strlen (argv[index]) && parent->func)
    {
      match = parent;
      argc--;
      argv[argc] = NULL;
    }

  if (! match || ! match->func)
    ret = CMD_NOT_FOUND;
  else
    ret = (*match->func) (shell, argc, argv);

  return ret;
}

int
command_shell_execute (struct shell *shell)
{
  int ret = 0;
  char *comment;

  /* ignore blank-line in startup-config. */
  if (! strlen (shell->command_line) &&
      ! FLAG_CHECK (shell->flag, SHELL_FLAG_INTERACTIVE))
    return 0;

  /* send a line-feed to terminate the command line. */
  shell_linefeed (shell);

  /* without the below fflush(), the newline will be sent after the pager
     output, and will break the output: the executed command-line
     will be erased, and after the command-output, it appears as
     the additional newline which seems to be redundant. */
  fflush (shell->terminal);

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

      /* do not enter pager mode when pasting. */
      if (shell->pager && ! shell->is_pasting)
        pager_start (shell);

      //shell_linefeed (shell);

#if 0
      if (FLAG_CHECK (shell->flag, SHELL_FLAG_DEBUG))
        fprintf (shell->terminal, "command started: %s%s",
                 shell->command_line, shell->NL);
#endif

#if 0
      ret = command_execute (shell->command_line, shell->cmdset, shell);
#else

  char *command_line_dup;
  int argc;
  char **argv;
  struct command_node **cmdnodes;
  struct command_set *cmdset;

  command_line_dup = strdup (shell->command_line);

  /* parse current command-line's argv. */
  command_argv_parse (command_line_dup, &argc, &argv);

  /* find matching command nodes. */
  command_matched_nodes (argc, argv, shell->command_line,
                         shell->cmdset, &cmdnodes);

      ret = command_execute2 (argc, argv, shell->cmdset, cmdnodes, shell);

  free (command_line_dup);
  free (argv);
  free (cmdnodes);
#endif

#if 0
      if (FLAG_CHECK (shell->flag, SHELL_FLAG_DEBUG))
        fprintf (shell->terminal, "command finished: %s%s",
                 shell->command_line, shell->NL);
#endif

      /* record the execution of the command. */
      DEBUG_ZCMDSH_LOG (COMMAND_LOG, "command-log: %s",
                        shell->command_line);

      shell->cmd_status = ret;
      if (ret == CMD_NOT_FOUND)
        {
          DEBUG_ZCMDSH_LOG (COMMAND_LOG, "command-log: command not found");
          fprintf (shell->terminal, "no such command: %s%s",
                   shell->command_line, shell->NL);
          ret = -1;
        }
      if (ret == CMD_FAILURE)
        {
          DEBUG_ZCMDSH_LOG (COMMAND_LOG, "command-log: command failed");
          fprintf (shell->terminal, "command failed: %s%s",
                   shell->command_line, shell->NL);
          ret = -1;
        }

      command_history_add (shell->command_line, shell->history, shell);

      if (shell->is_paging)
        {
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
command_ls_candidate (int argc, char **argv, struct command_set *cmdset,
                      struct command_node **cmdnodes, struct shell *shell)
{
  struct command_node *match;
  struct command_node *node;
  struct vector_node *vn;

  int index;
  int curr, upper;

  assert (argc > 0);
  index = argc - 1;

  if (index == 0)
    {
      if (cmdnodes[0])
        match = cmdnodes[0];
      else
        match = cmdset->root;
    }
  else
    {
      match = cmdnodes[index - 1];
    }

  curr = argc - 1;
  upper = (curr > 0 ? upper = curr - 1 : -1);

#if 0
  if (FLAG_CHECK (shell->flag, SHELL_FLAG_DEBUG))
    {
      fprintf (shell->terminal, "upper: %d curr: %d%s",
               upper, curr, shell->NL);
      fprintf (shell->terminal, "argv[%d]: \"%s\" (strlen: %lu)%s",
               index, argv[index], strlen (argv[index]), shell->NL);
      fprintf (shell->terminal, "cmdnodes[upper]: %p cmdnodes[curr]: %p%s",
               cmdnodes[upper], cmdnodes[curr], shell->NL);
      if (upper >= 0 && cmdnodes[upper])
      fprintf (shell->terminal, "cmdnodes[upper]: %s%s",
               cmdnodes[upper]->cmdstr, shell->NL);
      if (cmdnodes[curr])
      fprintf (shell->terminal, "cmdnodes[curr]: %s%s",
               cmdnodes[curr]->cmdstr, shell->NL);
    }
#endif

  if (upper >= 0 && cmdnodes[upper])
    {
      match = cmdnodes[upper];

      /* treat cases when "show port" command is installed and the
         current command-line is "show port ?".
         In the case, the current argv is "" and the
         upper matched node has the execution function. */
      if (! strlen (argv[curr]) && match->func)
        {
          if (is_command_regvar (shell->cmdset, match->cmdstr))
            {
              /* regvar match is completed
                 (i.e., "matched", not "matching") */
              if (is_command_match_regvar (shell->cmdset, match->cmdstr,
                                           argv[upper]))
                {
                  char *regvar_help;
                  regvar_help = (char *)
                    regvar_helpstr (shell->cmdset, match->cmdstr,
                                    match->helpstr, argv[upper]);
                  fprintf (shell->terminal, "  %-16s %s%s", "<cr>",
                          regvar_help, shell->NL);
                  free (regvar_help);
                }
            }
          else
            {
              fprintf (shell->terminal, "  %-16s %s%s", "<cr>",
                       match->helpstr, shell->NL);
            }
        }

      for (vn = vector_head (match->cmdvec); vn; vn = vector_next (vn))
        {
          node = (struct command_node *) vn->data;
          if (is_command_regvar (shell->cmdset, node->cmdstr))
            {
              if (is_command_match_regvar (shell->cmdset, node->cmdstr,
                                           argv[index]) ||
                  is_command_match_regvar_matching (shell->cmdset,
                                           node->cmdstr, argv[index]))
                {
                  char *regvar_spec;
                  char *regvar_help;
                  regvar_spec = (char *)
                    regvar_cmdstr (shell->cmdset, node->cmdstr, argv[index]);
                  regvar_help = (char *)
                    regvar_helpstr (shell->cmdset, node->cmdstr,
                                    node->helpstr, argv[index]);
                  fprintf (shell->terminal, "  %-16s %s%s",
                           regvar_spec, regvar_help, shell->NL);
                  free (regvar_spec);
                  free (regvar_help);
                }
            }
          else
          if (is_command_match (node->cmdstr, argv[index]))
            fprintf (shell->terminal, "  %-16s %s%s", node->cmdstr,
                     node->helpstr, shell->NL);

          if (file_spec (node->cmdstr))
            file_ls_candidate (shell, argv[index]);
        }
    }
  else if (argc == 1)
    {
      match = cmdset->root;
      for (vn = vector_head (match->cmdvec); vn; vn = vector_next (vn))
        {
          node = (struct command_node *) vn->data;
          if (is_command_match (node->cmdstr, argv[index]))
            fprintf (shell->terminal, "  %-16s %s%s", node->cmdstr,
                     node->helpstr, shell->NL);
        }
    }
}

int
command_shell_ls_candidate (struct shell *shell)
{
  char *cmd_dup;
  char *last;
  int last_head;
  struct command_node *match;
  struct command_node *exact_match;
  struct command_node *node;
  struct vector_node *vn;

  shell_terminate (shell);
  shell_linefeed (shell);

  if (shell->cursor != shell->end)
    {
      shell->cursor = shell->end;
      shell_refresh (shell);
    }

  char *command_line_dup;
  int argc;
  char **argv;
  struct command_node **cmdnodes;
  struct command_set *cmdset;

  command_line_dup = strdup (shell->command_line);

  /* parse current command-line's argv. */
  command_argv_parse (command_line_dup, &argc, &argv);

  /* find matching command nodes. */
  command_matched_nodes (argc, argv, shell->command_line,
                         shell->cmdset, &cmdnodes);

#if 1
  command_ls_candidate (argc, argv, shell->cmdset, cmdnodes, shell);
#else

  cmdset = shell->cmdset;
  int index;
  int curr, upper;

  assert (argc > 0);
  index = argc - 1;

  if (index == 0)
    {
      if (cmdnodes[0])
        match = cmdnodes[0];
      else
        match = cmdset->root;
    }
  else
    {
      match = cmdnodes[index - 1];
    }

  curr = argc - 1;
  upper = (curr > 0 ? upper = curr - 1 : -1);

#if 0
  if (FLAG_CHECK (shell->flag, SHELL_FLAG_DEBUG))
    {
      fprintf (shell->terminal, "upper: %d curr: %d%s",
               upper, curr, shell->NL);
      fprintf (shell->terminal, "argv[%d]: \"%s\" (strlen: %lu)%s",
               index, argv[index], strlen (argv[index]), shell->NL);
      fprintf (shell->terminal, "cmdnodes[upper]: %p cmdnodes[curr]: %p%s",
               cmdnodes[upper], cmdnodes[curr], shell->NL);
      if (upper >= 0 && cmdnodes[upper])
      fprintf (shell->terminal, "cmdnodes[upper]: %s%s",
               cmdnodes[upper]->cmdstr, shell->NL);
      if (cmdnodes[curr])
      fprintf (shell->terminal, "cmdnodes[curr]: %s%s",
               cmdnodes[curr]->cmdstr, shell->NL);
    }
#endif

  if (upper >= 0 && cmdnodes[upper])
    {
      match = cmdnodes[upper];

      /* treat cases when "show port" command is installed and the
         current command-line is "show port ?".
         In the case, the current argv is "" and the
         upper matched node has the execution function. */
      if (! strlen (argv[curr]) && match->func)
        fprintf (shell->terminal, "  %-16s %s%s", "<cr>",
                 match->helpstr, shell->NL);

      for (vn = vector_head (match->cmdvec); vn; vn = vector_next (vn))
        {
          node = (struct command_node *) vn->data;
          if (is_command_match_regvar (shell->cmdset, node->cmdstr, argv[index]))
            {
          char *regvar_spec;
          char *regvar_help;
            regvar_spec = (char *) regvar_cmdstr (shell->cmdset, node->cmdstr, argv[index]);
            regvar_help = (char *) regvar_helpstr (shell->cmdset, node->cmdstr, node->helpstr, argv[index]);
            fprintf (shell->terminal, "  %-16s %s%s",
                     regvar_spec,
                     regvar_help,
                     shell->NL);
            }
          else
          if (is_command_match (node->cmdstr, argv[index]))
            fprintf (shell->terminal, "  %-16s %s%s", node->cmdstr,
                     node->helpstr, shell->NL);

          if (file_spec (node->cmdstr))
            file_ls_candidate (shell, argv[index]);
        }
    }
  else if (argc == 1)
    {
      match = cmdset->root;
      for (vn = vector_head (match->cmdvec); vn; vn = vector_next (vn))
        {
          node = (struct command_node *) vn->data;
          if (is_command_match (node->cmdstr, argv[index]))
            fprintf (shell->terminal, "  %-16s %s%s", node->cmdstr,
                     node->helpstr, shell->NL);
        }
    }
#endif

  free (command_line_dup);
  free (argv);
  free (cmdnodes);

  //shell_linefeed (shell);
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

CLI_COMMAND2 (set_pager, "(set|no|enable|disable|) pager",
                "set command.\n",
                "no command.\n",
                "enable command.\n",
                "disable command.\n",
                "enable/disable pager.\n")
{
  struct shell *shell = (struct shell *) context;
  bool value = false;

  if (! strcmp (argv[0], "set"))
    value = true;
  else if (! strcmp (argv[0], "no"))
    value = false;
  else if (! strcmp (argv[0], "enable"))
    value = true;
  else if (! strcmp (argv[0], "disable"))
    value = false;
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
                "set pager.\n"
                "set pager command.\n"
                "set pager command option.\n")
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
                "set pager.\n"
                "set pager default.\n")
{
  struct shell *shell = (struct shell *) context;
  if (shell->pager_command)
    {
      free (shell->pager_command);
      shell->pager_command = NULL;
    }
  return 0;
}

DEFINE_COMMAND (set_pager_pipe,
                "set pager pipe <0-7> command <LINE>",
                "set.\n"
                "set pager.\n"
                "set pager pipe stage.\n"
                "specify pager pipe stage index.\n"
                "set pager pipe stage command.\n"
                "specify command and arguments (e.g. sponge).\n")
{
  struct shell *shell = (struct shell *) context;
  int index;
  int ret;
  char *line;
  int stage_argc;
  char *stage_argv[PAGER_PIPE_ARGV_MAX];

  index = strtol (argv[3], NULL, 0);

  /* argv[5] holds the rest of the line: split it into words. */
  line = strdup (argv[5]);
  stage_argc = pager_command_line_split (line, stage_argv,
                                         PAGER_PIPE_ARGV_MAX);
  if (stage_argc > 0)
    ret = pager_pipe_stage_set (index, stage_argc, stage_argv);
  else
    ret = -1;
  free (line);

  if (ret < 0)
    {
      fprintf (shell->terminal, "can't set pager pipe[%d].%s",
               index, shell->NL);
      return CMD_FAILURE;
    }

  fprintf (shell->terminal, "pager pipe[%d]: %s%s", index, argv[5], shell->NL);
  return CMD_SUCCESS;
}

DEFINE_COMMAND (set_pager_command_line,
                "set pager command <LINE>",
                "set.\n"
                "set pager.\n"
                "set pager command.\n"
                "specify command and arguments (e.g. less -FX).\n")
{
  struct shell *shell = (struct shell *) context;
  int ret;
  char *line;
  int cmd_argc;
  char *cmd_argv[PAGER_PIPE_ARGV_MAX];

  /* argv[3] holds the rest of the line: split it into words. */
  line = strdup (argv[3]);
  cmd_argc = pager_command_line_split (line, cmd_argv, PAGER_PIPE_ARGV_MAX);
  if (cmd_argc > 0)
    ret = pager_command_set (cmd_argc, cmd_argv);
  else
    ret = -1;
  free (line);

  if (ret < 0)
    {
      fprintf (shell->terminal, "can't set pager command.%s", shell->NL);
      return CMD_FAILURE;
    }

  fprintf (shell->terminal, "pager command: %s%s", argv[3], shell->NL);
  return CMD_SUCCESS;
}

DEFINE_COMMAND (no_pager_command,
                "no pager command",
                "negate command.\n"
                "pager.\n"
                "pager command: restore the default.\n")
{
  struct shell *shell = (struct shell *) context;

  pager_command_unset ();
  fprintf (shell->terminal, "pager command: default.%s", shell->NL);
  return CMD_SUCCESS;
}

CLI_COMMAND2 (show_pager_command,
              "show pager command",
              SHOW_HELP,
              "pager information.\n",
              "pager command.\n")
{
  struct shell *shell = (struct shell *) context;
  char pager_line[256];

  /* the whole pipeline: the pipe stages and the pager command. */
  pager_command_line_join (pager_line, sizeof (pager_line));
  fprintf (shell->terminal, "pager command%s: %s%s",
           (pager_command_config.argc ? "" : " (default)"),
           pager_line, shell->NL);
  return CMD_SUCCESS;
}

DEFINE_COMMAND (no_pager_pipe,
                "no pager pipe <0-7>",
                "negate command.\n"
                "pager.\n"
                "pager pipe stage.\n"
                "specify pager pipe stage index.\n")
{
  struct shell *shell = (struct shell *) context;
  int index;

  index = strtol (argv[3], NULL, 0);
  pager_pipe_stage_unset (index);
  fprintf (shell->terminal, "pager pipe[%d]: unset.%s", index, shell->NL);
  return CMD_SUCCESS;
}

CLI_COMMAND2 (show_pager_pipe,
              "show pager pipe",
              SHOW_HELP,
              "pager information.\n",
              "pager pipe stages.\n")
{
  struct shell *shell = (struct shell *) context;
  int i, j;

  for (i = 0; i < SHELL_PAGER_PIPE_MAX; i++)
    {
      struct pager_pipe_stage *stage = &pager_pipe_stages[i];
      if (! stage->argc)
        continue;
      fprintf (shell->terminal, "pager pipe[%d]:", i);
      for (j = 0; j < stage->argc; j++)
        fprintf (shell->terminal, " %s", stage->argv[j]);
      fprintf (shell->terminal, "%s", shell->NL);
    }
  return CMD_SUCCESS;
}

CLI_COMMAND2 (show_shell,
              "show shell",
              SHOW_HELP,
              "shell information.\n")
{
  struct shell *shell = (struct shell *) context;
  char pager_line[256];

  /* the effective pager pipeline (the pipe stages and the pager
     command) for display. */
  pager_command_line_join (pager_line, sizeof (pager_line));

  fprintf (shell->terminal, "shell winsize row: %d col: %d%s",
           shell->winsize.ws_row, shell->winsize.ws_col, shell->NL);
  fprintf (shell->terminal, "shell pager %d pager_command: %s%s",
           shell->pager,
           shell->pager_command ? shell->pager_command : pager_line,
           shell->NL);
  return CMD_SUCCESS;
}

CLI_COMMAND2 (set_shell_winsize,
              "set shell winsize row <0-1024> col <0-1024>",
              SHOW_HELP,
              "shell information.\n",
              "shell window size information.\n",
              "set shell window row size.\n",
              "set shell window column size.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  int row, col;
  char *endptr;
  row = strtol (argv[4], &endptr, 0);
  col = strtol (argv[6], &endptr, 0);
  shell->winsize.ws_row = row;
  shell->winsize.ws_col = col;
  fprintf (shell->terminal, "shell winsize row: %d col: %d%s",
           shell->winsize.ws_row, shell->winsize.ws_col, shell->NL);
  return CMD_SUCCESS;
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
  INSTALL_COMMAND2 (cmdset, set_pager_pipe);
  INSTALL_COMMAND2 (cmdset, no_pager_pipe);
  INSTALL_COMMAND2 (cmdset, show_pager_pipe);
  INSTALL_COMMAND2 (cmdset, set_pager_command_line);
  INSTALL_COMMAND2 (cmdset, no_pager_command);
  INSTALL_COMMAND2 (cmdset, show_pager_command);
  INSTALL_COMMAND2 (cmdset, show_shell);
  INSTALL_COMMAND2 (cmdset, set_shell_winsize);

  INSTALL_COMMAND (cmdset, enable_shell_debugging);
  INSTALL_COMMAND (cmdset, disable_shell_debugging);
}

struct shell *
command_shell_create ()
{
  struct shell *shell;

  shell = shell_create ();
  if (! shell)
    return NULL;
  shell->cmdset = command_set_create ();
  default_install_command (shell->cmdset);

  shell_install (shell, CONTROL ('J'), command_shell_execute);
  shell_install (shell, CONTROL ('M'), command_shell_execute);
  shell_install (shell, CONTROL ('I'), command_shell_completion);
  shell_install (shell, '?', command_shell_ls_candidate);
  shell_install (shell, CONTROL ('P'), command_history_prev);
  shell_install (shell, CONTROL ('N'), command_history_next);

  FUNC_STR_REGISTER (command_shell_execute);
  FUNC_STR_REGISTER (command_shell_completion);
  FUNC_STR_REGISTER (command_shell_ls_candidate);
  FUNC_STR_REGISTER (command_history_prev);
  FUNC_STR_REGISTER (command_history_next);

  shell->pager = true;

  return shell;
}

void
command_shell_delete (struct shell *shell)
{
  pager_cleanup (shell);
  command_set_delete (shell->cmdset);
  command_history_delete (shell->history);
  shell_delete (shell);
}

void
command_shell_init ()
{
  command_func_name_init ();
  cmdset_default = command_set_create ();
  default_install_command (cmdset_default);
}

void
command_shell_finish ()
{
  command_set_delete (cmdset_default);
  command_func_name_finish ();
}
