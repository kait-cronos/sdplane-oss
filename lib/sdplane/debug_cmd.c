/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
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

void
debug_cmdstr_init (char *cate, char *cmdstr, int size,
                   struct debug_type *debug_types, int types_size)
{
  int i;
  char *p;
  int ret, len;
  int debug_type_size;

#if 0
  debug_type_size = sizeof (debug_types) / sizeof (struct debug_type);

  p = &debug_cmdstr[0];
  len = sizeof (debug_cmdstr);
#else
  debug_type_size = types_size;

  p = cmdstr;
  len = size;
#endif

  ret = snprintf (p, len, "(|no) ");
  p += ret;
  len -= ret;

  ret = snprintf (p, len, "debug ");
  p += ret;
  len -= ret;

  ret = snprintf (p, len, "%s ", cate);
  p += ret;
  len -= ret;

  if (debug_type_size > 1)
    {
      ret = snprintf (p, len, "(");
      p += ret;
      len -= ret;
    }

  ret = snprintf (p, len, "all|");
  p += ret;
  len -= ret;

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
debug_helpstr_init (char *cate, char *helpstr, int size,
                    struct debug_type *debug_types, int types_size)
{
  int i;
  char *p;
  int ret, len;
  int debug_type_size;

#if 0
  debug_type_size = sizeof (debug_types) / sizeof (struct debug_type);

  p = &debug_helpstr[0];
  len = sizeof (debug_helpstr);
#else
  debug_type_size = types_size;

  p = helpstr;
  len = size;
#endif

  ret = snprintf (p, len, "disable command.\n");
  p += ret;
  len -= ret;

  ret = snprintf (p, len, "debug command.\n");
  p += ret;
  len -= ret;

  ret = snprintf (p, len, "debug %s.\n", cate);
  p += ret;
  len -= ret;

  ret = snprintf (p, len, "all types.\n");
  p += ret;
  len -= ret;

  for (i = 0; i < debug_type_size; i++)
    {
      ret =
          snprintf (p, len, "debug %s %s item.\n", cate, debug_types[i].name);
      p += ret;
      len -= ret;
    }
}
