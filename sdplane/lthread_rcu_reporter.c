#include <stdio.h>

#include <lthread.h>
#include <urcu/urcu-qsbr.h>

#include <sdplane/log.h>
#include <sdplane/debug.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>

#include "debug_sdplane.h"
#include "thread_info.h"

extern volatile bool force_quit;
extern volatile bool force_stop[RTE_MAX_LCORE];

extern int lthread_core;

static uint64_t loop_lthread_rcu_reporter = 0;

int
lthread_rcu_reporter (__rte_unused void *dummy)
{
  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);
  DEBUG_NEW (RCU_READ, "%s: started.", __func__);

  int thread_id;
  thread_id = thread_lookup (lthread_rcu_reporter);
  thread_register_loop_counter (thread_id, &loop_lthread_rcu_reporter);

  while (! force_quit && ! force_stop[lthread_core])
    {
      lthread_sleep (0); // yield.
      urcu_qsbr_quiescent_state ();
      loop_lthread_rcu_reporter++;
    }

  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);
  return 0;
}
