/*
 * Copyright (C) 2024 Yasuhiro Ohara. All rights reserved.
 */

#include "includes.h"

#include "log.h"

#include "flag.h"
#include "shell.h"
#include "command.h"

char *log_filename = NULL;

DEFINE_COMMAND (show_logging,
                "show logging",
                SHOW_HELP
                "show logging information.\n")
{
  struct shell *shell = (struct shell *) context;
  struct loginfo *log;
  log = &log_default;

  fprintf (shell->terminal, "log: syslog: %s%s",
           (log->flags & LOGINFO_SYSLOG ? "enabled" : "disabled"),
           shell->LF);

  fprintf (shell->terminal, "log: file: %s%s",
           (log->flags & LOGINFO_FILE ? "enabled" : "disabled"),
           shell->LF);
  if (log->flags & LOGINFO_FILE)
    {
      fprintf (shell->terminal, "log: file: filename: %s fp: %p%s",
               log_filename, log->fp, shell->LF);
    }

  fprintf (shell->terminal, "log: stdout: %s%s",
           (log->flags & LOGINFO_STDOUT ? "enabled" : "disabled"),
           shell->LF);
  fprintf (shell->terminal, "log: stderr: %s%s",
           (log->flags & LOGINFO_STDERR ? "enabled" : "disabled"),
           shell->LF);
}

DEFINE_COMMAND (log_cmd,
                "(|no) log (syslog|stdout|stderr)",
                NO_HELP
                "logging information.\n"
                "log to syslog.\n"
                "log to stdout.\n"
                "log to stderr.\n")
{
  struct shell *shell = (struct shell *) context;
  struct loginfo *log;
  int negate = 0;
  log = &log_default;

  if (! strcmp (argv[0], "no"))
    {
      negate++;
      argv++;
      argc--;
    }

  if (! strcmp (argv[1], "syslog"))
    {
      if (negate)
        log->flags &= ~LOGINFO_SYSLOG;
      else
        log->flags |= LOGINFO_SYSLOG;
    }
  else if (! strcmp (argv[1], "stdout"))
    {
      if (negate)
        log->flags &= ~LOGINFO_STDOUT;
      else
        log->flags |= LOGINFO_STDOUT;
    }
  else if (! strcmp (argv[1], "stderr"))
    {
      if (negate)
        log->flags &= ~LOGINFO_STDERR;
      else
        log->flags |= LOGINFO_STDERR;
    }
}

DEFINE_COMMAND (log_file_cmd,
                "log file <FILE>",
                "logging information.\n"
                "log to file.\n"
                "log filename.\n")
{
  struct shell *shell = (struct shell *) context;
  struct loginfo *log;
  int negate = 0;
  log = &log_default;

  if (log->fp)
    fclose (log->fp);
  if (log_filename)
    free (log_filename);
  log_filename = strdup (argv[2]);

  log->fp = fopen (log_filename, "a+");
  if (! log->fp)
    {
      fprintf (shell->terminal, "fopen() failed: %s%s",
               strerror (errno), shell->LF);
      return;
    }

  /* enable log file. */
  log->flags |= LOGINFO_FILE;
}

DEFINE_COMMAND (no_log_file_cmd,
                "no log file",
                NO_HELP
                "logging information.\n"
                "log to file.\n")
{
  struct shell *shell = (struct shell *) context;
  struct loginfo *log;
  int negate = 0;
  log = &log_default;

  /* disable log file. */
  log->flags &= ~LOGINFO_FILE;

  if (log->fp)
    fclose (log->fp);
  log->fp = NULL;

  if (log_filename)
    free (log_filename);
  log_filename = NULL;
}

void
log_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, show_logging);
  INSTALL_COMMAND2 (cmdset, log_cmd);
  INSTALL_COMMAND2 (cmdset, log_file_cmd);
  INSTALL_COMMAND2 (cmdset, no_log_file_cmd);
}

