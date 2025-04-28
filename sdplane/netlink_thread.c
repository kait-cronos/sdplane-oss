#include "include.h"

#include <lthread.h>

#include <sdplane/debug.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "sdplane.h"
#include "thread_info.h"

extern int lthread_core;
extern volatile bool force_stop[RTE_MAX_LCORE];

static __thread uint64_t loop_counter = 0;

void
netlink_thread (void *arg)
{
  unsigned lcore_id = rte_lcore_id ();

  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);
  DEBUG_SDPLANE_LOG (NETLINK, "%s: started.", __func__);

  int thread_id;
  thread_id = thread_lookup (netlink_thread);
  thread_register_loop_counter (thread_id, &loop_counter);

  while (! force_quit && ! force_stop[lthread_core])
    {
      lthread_sleep (100); // yield.
      //DEBUG_SDPLANE_LOG (NETLINK, "%s: schedule.", __func__);

      loop_counter++;
    }

  DEBUG_SDPLANE_LOG (NETLINK, "%s: terminating.", __func__);
  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);
}
