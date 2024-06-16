#ifndef __DEBUG_SDPLANE_H__
#define __DEBUG_SDPLANE_H__

#include <stdint.h>
#define DEBUG_SDPLANE_LTHREAD    (1ULL << 0)
#define DEBUG_SDPLANE_CONSOLE    (1ULL << 1)
#define DEBUG_SDPLANE_TAPHANDLER (1ULL << 2)
#define DEBUG_SDPLANE_L2FWD      (1ULL << 3)
#define DEBUG_SDPLANE_L3FWD      (1ULL << 4)

extern int debug_module_sdplane;

void debug_sdplane_init ();

#endif /*__DEBUG_SDPLANE_H__*/
