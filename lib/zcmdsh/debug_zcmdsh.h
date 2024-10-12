#ifndef __DEBUG_ZCMDSH_H__
#define __DEBUG_ZCMDSH_H__

/* mtcp types */
#define DEBUG_ZCMDSH_ZERO   (1ULL << 0)
#define DEBUG_ZCMDSH_ONE    (1ULL << 1)
#define DEBUG_ZCMDSH_TWO    (1ULL << 2)
#define DEBUG_ZCMDSH_THREE  (1ULL << 3)

#define DEBUG_ZCMDSH_LOG(type, format, ...) \
  DEBUG_LOG(ZCMDSH, type, format, ##__VA_ARGS__)

#endif /*__DEBUG_ZCMDSH_H__*/
