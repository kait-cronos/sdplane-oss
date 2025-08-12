/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#include <includes.h>

#include "log.h"

struct loginfo log_default = {
  LOGINFO_STDOUT, /* flags */
  LOG_DAEMON,     /* facility */
  LOG_INFO,       /* maskpri */
  NULL,           /* fp */
};

int
log_getmask ()
{
  return log_default.maskpri;
}

void
log_setmask (int mask)
{
  log_default.maskpri = mask;
}

#define TIMEBUFSIZ 32

static void
time_printbuf (char *buf, int size)
{
  int ret;
  time_t clock;
  struct tm *tm;

  time (&clock);
  tm = localtime (&clock);

  ret = strftime (buf, size, "%Y/%m/%d %H:%M:%S", tm);
  if (ret == 0)
    snprintf (buf, size, "TIME Error");
}

void
vlog (struct loginfo *log, int priority, const char *format, va_list *args)
{
  char timebuf[TIMEBUFSIZ];

  if (log == NULL)
    log = &log_default;

  time_printbuf (timebuf, TIMEBUFSIZ);

  if (priority > log->maskpri)
    return;

  if (log->flags & LOGINFO_SYSLOG)
    vsyslog (priority | log->facility, format, args[LOG_INDEX_SYSLOG]);

  if (log->flags & LOGINFO_FILE)
    {
      fprintf (log->fp, "%s ", timebuf);
      vfprintf (log->fp, format, args[LOG_INDEX_FILE]);
      fprintf (log->fp, "\n");
      fflush (log->fp);
    }

  if (log->flags & LOGINFO_STDOUT)
    {
      fprintf (stdout, "%s ", timebuf);
      vfprintf (stdout, format, args[LOG_INDEX_STDOUT]);
      fprintf (stdout, "\n");
      fflush (stdout);
    }

  if (log->flags & LOGINFO_STDERR)
    {
      fprintf (stderr, "%s ", timebuf);
      vfprintf (stderr, format, args[LOG_INDEX_STDERR]);
      fprintf (stderr, "\n");
      fflush (stderr);
    }
}

void
log_debug (const char *format, ...)
{
  va_list args[LOG_INDEX_MAX];
  int index;

  for (index = 0; index < LOG_INDEX_MAX; index++)
    va_start (args[index], format);

  vlog (NULL, LOG_DEBUG, format, args);

  for (index = 0; index < LOG_INDEX_MAX; index++)
    va_end (args[index]);
}

void
log_info (const char *format, ...)
{
  va_list args[LOG_INDEX_MAX];
  int index;

  for (index = 0; index < LOG_INDEX_MAX; index++)
    va_start (args[index], format);

  vlog (NULL, LOG_INFO, format, args);

  for (index = 0; index < LOG_INDEX_MAX; index++)
    va_end (args[index]);
}

void
log_notice (const char *format, ...)
{
  va_list args[LOG_INDEX_MAX];
  int index;

  for (index = 0; index < LOG_INDEX_MAX; index++)
    va_start (args[index], format);

  vlog (NULL, LOG_NOTICE, format, args);

  for (index = 0; index < LOG_INDEX_MAX; index++)
    va_end (args[index]);
}

void
log_warn (const char *format, ...)
{
  va_list args[LOG_INDEX_MAX];
  int index;

  for (index = 0; index < LOG_INDEX_MAX; index++)
    va_start (args[index], format);

  vlog (NULL, LOG_WARNING, format, args);

  for (index = 0; index < LOG_INDEX_MAX; index++)
    va_end (args[index]);
}
