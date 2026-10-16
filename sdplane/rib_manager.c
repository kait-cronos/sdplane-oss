#include "include.h"

#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/rtnetlink.h>

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
#include "fdb.h"
#include "mdb.h"
#include "sdplane.h"
#include "thread_info.h"
#include "tap.h"
#include "neigh_manager.h"
#include "nexthop.h"
#include "l2fwd_export.h"
#include "internal_message.h"
#include "radix.h"
#include "fib.h"

#if HAVE_LIBURCU_QSBR
#include <urcu/urcu-qsbr.h>
#endif /*HAVE_LIBURCU_QSBR*/

struct rte_ring *ring_up[RTE_MAX_ETHPORTS][MAX_RX_QUEUE_PER_LCORE];
struct rte_ring *ring_dn[RTE_MAX_ETHPORTS][MAX_RX_QUEUE_PER_LCORE];

struct rte_ring *router_if_ring_up[MAX_VSWITCH];
struct rte_ring *router_if_ring_dn[MAX_VSWITCH];
struct rte_ring *capture_if_ring_up[MAX_VSWITCH];
struct rte_ring *capture_if_ring_dn[MAX_VSWITCH];

struct rte_eth_dev_tx_buffer *tx_buffer_per_q[RTE_MAX_ETHPORTS][RTE_MAX_LCORE];

extern int lthread_core;
extern volatile bool force_stop[RTE_MAX_LCORE];

struct rte_ring *msg_queue_rib;

void *rcu_global_ptr_rib;
uint64_t rib_rcu_replace = 0;

struct rib_tree *rib_tree_master[ROUTE_TREE_SIZE];

static inline __attribute__ ((always_inline)) int
router_if_delete (struct rib_info *rib_info, uint16_t vswitch_id);
static inline __attribute__ ((always_inline)) int
capture_if_delete (struct rib_info *rib_info, uint16_t vswitch_id);

/*
 * vswitch
 */

static inline __attribute__ ((always_inline)) struct vswitch_conf *
vswitch_lookup (struct rib_info *new, uint16_t vswitch_id)
{
  int i;
  for (i = 0; i < new->vswitch_size; i++)
    {
      if (new->vswitch[i].vswitch_id == vswitch_id &&
          ! new->vswitch[i].is_deleted)
        return &new->vswitch[i];
    }
  return NULL;
}

static inline __attribute__ ((always_inline)) struct vswitch_conf *
vswitch_new (struct rib_info *new, uint16_t vswitch_id, uint16_t vlan_id)
{
  int i;
  struct vswitch_conf *vswitch;
  bool is_slot_found = false;

  for (i = 0; i < new->vswitch_size; i++)
    {
      if (new->vswitch[i].is_deleted)
        {
          is_slot_found = true;
          memset (&new->vswitch[i], 0, sizeof (struct vswitch_conf));
          vswitch = &new->vswitch[i];
          break;
        }
    }

  if (! is_slot_found)
    {
      if (new->vswitch_size >= MAX_VSWITCH)
        return NULL;
      vswitch = &new->vswitch[new->vswitch_size];
      new->vswitch_size++;
    }

  vswitch->vswitch_id = vswitch_id;
  vswitch->vlan_id = vlan_id;
  vswitch->router_if.sockfd = -1;
  vswitch->capture_if.sockfd = -1;
  vswitch->router_if.tap_ring_id = -1;
  vswitch->router_if.vlan_id = vlan_id;
  vswitch->capture_if.tap_ring_id = -1;
  vswitch->is_deleted = false;
  return vswitch;
}

static inline __attribute__ ((always_inline)) struct vswitch_link *
vswitch_link_lookup (struct rib_info *new, struct vswitch_conf *vswitch,
                     uint16_t port_id)
{
  int i;
  struct vswitch_link *vswitch_link;
  for (i = 0; i < new->vswitch_link_size; i++)
    {
      vswitch_link = &new->vswitch_link[i];
      if (vswitch_link->vswitch_id == vswitch->vswitch_id &&
          vswitch_link->port_id == port_id && ! vswitch_link->is_deleted)
        return vswitch_link;
    }
  return NULL;
}

static inline __attribute__ ((always_inline)) struct vswitch_link *
vswitch_link_new (struct rib_info *new, struct vswitch_conf *vswitch,
                  uint16_t port_id)
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
  vswitch_link->port_id = port_id;
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

  vswitch_link_remove_from_vswitch_port_array (
      vswitch_lookup (rib_info, vswitch_id), vswitch_link_id);

  vswitch_link_remove_from_port_vlan_array (&rib_info->port[port_id],
                                            vswitch_link_id);

  for (i = vswitch_link_id; i < rib_info->vswitch_link_size; i++)
    {
      if (rib_info->vswitch_link[i].vswitch_id == vswitch_id)
        rib_info->vswitch_link[i].vswitch_port--;
    }

  rib_info->vswitch_link[vswitch_link_id].is_deleted = true;
  DEBUG_NEW (RIB, "delete: link_id: %u", vswitch_link_id);
  return 0;
}

static inline __attribute__ ((always_inline)) int
vswitch_delete (struct rib_info *rib_info, uint16_t vswitch_id)
{
  struct vswitch_conf *vswitch = vswitch_lookup (rib_info, vswitch_id);
  if (! vswitch)
    {
      DEBUG_NEW (RIB, "delete: vswitch: %u not found", vswitch_id);
      return 0;
    }

  /* delete vswitch's router_if and capture_if */
  if (vswitch->router_if.sockfd >= 0)
    router_if_delete (rib_info, vswitch_id);
  if (vswitch->capture_if.sockfd >= 0)
    capture_if_delete (rib_info, vswitch_id);

  /* delete all vswitch's vswitch_links from port_conf */
  while (vswitch->vswitch_port_size > 0)
    vswitch_link_delete (rib_info, vswitch->vswitch_link_id[0]);

  vswitch->is_deleted = true;
  DEBUG_NEW (RIB, "delete: vswitch: %u", vswitch_id);
  return 0;
}

/*
 * router_if
 */

static inline __attribute__ ((always_inline)) int
router_if_delete (struct rib_info *rib_info, uint16_t vswitch_id)
{
  struct vswitch_conf *vswitch = vswitch_lookup (rib_info, vswitch_id);
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
  rif->ifindex = -1;
  DEBUG_NEW (RIB, "delete: router_if: vswitch %u", vswitch_id);
  return 0;
}

/*
 * capture_if
 */

static inline __attribute__ ((always_inline)) int
capture_if_delete (struct rib_info *rib_info, uint16_t vswitch_id)
{
  struct vswitch_conf *vswitch = vswitch_lookup (rib_info, vswitch_id);
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
  DEBUG_NEW (RIB, "delete: capture_if: vswitch %u", vswitch_id);
  return 0;
}

/*
 * port
 */

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

/*
 * route_table
 */

static inline __attribute__ ((always_inline)) int
route_table_lookup_id (int table_id, int family)
{
  int i;

  for (i = 0; i < ROUTE_TREE_SIZE; i++)
    {
      if (rib_tree_master[i] == NULL)
        continue;
      if (rib_tree_master[i]->family == family)
        return i;
    }
  return -1;
}

static inline __attribute__ ((always_inline)) int
route_table_alloc_slot (int table_id, int family)
{
  int tree_idx = (family == AF_INET) ? 0 : 1; // IPv4=0, IPv6=1

  if (! rib_tree_master[tree_idx])
    {
      rib_tree_master[tree_idx] = rib_new (NULL);
      if (! rib_tree_master[tree_idx])
        return -1;
      rib_tree_master[tree_idx]->family = family;
    }

  rib_tree_master[tree_idx]->table_id = table_id;
  return tree_idx;
}

/*
 * SRv6
 */

int
srv6_local_sid_find (struct rib_info *rib_info, const struct in6_addr *sid)
{
  int i;

  if (! rib_info || ! sid)
    return -1;

  for (i = 0; i < rib_info->srv6_local_sid_addr_num; i++)
    {
      if (memcmp (&rib_info->srv6_local_sid[i].sid, sid, sizeof (*sid)) == 0)
        return i;
    }
  return -1;
}

static inline __attribute__ ((always_inline)) void
srv6_local_sid_del (struct rib_info *rib_info,
                    const struct in6_addr *sid)
{
  int i, num = rib_info->srv6_local_sid_addr_num;

  for (i = 0; i < num; i++)
    {
      if (memcmp (&rib_info->srv6_local_sid[i].sid, sid, sizeof (*sid)) == 0)
        {
          int tail = num - 1;
          if (i < tail)
            memmove (&rib_info->srv6_local_sid[i],
                     &rib_info->srv6_local_sid[i + 1],
                     (tail - i) * sizeof (struct srv6_local_sid_entry));
          memset (&rib_info->srv6_local_sid[tail], 0,
                  sizeof (struct srv6_local_sid_entry));
          rib_info->srv6_local_sid_addr_num--;
          break;
        }
    }
}

/*
 * rib and rib_info
 */

static inline __attribute__ ((always_inline)) struct rib_info *
rib_info_create (struct rib_info *old)
{
  struct rib_info *new;
  int i;

  /* allocate new */
  new = malloc (sizeof (struct rib_info));
  if (! new)
    return NULL;

  if (! old)
    {
      memset (new, 0, sizeof (struct rib_info));
      for (i = 0; i < ROUTE_TREE_SIZE; i++)
        new->fib_tree[i] = fib_new (NULL);
      for (i = 0; i < MAX_ETH_PORTS; i++)
        new->port[i].dpdk_port_id = i;
      if (fdb_table_init (&new->fdb_table) < 0)
        {
          free (new);
          return NULL;
        }
      if (mdb_table_init (&new->mdb_table) < 0)
        {
          fdb_table_destroy (&new->fdb_table);
          free (new);
          return NULL;
        }
    }
  else
    {
      memcpy (new, old, sizeof (struct rib_info));
      for (i = 0; i < ROUTE_TREE_SIZE; i++)
        new->fib_tree[i] = fib_new (NULL);
      /* deep-copy the FDB table */
      if (fdb_table_copy (&new->fdb_table, &old->fdb_table) < 0)
        {
          free (new);
          return NULL;
        }
      if (mdb_table_copy (&new->mdb_table, &old->mdb_table) < 0)
        {
          fdb_table_destroy (&new->fdb_table);
          free (new);
          return NULL;
        }
    }

  new->ver++;
  return new;
}

static inline __attribute__ ((always_inline)) void
rib_info_delete (struct rib_info *old)
{
  int i;

  /* free each FIB tree */
  for (i = 0; i < ROUTE_TREE_SIZE; i++)
    {
      if (old->fib_tree[i])
        {
          /* free all FIB nodes in the tree */
          fib_free (old->fib_tree[i]);
        }
    }
  /* free FDB and MDB values and bucket array */
  fdb_table_destroy (&old->fdb_table);
  mdb_table_destroy (&old->mdb_table);
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

static inline __attribute__ ((always_inline)) int
rib_check (struct rib *new)
{
  struct lcore_qconf *lcore_qconf;
  int lcore;
  int i, ret;
  char ring_name[32];
  int j;

  DEBUG_NEW (RIB, "ver: %d rib: %p rib_info: %p.", new->rib_info->ver,
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

          DEBUG_NEW (RIB, "new rib: lcore: %d qconf[%d]: port: %d queue: %d",
                     lcore, i, rxq->port_id, rxq->queue_id);
        }
    }

#define MAX_PORT_QCONF 256
  struct port_queue_conf port_qconf[MAX_PORT_QCONF];
  int port_qconf_size = 0;
  memset (port_qconf, 0, sizeof (port_qconf));

  /* check port's #rxq/#txq */
  int max_lcore = 0;
  RTE_LCORE_FOREACH (lcore)
    {
      lcore_qconf = &new->rib_info->lcore_qconf[lcore];

      if (max_lcore < lcore)
        max_lcore = lcore;

      for (i = 0; i < lcore_qconf->nrxq; i++)
        {
          struct port_queue_conf *rxq;
          rxq = &lcore_qconf->rx_queue_list[i];

          memcpy (&port_qconf[port_qconf_size], rxq,
                  sizeof (struct port_queue_conf));
          port_qconf_size++;
        }
    }

  qsort (port_qconf, port_qconf_size, sizeof (struct port_queue_conf),
         port_qconf_compare);

  /* calculate the number of rx-queues based on the qconf. */
  int port_nrxq[RTE_MAX_ETHPORTS];
  memset (port_nrxq, 0, sizeof (port_nrxq));

  for (i = 0; i < port_qconf_size; i++)
    {
      struct port_queue_conf *rxq;
      rxq = &port_qconf[i];
      DEBUG_NEW (RIB, "port_qconf[%d]: port: %d queue: %d",
                 i, rxq->port_id, rxq->queue_id);
      if (port_nrxq[rxq->port_id] == rxq->queue_id)
        {
          port_nrxq[rxq->port_id]++;
        }
      else
        {
          DEBUG_NEW (RIB, "unorderd port_qconf[%d]: port: %d queue: %d",
                     i, rxq->port_id, rxq->queue_id);
          return -1;
        }
    }

  struct rte_eth_dev_info dev_info;

  int ntxq;
  ntxq = max_lcore + 1;
  DEBUG_NEW (RIB, "max_lcore: %d, ntxq: %d", max_lcore, ntxq);
  int nb_ports;
  nb_ports = rte_eth_dev_count_avail ();
  if (nb_ports > MAX_ETH_PORTS)
    nb_ports = MAX_ETH_PORTS;
  for (i = 0; i < nb_ports; i++)
    {
      int nrxq;
      nrxq = port_nrxq[i];
      rte_eth_dev_info_get (i, &dev_info);

      struct rte_eth_conf port_conf =
        { .txmode = { .mq_mode = RTE_ETH_MQ_TX_NONE, },
        };
      struct port_conf *port_cfg = &new->rib_info->port[i];

      /* update the current queue sizes. */
      port_cfg->nb_rx_queues = dev_info.nb_rx_queues;
      port_cfg->nb_tx_queues = dev_info.nb_tx_queues;

      /* make nrxq/ntxq default to the current dev_info queue sizes. */
      nrxq = dev_info.nb_rx_queues;
      ntxq = dev_info.nb_tx_queues;

      if (nrxq != port_nrxq[i])
        {
          DEBUG_NEW (RIB, "port[%d]: nrxq mismatch!: "
                     "configured: %d qconf: %d",
                     i, nrxq, port_nrxq[i]);
          /* Ideally it should be returned as a startup-config error.
             But currently we don't have a way to stop startup-config
             from here. For the time being we override it by the
             qconf value, which seems safer. */
#if 0
          return -1;
#else
          nrxq = port_nrxq[i];
#endif
        }

      if (port_cfg->rx_offloads_valid)
        port_conf.rxmode.offloads = port_cfg->rx_offloads;
      if (port_cfg->tx_offloads_valid)
        port_conf.txmode.offloads = port_cfg->tx_offloads;

      port_conf.rxmode.offloads &= dev_info.rx_offload_capa;
      port_conf.txmode.offloads &= dev_info.tx_offload_capa;

      if (dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE)
        port_conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE;
      else
        port_conf.txmode.offloads &= (~RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE);

      if (*dev_info.dev_flags & RTE_ETH_DEV_INTR_LSC)
        {
          port_conf.intr_conf.lsc = 1;
        }

      if (port_cfg->nrxq)
        {
          //if (nrxq < port_cfg->nrxq)
            nrxq = port_cfg->nrxq;
        }
      if (port_cfg->ntxq)
        {
          DEBUG_NEW (RIB, "ntxq: %d port_cfg->ntxq: %d",
                     ntxq, port_cfg->ntxq);
          //if (ntxq < port_cfg->ntxq)
            ntxq = port_cfg->ntxq;
        }

      if (port_cfg->rss_hf_valid)
        {
          uint64_t rss_hf;
          rss_hf = port_cfg->rss_hf & dev_info.flow_type_rss_offloads;
          if (rss_hf)
            {
              port_conf.rxmode.mq_mode = RTE_ETH_MQ_RX_RSS;
              port_conf.rx_adv_conf.rss_conf.rss_hf = rss_hf;
              if (dev_info.rx_offload_capa & RTE_ETH_RX_OFFLOAD_RSS_HASH)
                port_conf.rxmode.offloads |= RTE_ETH_RX_OFFLOAD_RSS_HASH;
            }
          else
            {
              port_conf.rxmode.mq_mode = RTE_ETH_MQ_RX_NONE;
              port_conf.rx_adv_conf.rss_conf.rss_hf = 0;
              port_conf.rxmode.offloads &= (~RTE_ETH_RX_OFFLOAD_RSS_HASH);
            }
        }

      /* Mellanox NICs does not seem to allow to set a tx-queue without
         a rx-queue. Setting nrxq: 0, ntxq: 1 fails (actually, crashes)
         in DPDK 24.11.4. */
      if (nrxq == 0)
        continue;

      DEBUG_NEW (RIB, "port[%d]: dev_configure: nrxq: %d ntxq: %d",
                 i, nrxq, ntxq);

      DEBUG_NEW (RTE_ETHDEV, "port[%d]: rte_eth_dev_stop().", i);
      rte_eth_dev_stop (i);

      port_cfg->rx_offloads_active = port_conf.rxmode.offloads;
      port_cfg->tx_offloads_active = port_conf.txmode.offloads;

      ret = rte_eth_dev_configure (i, nrxq, ntxq, &port_conf);
      DEBUG_NEW (RTE_ETHDEV, "rte_eth_dev_configure(): ret: %d "
                 "on port[%d]: nrxq: %d ntxq: %d",
                 ret, i, nrxq, ntxq);

      if (port_cfg->mtu)
        {
          uint16_t mtu = port_cfg->mtu;
          DEBUG_NEW (RIB, "port[%d]: dev_set_mtu: %d", i, mtu);
          ret = rte_eth_dev_set_mtu (i, mtu);
          if (ret < 0)
            WARNING ("port[%d]: dev_set_mtu: %d failed: %d", i, mtu, ret);
          DEBUG_NEW (RTE_ETHDEV, "rte_eth_dev_set_mtu(): ret: %d "
                 "on port[%d]: mtu: %d",
                 ret, i, mtu);
        }

      nb_rxd = RX_DESC_DEFAULT;
      if (port_cfg->nb_rxd)
        nb_rxd = port_cfg->nb_rxd;
      nb_txd = TX_DESC_DEFAULT;
      if (port_cfg->nb_txd)
        nb_txd = port_cfg->nb_txd;

      rxq_conf = dev_info.default_rxconf;
      rxq_conf.offloads = port_conf.rxmode.offloads;

      int socket;
      socket = rte_eth_dev_socket_id (i);

      for (j = 0; j < nrxq; j++)
        {
          ret = rte_eth_rx_queue_setup (i, j, nb_rxd, socket,
                                        &rxq_conf, l2fwd_pktmbuf_pool);
          DEBUG_NEW (RIB, "port[%d]: rx_queue_setup: rxq: %d rxd: %d",
                     i, j, nb_rxd);
          DEBUG_NEW (RTE_ETHDEV,
                     "port[%d]: rx_queue_setup: rxq: %d rxd: %d ret: %d",
                     i, j, nb_rxd, ret);
        }

      txq_conf = dev_info.default_txconf;
      txq_conf.offloads = port_conf.txmode.offloads;

      for (j = 0; j < ntxq; j++)
        {
          ret = rte_eth_tx_queue_setup (i, j, nb_txd, socket, &txq_conf);
          DEBUG_NEW (RIB, "port[%d]: tx_queue_setup: txq: %d txd: %d",
                     i, j, nb_txd);
          DEBUG_NEW (RTE_ETHDEV,
                     "port[%d]: tx_queue_setup: txq: %d txd: %d ret: %d",
                     i, j, nb_txd, ret);

          if (! tx_buffer_per_q[i][j])
            {
              DEBUG_NEW (L2_REPEATER, "tx_buffer_init: port: %d queue: %d",
                         i, j);
              tx_buffer_per_q[i][j] = rte_zmalloc_socket (
                  "tx_buffer", RTE_ETH_TX_BUFFER_SIZE (MAX_PKT_BURST), 0,
                  rte_eth_dev_socket_id (i));
              rte_eth_tx_buffer_init (tx_buffer_per_q[i][j], MAX_PKT_BURST);
            }
        }

      DEBUG_NEW (RTE_ETHDEV, "port[%d]: rte_eth_dev_start().", i);
      rte_eth_dev_start (i);
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
              DEBUG_NEW (RIB, "rib: create: %s: %p", ring_name,
                         ring_up[rxq->port_id][rxq->queue_id]);
            }

          if (! ring_dn[rxq->port_id][rxq->queue_id])
            {
              snprintf (ring_name, sizeof (ring_name), "ring_dn[%d][%d]",
                        rxq->port_id, rxq->queue_id);
              ring_dn[rxq->port_id][rxq->queue_id] = rte_ring_create (
                  ring_name, RING_TO_TAP_SIZE, rte_socket_id (),
                  (RING_F_SP_ENQ | RING_F_SC_DEQ));
              DEBUG_NEW (RIB, "rib: create: %s: %p", ring_name,
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
                  ring_name, RING_TO_TAP_SIZE, rte_socket_id (), 0);
              DEBUG_NEW (RIB, "rib: create: %s: %p", ring_name,
                         router_if_ring_up[i]);
            }
          if (! router_if_ring_dn[i])
            {
              snprintf (ring_name, sizeof (ring_name), "router_dn[%d]", i);
              router_if_ring_dn[i] = rte_ring_create (
                  ring_name, RING_TO_TAP_SIZE, rte_socket_id (), 0);
              DEBUG_NEW (RIB, "rib: create: %s: %p", ring_name,
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
                  ring_name, RING_TO_TAP_SIZE, rte_socket_id (), 0);
              DEBUG_NEW (RIB, "rib: create: %s: %p", ring_name,
                         capture_if_ring_up[i]);
            }
          if (! capture_if_ring_dn[i])
            {
              snprintf (ring_name, sizeof (ring_name), "capture_dn[%d]", i);
              capture_if_ring_dn[i] = rte_ring_create (
                  ring_name, RING_TO_TAP_SIZE, rte_socket_id (), 0);
              DEBUG_NEW (RIB, "rib: create: %s: %p", ring_name,
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
  DEBUG_NEW (RIB, "rib: replace: %'lu-th: rib: %p -> %p "
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
update_port_status (struct rib *new, uint16_t target_port)
{
  uint16_t nb_ports, port_id;
  nb_ports = rte_eth_dev_count_avail ();
  new->rib_info->port_size = nb_ports;
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      if (target_port != 255 && port_id != target_port)
        continue;
      rte_eth_dev_info_get (port_id, &new->rib_info->port[port_id].dev_info);
      rte_eth_link_get_nowait (port_id, &new->rib_info->port[port_id].link);
      DEBUG_NEW (PORT_STATUS, "port: %d link: %d ver: %d rib_info: %p.",
                 port_id, new->rib_info->port[port_id].link.link_status,
                 new->rib_info->ver, new->rib_info);
      new->rib_info->port[port_id].dpdk_port_id = port_id;
      gettimeofday (&new->rib_info->port[port_id].last_changed, NULL);
    }
}

void
set_stop_flag (struct rib *old)
{
  uint16_t nb_ports, port_id;
  nb_ports = rte_eth_dev_count_avail ();
  for (port_id = 0; port_id < nb_ports; port_id++)
    {
      old->rib_info->port[port_id].is_stopped = true;
      DEBUG_NEW (RIB, "port[%d]: is_stopped: %d", port_id,
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
      DEBUG_NEW (RIB, "port[%d]: is_stopped: %d", port_id,
                 old->rib_info->port[port_id].is_stopped);
    }
}

void
application_slot_add (struct rib_info *rib_info,
                      struct application_slot_entry *msg_appli_slot)
{
  if (rib_info->application_slot_size == APPLI_SLOT_SIZE)
    {
      WARNING ("application_slot: full: %d entries.",
               rib_info->application_slot_size);
      return;
    }
  memcpy (&rib_info->application_slot[rib_info->application_slot_size],
          msg_appli_slot, sizeof (struct application_slot_entry));
  rib_info->application_slot_size++;
  DEBUG_NEW (RIB, "application_slot added: %d entries.",
             rib_info->application_slot_size);
}

#define RIB_RETFLAG_NONE               0
#define RIB_RETFLAG_RETURN_IMMEDIATELY 1

static uint16_t
rib_manager_process_port_get (struct internal_msg_header *imsghdr)
{
  uint16_t retflag = 0;
  struct internal_msg_port_info *port_info;
  assert (imsghdr->type == INTERNAL_MSG_TYPE_PORT_GET_REQUEST);
  port_info = (struct internal_msg_port_info *)
    internal_msg_body (imsghdr);

  DEBUG_NEW (RIB, "port_get_request: imsghdr: %p", imsghdr);

  struct rib *new, *old;
  old = rcu_dereference (rcu_global_ptr_rib);
  new = rib_create (old);

  /* get port info update. */
  uint16_t port_id;
  port_id = port_info->port_id;
  new->rib_info->port[port_id].dpdk_port_id = port_id;
  rte_eth_dev_info_get (port_id, &new->rib_info->port[port_id].dev_info);
  rte_eth_link_get_nowait (port_id, &new->rib_info->port[port_id].link);

  rib_replace (new);

  struct internal_msg_header *resp;
  struct internal_msg_port_info resp_port_info;
  resp_port_info.port_id = port_id;
  memcpy (&resp_port_info.dev_info,
          &new->rib_info->port[port_id].dev_info,
          sizeof (struct rte_eth_dev_info));
  memcpy (&resp_port_info.link,
          &new->rib_info->port[port_id].link,
          sizeof (struct rte_eth_link));
  resp = internal_msg_create (INTERNAL_MSG_TYPE_PORT_GET_RESPONSE,
                              &resp_port_info, sizeof (resp_port_info));
  if (imsghdr->ring_response)
    {
      internal_msg_send_to (imsghdr->ring_response, resp, NULL);
      DEBUG_NEW (RIB, "send response: %p to ring: %p",
                 resp, imsghdr->ring_response);
    }
  else
    {
      WARNING ("cannot send response: %p", resp);
      WARNING ("probably no shell ring response. update libsdplane.");
      internal_msg_delete (resp);
    }

  free (imsghdr);
  FLAG_SET (retflag, RIB_RETFLAG_RETURN_IMMEDIATELY);
  return retflag;
}

/*
 * route entry add/delete
 *
 * Split out of rib_manager_process_message() so that the SRv6 local SID
 * handler can install the same kind of entry without duplicating this.
 */

static int
_route_entry_add (struct rib *new, struct internal_msg_route_entry *msg)
{
  struct route_entry route_entry;
  char dst_str[64] = { 0 };
  char gw_str[512] = { 0 };
  int i, sdplane_nh_id;

  /* register nexthop entry to rib_info->nexthop */
  sdplane_nh_id = nexthop_add_entry (new->rib_info, &msg->nh);
  if (sdplane_nh_id < 0)
    return -1;
  nexthop_increment_refcnt (new->rib_info, sdplane_nh_id);

  /* search for existing RIB tree with matching family and table_id */
  i = route_table_lookup_id (msg->table_id, msg->family);
  if (i < 0)
    {
      /* if not found, search for empty slot and create new table */
      i = route_table_alloc_slot (msg->table_id, msg->family);
      if (i < 0)
        {
          DEBUG_NEW (RIB, "failed to create route table");
          return -1;
        }
    }

  /* create route entry */
  memset (&route_entry, 0, sizeof (struct route_entry));
  memcpy (&route_entry.dst, &msg->dst, sizeof (struct route_dst_info));
  route_entry.sdplane_nh_id = sdplane_nh_id;
  route_entry.table_id = msg->table_id;

  /* register route entry to RIB */
  if (rib_route_add (rib_tree_master[i], (uint8_t *) &msg->dst.dst_ip_addr,
                     msg->dst.plen, &route_entry) > 0)
    {
      DEBUG_NEW (RIB, "failed to add route to RIB");
      return -1;
    }

  inet_ntop (msg->dst.family, (uint8_t *) &msg->dst.dst_ip_addr, dst_str,
             sizeof (dst_str));
  DEBUG_NEW (RIB, "route added: dst=%s/%d nexthop=%s", dst_str, msg->dst.plen,
             nexthop_format (&msg->nh, gw_str, sizeof (gw_str)));
  return 0;
}

static int
_route_entry_del (struct rib *new, struct internal_msg_route_entry *msg)
{
  char dst_str[64] = { 0 };
  int i, sdplane_nh_id;

  if (msg->is_nhid)
    sdplane_nh_id =
        nexthop_get_index_by_nhid (new->rib_info, msg->nh.kernel_nh_id);
  else
    sdplane_nh_id = nexthop_find_entry (new->rib_info, &msg->nh);

  if (sdplane_nh_id < 0)
    return -1;

  /* search for existing RIB tree with matching family and table_id */
  i = route_table_lookup_id (msg->table_id, msg->family);
  if (i < 0)
    {
      DEBUG_NEW (RIB, "route table not found: table_id=%d family=%d",
                 msg->table_id, msg->family);
      return -1;
    }

  if (rib_route_delete (rib_tree_master[i], (uint8_t *) &msg->dst.dst_ip_addr,
                        msg->dst.plen) != 0)
    {
      DEBUG_NEW (RIB, "failed to delete route to RIB");
      return -1;
    }

  inet_ntop (msg->family, (uint8_t *) &msg->dst.dst_ip_addr, dst_str,
             sizeof (dst_str));
  DEBUG_NEW (RIB, "route deleted: dst=%s/%d", dst_str, msg->dst.plen);

  /* Drop this route's reference only after the route was actually
   * removed from the RIB; a delete request for a non-existent
   * route whose nexthop matches an existing group must not touch
   * that group's refcnt. A LEGACY group is freed when the last
   * referencing route goes away. */
  nexthop_decrement_refcnt (new->rib_info, sdplane_nh_id);
  return 0;
}

/*
 * SRv6 local SID as a route
 *
 * A local SID is also installed as a /128 route whose nexthop carries the
 * SID entry, so that the forwarding path can reach it through the ordinary
 * FIB lookup instead of scanning rib_info->srv6_local_sid[].  The array is
 * kept as is for the srv6-router compatible path; when both are present the
 * array is consulted first, so it stays authoritative for that path.
 */

static void
_srv6_local_sid_route_msg (struct internal_msg_route_entry *msg,
                           const struct srv6_local_sid_entry *entry)
{
  memset (msg, 0, sizeof (*msg));
  msg->family = AF_INET6;
  /* RT_TABLE_MAIN keeps this off the RT_TABLE_LOCAL path in the forwarding
     code.  Note route_table_lookup_id() picks the tree by family alone, so
     table_id does not separate routes today; it only travels with the entry. */
  msg->table_id = RT_TABLE_MAIN;
  msg->dst.family = AF_INET6;
  msg->dst.dst_ip_addr.ipv6_addr = entry->sid;
  msg->dst.plen = 128;
  msg->is_nhid = false;
  msg->nh.kernel_nh_id = 0;
  msg->nh.nhcnt = 1;
  msg->nh.members[0].kernel_nh_id = 0;
  msg->nh.members[0].weight = 1;
  msg->nh.members[0].info.family = AF_INET6;
  msg->nh.members[0].info.type = NEXTHOP_TYPE_SRV6_END;
  msg->nh.members[0].info.oif = entry->oif;
  msg->nh.members[0].info.gw.srv6_local_sid = *entry;
}

void
rib_manager_process_message (void *msgp)
{
  int ret;
  int i, j;

  struct rib *new, *old;

  /* retrieve old */
#if HAVE_LIBURCU_QSBR
  old = rcu_dereference (rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

  new = rib_create (old);

  /* related neigh table */
  struct neigh_table *neigh_table;

  /* related vswitch operation */
  struct vswitch_conf *vswitch;
  struct port_conf *port;
  struct vswitch_link *link;

  /* change something according to the update instruction message. */
  struct internal_msg_header *msg_header;
  struct internal_msg_qconf *msg_qconf;
  struct internal_msg_neigh_entry *msg_neigh_entry;
  struct internal_msg_route_entry *msg_route_entry;
  struct internal_msg_nh_entry *msg_nexthop_entry;
  struct internal_msg_mac_addr *msg_mac_addr;
  struct internal_msg_ip_addr *msg_ip_addr;

  msg_header = (struct internal_msg_header *) msgp;

  uint16_t retflag = 0;

  switch (msg_header->type)
    {
    case INTERNAL_MSG_TYPE_PORT_GET_REQUEST:
      DEBUG_NEW (RIB, "port_get_request: receive");
      retflag = rib_manager_process_port_get (msg_header);
      if (FLAG_CHECK (retflag, RIB_RETFLAG_RETURN_IMMEDIATELY))
        {
          rib_delete (new);
          /* msgp is already freed inside rib_manager_process_port_get(). */
          return;
        }
      break;

    case INTERNAL_MSG_TYPE_PORT_STATUS:
      DEBUG_NEW (RIB, "recv msg_port_status: %p.", msgp);
      uint16_t *msg_port_id;
      msg_port_id = (uint16_t *) (msg_header + 1);
      update_port_status (new, *msg_port_id);
      break;

    case INTERNAL_MSG_TYPE_QCONF:
      DEBUG_NEW (RIB, "recv msg_qconf: %p.", msgp);
      msg_qconf = (struct internal_msg_qconf *) (msg_header + 1);
      RTE_LCORE_FOREACH (i)
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
      if (old)
        set_stop_flag (old);
      ret = rib_check (new);
      if (old)
        delete_stop_flag (old);

      /* if rib_check() is not a pass */
      if (ret < 0)
        {
          DEBUG_NEW (RIB, "rib_check() failed: return.");
          rib_delete (new);
          free (msgp);
          return;
        }

      /* for qconf change, we need an NULL intermittent state
         to avoid a conflict between different cores. */
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
      {
        struct internal_msg_txrx_desc *msg_txrx_desc;
        int portid;
        DEBUG_NEW (RIB, "recv msg_txrx_desc: %p.", msgp);
        msg_txrx_desc = (struct internal_msg_txrx_desc *) (msg_header + 1);
        portid = msg_txrx_desc->portid;
        if (msg_txrx_desc->mtu)
          {
            new->rib_info->port[portid].mtu = msg_txrx_desc->mtu;
            DEBUG_NEW (RIB, "set port: %d mtu: %d",
                       portid, msg_txrx_desc->mtu);
          }
        if (msg_txrx_desc->nrxq)
          {
            new->rib_info->port[portid].nrxq = msg_txrx_desc->nrxq;
            DEBUG_NEW (RIB, "set port: %d nrxq: %d",
                       portid, msg_txrx_desc->nrxq);
          }
        if (msg_txrx_desc->ntxq)
          {
            new->rib_info->port[portid].ntxq = msg_txrx_desc->ntxq;
            DEBUG_NEW (RIB, "set port: %d ntxq: %d",
                       portid, msg_txrx_desc->ntxq);
          }
        if (msg_txrx_desc->nb_rxd)
          {
            new->rib_info->port[portid].nb_rxd = msg_txrx_desc->nb_rxd;
            DEBUG_NEW (RIB, "set port: %d nb_rxd: %d",
                       portid, msg_txrx_desc->nb_rxd);
          }
        if (msg_txrx_desc->nb_txd)
          {
            new->rib_info->port[portid].nb_txd = msg_txrx_desc->nb_txd;
            DEBUG_NEW (RIB, "set port: %d nb_txd: %d",
                       portid, msg_txrx_desc->nb_txd);
          }
        if (msg_txrx_desc->rx_offload_mask)
          {
            new->rib_info->port[portid].rx_offloads =
                (new->rib_info->port[portid].rx_offloads &
                 (~msg_txrx_desc->rx_offload_mask)) |
                (msg_txrx_desc->rx_offload_value &
                 msg_txrx_desc->rx_offload_mask);
            new->rib_info->port[portid].rx_offloads_valid = true;
            DEBUG_NEW (RIB, "set port: %d rx_offloads: 0x%" PRIx64, portid,
                       new->rib_info->port[portid].rx_offloads);
          }
        if (msg_txrx_desc->tx_offload_mask)
          {
            new->rib_info->port[portid].tx_offloads =
                (new->rib_info->port[portid].tx_offloads &
                 (~msg_txrx_desc->tx_offload_mask)) |
                (msg_txrx_desc->tx_offload_value &
                 msg_txrx_desc->tx_offload_mask);
            new->rib_info->port[portid].tx_offloads_valid = true;
            DEBUG_NEW (RIB, "set port: %d tx_offloads: 0x%" PRIx64, portid,
                       new->rib_info->port[portid].tx_offloads);
          }
        if (msg_txrx_desc->rss_hf_mask)
          {
            new->rib_info->port[portid].rss_hf =
                (new->rib_info->port[portid].rss_hf &
                 (~msg_txrx_desc->rss_hf_mask)) |
                (msg_txrx_desc->rss_hf_value & msg_txrx_desc->rss_hf_mask);
            new->rib_info->port[portid].rss_hf_valid = true;
            DEBUG_NEW (RIB, "set port: %d rss_hf: 0x%" PRIx64, portid,
                       new->rib_info->port[portid].rss_hf);
          }

        //if (msg_txrx_desc->nrxq || msg_txrx_desc->ntxq)
        //if (startup_config_completed)
          {
            if (old)
              set_stop_flag (old);
            ret = rib_check (new);
            if (old)
              delete_stop_flag (old);
          }
        break;
      }

    case INTERNAL_MSG_TYPE_NEIGH_ENTRY_ADD:
      DEBUG_NEW (RIB, "recv msg_neigh_entry_add: %p.", msgp);
      msg_neigh_entry = (struct internal_msg_neigh_entry *) (msg_header + 1);
      neigh_table = &new->rib_info->neigh_tables[msg_neigh_entry->type];
      /* If the master’s neighbor table has more entries than the RIB’s,
       * it indicates that a new entry has been inserted. */
      if (msg_neigh_entry->num_entries > neigh_table->num_entries)
        {
          /* insert new entry */
          DEBUG_NEW (NEIGH, "rib: add: %s[%d]",
                     neigh_manager_table_str (msg_neigh_entry->type),
                     msg_neigh_entry->pos);
          memmove (&neigh_table->entries[msg_neigh_entry->pos + 1],
                   &neigh_table->entries[msg_neigh_entry->pos],
                   sizeof (struct neigh_entry) *
                       (neigh_table->num_entries - msg_neigh_entry->pos));
        }
      else
        /* update entry */
        DEBUG_NEW (NEIGH, "rib: update: %s[%d]",
                   neigh_manager_table_str (msg_neigh_entry->type),
                   msg_neigh_entry->pos);
      memcpy (&neigh_table->entries[msg_neigh_entry->pos],
              &msg_neigh_entry->data, sizeof (struct neigh_entry));
      // same as "neigh_table->num_entries++;"
      neigh_table->num_entries = msg_neigh_entry->num_entries;
      break;

    case INTERNAL_MSG_TYPE_NEIGH_ENTRY_DEL:
      DEBUG_NEW (RIB, "recv msg_neigh_entry_del: %p.", msgp);
      msg_neigh_entry = (struct internal_msg_neigh_entry *) (msg_header + 1);
      /* delete entry */
      DEBUG_NEW (NEIGH, "rib: del: %s[%d]",
                 neigh_manager_table_str (msg_neigh_entry->type),
                 msg_neigh_entry->pos);
      neigh_table = &new->rib_info->neigh_tables[msg_neigh_entry->type];
      // same as "neigh_table->num_entries--;"
      neigh_table->num_entries = msg_neigh_entry->num_entries;
      memmove (&neigh_table->entries[msg_neigh_entry->pos],
               &neigh_table->entries[msg_neigh_entry->pos + 1],
               sizeof (struct neigh_entry) *
                   (neigh_table->num_entries - msg_neigh_entry->pos));
      break;

    case INTERNAL_MSG_TYPE_VSWITCH_SET:
      {
        struct internal_msg_vswitch *msg_vswitch_set;
        DEBUG_NEW (RIB, "recv msg_vswitch_set: %p.", msgp);
        msg_vswitch_set = (struct internal_msg_vswitch *) (msg_header + 1);
        if (vswitch_lookup (new->rib_info, msg_vswitch_set->vswitch_id))
          {
            DEBUG_NEW (RIB, "vswitch already exists.");
            break;
          }
        vswitch = vswitch_new (new->rib_info, msg_vswitch_set->vswitch_id,
                               msg_vswitch_set->vlan_id);
        if (vswitch)
          DEBUG_NEW (VSWITCH, "create succeeded: vswitch: %u vlan_id: %u",
                     msg_vswitch_set->vswitch_id,
                     msg_vswitch_set->vlan_id);
        else
          DEBUG_NEW (VSWITCH, "create failed: vswitch: %u vlan_id: %u "
                     "max-vswitch: %d",
                     msg_vswitch_set->vswitch_id,
                     msg_vswitch_set->vlan_id, MAX_VSWITCH);
        break;
      }

    case INTERNAL_MSG_TYPE_VSWITCH_NO_SET:
      {
        struct internal_msg_vswitch *msg_vswitch_no_set;
        DEBUG_NEW (VSWITCH, "recv msg_vswitch_no_set: %p.", msgp);
        msg_vswitch_no_set = (struct internal_msg_vswitch *) (msg_header + 1);
        if (! vswitch_lookup (new->rib_info, msg_vswitch_no_set->vswitch_id))
          {
            DEBUG_NEW (VSWITCH, "delete vswitch not found");
            break;
          }
        vswitch_delete (new->rib_info, msg_vswitch_no_set->vswitch_id);
        break;
      }

    case INTERNAL_MSG_TYPE_VSWITCH_PORT_SET:
      {
        struct internal_msg_vswitch_port *msg_vswitch_port_set;
        DEBUG_NEW (VSWITCH_LINK, "recv msg_vswitch_port_set: %p.", msgp);
        msg_vswitch_port_set =
            (struct internal_msg_vswitch_port *) (msg_header + 1);

        DEBUG_NEW (VSWITCH_LINK, "msg_vswitch_port_set: %p "
                   "vswitch_id: %d port_id: %d tag: %d",
                   msgp,
                   msg_vswitch_port_set->vswitch_id,
                   msg_vswitch_port_set->port_id,
                   msg_vswitch_port_set->tag_id
                   );

        vswitch =
            vswitch_lookup (new->rib_info, msg_vswitch_port_set->vswitch_id);
        if (! vswitch)
          {
            DEBUG_NEW (VSWITCH_LINK, "vswitch: %u not found",
                       msg_vswitch_port_set->vswitch_id);
            break;
          }
        port = &new->rib_info->port[msg_vswitch_port_set->port_id];
        link = vswitch_link_new (new->rib_info, vswitch,
                                 msg_vswitch_port_set->port_id);
        if (! link)
          {
            DEBUG_NEW (VSWITCH_LINK, "create failed: vswitch: %u port %u tag: %u",
                       msg_vswitch_port_set->vswitch_id,
                       msg_vswitch_port_set->port_id,
                       msg_vswitch_port_set->tag_id);
            break;
          }

        DEBUG_NEW (VSWITCH_LINK, "msg_vswitch_port_set: %p "
                   "vswitch: %p port: %p link: %p",
                   msgp, vswitch, port, link);

        if (! msg_vswitch_port_set->is_tagged)
          {
            port_set_native_vlan (new->rib_info, port, link);
            DEBUG_NEW (VSWITCH_LINK, "create succeeded: link_id: %u "
                       "vswitch: %u port: %u native",
                link->vswitch_link_id, msg_vswitch_port_set->vswitch_id,
                msg_vswitch_port_set->port_id);
          }
        else
          {
            if (msg_vswitch_port_set->tag_id != 0 &&
                msg_vswitch_port_set->tag_id != link->tag_id)
              link->tag_id = msg_vswitch_port_set->tag_id; // enable tag modified
            port_add_tagged_vlan (new->rib_info, port, link);
            DEBUG_NEW (VSWITCH_LINK, "create succeeded: link_id: %u "
                       "vswitch: %u port: %u tag: %u",
                link->vswitch_link_id, msg_vswitch_port_set->vswitch_id,
                msg_vswitch_port_set->port_id, msg_vswitch_port_set->tag_id);
          }
        break;
      }

    case INTERNAL_MSG_TYPE_VSWITCH_PORT_NO_SET:
      {
        struct internal_msg_vswitch_port *msg_vswitch_port_no_set;
        DEBUG_NEW (VSWITCH_LINK, "recv msg_vswitch_port_no_set: %p.", msgp);
        msg_vswitch_port_no_set =
            (struct internal_msg_vswitch_port *) (msg_header + 1);
        vswitch =
            vswitch_lookup (new->rib_info, msg_vswitch_port_no_set->vswitch_id);
        if (! vswitch)
          {
            DEBUG_NEW (VSWITCH_LINK, "vswitch: %u not found",
                               msg_vswitch_port_no_set->vswitch_id);
            break;
          }
        link = vswitch_link_lookup (new->rib_info, vswitch,
                                    msg_vswitch_port_no_set->port_id);
        if (! link)
          {
            DEBUG_NEW (VSWITCH_LINK, "vswitch link: vswitch: %u port: %u not found",
                       msg_vswitch_port_no_set->vswitch_id,
                       msg_vswitch_port_no_set->port_id);
            break;
          }

        vswitch_link_delete (new->rib_info, link->vswitch_link_id);
        break;
      }

    case INTERNAL_MSG_TYPE_ROUTER_IF_SET:
      {
        struct internal_msg_tap_dev *msg_router_if_set;
        struct router_if *rif;
        DEBUG_NEW (RIB, "recv msg_router_if_set: %p.", msgp);
        msg_router_if_set = (struct internal_msg_tap_dev *) (msg_header + 1);

        vswitch = vswitch_lookup (new->rib_info, msg_router_if_set->vswitch_id);
        if (! vswitch)
          {
            DEBUG_NEW (RIB, "vswitch: %u not found",
                               msg_router_if_set->vswitch_id);
            break;
          }
        rif = &vswitch->router_if;

        rif->sockfd = tap_open (msg_router_if_set->tap_name);
        ioctl (rif->sockfd, TUNSETPERSIST, 1);
        rif->tap_ring_id = msg_router_if_set->vswitch_id;
        snprintf (rif->tap_name, sizeof (rif->tap_name), "%s",
                  msg_router_if_set->tap_name);
#define NO_VLAN_SPECIFIED 65535
        if (msg_router_if_set->vlan_id != NO_VLAN_SPECIFIED)
          rif->vlan_id = msg_router_if_set->vlan_id;
        else
          rif->vlan_id = vswitch->vlan_id;
        rif->ifindex = tap_get_ifindex (msg_router_if_set->tap_name);
        tap_admin_up (msg_router_if_set->tap_name);

        DEBUG_NEW (RIB, "create succeeded: router_if: %s vswitch: %u: vlan %u",
                   msg_router_if_set->tap_name,
                   msg_router_if_set->vswitch_id,
                   msg_router_if_set->vlan_id);

        // set router_if ring
        if (old)
          set_stop_flag (old);
        ret = rib_check (new);
        if (old)
          delete_stop_flag (old);

        /* if rib_check() is not a pass */
        if (ret < 0)
          {
            DEBUG_NEW (RIB, "rib_check() failed: return.");
            rib_delete (new);
            free (msgp);
            return;
          }
#if 0
        /*
         * TODO(#229):
         * Temporarily disabled due to duplicate TX on the same link
         * with enhanced repeater.
         * Expected to be resolved by future changes to the application_slot implementation.
         */

         app_slot.name = L3_TAP_HANDLER_APP_NAME;
         app_slot.ring = rif->ring_up;
         app_slot.is_packet_match = should_send_to_tap;
         application_slot_add (new->rib_info, &app_slot);
#endif
        break;
      }


    case INTERNAL_MSG_TYPE_ROUTER_IF_NO_SET:
      {
        struct internal_msg_tap_dev *msg_router_if_no_set;
        DEBUG_NEW (RIB, "recv msg_router_if_no_set: %p.", msgp);
        msg_router_if_no_set = (struct internal_msg_tap_dev *) (msg_header + 1);

        for (i = 0; i < new->rib_info->vswitch_size; i++)
          {
            if (! strcmp (msg_router_if_no_set->tap_name,
                          new->rib_info->vswitch[i].router_if.tap_name))
              router_if_delete (new->rib_info,
                                new->rib_info->vswitch[i].vswitch_id);
          }
        break;
      }

    case INTERNAL_MSG_TYPE_CAPTURE_IF_SET:
      {
        struct internal_msg_tap_dev *msg_capture_if_set;
        struct capture_if *cif;
        DEBUG_NEW (RIB, "recv msg_capture_if_set: %p.", msgp);
        msg_capture_if_set = (struct internal_msg_tap_dev *) (msg_header + 1);

        vswitch = vswitch_lookup (new->rib_info, msg_capture_if_set->vswitch_id);
        if (! vswitch)
          {
            DEBUG_NEW (RIB, "vswitch: %u not found",
                               msg_capture_if_set->vswitch_id);
            break;
          }
        cif = &vswitch->capture_if;

        cif->sockfd = tap_open (msg_capture_if_set->tap_name);
        ioctl (cif->sockfd, TUNSETPERSIST, 1);
        cif->tap_ring_id = msg_capture_if_set->vswitch_id;
        snprintf (cif->tap_name, sizeof (cif->tap_name), "%s",
                  msg_capture_if_set->tap_name);
        tap_admin_up (msg_capture_if_set->tap_name);

        DEBUG_NEW (RIB, "create: capture_if: %s vswitch: %u",
                   msg_capture_if_set->tap_name,
                   msg_capture_if_set->vswitch_id);

        // set capture_if ring
        set_stop_flag (old);
        ret = rib_check (new);
        delete_stop_flag (old);

        /* if rib_check() is not a pass */
        if (ret < 0)
          {
            DEBUG_NEW (RIB, "rib_check() failed: return.");
            rib_delete (new);
            free (msgp);
            return;
          }

        break;
      }

    case INTERNAL_MSG_TYPE_CAPTURE_IF_NO_SET:
      {
        struct internal_msg_tap_dev *msg_capture_if_no_set;
        DEBUG_NEW (RIB, "recv msg_capture_if_no_set: %p.", msgp);
        msg_capture_if_no_set = (struct internal_msg_tap_dev *) (msg_header + 1);
        for (i = 0; i < new->rib_info->vswitch_size; i++)
          {
            if (! strcmp (msg_capture_if_no_set->tap_name,
                          new->rib_info->vswitch[i].capture_if.tap_name))
              capture_if_delete (new->rib_info,
                                 new->rib_info->vswitch[i].vswitch_id);
          }
        break;
      }

    case INTERNAL_MSG_TYPE_FDB_ENTRY_ADD:
      {
        struct internal_msg_fdb_entry *msg_fdb_entry_add;
        DEBUG_NEW (RIB, "recv msg_fdb_entry_add: %p.", msgp);

        msg_fdb_entry_add = (struct internal_msg_fdb_entry *) (msg_header + 1);

        fdb_add_entry (new->rib_info, &msg_fdb_entry_add->mac_addr,
                       msg_fdb_entry_add->vlan_id, msg_fdb_entry_add->port);
        break;
      }

    case INTERNAL_MSG_TYPE_FDB_AGING:
      {
        DEBUG_SDPLANE_LOG (RIB, "recv msg_fdb_aging: %p.", msgp);
        struct internal_msg_fdb_aging *msg_fdb_aging;
        int aged;

        msg_fdb_aging = (struct internal_msg_fdb_aging *) (msg_header + 1);

        aged = fdb_aging_process (new->rib_info, msg_fdb_aging->aging_time);
        DEBUG_SDPLANE_LOG (FDB, "fdb aging: %d entries aged out", aged);
        break;
      }

    case INTERNAL_MSG_TYPE_MDB_ENTRY_ADD:
      {
        struct internal_msg_mdb_entry *msg_mdb_entry_add;
        DEBUG_NEW (RIB, "recv msg_mdb_entry_add: %p.", msgp);

        msg_mdb_entry_add = (struct internal_msg_mdb_entry *) (msg_header + 1);

        if (! vswitch_lookup (new->rib_info, msg_mdb_entry_add->vswitch_id))
          {
            DEBUG_NEW (RIB, "vswitch: %u not found",
                       (unsigned int) msg_mdb_entry_add->vswitch_id);
            break;
          }

        mdb_add_entry (new->rib_info, msg_mdb_entry_add->group_addr,
                       msg_mdb_entry_add->vswitch_id, msg_mdb_entry_add->port);
        break;
      }

    case INTERNAL_MSG_TYPE_MDB_ENTRY_DEL:
      {
        struct internal_msg_mdb_entry *msg_mdb_entry_del;
        DEBUG_NEW (RIB, "recv msg_mdb_entry_del: %p.", msgp);

        msg_mdb_entry_del = (struct internal_msg_mdb_entry *) (msg_header + 1);

        mdb_del_entry (new->rib_info, msg_mdb_entry_del->group_addr,
                       msg_mdb_entry_del->vswitch_id, msg_mdb_entry_del->port);
        break;
      }

    case INTERNAL_MSG_TYPE_APPLICATION_SLOT:
      {
        struct application_slot_entry *msg_appli_slot;
        DEBUG_NEW (RIB, "recv msg_appli_slot: %p.", msgp);
        msg_appli_slot =
          (struct application_slot_entry *) (msg_header + 1);
        application_slot_add (new->rib_info, msg_appli_slot);
        break;
      }

    case INTERNAL_MSG_TYPE_ROUTE_ENTRY_ADD:
      DEBUG_NEW (RIB, "recv msg_route_entry_add: %p.", msgp);
      msg_route_entry = (struct internal_msg_route_entry *) (msg_header + 1);
      /* these are declared here but shared with the cases below */
      int sdplane_nh_id = 0;
      char gw_addr[512] = {0};

      _route_entry_add (new, msg_route_entry);
      break;

    case INTERNAL_MSG_TYPE_ROUTE_ENTRY_DEL:
      DEBUG_NEW (RIB, "recv msg_route_entry_del: %p.", msgp);
      msg_route_entry = (struct internal_msg_route_entry *) (msg_header + 1);
      _route_entry_del (new, msg_route_entry);
      break;

    case INTERNAL_MSG_TYPE_NEXTHOP_ENTRY_ADD:
      // DEBUG_NEW (RIB, "recv msg_nexthop_entry_add: %p.", msgp);
      msg_nexthop_entry = (struct internal_msg_nh_entry *) (msg_header + 1);

      /* register nexthop entry to rib_info->nexthop */
      sdplane_nh_id = nexthop_add_entry (new->rib_info, msg_nexthop_entry);
      if (sdplane_nh_id < 0)
        break;

      DEBUG_NEW (RIB, "nexthop added: index=%d nexthop=%s",
                 sdplane_nh_id,
                 nexthop_format (msg_nexthop_entry, gw_addr,
                 sizeof (gw_addr)));
      break;

    case INTERNAL_MSG_TYPE_NEXTHOP_ENTRY_DEL:
      // DEBUG_NEW (RIB, "recv msg_nexthop_entry_add/del: %p.", msgp);
      msg_nexthop_entry = (struct internal_msg_nh_entry *) (msg_header + 1);

      /* delete nexthop entry */
      sdplane_nh_id = nexthop_del_entry (new->rib_info, msg_nexthop_entry);
      if (sdplane_nh_id < 0)
        break;

      DEBUG_NEW (RIB, "nexthop deleted: index=%d", sdplane_nh_id);
      break;

    case INTERNAL_MSG_TYPE_MAC_ADDR_ADD:
      DEBUG_NEW (RIB, "recv msg_mac_addr_add: %p.", msgp);
      msg_mac_addr = (struct internal_msg_mac_addr *) (msg_header + 1);
      char mac_str_add[RTE_ETHER_ADDR_FMT_SIZE];

      for (i = 0; i < new->rib_info->vswitch_size; i++)
        {
          if (memcmp (new->rib_info->vswitch[i].router_if.tap_name, msg_mac_addr->ifname,
                      sizeof (msg_mac_addr->ifname)) == 0)
            {
              memcpy (&new->rib_info->vswitch[i].router_if.mac_addr, &msg_mac_addr->mac_addr,
                      sizeof (struct rte_ether_addr));

              rte_ether_format_addr (mac_str_add, sizeof (mac_str_add), &msg_mac_addr->mac_addr);
              DEBUG_NEW (RIB, "add MAC address: ifname=%s mac=%s",
                         msg_mac_addr->ifname, mac_str_add);

              break;
            }
        }

      break;

    case INTERNAL_MSG_TYPE_MAC_ADDR_DEL:
      DEBUG_NEW (RIB, "recv msg_mac_addr_del: %p.", msgp);
      msg_mac_addr = (struct internal_msg_mac_addr *) (msg_header + 1);
      char mac_str_del[RTE_ETHER_ADDR_FMT_SIZE];

      for (i = 0; i < new->rib_info->vswitch_size; i++)
        {
          if (memcmp (new->rib_info->vswitch[i].router_if.tap_name, msg_mac_addr->ifname,
                      sizeof (msg_mac_addr->ifname)) == 0 &&
              memcmp (&new->rib_info->vswitch[i].router_if.mac_addr, &msg_mac_addr->mac_addr,
                      sizeof (struct rte_ether_addr)))
            {
              memset (&new->rib_info->vswitch[i].router_if.mac_addr, 0,
                      sizeof (struct rte_ether_addr));

              rte_ether_format_addr (mac_str_del, sizeof (mac_str_del), &msg_mac_addr->mac_addr);
              DEBUG_NEW (RIB, "delete MAC address: ifname=%s mac=%s",
                         msg_mac_addr->ifname, mac_str_del);

              break;
            }
        }

      break;

    case INTERNAL_MSG_TYPE_IP_ADDR_ADD:
      DEBUG_NEW (RIB, "recv msg_ip_addr_add: %p.", msgp);
      msg_ip_addr = (struct internal_msg_ip_addr *) (msg_header + 1);
      char ip_str_add[INET6_ADDRSTRLEN];

      for (i = 0; i < new->rib_info->vswitch_size; i++)
        {
          if (memcmp (new->rib_info->vswitch[i].router_if.tap_name, msg_ip_addr->ifname,
                      sizeof (msg_ip_addr->ifname)) == 0)
            {
              if (msg_ip_addr->family == AF_INET)
                {
                  memcpy (&new->rib_info->vswitch[i].router_if.ipv4_addr, &msg_ip_addr->ip_addr.ipv4_addr,
                          sizeof (struct in_addr));

                  inet_ntop (AF_INET, &msg_ip_addr->ip_addr.ipv4_addr, ip_str_add,
                             sizeof (ip_str_add));
                  DEBUG_NEW (RIB, "add IPv4 address: ifname=%s ip=%s",
                             msg_ip_addr->ifname, ip_str_add);
                }
              else
                {
                  if (msg_ip_addr->is_ll_addr)
                    memcpy (&new->rib_info->vswitch[i].router_if.ll_addr, &msg_ip_addr->ip_addr.ipv6_addr,
                            sizeof (struct in6_addr));
                  else
                    memcpy (&new->rib_info->vswitch[i].router_if.ipv6_addr, &msg_ip_addr->ip_addr.ipv6_addr,
                            sizeof (struct in6_addr));

                  inet_ntop (AF_INET6, &msg_ip_addr->ip_addr.ipv6_addr, ip_str_add,
                             sizeof (ip_str_add));
                  DEBUG_NEW (RIB, "add IPv6 address: ifname=%s ip=%s",
                             msg_ip_addr->ifname, ip_str_add);
                }

              break;
            }
        }

      break;

    case INTERNAL_MSG_TYPE_IP_ADDR_DEL:
      DEBUG_NEW (RIB, "recv msg_ip_addr_del: %p.", msgp);
      msg_ip_addr = (struct internal_msg_ip_addr *) (msg_header + 1);
      char ip_str_del[INET6_ADDRSTRLEN];

      for (i = 0; i < new->rib_info->vswitch_size; i++)
        {
          if (memcmp (new->rib_info->vswitch[i].router_if.tap_name, msg_ip_addr->ifname,
                      sizeof (msg_ip_addr->ifname)) == 0)
            {
              if (msg_ip_addr->family == AF_INET)
                {
                  if (memcmp (&new->rib_info->vswitch[i].router_if.ipv4_addr, &msg_ip_addr->ip_addr.ipv4_addr,
                              sizeof (struct in_addr)) == 0)
                    {
                      memset (&new->rib_info->vswitch[i].router_if.ipv4_addr, 0, sizeof (struct in_addr));

                      inet_ntop (AF_INET, &msg_ip_addr->ip_addr.ipv4_addr, ip_str_del,
                                 sizeof (ip_str_del));
                      DEBUG_NEW (RIB, "delete IPv4 address: ifname=%s ip=%s",
                                         msg_ip_addr->ifname, ip_str_del);
                    }
                }
              else
                {
                  if (msg_ip_addr->is_ll_addr)
                    {
                      if (memcmp (&new->rib_info->vswitch[i].router_if.ll_addr, &msg_ip_addr->ip_addr.ipv6_addr,
                                  sizeof (struct in6_addr)) == 0)
                        {
                          memset (&new->rib_info->vswitch[i].router_if.ll_addr, 0, sizeof (struct in6_addr));

                          inet_ntop (AF_INET6, &msg_ip_addr->ip_addr.ipv6_addr, ip_str_del,
                                    sizeof (ip_str_del));
                          DEBUG_NEW (RIB, "delete IPv6 address: ifname=%s ip=%s",
                                            msg_ip_addr->ifname, ip_str_del);
                        }
                    }
                  else
                    {
                      if (memcmp (&new->rib_info->vswitch[i].router_if.ipv6_addr, &msg_ip_addr->ip_addr.ipv6_addr,
                                  sizeof (struct in6_addr)) == 0)
                        {
                          memset (&new->rib_info->vswitch[i].router_if.ipv6_addr, 0, sizeof (struct in6_addr));

                          inet_ntop (AF_INET6, &msg_ip_addr->ip_addr.ipv6_addr, ip_str_del,
                                    sizeof (ip_str_del));
                          DEBUG_NEW (RIB, "delete IPv6 address: ifname=%s ip=%s",
                                            msg_ip_addr->ifname, ip_str_del);
                        }
                    }
                }

              break;
            }
        }

      break;

    case INTERNAL_MSG_TYPE_SRV6_LOCAL_SID_ADD:
      {
        struct internal_msg_srv6_local_sid *msg_srv6_local_sid;
        char addr_str[64];
        DEBUG_NEW (RIB, "recv msg_srv6_local_sid: %p.", msgp);
        msg_srv6_local_sid =
          (struct internal_msg_srv6_local_sid *) (msg_header + 1);
        inet_ntop (AF_INET6, &msg_srv6_local_sid->srv6_local_sid_addr,
                   addr_str, sizeof (addr_str));
        DEBUG_NEW (RIB, "recv msg_srv6_local_sid: addr: %s index: %d.",
                   addr_str, msg_srv6_local_sid->srv6_local_sid_addr_index);
        int num_sids = new->rib_info->srv6_local_sid_addr_num;
        int index = msg_srv6_local_sid->srv6_local_sid_addr_index;
        int existing_idx = srv6_local_sid_find (new->rib_info,
                                                &msg_srv6_local_sid->srv6_local_sid_addr);
        if (existing_idx >= 0)
          index = existing_idx;
        /* UINT8_MAX means auto-assign */
        else if (index == UINT8_MAX)
          index = num_sids;
        if (0 <= num_sids && num_sids < MAX_SRV6_LOCAL_SID_ADDR_NUM &&
            0 <= index && index <= num_sids)
          {
            DEBUG_NEW (RIB, "saving at local_sid_addr[%d/%d] success: addr: %s.",
                       index, num_sids, addr_str);
            struct srv6_local_sid_entry *entry = &new->rib_info->srv6_local_sid[index];
            /* keep the previous content: the route nexthop is matched by
               value, so replacing one needs the entry as it was */
            struct srv6_local_sid_entry old_entry = *entry;
            memcpy (&entry->sid, &msg_srv6_local_sid->srv6_local_sid_addr,
                    sizeof (struct in6_addr));
            entry->action = msg_srv6_local_sid->action;
            entry->oif = msg_srv6_local_sid->oif;
            memcpy (&entry->param, &msg_srv6_local_sid->param,
                    sizeof (entry->param));
            if (index == num_sids)
              new->rib_info->srv6_local_sid_addr_num++;
            DEBUG_NEW (RIB, "local_sid_addr_num: %d.",
                       new->rib_info->srv6_local_sid_addr_num);

            /* also install it as a /128 route so that the forwarding path
               can resolve it through the FIB */
            struct internal_msg_route_entry sid_route;
            if (existing_idx >= 0)
              {
                _srv6_local_sid_route_msg (&sid_route, &old_entry);
                _route_entry_del (new, &sid_route);
              }
            _srv6_local_sid_route_msg (&sid_route, entry);
            if (_route_entry_add (new, &sid_route) < 0)
              DEBUG_NEW (RIB, "failed to add local SID route: addr: %s.",
                         addr_str);
          }
        else
          DEBUG_NEW (RIB, "saving at local_sid_addr[%d/%d] failed: addr: %s.",
                     index, num_sids, addr_str);
        break;
      }

    case INTERNAL_MSG_TYPE_SRV6_LOCAL_SID_DEL:
      {
        struct internal_msg_srv6_local_sid *msg_srv6_local_sid;
        char addr_str[64];
        msg_srv6_local_sid =
          (struct internal_msg_srv6_local_sid *) (msg_header + 1);
        inet_ntop (AF_INET6, &msg_srv6_local_sid->srv6_local_sid_addr,
                   addr_str, sizeof (addr_str));
        DEBUG_NEW (RIB, "del local_sid_addr: %s.", addr_str);

        /* drop the /128 route first: its nexthop is matched by value
           against the entry, which srv6_local_sid_del() is about to move */
        int del_idx =
            srv6_local_sid_find (new->rib_info,
                                 &msg_srv6_local_sid->srv6_local_sid_addr);
        if (del_idx >= 0)
          {
            struct internal_msg_route_entry sid_route;
            _srv6_local_sid_route_msg (
                &sid_route, &new->rib_info->srv6_local_sid[del_idx]);
            _route_entry_del (new, &sid_route);
          }

        srv6_local_sid_del (new->rib_info,
                            &msg_srv6_local_sid->srv6_local_sid_addr);
        break;
      }

    case INTERNAL_MSG_TYPE_SRV6_ENCAP_SRC_SET:
      {
        struct internal_msg_srv6_encap_src *msg_encap_src;
        char src_str[INET6_ADDRSTRLEN];
        msg_encap_src = (struct internal_msg_srv6_encap_src *) (msg_header + 1);
        new->rib_info->srv6_tunnel_src_addr = msg_encap_src->src_addr;
        inet_ntop (AF_INET6, &msg_encap_src->src_addr, src_str,
                   sizeof (src_str));
        DEBUG_NEW (RIB, "recv msg_srv6_encap_src_set: %p,"
                   " set srv6_encap_src: %s",
                   msgp, src_str);
        break;
      }
    default:
      DEBUG_NEW (RIB, "recv msg unknown: %p.", msgp);
      break;
    }

  free (msgp);

  /* rebuild FIB from RIB for each routing table */
  for (i = 0; i < ROUTE_TREE_SIZE; i++)
    {
      /* skip NULL pointers and uninitialized RIB trees (family == 0) */
      if (rib_tree_master[i] && new->rib_info->fib_tree[i] &&
          rib_tree_master[i]->family != 0)
        {
          if (rebuild_fib_from_rib (rib_tree_master[i],
                                    new->rib_info->fib_tree[i]) != 0)
            DEBUG_NEW (RIB, "failed to rebuild FIB[%d] (family=%d)", i,
                               rib_tree_master[i]->family);
        }
    }
  rib_replace (new);
}

#if 0
void
rib_manager_send_message (void *msgp, struct shell *shell)
{
  if (msg_queue_rib)
    {
      DEBUG_NEW (RIB, "%s: sending message %p.", __func__, msgp);
      rte_ring_enqueue (msg_queue_rib, msgp);
    }
  else
    {
      fprintf (shell->terminal, "can't send message to rib: queue: NULL.%s",
               shell->NL);
    }
}
#endif

static __thread uint64_t loop_counter = 0;
static __thread time_t last_fdb_aging_time = 0;

int
rib_manager (void *arg)
{
  int i;
  void *msgp;
  unsigned lcore_id = rte_lcore_id ();

  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);
  DEBUG_NEW (RIB, "%s: started.", __func__);

  /* the tx_buffer_per_q is initialized in rib_manager. */
  memset (tx_buffer_per_q, 0, sizeof (tx_buffer_per_q));

  /* initialize */
  msg_queue_rib =
      rte_ring_create ("msg_queue_rib", 1024, SOCKET_ID_ANY, RING_F_SC_DEQ);
  DEBUG_NEW (RIB, "msg_queue_rib: %p", msg_queue_rib);
  if (! msg_queue_rib)
  {
    printf ("%s[%d]: %s: failed to start.\n", __FILE__, __LINE__, __func__);
    DEBUG_NEW (RIB, "rte_ring_create failed: %s", rte_strerror (rte_errno));
    return -1;
  }

  int thread_id;
  thread_id = thread_lookup_by_lcore (rib_manager, lcore_id);
  thread_register_loop_counter (thread_id, &loop_counter);

  if (IS_LTHREAD ())
    DEBUG_NEW (RIB, "started as a lthread.");
  else
    DEBUG_NEW (RIB, "started on lcore: %d.", lcore_id);

#if 0
  /* This is the rcu producer. do we need to register ? */
#if HAVE_LIBURCU_QSBR
  if (! IS_LTHREAD ())
    urcu_qsbr_register_thread ();
#endif /*HAVE_LIBURCU_QSBR*/
#endif

  /* initialize fdb aging timer */
  last_fdb_aging_time = time (NULL);

  nexthop_init ();

  while (! force_quit && ! force_stop[lcore_id])
    {
      if (IS_LTHREAD ())
        lthread_sleep (0); // yield.
#if 0
      DEBUG_NEW (RIB, "%s: schedule (loop: %lu).",
                 __func__, loop_counter);
#endif

      /* rte_ring_dequeue_burst() or not */
#if 0
      msgp = internal_msg_recv (msg_queue_rib);
      if (msgp)
        rib_manager_process_message (msgp);
#else
#define RIB_RING_BURST_SIZE 512
      int num = 0;
      struct internal_msg_header *msg_table[RIB_RING_BURST_SIZE];
      num = internal_msg_recv_burst (msg_queue_rib, msg_table,
                                     RIB_RING_BURST_SIZE);
      for (int i = 0; i < num; i++)
        {
          msgp = msg_table[i];
          rib_manager_process_message (msgp);
        }
#endif

      /* fdb aging process - run every FDB_AGING_INTERVAL seconds */
      time_t current_time = time (NULL);
      struct rib *current_rib = NULL;
#if HAVE_LIBURCU_QSBR
      /* This is the rcu producer.
         I think we don't need to lock/unlock/quiescent. */
        {
          //urcu_qsbr_read_lock ();
          current_rib = rcu_dereference (rcu_global_ptr_rib);
        }
#endif /*HAVE_LIBURCU_QSBR*/

      if (current_time - last_fdb_aging_time >= FDB_AGING_INTERVAL
          && current_rib && current_rib->rib_info)
        {
          last_fdb_aging_time = current_time;
          /* Skip the expensive copy-on-write when there is nothing to age out. */
          if (fdb_count_expired (current_rib->rib_info, FDB_AGING_TIME_DEFAULT) > 0)
            {
              /* Run aging through the normal RIB-update path. */
              struct internal_msg_fdb_aging fdb_aging_payload
                  = {.aging_time = FDB_AGING_TIME_DEFAULT};
              struct internal_msg_header *msgp
                  = internal_msg_create (INTERNAL_MSG_TYPE_FDB_AGING,
                                         &fdb_aging_payload, sizeof (fdb_aging_payload));
              if (msgp)
                rib_manager_process_message (msgp);
            }
        }

#if HAVE_LIBURCU_QSBR
      /* This is the rcu producer.
         I think we don't need to lock/unlock/quiescent. */
        {
          //urcu_qsbr_read_unlock ();
          //urcu_qsbr_quiescent_state ();
        }
#endif /*HAVE_LIBURCU_QSBR*/

      loop_counter++;
    }

  rte_ring_free (msg_queue_rib);
  for (i = 0; i < ROUTE_TREE_SIZE; i++)
    {
      rib_free (rib_tree_master[i]);
    }

  nexthop_cleanup ();

  DEBUG_NEW (RIB, "%s: terminating.", __func__);
  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);

#if 0
  /* This is the rcu producer. do we need to register ? */
#if HAVE_LIBURCU_QSBR
  if (! IS_LTHREAD ())
    urcu_qsbr_unregister_thread ();
#endif /*HAVE_LIBURCU_QSBR*/
#endif

  return 0;
}
