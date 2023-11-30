/*
 * Copyright (C) 2007-2023 Yasuhiro Ohara. All rights reserved.
 */

#include "includes.h"

#include "flag.h"
#include "debug.h"
#include "shell.h"
#include "command.h"

struct debug_type debug_types[] =
{
  { DEBUG_SHELL,   "shell" },
  { DEBUG_COMMAND, "command" },
  { DEBUG_TERMIOS, "termios" },
};

struct command_header debug_cmd;

/* assume 128 debug items of max-name-len: 16 */
char debug_cmdstr[128 * 16];

/* assume 128 debug items of max-helpstr-len: 64 */
char debug_helpstr[128 * 64];

void
debug_cmdfunc (void *context, int argc, char **argv)
{
  //struct shell *shell = (struct shell *) context;
}

void
debug_cmd_init ()
{
  int i;
  int debug_type_size;
  char *p;
  int ret, len;

  p = &debug_cmdstr[0];
  len = sizeof (debug_cmdstr);
  debug_type_size = sizeof (debug_types) / sizeof (struct debug_type);

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

  printf ("debug_cmdstr: %s\n", debug_cmdstr);

  debug_cmd.cmdstr = debug_cmdstr;
  debug_cmd.helpstr = debug_helpstr;
  debug_cmd.cmdfunc = debug_cmdfunc;
}

