
#include <stdio.h>

#include <rte_common.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>

#include <zcmdsh/debug.h>
#include <zcmdsh/command.h>

/* should be moved to tap_handler module. */
struct rte_ring *tap_ring_by_lcore[RTE_MAX_LCORE];
__thread struct rte_ring *thread_ring_to_tap;

bool enable_tap_copy = true;

/* also should be moved to tap_handler module? */
void
l2fwd_init_tap_ring ()
{
#define RING_TO_TAP_SIZE 64
  char ring_name[32];
  int lcore_id = rte_lcore_id ();
  snprintf (ring_name, sizeof (ring_name), "ring_to_tap_%d", lcore_id);
  thread_ring_to_tap =
    rte_ring_create (ring_name, RING_TO_TAP_SIZE, rte_socket_id (),
                     (RING_F_SP_ENQ | RING_F_SC_DEQ));
  tap_ring_by_lcore[lcore_id] = thread_ring_to_tap;
}

extern int mac_updating;

void
l2fwd_support_init ()
{
  l2fwd_init_tap_ring ();

  /* MAC updating disabled for sdplane. */
  mac_updating = 0;
}

