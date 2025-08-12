/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef __DEBUG_LOG_H__
#define __DEBUG_LOG_H__

#include <execinfo.h> //backtrace
#include <stdint.h>   //uint64_t
#include <stdlib.h>   //free in backtrace

#ifndef FLAG_SET
#define FLAG_CHECK(V, F) ((V) & (F))
#define FLAG_SET(V, F)   ((V) |= (F))
#define FLAG_UNSET(V, F) ((V) &= ~(F))
#define FLAG_CLEAR(V, F) ((V) &= ~(F))
#define FLAG_RESET(V)    ((V) = 0)
#define FLAG_ZERO(V)     ((V) = 0)
#endif /*FLAG_SET*/

/* output index */
#define DEBUG_INDEX_STDOUT 0
#define DEBUG_INDEX_STDERR 1
#define DEBUG_INDEX_SYSLOG 2
#define DEBUG_INDEX_FILE   3
#define DEBUG_INDEX_MAX    4

/* output flag */
#define DEBUG_OUTPUT_STDOUT (1ULL << DEBUG_INDEX_STDOUT)
#define DEBUG_OUTPUT_STDERR (1ULL << DEBUG_INDEX_STDERR)
#define DEBUG_OUTPUT_SYSLOG (1ULL << DEBUG_INDEX_SYSLOG)
#define DEBUG_OUTPUT_FILE   (1ULL << DEBUG_INDEX_FILE)

/* syslog */
extern char *ident;
extern int option;
extern int level;
extern int facility;

/* file */
extern char *debug_log_filename;
extern FILE *debug_log_file;

/* config, output */
extern uint64_t debug_config_g[];
extern uint64_t debug_output;

int debug_vlog (const char *format, va_list *args);
int debug_log (const char *format, ...);

void debug_log_open_syslog ();
void debug_log_close_syslog ();

void debug_log_open_file (char *filename);
void debug_log_close_file ();
void debug_log_rotate_file ();

void debug_log_init (char *progname);

#define DEBUG_OUTPUT_SET(output_type)                                         \
  do                                                                          \
    {                                                                         \
      FLAG_SET (debug_output, DEBUG_OUTPUT_##output_type);                    \
    }                                                                         \
  while (0)

#define DEBUG_OUTPUT_UNSET(output_type)                                       \
  do                                                                          \
    {                                                                         \
      FLAG_UNSET (debug_output, DEBUG_OUTPUT_##output_type);                  \
    }                                                                         \
  while (0)

#define DEBUG_OUTPUT_FILE_SET(filename)                                       \
  do                                                                          \
    {                                                                         \
      debug_log_open_file (filename);                                         \
    }                                                                         \
  while (0)
#define DEBUG_OUTPUT_FILE_UNSET()                                             \
  do                                                                          \
    {                                                                         \
      debug_log_close_file ();                                                \
    }                                                                         \
  while (0)

#define DEBUG_CONFIG(cate)     (debug_config_g[DEBUG_##cate])
#define DEBUG_TYPE(cate, type) (DEBUG_##cate##_##type)

#define DEBUG_SET(cate, type)                                                 \
  do                                                                          \
    {                                                                         \
      FLAG_SET (DEBUG_CONFIG (cate), DEBUG_TYPE (cate, type));                \
    }                                                                         \
  while (0)

#define DEBUG_UNSET(cate, type)                                               \
  do                                                                          \
    {                                                                         \
      FLAG_UNSET (DEBUG_CONFIG (cate), DEBUG_TYPE (cate, type));              \
    }                                                                         \
  while (0)

#define DEBUG_LOG_MSG(format, ...)                                            \
  do                                                                          \
    {                                                                         \
      debug_log ("%s[%d] %s(): " format, __FILE__, __LINE__, __func__,        \
                 ##__VA_ARGS__);                                              \
    }                                                                         \
  while (0)

#define DEBUG_LOG(cate, type, format, ...)                                    \
  do                                                                          \
    {                                                                         \
      if (FLAG_CHECK (DEBUG_CONFIG (cate), DEBUG_TYPE (cate, type)))          \
        DEBUG_LOG_MSG (format, ##__VA_ARGS__);                                \
    }                                                                         \
  while (0)

/* default types */
#define DEBUG_DEFAULT_LOGGING   (1ULL << 0)
#define DEBUG_DEFAULT_BACKTRACE (1ULL << 1)

#define DEBUG_DEFAULT_LOG(type, format, ...)                                  \
  DEBUG_LOG (DEFAULT, type, format, ##__VA_ARGS__)

#endif /*__DEBUG_LOG_H__*/
