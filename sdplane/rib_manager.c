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

struct rte_ring *ring_up[RTE_MAX_ETHPORTS][MAX_RX_QUEUE_PER_LCORE];
struct rte_ring *ring_dn[RTE_MAX_ETHPORTS][MAX_RX_QUEUE_PER_LCORE];

struct rte_eth_dev_tx_buffer *tx_buffer_per_q[RTE_MAX_ETHPORTS][RTE_MAX_LCORE];

extern int lthread_core;
extern volatile bool force_stop[RTE_MAX_LCORE];

struct rte_ring *msg_queue_rib;

void *rcu_global_ptr_rib;
uint64_t rib_rcu_replace = 0;

static __thread struct rib *rib = NULL;

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

  DEBUG_SDPLANE_LOG (RIB, "ver: %d rib: %p rib_info: %p.",
                     new->rib_info->ver, new, new->rib_info);

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

          DEBUG_SDPLANE_LOG (RIB, "new rib: lcore: %d qconf[%d]: port: %d queue: %d",
                         lcore, i, rxq->port_id, rxq->queue_id);
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

          port_qconf[port_qconf_size++] = *rxq;
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
      DEBUG_SDPLANE_LOG (RIB, "port_qconf[%d]: port: %d queue: %d",
                         i, rxq->port_id, rxq->queue_id);
      if (port_nrxq[rxq->port_id] == rxq->queue_id)
        {
          port_nrxq[rxq->port_id]++;
        }
      else
        {
          DEBUG_SDPLANE_LOG (RIB, "unorderd port_qconf[%d]: port: %d queue: %d",
                             i, rxq->port_id, rxq->queue_id);
          return -1;
        }
    }

  struct rte_eth_conf port_conf =
    { .txmode = { .mq_mode = RTE_ETH_MQ_TX_NONE, }, };
  struct rte_eth_dev_info dev_info;

  int ntxq;
  ntxq = max_lcore + 1;
  DEBUG_SDPLANE_LOG (RIB, "max_lcore: %d, ntxq: %d", max_lcore, ntxq);
  int nb_ports;
  nb_ports = rte_eth_dev_count_avail ();
  for (i = 0; i < nb_ports; i++)
    {
      int nrxq;
      nrxq = port_nrxq[i];
      ret = rte_eth_dev_info_get (i, &dev_info);
      if (dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE)
        port_conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE;
      else
        port_conf.txmode.offloads &= (~RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE);
      DEBUG_SDPLANE_LOG (RIB, "port[%d]: dev_configure: nrxq: %d ntxq: %d",
                         i, nrxq, ntxq);
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
          ret = rte_eth_rx_queue_setup (i, j, nb_rxd,
                                        rte_eth_dev_socket_id (i),
                                        &rxq_conf,
                                        l2fwd_pktmbuf_pool);
          DEBUG_SDPLANE_LOG (RIB, "port[%d]: rx_queue_setup: rxq: %d rxd: %d",
                             i, j, nb_rxd);
        }

      txq_conf = dev_info.default_txconf;
      txq_conf.offloads = port_conf.txmode.offloads;

      for (j = 0; j < ntxq; j++)
        {
          ret = rte_eth_tx_queue_setup (i, j, nb_txd,
                                        rte_eth_dev_socket_id (i),
                                        &txq_conf);
          DEBUG_SDPLANE_LOG (RIB, "port[%d]: tx_queue_setup: txq: %d txd: %d",
                             i, j, nb_txd);

          if (! tx_buffer_per_q[i][j])
            {
              tx_buffer_per_q[i][j] =
                rte_zmalloc_socket ("tx_buffer",
                                RTE_ETH_TX_BUFFER_SIZE (MAX_PKT_BURST), 0,
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
              ring_up[rxq->port_id][rxq->queue_id] =
                rte_ring_create (ring_name, RING_TO_TAP_SIZE,
                                 rte_socket_id (),
                                 (RING_F_SP_ENQ | RING_F_SC_DEQ));
              DEBUG_SDPLANE_LOG (RIB, "rib: create: %s: %p",
                                 ring_name,
                                 ring_up[rxq->port_id][rxq->queue_id]);
            }

          if (! ring_dn[rxq->port_id][rxq->queue_id])
            {
              snprintf (ring_name, sizeof (ring_name), "ring_dn[%d][%d]",
                        rxq->port_id, rxq->queue_id);
              ring_dn[rxq->port_id][rxq->queue_id] =
                rte_ring_create (ring_name, RING_TO_TAP_SIZE,
                                 rte_socket_id (),
                                 (RING_F_SP_ENQ | RING_F_SC_DEQ));
              DEBUG_SDPLANE_LOG (RIB, "rib: create: %s: %p",
                                 ring_name,
                                 ring_dn[rxq->port_id][rxq->queue_id]);
            }
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
  DEBUG_SDPLANE_LOG (RIB, "rib: replace: %'lu-th: "
                     "rib: %p -> %p "
                     "rib_info: ver.%d (%p) -> ver.%d (%p)",
                     rib_rcu_replace,
                     old, new,
                     (old && old->rib_info ? old->rib_info->ver : -1),
                     (old ? old->rib_info : NULL),
                     (new && new->rib_info ? new->rib_info->ver : -1),
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

  msg_header = (struct internal_msg_header *) msgp;
  switch (msg_header->type)
    {
    case INTERNAL_MSG_TYPE_PORT_STATUS:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_port_status: %p.", msgp);
      update_port_status (new);
      break;

    case INTERNAL_MSG_TYPE_ETH_LINK:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_eth_link: %p.", msgp);
      /* this message is functionally substituted by the above
         update_port_status(). */
#if 0
      msg_eth_link = (struct internal_msg_eth_link *) (msg_header + 1);
      memcpy (new->link, msg_eth_link->link,
              sizeof (struct rte_eth_link) * RTE_MAX_ETHPORTS);
      for (i = 0; i < RTE_MAX_ETHPORTS; i++)
        {
          if (msg_eth_link->link[i].link_speed ||
              msg_eth_link->link[i].link_duplex ||
              msg_eth_link->link[i].link_autoneg ||
              msg_eth_link->link[i].link_status)
            {
              memcpy (&new->rib_info->port[i].link, &msg_eth_link->link[i],
                      sizeof (struct rte_eth_link));
              new->rib_info->port_size = i + 1;
            }
        }
#endif
      break;

    case INTERNAL_MSG_TYPE_QCONF:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_qconf: %p.", msgp);
      msg_qconf = (struct internal_msg_qconf *) (msg_header + 1);
#if 0
      memcpy (new->qconf, msg_qconf->qconf,
              sizeof (struct sdplane_queue_conf) * RTE_MAX_LCORE);
      //new->rib_info->lcore_size = rte_eth_dev_count_avail ();
#endif
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
                  new->rib_info->lcore_qconf[i].rx_queue_list[j] =
                    msg_qconf->qconf[i].rx_queue_list[j];
                }
            }
        }

      /* for qconf change, we need strict rib_check(). */
      ret = rib_check (new);
      if (ret < 0)
        {
          DEBUG_SDPLANE_LOG (RIB, "rib_check() failed: return.");
          return;
        }

      /* for qconf change, we need an intermittent state to avoid
         a conflict between different cores. */
      /* XXX, we can use smarter intermitent state. */
      struct rib *zero;
      zero = malloc (sizeof (struct rib));
      if (zero)
        {
          memset (zero, 0, sizeof (struct rib));
          rib_replace (zero);
        }

      break;

    case INTERNAL_MSG_TYPE_QCONF2:
      DEBUG_SDPLANE_LOG (RIB, "recv msg_qconf2: %p.", msgp);
      msg_qconf = (struct internal_msg_qconf *) (msg_header + 1);
      new->rib_info->lcore_size = rte_lcore_count ();
      new->rib_info->lcore_qconf[0].nrxq = 0;
      new->rib_info->lcore_qconf[1].nrxq = 0;
      new->rib_info->lcore_qconf[2].nrxq = 2;
      new->rib_info->lcore_qconf[2].rx_queue_list[0].port_id = 0;
      new->rib_info->lcore_qconf[2].rx_queue_list[0].queue_id = 0;
      new->rib_info->lcore_qconf[2].rx_queue_list[1].port_id = 1;
      new->rib_info->lcore_qconf[2].rx_queue_list[1].queue_id = 0;

      /* for qconf change, we need strict rib_check(). */
      ret = rib_check (new);
      if (ret < 0)
        {
          DEBUG_SDPLANE_LOG (RIB, "rib_check() failed: return.");
          return;
        }

      /* for qconf change, we need an intermittent state to avoid
         a conflict between different cores. */
      /* XXX, we can use smarter intermitent state. */
      //struct rib *zero;
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
      //DEBUG_SDPLANE_LOG (RIB, "%s: schedule.", __func__);

      msgp = internal_msg_recv (msg_queue_rib);
      if (msgp)
        rib_manager_process_message (msgp);

      loop_counter++;
    }

  rte_ring_free (msg_queue_rib);

  DEBUG_SDPLANE_LOG (RIB, "%s: terminating.", __func__);
  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);
}
