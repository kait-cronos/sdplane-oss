/*
 * Copyright (C) 2007  Yasuhiro Ohara
 * 
 * This file is part of SimRouting.
 * 
 * SimRouting is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * SimRouting is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _LOG_H_
#define _LOG_H_

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

#endif /*_LOG_H_*/

