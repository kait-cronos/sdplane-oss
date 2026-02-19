#ifndef __DEBUG_LOG_H__
#define __DEBUG_LOG_H__

#include <execinfo.h> //backtrace
#include <stdint.h>   //uint64_t
#include <stdio.h>

#include "command.h"

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
  do {                                                                        \
    FLAG_SET (debug_output, DEBUG_OUTPUT_##output_type);                      \
  } while (0)

#define DEBUG_OUTPUT_UNSET(output_type)                                       \
  do {                                                                        \
    FLAG_UNSET (debug_output, DEBUG_OUTPUT_##output_type);                    \
  } while (0)

#define DEBUG_OUTPUT_FILE_SET(filename)                                       \
  do {                                                                        \
    debug_log_open_file (filename);                                           \
  } while (0)
#define DEBUG_OUTPUT_FILE_UNSET()                                             \
  do {                                                                        \
    debug_log_close_file ();                                                  \
  } while (0)

/* category */
#define DEBUG_DEFAULT      0
#define DEBUG_ZCMDSH       1
#define DEBUG_SDPLANE      2
#define DEBUG_CATEGORY_MAX 3

#define DEBUG_CONFIG(cate)     (debug_config_g[DEBUG_##cate])
#define DEBUG_TYPE(cate, type) (DEBUG_##cate##_##type)

#define DEBUG_SET(cate, type)                                                 \
  do {                                                                        \
    FLAG_SET (DEBUG_CONFIG (cate), DEBUG_TYPE (cate, type));                  \
  } while (0)

#define DEBUG_UNSET(cate, type)                                               \
  do {                                                                        \
    FLAG_UNSET (DEBUG_CONFIG (cate), DEBUG_TYPE (cate, type));                \
  } while (0)

#define DEBUG_LOG_MSG(format, ...)                                            \
  do {                                                                        \
    debug_log ("%s[%d] %s(): " format, __FILE__, __LINE__, __func__,          \
               ##__VA_ARGS__);                                                \
  } while (0)

#define DEBUG_LOG(cate, type, format, ...)                                    \
  do {                                                                        \
    if (FLAG_CHECK (DEBUG_CONFIG (cate), DEBUG_TYPE (cate, type)))            \
      DEBUG_LOG_MSG (format, ##__VA_ARGS__);                                  \
  } while (0)

/* new log-system */
#define DEBUG_DOMAIN_OF_SHELL   ZCMDSH
#define DEBUG_DOMAIN_OF_COMMAND ZCMDSH
#define DEBUG_DOMAIN_OF_PAGER   ZCMDSH
#define DEBUG_DOMAIN_OF_TIMER   ZCMDSH
#define DEBUG_DOMAIN_OF_UNICODE ZCMDSH
#define DEBUG_DOMAIN_OF_TERMIO  ZCMDSH
#define DEBUG_DOMAIN_OF_TELNET  ZCMDSH
#define DEBUG_DOMAIN_OF_COMMAND_SHELL  ZCMDSH
#define DEBUG_DOMAIN_OF_COMMAND_LOG    ZCMDSH
#define DEBUG_DOMAIN_OF_PAGER_CONTENTS ZCMDSH

//#define DEBUG_DOMAIN_OF_DHCP_SERVER   SDPLANE

#define CONCAT(a,b) a##b
#define TYPE_TO_DOMAIN(type) CONCAT (DEBUG_DOMAIN_OF_, type)
#define DEBUG_CONFIG_DOMAIN(domain) (DEBUG_CONFIG (domain))
#define DEBUG_CONFIG_TYPE(type) (DEBUG_CONFIG_DOMAIN (TYPE_TO_DOMAIN (type)))

#define MAKE_CANONICAL_LOG_TYPE_CONCAT(y,z) DEBUG_##y##_##z
#define MAKE_CANONICAL_LOG_TYPE(domain,type) \
    (MAKE_CANONICAL_LOG_TYPE_CONCAT (domain, type))
#define DEBUG_TYPE_CANONICAL(type) \
    (MAKE_CANONICAL_LOG_TYPE (TYPE_TO_DOMAIN (type), type))

#define IS_DEBUG(type) \
    (FLAG_CHECK (DEBUG_CONFIG_TYPE(type), DEBUG_TYPE_CANONICAL(type)))

#define DEBUG_NEW(type, format, ...)                                          \
  do {                                                                        \
    if (IS_DEBUG(type))                                                       \
      DEBUG_LOG_MSG (format, ##__VA_ARGS__);                                  \
  } while (0)

#define ERROR_MSG(format, ...)                                                \
  do {                                                                        \
    debug_log ("%s[%d] %s(): *error*: " format,                               \
               __FILE__, __LINE__, __func__, ##__VA_ARGS__);                  \
  } while (0)

#define WARNING(format, ...)                                                  \
  do {                                                                        \
    debug_log ("%s[%d] %s(): warning: " format,                               \
               __FILE__, __LINE__, __func__, ##__VA_ARGS__);                  \
  } while (0)

/* default types */
#define DEBUG_DEFAULT_LOGGING   (1ULL << 0)
#define DEBUG_DEFAULT_BACKTRACE (1ULL << 1)

#define DEBUG_DEFAULT_LOG(type, format, ...)                                  \
  DEBUG_LOG (DEFAULT, type, format, ##__VA_ARGS__)

/* types in zcmdsh category */
#define DEBUG_ZCMDSH_SHELL   (1ULL << 0)
#define DEBUG_ZCMDSH_COMMAND (1ULL << 1)
#define DEBUG_ZCMDSH_PAGER   (1ULL << 2)
#define DEBUG_ZCMDSH_TIMER   (1ULL << 3)
#define DEBUG_ZCMDSH_UNICODE (1ULL << 4)
#define DEBUG_ZCMDSH_TERMIO  (1ULL << 5)
#define DEBUG_ZCMDSH_TELNET  (1ULL << 6)
#define DEBUG_ZCMDSH_COMMAND_SHELL   (1ULL << 7)
#define DEBUG_ZCMDSH_COMMAND_LOG   (1ULL << 8)
#define DEBUG_ZCMDSH_PAGER_CONTENTS   (1ULL << 9)

#define DEBUG_ZCMDSH_LOG(type, format, ...)                                   \
  DEBUG_LOG (ZCMDSH, type, format, ##__VA_ARGS__)

EXTERN_COMMAND (debug_zcmdsh);
EXTERN_COMMAND (show_debug_zcmdsh);
void debug_zcmdsh_cmd_init ();

extern uint64_t debug_config;

#define DEBUG_SHELL           (1ULL << 0)
#define DEBUG_COMMAND         (1ULL << 1)
#define DEBUG_TERMIO          (1ULL << 2)
#define DEBUG_TIMER           (1ULL << 3)
#define DEBUG_SDPLANE_WIRETAP (1ULL << 4)

struct debug_type
{
  uint64_t flag;
  const char *name;
};

#endif /*__DEBUG_LOG_H__*/
