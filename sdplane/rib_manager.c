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
#include "thread_info.h"

#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

extern int lthread_core;
extern volatile bool force_stop[RTE_MAX_LCORE];

struct rte_ring *msg_queue_rib;

void *rcu_global_ptr_rib;
uint64_t rib_rcu_replace = 0;

static __thread struct rib *rib;

void
rib_replace (void *new)
{
  void *old;
  old = rcu_dereference (rcu_global_ptr_rib);

  /* assign new */
  rcu_assign_pointer (rcu_global_ptr_rib, new);
  DEBUG_SDPLANE_LOG (RIB, "rib: replace: %'lu-th: %p -> %p",
                     rib_rcu_replace, old, new);

  /* reclaim old */
  urcu_qsbr_synchronize_rcu ();
  free (old);

  rib_rcu_replace++;
}

void
rib_manager_recv_message (void *msgp)
{
  DEBUG_SDPLANE_LOG (RIB, "%s: %p.", __func__, msgp);

#if HAVE_LIBURCU_QSBR
  struct rib *new, *old;

  /* allocate new */
  new = malloc (sizeof (struct rib));
  if (! new)
    return;
  rib = new;

  /* retrieve old */
  old = rcu_dereference (rcu_global_ptr_rib);
  if (old)
    memcpy (new, old, sizeof (struct rib));

  /* change something according to the update instruction message. */
  struct stream_msg_header *msg_header;
  struct stream_msg_eth_link *msg_eth_link;
  struct stream_msg_qconf *msg_qconf;

  msg_header = (struct stream_msg_header *) msgp;
  switch (msg_header->type)
    {
    case STREAM_MSG_TYPE_ETH_LINK:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_eth_link: %p.", msgp);
      msg_eth_link = (struct stream_msg_eth_link *) (msg_header + 1);
      memcpy (new->link, msg_eth_link->link,
              sizeof (struct rte_eth_link) * RTE_MAX_ETHPORTS);
      break;
    case STREAM_MSG_TYPE_QCONF:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_qconf: %p.", msgp);
      msg_qconf = (struct stream_msg_qconf *) (msg_header + 1);
      memcpy (new->qconf, msg_qconf->qconf,
              sizeof (struct sdplane_queue_conf) * RTE_MAX_LCORE);
      break;
    default:
      DEBUG_SDPLANE_LOG (RIB, "recv msg unknown: %p.", msgp);
      break;
    }

  free (msgp);

#if 1
  struct rib *zero;
  zero = malloc (sizeof (struct rib));
  if (zero)
    {
      memset (zero, 0, sizeof (struct rib));
      rib_replace (zero);
    }
#endif

  rib_replace (new);
#endif /*HAVE_LIBURCU_QSBR*/
}

void
rib_manager_send_message (void *msgp, struct shell *shell)
{
  if (msg_queue_rib)
    {
      DEBUG_SDPLANE_LOG (RIB, "%s: sending message %p.", __func__, msgp);
      rte_ring_enqueue (msg_queue_rib, msgp);
    }
  else
    {
      fprintf (shell->terminal, "can't send message to rib: queue: NULL.%s",
               shell->NL);
    }
}

static __thread uint64_t loop_counter = 0;

void
rib_manager (void *arg)
{
  int ret;
  void *msgp;
  unsigned lcore_id = rte_lcore_id ();

  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);
  DEBUG_SDPLANE_LOG (RIB, "%s: started.", __func__);

  /* initialize */
  msg_queue_rib =
    rte_ring_create ("msg_queue_rib", 32, SOCKET_ID_ANY, RING_F_SC_DEQ);

  int thread_id;
  thread_id = thread_lookup (rib_manager);
  thread_register_loop_counter (thread_id, &loop_counter);

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
