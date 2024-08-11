/*
 * Copyright (C) 2007-2023 Yasuhiro Ohara. All rights reserved.
 */

#include "includes.h"

#include "flag.h"
#include "debug.h"
#include "shell.h"
#include "command.h"
#include "debug_module.h"
#include "debug_module_cmd.h"

static int debug_module_size;

struct debug_module_type debug_module_slots[DEBUG_MODULE_MAX];

#if 0
/* placeholder for names for all debug modules */
struct debug_type debug_module_types[DEBUG_MODULE_MAX][64];
#endif

struct command_header debug_module_cmd[DEBUG_MODULE_MAX];

/* assume each modules of 64 debug items of max-name-len: 16 bytes*/
char debug_module_cmdstr[DEBUG_MODULE_MAX][64 * 16];

/* assume each modules of 64 debug items of max-helpstr-len: 64 */
char debug_module_helpstr[DEBUG_MODULE_MAX][64 * 64];

void
debug_module_cmdstr_init (int module)
{
  int i;
  char *p;
  int ret, len;
  int debug_type_size;
  struct debug_module_type *slot;
  struct debug_type *debug_types;

  slot = &debug_module_slots[module];
  debug_type_size = slot->debug_type_size;
  debug_types = slot->debug_types;

  p = &debug_module_cmdstr[module][0];
  len = sizeof (debug_module_cmdstr[module]);

  ret = snprintf (p, len, "(|no) ");
  p += ret;
  len -= ret;

  ret = snprintf (p, len, "debug ");
  p += ret;
  len -= ret;

  ret = snprintf (p, len, "%s ", slot->module_name);
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
debug_module_helpstr_init (int module)
{
  int i;
  char *p;
  int ret, len;
  int debug_type_size;
  struct debug_module_type *slot;
  struct debug_type *debug_types;

  slot = &debug_module_slots[module];
  debug_type_size = slot->debug_type_size;
  debug_types = slot->debug_types;

  p = &debug_module_helpstr[module][0];
  len = sizeof (debug_module_helpstr[module]);

  ret = snprintf (p, len, "disable command.\n");
  p += ret;
  len -= ret;

  ret = snprintf (p, len, "debug command.\n");
  p += ret;
  len -= ret;

  ret = snprintf (p, len, "debug %s module.\n", slot->module_name);
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
debug_module_func (void *context, int argc, char **argv)
{
  struct shell *shell = (struct shell *) context;
  int negate = 0;
  int i;
  int debug_type_size;
  struct debug_module_type *slot;
  struct debug_type *debug_types;
  int match;

  if (FLAG_CHECK (debug_config, DEBUG_COMMAND))
    {
      printf ("%s: argc: %d\n", __func__, argc);
      for (i = 0; i < argc; i++)
        printf ("%s: argv[%d]: %s\n", __func__, i, argv[i]);
    }

  if (! strcmp (argv[0], "no"))
    {
      negate++;
      argv++;
      argc--;
    }

  slot = NULL;
  match = -1;
  for (i = 0; i < debug_module_size; i++)
    {
      slot = &debug_module_slots[i];
      if (! strcmp (argv[1], slot->module_name))
        match = i;
    }
  assert (match >= 0);

  slot = &debug_module_slots[match];
  debug_type_size = slot->debug_type_size;
  debug_types = slot->debug_types;

  for (i = 0; i < debug_type_size; i++)
    {
      if (! strcmp (argv[2], debug_types[i].name))
        {
          if (negate)
            {
              FLAG_CLEAR (debug_module_config[match], debug_types[i].flag);
              fprintf (shell->terminal, "debug: %s: %s: disabled.\n",
                       slot->module_name, debug_types[i].name);
            }
          else
            {
              FLAG_SET (debug_module_config[match], debug_types[i].flag);
              fprintf (shell->terminal, "debug: %s: %s: enabled.\n",
                       slot->module_name, debug_types[i].name);
            }
        }
    }
}

DEFINE_COMMAND (show_debug_module,
                "show debugging modules",
                SHOW_HELP
                "show debugging information.\n"
                "show debugging module information.\n"
                )
{
  struct shell *shell = (struct shell *) context;
  int i, j;
  int debug_type_size;
  struct debug_module_type *slot;
  struct debug_type *debug_types;

  for (j = 0; j < debug_module_size; j++)
    {
      slot = &debug_module_slots[j];
      debug_type_size = slot->debug_type_size;
      debug_types = slot->debug_types;

      fprintf (shell->terminal,
               "debug: module[%d]: %s: %p%s",
               j, slot->module_name,
               (void *)&debug_module_config[j],
               shell->LF);

      for (i = 0; i < debug_type_size; i++)
        {
          fprintf (shell->terminal,
                   "debug: module[%d]: %s: type[%d]: %s: flag: %#lx: %s.%s",
                   j, slot->module_name,
                   i, debug_types[i].name,
                   debug_types[i].flag,
                   (FLAG_CHECK (debug_module_config[i], debug_types[i].flag) ?
                   "on" : "off"), shell->LF);
        }
    }
}

int
debug_module_get_size ()
{
  return debug_module_size;
}

void
debug_module_register (int module, char *name, int debug_type_size,
                       struct debug_type *debug_types)
{
  debug_module_slots[module].module_name = name;
  debug_module_slots[module].debug_type_size = debug_type_size;
  debug_module_slots[module].debug_types = debug_types;
  if (debug_module_size < module + 1)
    debug_module_size = module + 1;
}

void
debug_module_cmd_init ()
{
  int i;
  for (i = 0; i < debug_module_size; i++)
    {
      debug_module_cmdstr_init (i);
      debug_module_helpstr_init (i);

      if (FLAG_CHECK (debug_config, DEBUG_COMMAND))
        {
          printf ("debug_module_cmdstr[%d]: %s\n", i, debug_module_cmdstr[i]);
          printf ("debug_module_helpstr[%d]: %s\n", i, debug_module_helpstr[i]);
        }
      debug_module_cmd[i].cmdstr = debug_module_cmdstr[i];
      debug_module_cmd[i].helpstr = debug_module_helpstr[i];
      debug_module_cmd[i].cmdfunc = debug_module_func;
    }
}

