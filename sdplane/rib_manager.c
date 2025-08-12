/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#include "include.h"

#include <lthread.h>

#include <rte_common.h>
#include <rte_launch.h>
#include <rte_ether.h>
#include <rte_malloc.h>

#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>

#include <sdplane/debug.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "rib_manager.h"
#include "sdplane.h"
#include "thread_info.h"
#include "tap.h"

#include "l2fwd_export.h"

#include "internal_message.h"

#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

struct rte_ring *ring_up[RTE_MAX_ETHPORTS][MAX_RX_QUEUE_PER_LCORE];
struct rte_ring *ring_dn[RTE_MAX_ETHPORTS][MAX_RX_QUEUE_PER_LCORE];

struct rte_ring *router_if_ring_up[MAX_VSWITCH_ID];
struct rte_ring *router_if_ring_dn[MAX_VSWITCH_ID];
struct rte_ring *capture_if_ring_up[MAX_VSWITCH_ID];
struct rte_ring *capture_if_ring_dn[MAX_VSWITCH_ID];

struct rte_eth_dev_tx_buffer *tx_buffer_per_q[RTE_MAX_ETHPORTS][RTE_MAX_LCORE];

extern int lthread_core;
extern volatile bool force_stop[RTE_MAX_LCORE];

struct rte_ring *msg_queue_rib;

void *rcu_global_ptr_rib;
uint64_t rib_rcu_replace = 0;

static __thread struct rib *rib = NULL;

static inline __attribute__ ((always_inline)) struct vswitch_conf *
vswitch_new (struct rib_info *new, uint16_t vlan_id)
{
  uint16_t vswitch_id;
  struct vswitch_conf *vswitch;
  bool is_slot_found = false;

  for (vswitch_id = 0; vswitch_id < new->vswitch_size; vswitch_id++)
    {
      if (new->vswitch[vswitch_id].is_deleted)
        {
          is_slot_found = true;
          memset (&new->vswitch[vswitch_id], 0, sizeof (struct vswitch_conf));
          break;
        }
    }

  if (! is_slot_found)
    {
      if (new->vswitch_size >= MAX_VSWITCH_ID)
        return NULL;
      vswitch_id = new->vswitch_size++;
    }

  vswitch = &new->vswitch[vswitch_id];
  vswitch->vswitch_id = vswitch_id;
  vswitch->vlan_id = vlan_id;
  vswitch->router_if.sockfd = -1;
  vswitch->capture_if.sockfd = -1;
  vswitch->router_if.tap_ring_id = -1;
  vswitch->capture_if.tap_ring_id = -1;
  vswitch->is_deleted = false;
  return vswitch;
}

static inline __attribute__ ((always_inline)) struct vswitch_link *
vswitch_link_lookup (struct rib_info *new, struct vswitch_conf *vswitch,
                     struct port_conf *port)
{
  int i;
  struct vswitch_link *vswitch_link;
  for (i = 0; i < new->vswitch_link_size; i++)
    {
      vswitch_link = &new->vswitch_link[i];
      if (vswitch_link->vswitch_id == vswitch->vswitch_id &&
          vswitch_link->port_id == port->dpdk_port_id)
        return vswitch_link;
    }
  return NULL;
}

static inline __attribute__ ((always_inline)) struct vswitch_link *
vswitch_link_new (struct rib_info *new, struct vswitch_conf *vswitch,
                  struct port_conf *port)
{
  uint16_t vswitch_link_id;
  struct vswitch_link *vswitch_link;
  bool is_slot_found = false;

  for (vswitch_link_id = 0; vswitch_link_id < new->vswitch_link_size;
       vswitch_link_id++)
    {
      if (new->vswitch_link[vswitch_link_id].is_deleted)
        {
          is_slot_found = true;
          memset (&new->vswitch_link[vswitch_link_id], 0,
                  sizeof (struct vswitch_link));
          break;
        }
    }

  if (! is_slot_found)
    {
      if (new->vswitch_link_size >= MAX_VSWITCH_LINK)
        return NULL;
      vswitch_link_id = new->vswitch_link_size++;
    }

  vswitch_link = &new->vswitch_link[vswitch_link_id];
  vswitch_link->vswitch_link_id = vswitch_link_id;
  vswitch_link->port_id = port->dpdk_port_id;
  vswitch_link->vlan_id = vswitch->vlan_id;
  vswitch_link->tag_id = vswitch->vlan_id; // tagged.
  vswitch_link->vswitch_id = vswitch->vswitch_id;
  vswitch_link->is_deleted = false;

  uint16_t vswitch_port;
  if (vswitch->vswitch_port_size < MAX_VSWITCH_PORTS)
    {
      vswitch_port = vswitch->vswitch_port_size++;
      vswitch->vswitch_link_id[vswitch_port] = vswitch_link_id;
      vswitch_link->vswitch_port = vswitch_port;
    }

  return vswitch_link;
}

static inline __attribute__ ((always_inline)) int
router_if_delete (struct rib_info *rib_info, uint16_t vswitch_id)
{
  struct vswitch_conf *vswitch = &rib_info->vswitch[vswitch_id];
  struct router_if *rif = &vswitch->router_if;

  if (rif->sockfd >= 0)
    {
      close (rif->sockfd);
      rif->sockfd = -1;
    }

  if (router_if_ring_up[vswitch_id])
    {
      rte_ring_free (router_if_ring_up[vswitch_id]);
      router_if_ring_up[vswitch_id] = NULL;
    }
  if (router_if_ring_dn[vswitch_id])
    {
      rte_ring_free (router_if_ring_dn[vswitch_id]);
      router_if_ring_dn[vswitch_id] = NULL;
    }

  memset (rif, 0, sizeof (struct router_if));
  rif->sockfd = -1;
  rif->tap_ring_id = -1;
  DEBUG_SDPLANE_LOG (RIB, "delete: router_if: vswitch %u", vswitch_id);
  return 0;
}

static inline __attribute__ ((always_inline)) int
capture_if_delete (struct rib_info *rib_info, uint16_t vswitch_id)
{
  struct vswitch_conf *vswitch = &rib_info->vswitch[vswitch_id];
  struct capture_if *cif = &vswitch->capture_if;

  if (cif->sockfd >= 0)
    {
      close (cif->sockfd);
      cif->sockfd = -1;
    }

  if (capture_if_ring_up[vswitch_id])
    {
      rte_ring_free (capture_if_ring_up[vswitch_id]);
      capture_if_ring_up[vswitch_id] = NULL;
    }
  if (capture_if_ring_dn[vswitch_id])
    {
      rte_ring_free (capture_if_ring_dn[vswitch_id]);
      capture_if_ring_dn[vswitch_id] = NULL;
    }

  memset (cif, 0, sizeof (struct capture_if));
  cif->sockfd = -1;
  cif->tap_ring_id = -1;
  DEBUG_SDPLANE_LOG (RIB, "delete: capture_if: vswitch %u", vswitch_id);
  return 0;
}

static inline __attribute__ ((always_inline)) void
vswitch_link_remove_from_port_vlan_array (struct port_conf *port,
                                          uint16_t vswitch_link_id)
{
  int i;
  for (i = 0; i < port->vlan_size; i++)
    {
      if (port->vswitch_link_id_of_vlan[i] == vswitch_link_id)
        {
          memmove (&port->vswitch_link_id_of_vlan[i],
                   &port->vswitch_link_id_of_vlan[i + 1],
                   (port->vlan_size - i - 1) * sizeof (uint16_t));
          port->vlan_size--;
          break;
        }
      else if (port->vswitch_link_id_of_native_vlan == vswitch_link_id)
        {
          port->vswitch_link_id_of_native_vlan = -1;
          break;
        }
    }
}

static inline __attribute__ ((always_inline)) void
vswitch_link_remove_from_vswitch_port_array (struct vswitch_conf *vswitch,
                                             uint16_t vswitch_link_id)
{
  int i;
  for (i = 0; i < vswitch->vswitch_port_size; i++)
    {
      if (vswitch->vswitch_link_id[i] == vswitch_link_id)
        {
          memmove (&vswitch->vswitch_link_id[i],
                   &vswitch->vswitch_link_id[i + 1],
                   (vswitch->vswitch_port_size - i - 1) * sizeof (uint16_t));
          vswitch->vswitch_port_size--;
        }
    }
}

static inline __attribute__ ((always_inline)) int
vswitch_link_delete (struct rib_info *rib_info, uint16_t vswitch_link_id)
{
  int i;
  uint16_t vswitch_id = rib_info->vswitch_link[vswitch_link_id].vswitch_id;
  uint16_t port_id = rib_info->vswitch_link[vswitch_link_id].port_id;

  rib_info->vswitch_link[vswitch_link_id].is_deleted = true;

  vswitch_link_remove_from_vswitch_port_array (&rib_info->vswitch[vswitch_id],
                                               vswitch_link_id);

  vswitch_link_remove_from_port_vlan_array (&rib_info->port[port_id],
                                            vswitch_link_id);

  for (i = vswitch_link_id; i < rib_info->vswitch_link_size; i++)
    {
      if (rib_info->vswitch_link[i].vswitch_id == vswitch_id)
        rib_info->vswitch_link[i].vswitch_port--;
    }

  DEBUG_SDPLANE_LOG (RIB, "delete: link_id: %u", vswitch_link_id);
  return 0;
}

static inline __attribute__ ((always_inline)) int
vswitch_delete (struct rib_info *rib_info, uint16_t vswitch_id)
{
  rib_info->vswitch[vswitch_id].is_deleted = true;

  /* delete vswitch's router_if and capture_if */
  router_if_delete (rib_info, vswitch_id);
  capture_if_delete (rib_info, vswitch_id);

  /* delete all vswitch's vswitch_links from port_conf */
  while (rib_info->vswitch[vswitch_id].vswitch_port_size > 0)
    vswitch_link_delete (rib_info,
                         rib_info->vswitch[vswitch_id].vswitch_link_id[0]);

  DEBUG_SDPLANE_LOG (RIB, "delete: vswitch: %u", vswitch_id);
  return 0;
}

static inline __attribute__ ((always_inline)) void
port_set_native_vlan (struct rib_info *new, struct port_conf *port,
                      struct vswitch_link *vswitch_link)
{
  vswitch_link->tag_id = 0;
  port->vswitch_link_id_of_native_vlan = vswitch_link->vswitch_link_id;
}

static inline __attribute__ ((always_inline)) void
port_add_tagged_vlan (struct rib_info *new, struct port_conf *port,
                      struct vswitch_link *vswitch_link)
{
  if (port->vlan_size >= MAX_VLAN_PER_PORT)
    return;
  uint16_t index = port->vlan_size++;
  port->vswitch_link_id_of_vlan[index] = vswitch_link->vswitch_link_id;
}

#if 0
static inline __attribute__ ((always_inline)) void
rib_info_hard_coding (struct rib_info *new)
{
  uint8_t default_vlan_vswitch_id = 0;
  struct vswitch_conf *default_vlan_vswitch;

  uint16_t port_id;
  struct port_conf *port;
  uint16_t vswitch_link_id = 0;
  struct vswitch_link *vswitch_link;

  /* default vlan vswitch check */
  /* default_vlan_vswitch_id is always 0. */
  default_vlan_vswitch_id = 0;
  if (new->vswitch_size == 0)
    default_vlan_vswitch = vswitch_new (new, default_vlan_vswitch_id);
  else
    default_vlan_vswitch = &new->vswitch[default_vlan_vswitch_id];

  new->port_size = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < new->port_size; port_id++)
    {
      port = &new->port[port_id];
      port->dpdk_port_id = port_id;

      DEBUG_SDPLANE_LOG (RIB, "port_id: %d vswitch_link_id_of_native_vlan: %d",
                         port_id, port->vswitch_link_id_of_native_vlan);

      /* native vlan check. */
      vswitch_link_id = port->vswitch_link_id_of_native_vlan;
      DEBUG_SDPLANE_LOG (RIB, "vswitch[%d]: port_id: %d == port_id: %d",
                         vswitch_link_id,
                         new->vswitch_link[vswitch_link_id].port_id, port_id);
      if (new->vswitch_link_size == 0 ||
          new->vswitch_link[vswitch_link_id].port_id != port_id)
        {
          uint16_t vswitch_port_id;

          /* create a new vswitch_link */
          vswitch_link = vswitch_link_new (new, default_vlan_vswitch, port);

          /* connect the vswitch_link to the dpdk port. */
          port_set_native_vlan (new, port, vswitch_link);
        }
    }

#if 0
  int vswitch_id_2337 = -1;
  int vswitch_id_2410 = -1;
  int vswitch_id_2411 = -1;
  int i;
  for (i = 0; i < new->vswitch_size; i++)
    {
      struct vswitch_conf *vswitch = &new->vswitch[i];
      if (vswitch->vlan_id == 2337)
        vswitch_id_2337 = i;
      else if (vswitch->vlan_id == 2410)
        vswitch_id_2410 = i;
      else if (vswitch->vlan_id == 2411)
        vswitch_id_2411 = i;
    }

  struct vswitch_conf *vlan_2337 = NULL;
  struct vswitch_conf *vlan_2410 = NULL;
  struct vswitch_conf *vlan_2411 = NULL;
  if (vswitch_id_2337 >= 0)
    vlan_2337 = &new->vswitch[vswitch_id_2337];
  if (vswitch_id_2410 >= 0)
    vlan_2410 = &new->vswitch[vswitch_id_2410];
  if (vswitch_id_2411 >= 0)
    vlan_2411 = &new->vswitch[vswitch_id_2411];
  struct port_conf *port_0 = &new->port[0];
  struct port_conf *port_1 = &new->port[1];
  struct port_conf *port_2 = &new->port[2];
  struct port_conf *port_3 = &new->port[3];

  if (! vlan_2337)
    {
      vlan_2337 = vswitch_new (new, 2337);
      vswitch_link = vswitch_link_new (new, vlan_2337, port_0);
      port_add_tagged_vlan (new, port_0, vswitch_link);
      vswitch_link = vswitch_link_new (new, vlan_2337, port_1);
      port_add_tagged_vlan (new, port_1, vswitch_link);
    }

  if (! vlan_2410) // BBIX
    {
      vlan_2410 = vswitch_new (new, 2410);
      vswitch_link = vswitch_link_new (new, vlan_2410, port_0);
      port_add_tagged_vlan (new, port_0, vswitch_link);
      vswitch_link = vswitch_link_new (new, vlan_2410, port_1);
      port_add_tagged_vlan (new, port_1, vswitch_link);
    }


  if (! vlan_2411)
    {
      vlan_2411 = vswitch_new (new, 2411);
      vswitch_link = vswitch_link_new (new, vlan_2411, port_0);
      port_add_tagged_vlan (new, port_0, vswitch_link);
      vswitch_link = vswitch_link_new (new, vlan_2411, port_1);
      //port_add_tagged_vlan (new, port_1, vswitch_link);
      port_set_native_vlan (new, port_1, vswitch_link);
    }
#endif
}
#endif

static inline __attribute__ ((always_inline)) struct rib_info *
rib_info_create (struct rib_info *old)
{
  struct rib_info *new;

  /* allocate new */
  new = malloc (sizeof (struct rib_info));
  if (! new)
    return NULL;

  if (! old)
    memset (new, 0, sizeof (struct rib_info));
  else
    memcpy (new, old, sizeof (struct rib_info));

#if 0
  /* XXX hard-coding part. */
  rib_info_hard_coding (new);
#endif

  new->ver++;
  return new;
}

static inline __attribute__ ((always_inline)) void
rib_info_delete (struct rib_info *old)
{
  free (old);
}

static inline __attribute__ ((always_inline)) struct rib *
rib_create (struct rib *old)
{
  struct rib *new;

  /* allocate new */
  new = malloc (sizeof (struct rib));
  if (! new)
    return NULL;

  if (! old)
    {
      memset (new, 0, sizeof (struct rib));
      new->rib_info = rib_info_create (NULL);
    }
  else
    {
      memcpy (new, old, sizeof (struct rib));
      new->rib_info = rib_info_create (old->rib_info);
    }

  return new;
}

static inline __attribute__ ((always_inline)) void
rib_delete (struct rib *old)
{
  if (old->rib_info)
    {
      rib_info_delete (old->rib_info);
      old->rib_info = NULL;
    }
  free (old);
}

int
port_qconf_compare (const void *a, const void *b)
{
  struct port_queue_conf *pa = (struct port_queue_conf *) a;
  struct port_queue_conf *pb = (struct port_queue_conf *) b;
  if (pa->port_id < pb->port_id)
    return -1;
  else if (pa->port_id > pb->port_id)
    return 1;
  else if (pa->queue_id < pb->queue_id)
    return -1;
  else if (pa->queue_id > pb->queue_id)
    return 1;
  return 0;
}

static inline __attribute__ ((always_inline)) int
rib_check (struct rib *new)
{
  struct sdplane_queue_conf *qconf;
  struct lcore_qconf *lcore_qconf;
  int lcore;
  int i, ret;
  char ring_name[32];
  int j;

  DEBUG_SDPLANE_LOG (RIB, "ver: %d rib: %p rib_info: %p.", new->rib_info->ver,
                     new, new->rib_info);

  struct rte_eth_rxconf rxq_conf;
  struct rte_eth_txconf txq_conf;

#define RX_DESC_DEFAULT 1024
#define TX_DESC_DEFAULT 1024
  uint16_t nb_rxd = RX_DESC_DEFAULT;
  uint16_t nb_txd = TX_DESC_DEFAULT;

  for (lcore = 0; lcore < RTE_MAX_LCORE; lcore++)
    {
      lcore_qconf = &new->rib_info->lcore_qconf[lcore];
      for (i = 0; i < lcore_qconf->nrxq; i++)
        {
          struct port_queue_conf *rxq;
          rxq = &lcore_qconf->rx_queue_list[i];

          DEBUG_SDPLANE_LOG (
              RIB, "new rib: lcore: %d qconf[%d]: port: %d queue: %d", lcore,
              i, rxq->port_id, rxq->queue_id);
        }
    }

#define MAX_PORT_QCONF 256
  struct port_queue_conf port_qconf[MAX_PORT_QCONF];
  int port_qconf_size = 0;
  memset (port_qconf, 0, sizeof (port_qconf));

  /* check port's #rxq/#txq */
  int max_lcore = 0;
  for (lcore = 0; lcore < new->rib_info->lcore_size; lcore++)
    {
      lcore_qconf = &new->rib_info->lcore_qconf[lcore];
      for (i = 0; i < lcore_qconf->nrxq; i++)
        {
          struct port_queue_conf *rxq;
          rxq = &lcore_qconf->rx_queue_list[i];

          if (max_lcore < lcore)
            max_lcore = lcore;

          memcpy (&port_qconf[port_qconf_size], rxq,
                  sizeof (struct port_queue_conf));
          port_qconf_size++;
        }
    }

  qsort (port_qconf, port_qconf_size, sizeof (struct port_queue_conf),
         port_qconf_compare);

  int port_nrxq[RTE_MAX_ETHPORTS];
  memset (port_nrxq, 0, sizeof (port_nrxq));

  for (i = 0; i < port_qconf_size; i++)
    {
      struct port_queue_conf *rxq;
      rxq = &port_qconf[i];
      DEBUG_SDPLANE_LOG (RIB, "port_qconf[%d]: port: %d queue: %d", i,
                         rxq->port_id, rxq->queue_id);
      if (port_nrxq[rxq->port_id] == rxq->queue_id)
        {
          port_nrxq[rxq->port_id]++;
        }
      else
        {
          DEBUG_SDPLANE_LOG (RIB,
                             "unorderd port_qconf[%d]: port: %d queue: %d", i,
                             rxq->port_id, rxq->queue_id);
          return -1;
        }
    }

  struct rte_eth_conf port_conf =
    { .txmode = { .mq_mode = RTE_ETH_MQ_TX_NONE, },
      .intr_conf = { .lsc = 1 },
    };
  struct rte_eth_dev_info dev_info;

  int ntxq;
  ntxq = max_lcore + 1;
  DEBUG_SDPLANE_LOG (RIB, "max_lcore: %d, ntxq: %d", max_lcore, ntxq);
  int nb_ports;
  nb_ports = rte_eth_dev_count_avail ();
  if (nb_ports > MAX_ETH_PORTS)
    nb_ports = MAX_ETH_PORTS;
  for (i = 0; i < nb_ports; i++)
    {
      int nrxq;
      nrxq = port_nrxq[i];
      ret = rte_eth_dev_info_get (i, &dev_info);
      if (dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE)
        port_conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE;
      else
        port_conf.txmode.offloads &= (~RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE);
      DEBUG_SDPLANE_LOG (RIB, "port[%d]: dev_configure: nrxq: %d ntxq: %d", i,
                         nrxq, ntxq);
      ret = rte_eth_dev_stop (i);
      ret = rte_eth_dev_configure (i, nrxq, ntxq, &port_conf);

      nb_rxd = RX_DESC_DEFAULT;
      if (new->rib_info->port[i].nb_rxd)
        nb_rxd = new->rib_info->port[i].nb_rxd;
      nb_txd = TX_DESC_DEFAULT;
      if (new->rib_info->port[i].nb_txd)
        nb_txd = new->rib_info->port[i].nb_txd;

      rxq_conf = dev_info.default_rxconf;
      rxq_conf.offloads = port_conf.rxmode.offloads;

      for (j = 0; j < nrxq; j++)
        {
          ret =
              rte_eth_rx_queue_setup (i, j, nb_rxd, rte_eth_dev_socket_id (i),
                                      &rxq_conf, l2fwd_pktmbuf_pool);
          DEBUG_SDPLANE_LOG (RIB, "port[%d]: rx_queue_setup: rxq: %d rxd: %d",
                             i, j, nb_rxd);
        }

      txq_conf = dev_info.default_txconf;
      txq_conf.offloads = port_conf.txmode.offloads;

      for (j = 0; j < ntxq; j++)
        {
          ret = rte_eth_tx_queue_setup (i, j, nb_txd,
                                        rte_eth_dev_socket_id (i), &txq_conf);
          DEBUG_SDPLANE_LOG (RIB, "port[%d]: tx_queue_setup: txq: %d txd: %d",
                             i, j, nb_txd);

          if (! tx_buffer_per_q[i][j])
            {
              DEBUG_SDPLANE_LOG (L2_REPEATER,
                                 "tx_buffer_init: port: %d queue: %d", i, j);
              tx_buffer_per_q[i][j] = rte_zmalloc_socket (
                  "tx_buffer", RTE_ETH_TX_BUFFER_SIZE (MAX_PKT_BURST), 0,
                  rte_eth_dev_socket_id (i));
              rte_eth_tx_buffer_init (tx_buffer_per_q[i][j], MAX_PKT_BURST);
            }
        }

      ret = rte_eth_dev_start (i);
    }

    /* prepare rte_ring "ring_up/dn[][]" */
#define RING_TO_TAP_SIZE 64
  for (lcore = 0; lcore < RTE_MAX_LCORE; lcore++)
    {
      lcore_qconf = &new->rib_info->lcore_qconf[lcore];
      for (i = 0; i < lcore_qconf->nrxq; i++)
        {
          struct port_queue_conf *rxq;
          rxq = &lcore_qconf->rx_queue_list[i];

          if (! ring_up[rxq->port_id][rxq->queue_id])
            {
              snprintf (ring_name, sizeof (ring_name), "ring_up[%d][%d]",
                        rxq->port_id, rxq->queue_id);
              ring_up[rxq->port_id][rxq->queue_id] = rte_ring_create (
                  ring_name, RING_TO_TAP_SIZE, rte_socket_id (),
                  (RING_F_SP_ENQ | RING_F_SC_DEQ));
              DEBUG_SDPLANE_LOG (RIB, "rib: create: %s: %p", ring_name,
                                 ring_up[rxq->port_id][rxq->queue_id]);
            }

          if (! ring_dn[rxq->port_id][rxq->queue_id])
            {
              snprintf (ring_name, sizeof (ring_name), "ring_dn[%d][%d]",
                        rxq->port_id, rxq->queue_id);
              ring_dn[rxq->port_id][rxq->queue_id] = rte_ring_create (
                  ring_name, RING_TO_TAP_SIZE, rte_socket_id (),
                  (RING_F_SP_ENQ | RING_F_SC_DEQ));
              DEBUG_SDPLANE_LOG (RIB, "rib: create: %s: %p", ring_name,
                                 ring_dn[rxq->port_id][rxq->queue_id]);
            }
        }
    }

  for (i = 0; i < new->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vswitch = &new->rib_info->vswitch[i];

      if (vswitch->vlan_id == 0)
        continue;

      if (vswitch->router_if.sockfd >= 0)
        {
          if (! router_if_ring_up[i])
            {
              snprintf (ring_name, sizeof (ring_name), "router_up[%d]", i);
              router_if_ring_up[i] = rte_ring_create (
                  ring_name, RING_TO_TAP_SIZE, rte_socket_id (),
                  RING_F_SP_ENQ | RING_F_SC_DEQ);
              DEBUG_SDPLANE_LOG (RIB, "rib: create: %s: %p", ring_name,
                                 router_if_ring_up[i]);
            }
          if (! router_if_ring_dn[i])
            {
              snprintf (ring_name, sizeof (ring_name), "router_dn[%d]", i);
              router_if_ring_dn[i] = rte_ring_create (
                  ring_name, RING_TO_TAP_SIZE, rte_socket_id (),
                  RING_F_SP_ENQ | RING_F_SC_DEQ);
              DEBUG_SDPLANE_LOG (RIB, "rib: create: %s: %p", ring_name,
                                 router_if_ring_dn[i]);
            }

          vswitch->router_if.ring_up = router_if_ring_up[i];
          vswitch->router_if.ring_dn = router_if_ring_dn[i];
        }

      if (vswitch->capture_if.sockfd >= 0)
        {
          if (! capture_if_ring_up[i])
            {
              snprintf (ring_name, sizeof (ring_name), "capture_up[%d]", i);
              capture_if_ring_up[i] = rte_ring_create (
                  ring_name, RING_TO_TAP_SIZE, rte_socket_id (),
                  RING_F_SP_ENQ | RING_F_SC_DEQ);
              DEBUG_SDPLANE_LOG (RIB, "rib: create: %s: %p", ring_name,
                                 capture_if_ring_up[i]);
            }
          if (! capture_if_ring_dn[i])
            {
              snprintf (ring_name, sizeof (ring_name), "capture_dn[%d]", i);
              capture_if_ring_dn[i] = rte_ring_create (
                  ring_name, RING_TO_TAP_SIZE, rte_socket_id (),
                  RING_F_SP_ENQ | RING_F_SC_DEQ);
              DEBUG_SDPLANE_LOG (RIB, "rib: create: %s: %p", ring_name,
                                 capture_if_ring_dn[i]);
            }

          vswitch->capture_if.ring_up = capture_if_ring_up[i];
          vswitch->capture_if.ring_dn = capture_if_ring_dn[i];
        }
    }

  return 0;
}

static inline __attribute__ ((always_inline)) void
rib_replace (struct rib *new)
{
  struct rib *old;
  old = rcu_dereference (rcu_global_ptr_rib);

  /* assign new */
  rcu_assign_pointer (rcu_global_ptr_rib, new);
  DEBUG_SDPLANE_LOG (RIB,
                     "rib: replace: %'lu-th: "
                     "rib: %p -> %p "
                     "rib_info: ver.%d (%p) -> ver.%d (%p)",
                     rib_rcu_replace, old, new,
                     (old && old->rib_info ? old->rib_info->ver : -1),
                     (old ? old->rib_info : NULL),
                     (new &&new->rib_info ? new->rib_info->ver : -1),
                     (new ? new->rib_info : NULL));

  /* reclaim old */
  if (old)
    {
      urcu_qsbr_synchronize_rcu ();
      rib_delete (old);
    }

  rib_rcu_replace++;
}

void
update_port_status (struct rib *new)
{
  uint16_t nb_ports, port_id;
  DEBUG_SDPLANE_LOG (RIB, "update port status: ver: %d rib: %p rib_info: %p.",
                     new->rib_info->ver, new, new->rib_info);
  nb_ports = rte_eth_dev_count_avail ();
  new->rib_info->port_size = nb_ports;
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      rte_eth_dev_info_get (port_id, &new->rib_info->port[port_id].dev_info);
      rte_eth_link_get_nowait (port_id, &new->rib_info->port[port_id].link);
      DEBUG_SDPLANE_LOG (RIB, "port: %d link: %d ver: %d rib_info: %p.",
                         port_id,
                         new->rib_info->port[port_id].link.link_status,
                         new->rib_info->ver, new->rib_info);
    }

  uint16_t lcore_size;
  lcore_size = rte_lcore_count ();
  new->rib_info->lcore_size = lcore_size;
}

void
set_stop_flag (struct rib *old)
{
  uint16_t nb_ports, port_id;
  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      old->rib_info->port[port_id].is_stopped = true;
      DEBUG_SDPLANE_LOG (RIB, "port[%d]: is_stopped: %d", port_id,
                         old->rib_info->port[port_id].is_stopped);
    }
}

void
delete_stop_flag (struct rib *old)
{
  uint16_t nb_ports, port_id;
  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      old->rib_info->port[port_id].is_stopped = false;
      DEBUG_SDPLANE_LOG (RIB, "port[%d]: is_stopped: %d", port_id,
                         old->rib_info->port[port_id].is_stopped);
    }
}

void
rib_manager_process_message (void *msgp)
{
  int ret;
  int i, j;
  DEBUG_SDPLANE_LOG (RIB, "%s: msg: %p.", __func__, msgp);

#if HAVE_LIBURCU_QSBR
  struct rib *new, *old;

  /* retrieve old */
  old = rcu_dereference (rcu_global_ptr_rib);

  new = rib_create (old);

  /* change something according to the update instruction message. */
  struct internal_msg_header *msg_header;
  struct internal_msg_eth_link *msg_eth_link;
  struct internal_msg_qconf *msg_qconf;
  struct internal_msg_neigh_entry *msg_neigh_entry;

  msg_header = (struct internal_msg_header *) msgp;
  switch (msg_header->type)
    {
    case INTERNAL_MSG_TYPE_PORT_STATUS:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_port_status: %p.", msgp);
      update_port_status (new);
      break;

#if 0
    case INTERNAL_MSG_TYPE_ETH_LINK:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_eth_link: %p.", msgp);
      /* this message is functionally substituted by the above
         update_port_status(). */
      break;
#endif

    case INTERNAL_MSG_TYPE_QCONF:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_qconf: %p.", msgp);
      msg_qconf = (struct internal_msg_qconf *) (msg_header + 1);
      uint16_t lcore_size;
      lcore_size = rte_lcore_count ();
      new->rib_info->lcore_size = lcore_size;
      for (i = 0; i < lcore_size; i++)
        {
          if (msg_qconf->qconf[i].nrxq)
            {
              new->rib_info->lcore_qconf[i].nrxq = msg_qconf->qconf[i].nrxq;
              for (j = 0; j < msg_qconf->qconf[i].nrxq; j++)
                {
                  char *src, *dst;
                  int len;
                  dst =
                      (char *) &new->rib_info->lcore_qconf[i].rx_queue_list[j];
                  src = (char *) &msg_qconf->qconf[i].rx_queue_list[j];
                  len =
                      sizeof (new->rib_info->lcore_qconf[i].rx_queue_list[j]);
                  memcpy (dst, src, len);
                }
            }
        }

      /* for qconf change, we need strict rib_check(). */
      set_stop_flag (old);
      ret = rib_check (new);
      if (ret < 0)
        {
          DEBUG_SDPLANE_LOG (RIB, "rib_check() failed: return.");
          return;
        }
      delete_stop_flag (old);

      /* for qconf change, we need an intermittent state to avoid
         a conflict between different cores. */
      /* XXX, we can use smarter intermittent state. */
      struct rib *zero;
      zero = malloc (sizeof (struct rib));
      if (zero)
        {
          memset (zero, 0, sizeof (struct rib));
          rib_replace (zero);
        }
      break;

    case INTERNAL_MSG_TYPE_TXRX_DESC:
      struct internal_msg_txrx_desc *msg_txrx_desc;
      int portid;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_txrx_desc: %p.", msgp);
      msg_txrx_desc = (struct internal_msg_txrx_desc *) (msg_header + 1);
      portid = msg_txrx_desc->portid;
      new->rib_info->port[portid].nb_rxd = msg_txrx_desc->nb_rxd;
      new->rib_info->port[portid].nb_txd = msg_txrx_desc->nb_txd;
      break;

    case INTERNAL_MSG_TYPE_VSWITCH_CREATE:
      struct internal_msg_vswitch_create *msg_vswitch_create;
      struct vswitch_conf *new_vswitch;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_vswitch_create: %p.", msgp);
      msg_vswitch_create =
          (struct internal_msg_vswitch_create *) (msg_header + 1);
      new_vswitch = vswitch_new (new->rib_info, msg_vswitch_create->vlan_id);
      if (new_vswitch)
        DEBUG_SDPLANE_LOG (RIB, "create: vswitch: %u vlan_id: %u",
                           new_vswitch->vswitch_id, new_vswitch->vlan_id);
      break;

    case INTERNAL_MSG_TYPE_VSWITCH_DELETE:
      struct internal_msg_vswitch_delete *msg_vswitch_delete;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_vswitch_delete: %p.", msgp);
      msg_vswitch_delete =
          (struct internal_msg_vswitch_delete *) (msg_header + 1);
      if (msg_vswitch_delete->vswitch_id >= new->rib_info->vswitch_size)
        break;

      vswitch_delete (new->rib_info, msg_vswitch_delete->vswitch_id);
      break;

    case INTERNAL_MSG_TYPE_VSWITCH_LINK_CREATE:
      struct internal_msg_vswitch_link_create *msg_vswitch_link_create;
      struct vswitch_conf *target_vswitch_link;
      struct port_conf *target_port;
      struct vswitch_link *new_link;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_vswitch_link_create: %p.", msgp);
      msg_vswitch_link_create =
          (struct internal_msg_vswitch_link_create *) (msg_header + 1);
      if (msg_vswitch_link_create->vswitch_id >= new->rib_info->vswitch_size ||
          msg_vswitch_link_create->port_id >= new->rib_info->port_size)
        break;

      target_vswitch_link =
          &new->rib_info->vswitch[msg_vswitch_link_create->vswitch_id];
      target_port = &new->rib_info->port[msg_vswitch_link_create->port_id];

      new_link =
          vswitch_link_new (new->rib_info, target_vswitch_link, target_port);
      if (! new_link)
        break;

      new_link->port_id = msg_vswitch_link_create->port_id;
      new_link->tag_id = msg_vswitch_link_create->tag_id;
      if (msg_vswitch_link_create->tag_id == 0)
        {
          port_set_native_vlan (new->rib_info, target_port, new_link);
          DEBUG_SDPLANE_LOG (
              RIB, "create: link_id: %u vswitch: %u port: %u native",
              new_link->vswitch_link_id, msg_vswitch_link_create->vswitch_id,
              msg_vswitch_link_create->port_id);
        }
      else
        {
          port_add_tagged_vlan (new->rib_info, target_port, new_link);
          DEBUG_SDPLANE_LOG (
              RIB, "create: link_id: %u vswitch: %u port: %u tag: %u",
              new_link->vswitch_link_id, msg_vswitch_link_create->vswitch_id,
              msg_vswitch_link_create->port_id,
              msg_vswitch_link_create->tag_id);
        }
      break;

    case INTERNAL_MSG_TYPE_VSWITCH_LINK_DELETE:
      struct internal_msg_vswitch_link_delete *msg_vswitch_link_delete;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_vswitch_link_delete: %p.", msgp);
      msg_vswitch_link_delete =
          (struct internal_msg_vswitch_link_delete *) (msg_header + 1);
      if (msg_vswitch_link_delete->vswitch_link_id >=
          new->rib_info->vswitch_link_size)
        break;

      vswitch_link_delete (new->rib_info,
                           msg_vswitch_link_delete->vswitch_link_id);
      break;

    case INTERNAL_MSG_TYPE_ROUTER_IF_CREATE:
      struct internal_msg_router_if_create *msg_router_if_create;
      struct vswitch_conf *vswitch_router_if;
      struct router_if *rif;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_router_if_create: %p.", msgp);
      msg_router_if_create =
          (struct internal_msg_router_if_create *) (msg_header + 1);
      if (msg_router_if_create->vswitch_id >= new->rib_info->vswitch_size)
        break;

      vswitch_router_if =
          &new->rib_info->vswitch[msg_router_if_create->vswitch_id];
      rif = &vswitch_router_if->router_if;

      rif->sockfd = tap_open (msg_router_if_create->tap_name);
      rif->tap_ring_id = msg_router_if_create->vswitch_id;
      tap_admin_up (msg_router_if_create->tap_name);

      DEBUG_SDPLANE_LOG (RIB, "create: router_if: %s vswitch %u",
                         msg_router_if_create->tap_name,
                         msg_router_if_create->vswitch_id);

      // set router_if ring
      set_stop_flag (old);
      ret = rib_check (new);
      if (ret < 0)
        {
          DEBUG_SDPLANE_LOG (RIB, "rib_check() failed: return.");
          return;
        }
      delete_stop_flag (old);

      break;

    case INTERNAL_MSG_TYPE_ROUTER_IF_DELETE:
      struct internal_msg_router_if_delete *msg_router_if_delete;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_router_if_delete: %p.", msgp);
      msg_router_if_delete =
          (struct internal_msg_router_if_delete *) (msg_header + 1);
      if (msg_router_if_delete->vswitch_id >= new->rib_info->vswitch_size)
        break;

      router_if_delete (new->rib_info, msg_router_if_delete->vswitch_id);
      break;

    case INTERNAL_MSG_TYPE_CAPTURE_IF_CREATE:
      struct internal_msg_capture_if_create *msg_capture_if_create;
      struct vswitch_conf *vswitch_capture_if;
      struct capture_if *cif;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_capture_if_create: %p.", msgp);
      msg_capture_if_create =
          (struct internal_msg_capture_if_create *) (msg_header + 1);
      if (msg_capture_if_create->vswitch_id >= new->rib_info->vswitch_size)
        break;

      vswitch_capture_if =
          &new->rib_info->vswitch[msg_capture_if_create->vswitch_id];
      cif = &vswitch_capture_if->capture_if;

      cif->sockfd = tap_open (msg_capture_if_create->tap_name);
      cif->tap_ring_id = msg_capture_if_create->vswitch_id;
      tap_admin_up (msg_capture_if_create->tap_name);

      DEBUG_SDPLANE_LOG (RIB, "create: capture_if: %s vswitch: %u",
                         msg_capture_if_create->tap_name,
                         msg_capture_if_create->vswitch_id);

      // set capture_if ring
      set_stop_flag (old);
      ret = rib_check (new);
      if (ret < 0)
        {
          DEBUG_SDPLANE_LOG (RIB, "rib_check() failed: return.");
          return;
        }
      delete_stop_flag (old);

      break;

    case INTERNAL_MSG_TYPE_CAPTURE_IF_DELETE:
      struct internal_msg_capture_if_delete *msg_capture_if_delete;
      DEBUG_SDPLANE_LOG (RIB, "recv msg_capture_if_delete: %p.", msgp);
      msg_capture_if_delete =
          (struct internal_msg_capture_if_delete *) (msg_header + 1);
      if (msg_capture_if_delete->vswitch_id >= new->rib_info->vswitch_size)
        break;

      capture_if_delete (new->rib_info, msg_capture_if_delete->vswitch_id);
      break;

    case INTERNAL_MSG_TYPE_NEIGH_ENTRY_ADD:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_neigh_entry_add: %p.", msgp);
      msg_neigh_entry = (struct internal_msg_neigh_entry *) (msg_header + 1);
      memcpy (&new->rib_info->neigh_tables[msg_neigh_entry->index]
                   .entries[msg_neigh_entry->hash],
              &msg_neigh_entry->data, sizeof (struct neigh_entry));
      break;
    case INTERNAL_MSG_TYPE_NEIGH_ENTRY_DEL:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_neigh_entry_del: %p.", msgp);
      msg_neigh_entry = (struct internal_msg_neigh_entry *) (msg_header + 1);
      memset (&new->rib_info->neigh_tables[msg_neigh_entry->index]
                   .entries[msg_neigh_entry->hash],
              0, sizeof (struct neigh_entry));
      break;

    default:
      DEBUG_SDPLANE_LOG (RIB, "recv msg unknown: %p.", msgp);
      break;
    }

  free (msgp);

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

  /* the tx_buffer_per_q is initialized in rib_manager. */
  memset (tx_buffer_per_q, 0, sizeof (tx_buffer_per_q));

  /* initialize */
  msg_queue_rib =
      rte_ring_create ("msg_queue_rib", 32, SOCKET_ID_ANY, RING_F_SC_DEQ);

  int thread_id;
  thread_id = thread_lookup (rib_manager);
  thread_register_loop_counter (thread_id, &loop_counter);

  while (! force_quit && ! force_stop[lthread_core])
    {
      lthread_sleep (100); // yield.
      // DEBUG_SDPLANE_LOG (RIB, "%s: schedule.", __func__);

      msgp = internal_msg_recv (msg_queue_rib);
      if (msgp)
        rib_manager_process_message (msgp);

      loop_counter++;
    }

  rte_ring_free (msg_queue_rib);

  DEBUG_SDPLANE_LOG (RIB, "%s: terminating.", __func__);
  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);
}
