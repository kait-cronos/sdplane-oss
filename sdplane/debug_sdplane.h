#ifndef __DEBUG_SDPLANE_H__
#define __DEBUG_SDPLANE_H__

#include <stdint.h>
#define DEBUG_SDPLANE_LTHREAD    (1ULL << 0)
#define DEBUG_SDPLANE_CONSOLE    (1ULL << 1)
#define DEBUG_SDPLANE_TAPHANDLER (1ULL << 2)
#define DEBUG_SDPLANE_L2FWD      (1ULL << 3)
#define DEBUG_SDPLANE_L3FWD      (1ULL << 4)
#define DEBUG_SDPLANE_VTY_SERVER (1ULL << 5)
#define DEBUG_SDPLANE_VTY_SHELL  (1ULL << 6)
#define DEBUG_SDPLANE_TELNET_OPT (1ULL << 7)
#define DEBUG_SDPLANE_STAT_COLLECTOR (1ULL << 8)

extern int debug_module_sdplane;

void debug_sdplane_init ();

/* types in zcmdsh category */
#define DEBUG_SDPLANE_DEFAULT   (1ULL << 0)
#define DEBUG_SDPLANE_SCHED     (1ULL << 1)
#define DEBUG_SDPLANE_VTY       (1ULL << 2)

#define DEBUG_SDPLANE_LOG(type, format, ...) \
  DEBUG_LOG(SDPLANE, type, format, ##__VA_ARGS__)

#endif /*__DEBUG_SDPLANE_H__*/
