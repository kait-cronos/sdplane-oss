#include "include.h"

#include <lthread.h>

#include <rte_common.h>
#include <rte_launch.h>
#include <rte_ether.h>
#include <rte_malloc.h>

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
#include "thread_info.h"

#include "l2fwd_export.h"

#include "internal_message.h"

#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

extern int lthread_core;
extern volatile bool force_stop[RTE_MAX_LCORE];

struct rte_ring *msg_queue_nettlp;

static __thread uint64_t loop_counter = 0;
static __thread struct rib *rib;

void
nettlp_send_dma_write ()
{
  DEBUG_SDPLANE_LOG (NETTLP, "send DMA write.");
}

int
nettlp_thread (void *arg)
{
  int ret;
  void *msgp;
  unsigned lcore_id = rte_lcore_id ();

  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);
  DEBUG_SDPLANE_LOG (NETTLP, "%s: started.", __func__);

  /* initialize */
  msg_queue_nettlp =
    rte_ring_create ("msg_queue_nettlp", 32, SOCKET_ID_ANY, RING_F_SC_DEQ);

  int thread_id;
  thread_id = thread_lookup (nettlp_thread);
  thread_register_loop_counter (thread_id, &loop_counter);

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_register_thread ();
#endif /*HAVE_LIBURCU_QSBR*/

  while (! force_quit && ! force_stop[lthread_core])
    {
      //lthread_sleep (100); // yield.
      //DEBUG_SDPLANE_LOG (NETTLP, "%s: schedule.", __func__);

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_lock ();
      rib = (struct rib *) rcu_dereference (rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

      msgp = internal_msg_recv (msg_queue_nettlp);
      if (msgp)
        {
          nettlp_send_dma_write ();
          internal_msg_delete (msgp);
        }

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_unlock ();
      urcu_qsbr_quiescent_state ();
#endif /*HAVE_LIBURCU_QSBR*/

      loop_counter++;
    }

  rte_ring_free (msg_queue_rib);

  DEBUG_SDPLANE_LOG (NETTLP, "%s: terminating.", __func__);
  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);

#if HAVE_LIBURCU_QSBR
  urcu_qsbr_unregister_thread ();
#endif /*HAVE_LIBURCU_QSBR*/
}

CLI_COMMAND2 (nettlp_send_dma_write,
              "nettlp-send dma-write",
              "NetTLP send command\n",
              "DMA write packet\n"
              )
{
  struct shell *shell = (struct shell *) context;

  void *msgp;
  struct internal_msg_qconf *msg_qconf;

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_QCONF, thread_qconf,
                              sizeof (thread_qconf));
  internal_msg_send_to (msg_queue_nettlp, msgp, shell);
}

void
nettlp_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, nettlp_send_dma_write);
}

