#ifndef __DEBUG_ZCMDSH_H__
#define __DEBUG_ZCMDSH_H__

/* types in zcmdsh category */
#define DEBUG_ZCMDSH_SHELL   (1ULL << 0)
#define DEBUG_ZCMDSH_COMMAND (1ULL << 1)
#define DEBUG_ZCMDSH_PAGER   (1ULL << 2)
#define DEBUG_ZCMDSH_TIMER   (1ULL << 3)
#define DEBUG_ZCMDSH_UNICODE (1ULL << 4)
#define DEBUG_ZCMDSH_TERMIO  (1ULL << 5)

#define DEBUG_ZCMDSH_LOG(type, format, ...) \
  DEBUG_LOG(ZCMDSH, type, format, ##__VA_ARGS__)

//#define DEBUG_ZCMDSH(type, format, ...) \
//  DEBUG_LOG(ZCMDSH, type, format, ##__VA_ARGS__)

#endif /*__DEBUG_ZCMDSH_H__*/
