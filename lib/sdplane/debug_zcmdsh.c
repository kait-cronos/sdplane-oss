/*
 * Copyright (C) 2007-2023 Yasuhiro Ohara. All rights reserved.
 */

#include <includes.h>

#include "command.h"
#include "debug.h"
#include "flag.h"
#include "shell.h"

#include "debug_category.h"
#include "debug_log.h"
#include "debug_backtrace.h"
#include "debug_zcmdsh.h"

#include "debug_cmd.h"

struct debug_type debug_zcmdsh_types[] = {
  { DEBUG_ZCMDSH_SHELL, "shell" },     { DEBUG_ZCMDSH_COMMAND, "command" },
  { DEBUG_ZCMDSH_PAGER, "pager" },     { DEBUG_ZCMDSH_TIMER, "timer" },
  { DEBUG_ZCMDSH_UNICODE, "unicode" }, { DEBUG_ZCMDSH_TERMIO, "termio" },
  { DEBUG_ZCMDSH_TELNET, "telnet" },
  { DEBUG_ZCMDSH_COMMAND_SHELL, "command-shell" },
  { DEBUG_ZCMDSH_COMMAND_LOG, "command-log" },
  { DEBUG_ZCMDSH_PAGER_CONTENTS, "pager-contents" },
};

struct command_header debug_zcmdsh_cmd;

/* assume 128 debug items of max-name-len: 16 */
char debug_zcmdsh_cmdstr[128 * 16];

/* assume 128 debug items of max-helpstr-len: 64 */
char debug_zcmdsh_helpstr[128 * 64];

int
debug_zcmdsh_func (void *context, int argc, char **argv)
{
  struct shell *shell = (struct shell *) context;
  int negate = 0;
  int i;
  int debug_type_size;

  if (FLAG_CHECK (DEBUG_CONFIG (ZCMDSH), DEBUG_TYPE (ZCMDSH, COMMAND)))
    {
      DEBUG_LOG_MSG ("%s: argc: %d", __func__, argc);
      for (i = 0; i < argc; i++)
        DEBUG_LOG_MSG ("%s: argv[%d]: %s", __func__, i, argv[i]);
    }

  struct debug_type *debug_types = debug_zcmdsh_types;
  debug_type_size = sizeof (debug_zcmdsh_types) / sizeof (struct debug_type);

  if (! strcmp (argv[0], "no"))
    {
      negate++;
      argv++;
      argc--;
    }

  for (i = 0; i < debug_type_size; i++)
    {
      if (! strcmp (argv[2], debug_types[i].name) ||
          ! strcmp (argv[2], "all"))
        {
          if (negate)
            {
              FLAG_CLEAR (DEBUG_CONFIG (ZCMDSH), debug_types[i].flag);
              fprintf (shell->terminal, "debug: %s: disabled.%s",
                       debug_types[i].name, shell->NL);
            }
          else
            {
              FLAG_SET (DEBUG_CONFIG (ZCMDSH), debug_types[i].flag);
              fprintf (shell->terminal, "debug: %s: enabled.%s",
                       debug_types[i].name, shell->NL);
            }
        }
    }
  return 0;
}

DEFINE_COMMAND (show_debug_zcmdsh, "show debugging zcmdsh",
                SHOW_HELP "show debugging information.\n"
                          "zcmdsh\n")
{
  struct shell *shell = (struct shell *) context;
  int i;
  int debug_type_size;
  debug_type_size = sizeof (debug_zcmdsh_types) / sizeof (struct debug_type);

  for (i = 0; i < debug_type_size; i++)
    {
      fprintf (shell->terminal, "debug: zcmdsh: %s: %s.%s",
               debug_zcmdsh_types[i].name,
               (FLAG_CHECK (DEBUG_CONFIG (ZCMDSH), debug_zcmdsh_types[i].flag)
                    ? "on"
                    : "off"),
               shell->NL);
    }
  return 0;
}

void
debug_zcmdsh_cmd_init ()
{
  int debug_type_size;
  debug_type_size = sizeof (debug_zcmdsh_types) / sizeof (struct debug_type);

  debug_cmdstr_init ("zcmdsh", debug_zcmdsh_cmdstr,
                     sizeof (debug_zcmdsh_cmdstr), debug_zcmdsh_types,
                     debug_type_size);
  debug_helpstr_init ("zcmdsh", debug_zcmdsh_helpstr,
                      sizeof (debug_zcmdsh_helpstr), debug_zcmdsh_types,
                      debug_type_size);

  if (FLAG_CHECK (DEBUG_CONFIG (ZCMDSH), DEBUG_TYPE (ZCMDSH, COMMAND)))
    {
      DEBUG_LOG_MSG ("debug_zcmdsh_cmdstr: %s\n", debug_zcmdsh_cmdstr);
      DEBUG_LOG_MSG ("debug_zcmdsh_helpstr: %s\n", debug_zcmdsh_helpstr);
    }

  debug_zcmdsh_cmd.cmdstr = debug_zcmdsh_cmdstr;
  debug_zcmdsh_cmd.helpstr = debug_zcmdsh_helpstr;
  debug_zcmdsh_cmd.cmdfunc = debug_zcmdsh_func;
}
