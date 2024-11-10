#ifndef __TAP_HANDLER_H__
#define __TAP_HANDLER_H__

#include <zcmdsh/debug.h>
#include <zcmdsh/command.h>

#include "sdplane.h"

//extern struct rte_ring *tap_ring_by_lcore[RTE_MAX_LCORE];
extern __thread struct rte_ring *thread_ring_to_tap;

extern bool enable_tap_copy;

static inline __attribute__((always_inline)) void
l2fwd_copy_to_tap_ring (struct rte_mbuf *m, unsigned portid)
{
  struct rte_mbuf *c;
  uint32_t pkt_len;
  uint16_t data_len;
  int ret;
  pkt_len = rte_pktmbuf_pkt_len (m);
  data_len = rte_pktmbuf_data_len (m);
  if (FLAG_CHECK (debug_config, DEBUG_SDPLANE_WIRETAP))
    printf ("%s: m: %p: len: %d/%d from port %d to ring %p\n",
            __func__, m, data_len, pkt_len, portid, thread_ring_to_tap);
  if (! thread_ring_to_tap)
    return;
  c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
  ret = rte_ring_enqueue (thread_ring_to_tap, c);
  if (ret)
    {
      if (FLAG_CHECK (debug_config, DEBUG_SDPLANE_WIRETAP))
        printf ("%s: m: %p: rte_ring_enqueue() returned: %d\n",
                __func__, m, ret);
      rte_pktmbuf_free (c);
    }
}

void per_thread_tap_ring_init ();

int tap_handler (__rte_unused void *dummy);

#endif /*__L2fWD_SUPPORT_H__*/
