/*
 * Copyright (C) 2007-2023 Yasuhiro Ohara. All rights reserved.
 */

#include "includes.h"

#include "flag.h"
#include "debug.h"
#include "shell.h"
#include "command.h"

#include "debug_log.h"
#include "debug_category.h"
#include "debug_zcmdsh.h"
#include "debug_backtrace.h"

struct debug_type debug_types[] =
{
  { DEBUG_ZCMDSH_SHELL,   "shell" },
  { DEBUG_ZCMDSH_COMMAND, "command" },
  { DEBUG_ZCMDSH_PAGER,   "pager" },
  { DEBUG_ZCMDSH_TIMER,   "timer" },
  { DEBUG_ZCMDSH_UNICODE, "unicode" },
  { DEBUG_ZCMDSH_TERMIO,  "termio" },
  { DEBUG_ZCMDSH_TELNET,  "telnet" },
};

struct command_header debug_cmd;

/* assume 128 debug items of max-name-len: 16 */
char debug_cmdstr[128 * 16];

/* assume 128 debug items of max-helpstr-len: 64 */
char debug_helpstr[128 * 64];

void
debug_cmdstr_init ()
{
  int i;
  char *p;
  int ret, len;
  int debug_type_size;

  debug_type_size = sizeof (debug_types) / sizeof (struct debug_type);

  p = &debug_cmdstr[0];
  len = sizeof (debug_cmdstr);

  ret = snprintf (p, len, "(|no) ");
  p += ret;
  len -= ret;

  ret = snprintf (p, len, "debug ");
  p += ret;
  len -= ret;

  ret = snprintf (p, len, "zcmdsh ");
  p += ret;
  len -= ret;

  if (debug_type_size > 1)
    {
      ret = snprintf (p, len, "(");
      p += ret;
      len -= ret;
    }
  for (i = 0; i < debug_type_size; i++)
    {
      if (i + 1 < debug_type_size)
        ret = snprintf (p, len, "%s|", debug_types[i].name);
      else
        ret = snprintf (p, len, "%s", debug_types[i].name);
      p += ret;
      len -= ret;
    }
  if (debug_type_size > 1)
    {
      ret = snprintf (p, len, ")");
      p += ret;
      len -= ret;
    }
}

void
debug_helpstr_init ()
{
  int i;
  char *p;
  int ret, len;
  int debug_type_size;

  debug_type_size = sizeof (debug_types) / sizeof (struct debug_type);

  p = &debug_helpstr[0];
  len = sizeof (debug_helpstr);

  ret = snprintf (p, len, "disable command.\n");
  p += ret;
  len -= ret;

  ret = snprintf (p, len, "debug command.\n");
  p += ret;
  len -= ret;

  ret = snprintf (p, len, "debug zcmdsh.\n");
  p += ret;
  len -= ret;

  for (i = 0; i < debug_type_size; i++)
    {
      ret = snprintf (p, len, "debug %s item.\n", debug_types[i].name);
      p += ret;
      len -= ret;
    }
}

void
debug_func (void *context, int argc, char **argv)
{
  struct shell *shell = (struct shell *) context;
  int negate = 0;
  int i;
  int debug_type_size;

  if (FLAG_CHECK (DEBUG_CONFIG(ZCMDSH), DEBUG_TYPE(ZCMDSH, COMMAND)))
    {
      DEBUG_LOG_MSG ("%s: argc: %d", __func__, argc);
      for (i = 0; i < argc; i++)
        DEBUG_LOG_MSG ("%s: argv[%d]: %s", __func__, i, argv[i]);
    }

  debug_type_size = sizeof (debug_types) / sizeof (struct debug_type);

  if (! strcmp (argv[0], "no"))
    {
      negate++;
      argv++;
      argc--;
    }

  for (i = 0; i < debug_type_size; i++)
    {
      if (! strcmp (argv[2], debug_types[i].name))
        {
          if (negate)
            {
              FLAG_CLEAR (DEBUG_CONFIG(ZCMDSH), debug_types[i].flag);
              fprintf (shell->terminal, "debug: %s: disabled.%s",
                       debug_types[i].name, shell->NL);
            }
          else
            {
              FLAG_SET (DEBUG_CONFIG(ZCMDSH), debug_types[i].flag);
              fprintf (shell->terminal, "debug: %s: enabled.%s",
                       debug_types[i].name, shell->NL);
            }
        }
    }
}

DEFINE_COMMAND (show_debug,
                "show debugging",
                SHOW_HELP
                "show debugging information.\n")
{
  struct shell *shell = (struct shell *) context;
  int i;
  int debug_type_size;
  debug_type_size = sizeof (debug_types) / sizeof (struct debug_type);

  for (i = 0; i < debug_type_size; i++)
    {
      fprintf (shell->terminal, "debug: zcmdsh: %s: %s.%s",
               debug_types[i].name,
               (FLAG_CHECK (DEBUG_CONFIG(ZCMDSH), debug_types[i].flag) ?
               "on" : "off"), shell->NL);
    }
}

void
debug_cmd_init ()
{
  debug_cmdstr_init ();
  debug_helpstr_init ();

  if (FLAG_CHECK (DEBUG_CONFIG(ZCMDSH), DEBUG_TYPE(ZCMDSH,COMMAND)))
    {
      DEBUG_LOG_MSG ("debug_cmdstr: %s\n", debug_cmdstr);
      DEBUG_LOG_MSG ("debug_helpstr: %s\n", debug_helpstr);
    }

  debug_cmd.cmdstr = debug_cmdstr;
  debug_cmd.helpstr = debug_helpstr;
  debug_cmd.cmdfunc = debug_func;
}

