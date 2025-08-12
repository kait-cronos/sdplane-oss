/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef __NEIGH_MANAGER_H__
#define __NEIGH_MANAGER_H__

#include "rib_manager.h"

#define NEIGH_STATE_NONE       0x00
#define NEIGH_STATE_INCOMPLETE 0x01
#define NEIGH_STATE_REACHABLE  0x02
#define NEIGH_STATE_STALE      0x04
#define NEIGH_STATE_DELAY      0x08
#define NEIGH_STATE_PROBE      0x10
#define NEIGH_STATE_FAILED     0x20
#define NEIGH_STATE_NOARP      0x40
#define NEIGH_STATE_PERMANENT  0x80

static inline __attribute__ ((always_inline)) void
arp_copy_to_tap_ring (struct rte_mbuf *m, unsigned portid)
{
  struct rte_mbuf *c;
  uint32_t pkt_len;
  uint16_t data_len;
  int ret;
  struct rte_ring *ring;
  struct rte_ether_hdr *eth_hdr;
  uint16_t etype;

  ring = ring_up[portid][0];
  pkt_len = rte_pktmbuf_pkt_len (m);
  data_len = rte_pktmbuf_data_len (m);
  if (FLAG_CHECK (debug_config, DEBUG_SDPLANE_WIRETAP))
    printf ("%s: m: %p: len: %d/%d from port %d to ring %p\n", __func__, m,
            data_len, pkt_len, portid, ring);
  if (! ring)
    return;
  c = rte_pktmbuf_copy (m, m->pool, 0, UINT32_MAX);
  ret = rte_ring_enqueue (ring, c);
  if (ret)
    {
      if (FLAG_CHECK (debug_config, DEBUG_SDPLANE_WIRETAP))
        printf ("%s: m: %p: rte_ring_enqueue() returned: %d\n", __func__, m,
                ret);
      rte_pktmbuf_free (c);
    }
}

int neigh_manager_lookup (const struct neigh_table *neigh_table,
                          const int index, const void *key,
                          struct neigh_entry *out);
void neigh_manager_show_table (const int index, const struct shell *shell);
void neigh_manager_process_message (void *msgp,
                                    struct neigh_table *neigh_tables);
int neigh_manager (void *arg __rte_unused);

#endif /*__NEIGH_MANAGER_H__*/
