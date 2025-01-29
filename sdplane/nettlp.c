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

extern struct rte_eth_dev_tx_buffer *tx_buffer_per_q[RTE_MAX_ETHPORTS][RTE_MAX_LCORE];

struct rte_ring *msg_queue_nettlp;

static __thread  unsigned lcore_id;
static __thread uint64_t loop_counter = 0;
static __thread struct rib *rib;

void
nettlp_send_dma_write ()
{
  int tx_portid;
  uint16_t tx_queueid;
  struct rte_mbuf *m;
  uint16_t length;
  struct rte_ether_hdr *eth;
  struct rte_ipv4_hdr *ipv4;
  struct rte_udp_hdr *udp;
  struct rte_eth_dev_tx_buffer *buffer;

  tx_portid = 1;
  tx_queueid = lcore_id;
  DEBUG_SDPLANE_LOG (NETTLP, "send DMA write: to port: %d queue %d.",
                     tx_portid, tx_queueid);
  m = rte_pktmbuf_alloc (l2fwd_pktmbuf_pool);
  length = sizeof (struct rte_ipv4_hdr) + sizeof (struct rte_udp_hdr) + 64;
  rte_pktmbuf_append (m, sizeof (struct rte_ether_hdr) + length);

  eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  memset (eth, 0, length);
  ipv4 = (struct rte_ipv4_hdr *) (eth + 1);
  udp = (struct rte_udp_hdr *) (ipv4 + 1);

  eth->ether_type = rte_cpu_to_be_16 (RTE_ETHER_TYPE_IPV4);

  ipv4->version_ihl = 0x45;
  ipv4->total_length = rte_cpu_to_be_16 (length);
  ipv4->src_addr = rte_cpu_to_be_32 (0x0b0b0b0b);
  ipv4->dst_addr = rte_cpu_to_be_32 (0x0c0c0c0c);
  ipv4->next_proto_id = IPPROTO_UDP;
  ipv4->hdr_checksum = rte_ipv4_cksum (ipv4);

  udp->src_port = rte_cpu_to_be_16 (1030);
  udp->dst_port = rte_cpu_to_be_16 (1040);
  udp->dgram_len = rte_cpu_to_be_16 (sizeof (struct rte_udp_hdr) + 64);

  if (! tx_buffer_per_q[tx_portid][tx_queueid])
    {
      tx_buffer_per_q[tx_portid][tx_queueid] =
        rte_zmalloc_socket ("tx_buffer",
                        RTE_ETH_TX_BUFFER_SIZE (MAX_PKT_BURST), 0,
                        rte_eth_dev_socket_id (tx_portid));
      rte_eth_tx_buffer_init (tx_buffer_per_q[tx_portid][tx_queueid],
                              MAX_PKT_BURST);
    }

  buffer = tx_buffer_per_q[tx_portid][lcore_id];

  if (buffer)
    {
      rte_eth_tx_buffer (tx_portid, tx_queueid, buffer, m);
      rte_eth_tx_buffer_flush (tx_portid, tx_queueid, buffer);
      DEBUG_SDPLANE_LOG (NETTLP, "sent.");
    }
  else
    DEBUG_SDPLANE_LOG (NETTLP, "no tx buffer.");
}

int
nettlp_thread (void *arg)
{
  int ret;
  void *msgp;

  lcore_id = rte_lcore_id ();

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

