
#include <zcmdsh/debug.h>
#include <zcmdsh/debug_cmd.h>
#include <zcmdsh/debug_module.h>
#include <zcmdsh/debug_module_cmd.h>

#include "debug_sdplane.h"

struct debug_type debug_sdplane_types[] =
{
  { DEBUG_SDPLANE_LTHREAD,    "lthread" },
  { DEBUG_SDPLANE_CONSOLE,    "console" },
  { DEBUG_SDPLANE_TAPHANDLER, "tap-handler" },
  { DEBUG_SDPLANE_L2FWD,      "l2fwd" },
  { DEBUG_SDPLANE_L3FWD,      "l3fwd" },
  { DEBUG_SDPLANE_VTY_SERVER, "vty-server" },
  { DEBUG_SDPLANE_VTY_SHELL,  "vty-shell" },
  { DEBUG_SDPLANE_TELNET_OPT, "telnet-opt" },
  { DEBUG_SDPLANE_STAT_COLLECTOR, "stat-collector" },
};

int debug_module_sdplane = 0;

void
debug_sdplane_init ()
{
#if 0
  int debug_type_size;
  debug_module_sdplane = debug_module_get_size ();
  debug_type_size =
    sizeof (debug_sdplane_types) / sizeof (debug_sdplane_types[0]);
  debug_module_register (debug_module_sdplane, "sdplane",
                         debug_type_size, debug_sdplane_types);
#endif
}

