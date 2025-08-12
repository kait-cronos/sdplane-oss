/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "debug_category.h"
#include "debug_log.h"

uint64_t debug_config_g[DEBUG_CATEGORY_MAX];
uint64_t debug_output;

/* syslog */
char *ident = "debug_log";
int option = LOG_CONS | LOG_NDELAY | LOG_PID;
int level = LOG_INFO;
int facility = LOG_USER;

/* file */
char *debug_log_filename;
FILE *debug_log_file;

static int
snprintf_time (char *buf, int size)
{
  char timebuf[32];
  time_t clock;
  struct tm *tm;
  time (&clock);
  tm = localtime (&clock);
  strftime (timebuf, sizeof (timebuf), "%Y/%m/%d %H:%M:%S", tm);
  return snprintf (buf, size, "%s", timebuf);
}

int
debug_vlog (const char *format, va_list *args)
{
  int ret = 0;
  char timebuf[32];
  snprintf_time (timebuf, sizeof (timebuf));
  int priority;
  pid_t pid;

  pid = getpid ();

  if (FLAG_CHECK (debug_output, DEBUG_OUTPUT_STDOUT))
    {
      ret = fprintf (stdout, "%s ", timebuf);
      ret += fprintf (stdout, "%s[%d]: ", ident, pid);
      ret += vfprintf (stdout, format, args[DEBUG_INDEX_STDOUT]);
      ret += fprintf (stdout, "\n");
      fflush (stdout);
    }

  if (FLAG_CHECK (debug_output, DEBUG_OUTPUT_STDERR))
    {
      ret = fprintf (stderr, "%s ", timebuf);
      ret += fprintf (stderr, "%s[%d]: ", ident, pid);
      ret += vfprintf (stderr, format, args[DEBUG_INDEX_STDERR]);
      ret += fprintf (stderr, "\n");
      fflush (stderr);
    }

  if (FLAG_CHECK (debug_output, DEBUG_OUTPUT_SYSLOG))
    {
      priority = level | facility;
      vsyslog (priority, format, args[DEBUG_INDEX_SYSLOG]);
    }

  if (FLAG_CHECK (debug_output, DEBUG_OUTPUT_FILE))
    {
      ret = fprintf (debug_log_file, "%s ", timebuf);
      ret += fprintf (debug_log_file, "%s[%d]: ", ident, pid);
      ret += vfprintf (debug_log_file, format, args[DEBUG_INDEX_FILE]);
      ret += fprintf (debug_log_file, "\n");
      fflush (debug_log_file);
    }

  return ret;
}

int
debug_log (const char *format, ...)
{
  va_list args[DEBUG_INDEX_MAX];
  int index;
  int ret;

  for (index = 0; index < DEBUG_INDEX_MAX; index++)
    va_start (args[index], format);

  ret = debug_vlog (format, args);

  for (index = 0; index < DEBUG_INDEX_MAX; index++)
    va_end (args[index]);

  return ret;
}

void
debug_log_open_syslog ()
{
  FLAG_SET (debug_output, DEBUG_OUTPUT_SYSLOG);
  openlog (ident, option, facility);
}

void
debug_log_close_syslog ()
{
  FLAG_UNSET (debug_output, DEBUG_OUTPUT_SYSLOG);
  closelog ();
}

void
debug_log_open_file (char *filename)
{
  if (! filename)
    return;
  if (! strlen (filename))
    return;

  FLAG_SET (debug_output, DEBUG_OUTPUT_FILE);
  if (debug_log_filename)
    free (debug_log_filename);
  debug_log_filename = strdup (filename);
  if (debug_log_file)
    fclose (debug_log_file);
  debug_log_file = fopen (debug_log_filename, "a");
}

void
debug_log_close_file ()
{
  FLAG_UNSET (debug_output, DEBUG_OUTPUT_FILE);
  if (debug_log_filename)
    free (debug_log_filename);
  debug_log_filename = NULL;
  if (debug_log_file)
    fclose (debug_log_file);
  debug_log_file = NULL;
}

void
debug_log_rotate_file ()
{
  if (! FLAG_CHECK (debug_output, DEBUG_OUTPUT_FILE))
    return;

  if (debug_log_file)
    fclose (debug_log_file);
  if (debug_log_filename)
    debug_log_file = fopen (debug_log_filename, "a");
  else
    debug_log_file = NULL;
}

void
debug_log_init (char *progname)
{
  char *p;
  if (progname)
    {
      p = rindex (progname, '/');
      if (p)
        p++;
      if (p && strlen (p))
        ident = p;
      else
        ident = progname;
    }

  DEBUG_SET (DEFAULT, LOGGING);
  DEBUG_SET (DEFAULT, BACKTRACE);
}
