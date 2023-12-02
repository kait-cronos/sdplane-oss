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
  { DEBUG_TERMIO,  "termio" },
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

  if (FLAG_CHECK (debug_config, DEBUG_COMMAND))
    {
  printf ("%s: argc: %d\n", __func__, argc);
  for (i = 0; i < argc; i++)
    printf ("%s: argv[%d]: %s\n", __func__, i, argv[i]);
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
      if (! strcmp (argv[1], debug_types[i].name))
        {
          if (negate)
            {
              FLAG_CLEAR (debug_config, debug_types[i].flag);
              fprintf (shell->terminal, "debug: %s: disabled.\n",
                       debug_types[i].name);
            }
          else
            {
              FLAG_SET (debug_config, debug_types[i].flag);
              fprintf (shell->terminal, "debug: %s: enabled.\n",
                       debug_types[i].name);
            }
        }
    }
}

DEFINE_COMMAND (show_debug,
                "show debugging",
                "show information.\n"
                "show debugging information.\n")
{
  struct shell *shell = (struct shell *) context;
  int i;
  int debug_type_size;
  debug_type_size = sizeof (debug_types) / sizeof (struct debug_type);

  for (i = 0; i < debug_type_size; i++)
    {
      fprintf (shell->terminal, "debug: %s: %s.\n",
               debug_types[i].name,
               (FLAG_CHECK (debug_config, debug_types[i].flag) ?
               "on" : "off"));
    }
}

void
debug_cmd_init ()
{
  debug_cmdstr_init ();
  debug_helpstr_init ();

  if (FLAG_CHECK (debug_config, DEBUG_COMMAND))
    {
  printf ("debug_cmdstr: %s\n", debug_cmdstr);
  printf ("debug_helpstr: %s\n", debug_helpstr);
    }

  debug_cmd.cmdstr = debug_cmdstr;
  debug_cmd.helpstr = debug_helpstr;
  debug_cmd.cmdfunc = debug_func;
}

