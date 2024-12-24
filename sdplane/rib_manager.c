#include "include.h"

#include <lthread.h>

#include <rte_common.h>
#include <rte_launch.h>
#include <rte_ether.h>

#include <zcmdsh/shell.h>
#include <zcmdsh/command.h>
#include <zcmdsh/command_shell.h>

#include <zcmdsh/debug.h>
#include <zcmdsh/debug_cmd.h>
#include <zcmdsh/debug_log.h>
#include <zcmdsh/debug_category.h>
#include <zcmdsh/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "rib_manager.h"
#include "sdplane.h"

#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

extern int lthread_core;
extern volatile bool force_stop[RTE_MAX_LCORE];

struct rte_ring *msg_queue_rib;

void *rcu_global_ptr_rib;
uint64_t rib_rcu_replace = 0;

struct rib *rib;

void
rib_manager_recv_message (void *msgp)
{
  DEBUG_SDPLANE_LOG (RIB, "%s: %p.", __func__, msgp);

#if HAVE_LIBURCU_QSBR
  struct rib *new, *old;
  new = malloc (sizeof (struct rib));

  /* change something according to the update instruction message. */
  free (msgp);
  rib = new;

  old = rcu_dereference (rcu_global_ptr_rib);

  rcu_assign_pointer (rcu_global_ptr_rib, new);
  DEBUG_SDPLANE_LOG (RIB, "rcu: rib: assign new: %'llu: %p",
                     rib_rcu_replace, new);

  urcu_qsbr_synchronize_rcu ();
  DEBUG_SDPLANE_LOG (RIB, "rcu: rib: free old: %'llu: %p",
                     rib_rcu_replace, old);
  free (old);
  rib_rcu_replace++;
#endif /*HAVE_LIBURCU_QSBR*/
}

void
rib_manager (void *arg)
{
  int ret;
  void *msgp;
  unsigned lcore_id = rte_lcore_id ();
  uint64_t loop_counter = 0;

  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);
  DEBUG_SDPLANE_LOG (RIB, "%s: started.", __func__);

  /* initialize */
  msg_queue_rib =
    rte_ring_create ("msg_queue_rib", 32, SOCKET_ID_ANY, RING_F_SC_DEQ);

  while (! force_quit && ! force_stop[lthread_core])
    {
      lthread_sleep (100); // yield.
      //DEBUG_SDPLANE_LOG (RIB, "%s: schedule.", __func__);

      ret = rte_ring_dequeue (msg_queue_rib, &msgp);
      if (ret != -ENOENT)
        {
          rib_manager_recv_message (msgp);
        }

      loop_counter++;
    }

  rte_ring_free (msg_queue_rib);

  DEBUG_SDPLANE_LOG (RIB, "%s: terminating.", __func__);
  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);
}
