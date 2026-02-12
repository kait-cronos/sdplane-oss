/*
 * Copyright (C) 2025 Yasuhiro Ohara. All rights reserved.
 */

#include "includes.h"

#include <poll.h>
#include <stdbool.h>

#include "termio.h"

#if 0

#include "command.h"
#include "command_shell.h"
#include "file.h"
#include "shell.h"
#include "shell_keyfunc.h"
#include "vector.h"

#endif /*0*/

int duration_limit = 0;
time_t start = 0;
time_t end = 0;
time_t limit = 0;
char start_str[64], end_str[64], limit_str[64];

struct tm start_tm_buf, end_tm_buf, limit_tm_buf;
struct tm *start_tm, *end_tm, *limit_tm;

void
timer_init (int dura_limit, char *date_limit)
{
  int ret;

  /* reset all */
  duration_limit = 0;
  start = end = limit = 0;
  memset (start_str, 0, sizeof (start_str));
  memset (start_str, 0, sizeof (start_str));

  /* start time */
  time (&start);
  start_tm = localtime_r (&start, &start_tm_buf);
  strftime (start_str, sizeof (start_str), "%Y/%m/%d %H:%M:%S", start_tm);

  if (dura_limit)
    {
      duration_limit = dura_limit;
      end = start + duration_limit;
      end_tm = localtime_r (&end, &end_tm_buf);
      strftime (end_str, sizeof (end_str), "%Y/%m/%d %H:%M:%S", end_tm);
    }

  if (date_limit)
    {
      ret = sscanf (date_limit, "%d/%d/%d %d:%d:%d", &limit_tm_buf.tm_year,
                    &limit_tm_buf.tm_mon, &limit_tm_buf.tm_mday,
                    &limit_tm_buf.tm_hour, &limit_tm_buf.tm_min,
                    &limit_tm_buf.tm_sec);
      assert (ret == 6);
      limit_tm = &limit_tm_buf;

      /* adjust year and month. */
      limit_tm->tm_year -= 1900;
      limit_tm->tm_mon -= 1;

      limit = mktime (limit_tm);
      strftime (limit_str, sizeof (limit_str), "%Y/%m/%d %H:%M:%S", limit_tm);
    }
}

void
timer_check ()
{
  time_t current;
  char current_str[64];
  struct tm *current_tm, current_tm_buf;

  time (&current);
  current_tm = localtime_r (&current, &current_tm_buf);
  strftime (current_str, sizeof (current_str), "%Y/%m/%d %H:%M:%S",
            current_tm);

  double diff_end, diff_limit;

  if (end)
    {
      diff_end = difftime (end, current);
      if (diff_end < 0)
        {
          printf ("zcmdsh: beta-version: duration-limit: %'d secs\n",
                  duration_limit);
          printf ("zcmdsh: shutdown.\n");
          termio_finish ();
          exit (1);
        }
    }

  if (limit)
    {
      diff_limit = difftime (limit, current);
      if (diff_limit < 0)
        {
          printf ("zcmdsh: beta-version: date-limit: %s\n", limit_str);
          printf ("zcmdsh: shutdown.\n");
          termio_finish ();
          exit (1);
        }
    }
}
