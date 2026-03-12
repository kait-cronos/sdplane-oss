#ifndef __TAP_HANDLER_H__
#define __TAP_HANDLER_H__

#include <sdplane/debug.h>
#include <sdplane/command.h>

#include "rib_manager.h"
#include "internal_message.h"

extern int capture_fd;
extern char capture_ifname[64];
extern int capture_if_persistent;

static inline __attribute__ ((always_inline)) void
l2fwd_copy_to_tap_ring (struct rte_mbuf *m, unsigned portid)
{
  struct rte_mbuf *c;
  uint32_t pkt_len;
  uint16_t data_len;
  int ret;
  struct rte_ring *ring;
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

static inline __attribute__ ((always_inline)) bool
is_fdb_send_add (struct rib_info *rib_info,
                 struct rte_mbuf *m, uint16_t vlan_id)
{
  struct rte_ether_hdr *eth_hdr;
  int ret;

  eth_hdr = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  ret = fdb_lookup_entry (rib_info, &eth_hdr->src_addr, vlan_id);
  if (ret < 0)
    return true;
  return false;
}

static inline __attribute__ ((always_inline)) void
send_fdb_entry_add_msg (struct rte_mbuf *m, uint16_t vlan_id)
{
  struct internal_msg_fdb_entry fdb_entry_add;
  struct internal_msg_header *msgp;
  struct rte_ether_hdr *eth_hdr;

  eth_hdr = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  fdb_entry_add.mac_addr = eth_hdr->src_addr;
  fdb_entry_add.port = m->port;
  fdb_entry_add.vlan_id = vlan_id;

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_FDB_ENTRY_ADD, &fdb_entry_add,
                              sizeof (fdb_entry_add));
  rib_send_message (msgp);
}

int tap_handler (__rte_unused void *dummy);
int l3_tap_handler (__rte_unused void *dummy);

#endif /*__L2fWD_SUPPORT_H__*/
