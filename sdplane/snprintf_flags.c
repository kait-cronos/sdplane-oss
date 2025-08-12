/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#include <stdio.h>
#include <stdint.h>

#include "snprintf_flags.h"

int
snprintf_flags (char *buf, int size, uint64_t flags,
                struct flag_name *flag_names, char *delim, int flag_names_size)
{
  char *p = buf;
  int bufsize = size;
  int ret = 0;
  int num = 0;
  int i;

  if (! delim)
    delim = "|";

  for (i = flag_names_size - 1; i >= 0; i--)
    {
      if (! (flags & flag_names[i].val))
        continue;
      if (bufsize <= 0)
        continue;

      if (num == 0)
        ret = snprintf (p, bufsize, "%s", flag_names[i].name);
      else
        ret = snprintf (p, bufsize, "%s%s", delim, flag_names[i].name);

      p += ret;
      bufsize -= ret;
      num++;
    }
  return num;
}
