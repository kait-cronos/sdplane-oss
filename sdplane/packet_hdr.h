#ifndef __PACKET_HDR_H__
#define __PACKET_HDR_H__

#include <stdbool.h>
#include <rte_mbuf.h>
#include <rte_ethdev.h>

static inline __attribute__ ((always_inline)) bool
is_rte_vlan_hdr (struct rte_mbuf *m)
{
  struct rte_ether_hdr *eth_hdr;
  uint16_t eth_type;
  eth_hdr = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  eth_type = rte_be_to_cpu_16 (eth_hdr->ether_type);
  if (eth_type == RTE_ETHER_TYPE_VLAN)
    return true;
  return false;
}

static inline __attribute__ ((always_inline)) struct rte_vlan_hdr *
rte_vlan_hdr (struct rte_mbuf *m)
{
  assert (is_rte_vlan_hdr (m));
  struct rte_ether_hdr *eth_hdr;
  eth_hdr = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  return (struct rte_vlan_hdr *) (eth_hdr + 1);
}

static inline __attribute__ ((always_inline)) void
rte_vlan_hdr_set (struct rte_mbuf *m, uint16_t vlan_id)
{
  struct rte_vlan_hdr *vlan_hdr;
  uint16_t old_vlan_tci, new_vlan_tci;
  assert (is_rte_vlan_hdr (m));
  vlan_hdr = rte_vlan_hdr (m);
  old_vlan_tci = rte_be_to_cpu_16 (vlan_hdr->vlan_tci);
  new_vlan_tci =
      ((old_vlan_tci & 0xf000) | (vlan_id & 0x0fff));
  vlan_hdr->vlan_tci = rte_cpu_to_be_16 (new_vlan_tci);
}

#endif /*__PACKET_HDR_H__*/
