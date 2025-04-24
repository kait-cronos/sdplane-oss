/*
 * Copyright (C) 2024 Yasuhiro Ohara. All rights reserved.
 */

#include <includes.h>

#include "command.h"
#include "flag.h"
#include "shell.h"

#include "debug_category.h"
#include "debug_log.h"

char *log_filename = NULL;

DEFINE_COMMAND (show_logging, "show logging",
                SHOW_HELP "show logging information.\n")
{
  struct shell *shell = (struct shell *) context;

  fprintf (shell->terminal, "log: syslog: %s%s",
           (FLAG_CHECK (debug_output, DEBUG_OUTPUT_SYSLOG) ? "enabled"
                                                           : "disabled"),
           shell->NL);

  fprintf (
      shell->terminal, "log: file: %s%s",
      (FLAG_CHECK (debug_output, DEBUG_OUTPUT_FILE) ? "enabled" : "disabled"),
      shell->NL);
  if (debug_log_filename)
    {
      fprintf (shell->terminal, "log: file: filename: %s fp: %p%s",
               debug_log_filename, (void *) debug_log_file, shell->NL);
    }

  fprintf (shell->terminal, "log: stdout: %s%s",
           (FLAG_CHECK (debug_output, DEBUG_OUTPUT_STDOUT) ? "enabled"
                                                           : "disabled"),
           shell->NL);
  fprintf (shell->terminal, "log: stderr: %s%s",
           (FLAG_CHECK (debug_output, DEBUG_OUTPUT_STDERR) ? "enabled"
                                                           : "disabled"),
           shell->NL);
  return 0;
}

DEFINE_COMMAND (log_cmd,
                "(|no) log (syslog|stdout|stderr)",
                NO_HELP
                "logging information.\n"
                "log to syslog.\n"
                "log to stdout.\n"
                "log to stderr.\n")
{
  //struct shell *shell = (struct shell *) context;
  int negate = 0;

  if (! strcmp (argv[0], "no"))
    {
      negate++;
      argv++;
      argc--;
    }

  if (! strcmp (argv[1], "syslog"))
    {
      if (negate)
        FLAG_UNSET (debug_output, DEBUG_OUTPUT_SYSLOG);
      else
        FLAG_SET (debug_output, DEBUG_OUTPUT_SYSLOG);
    }
  else if (! strcmp (argv[1], "stdout"))
    {
      if (negate)
        FLAG_UNSET (debug_output, DEBUG_OUTPUT_STDOUT);
      else
        FLAG_SET (debug_output, DEBUG_OUTPUT_STDOUT);
    }
  else if (! strcmp (argv[1], "stderr"))
    {
      if (negate)
        FLAG_UNSET (debug_output, DEBUG_OUTPUT_STDERR);
      else
        FLAG_SET (debug_output, DEBUG_OUTPUT_STDERR);
    }
  return 0;
}

DEFINE_COMMAND (log_file_cmd, "log file <FILE>",
                "logging information.\n"
                "log to file.\n"
                "log filename.\n")
{
  //struct shell *shell = (struct shell *) context;
  DEBUG_OUTPUT_FILE_SET (argv[2]);
  return 0;
}

DEFINE_COMMAND (no_log_file_cmd, "no log file",
                NO_HELP "logging information.\n"
                        "log to file.\n")
{
  //struct shell *shell = (struct shell *) context;
  DEBUG_OUTPUT_FILE_UNSET ();
  return 0;
}

void
log_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, show_logging);
  INSTALL_COMMAND2 (cmdset, log_cmd);
  INSTALL_COMMAND2 (cmdset, log_file_cmd);
  INSTALL_COMMAND2 (cmdset, no_log_file_cmd);
}
