/*
 * Copyright (C) 2007-2023 Yasuhiro Ohara. All rights reserved.
 */

#ifndef __LOG_H__
#define __LOG_H__

struct loginfo
{
  int flags;
  int facility;
  int maskpri;
  FILE *fp;
};

extern struct loginfo log_default;

/* flags */
#define LOGINFO_FILE   0x01
#define LOGINFO_STDOUT 0x02
#define LOGINFO_STDERR 0x04
#define LOGINFO_SYSLOG 0x08

#define LOG_INDEX_FILE   0
#define LOG_INDEX_STDOUT 1
#define LOG_INDEX_STDERR 2
#define LOG_INDEX_SYSLOG 3
#define LOG_INDEX_MAX    4

int log_getmask ();
void log_setmask (int mask);

void log_debug (const char *format, ...);
void log_info (const char *format, ...);
void log_notice (const char *format, ...);
void log_warn (const char *format, ...);

#endif /*__LOG_H__*/
