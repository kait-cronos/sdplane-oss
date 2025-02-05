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

/*
 * NetTLP specific header
 */
struct nettlp_hdr {
        uint16_t        seq;
        uint32_t        tstamp;
} __attribute__((packed));

struct tlp_hdr {
        uint8_t         fmt_type;       /* Formant and Type */
        uint8_t         tclass;         /* Traffic Class */
        uint16_t        falen;          /* Flag, Attr, Reseved, and Length */
} __attribute__((packed));

struct tlp_mr_hdr {
        struct tlp_hdr tlp;

        uint16_t requester;
        uint8_t tag;
#if __BYTE_ORDER == __LITTLE_ENDIAN
        uint8_t fstdw : 4;
        uint8_t lstdw : 4;
#elif __BYTE_ORDER == __BIG_ENDIAN
        uint8_t lstdw : 4;
        uint8_t fstdw : 4;
#else
# error "Please fix <bits/endian.h>"
#endif

} __attribute__((packed));



int tx_portid;
struct rte_ether_addr local_ether;
struct rte_ether_addr remote_ether;
struct in_addr local_addr;
struct in_addr remote_addr;
uint16_t src_port;
uint16_t dst_port;
uint16_t bus_number;
uint16_t dev_number;
uint8_t pci_tag; /* PCI tag value */
uintptr_t memory_addr; /* DMA memory address */
int size;
int max_payload_size; /*Max Payload Size */
char payload_string[4096];

void
nettlp_send_dma_write ()
{
  uint16_t tx_queueid;
  struct rte_mbuf *m;
  uint16_t udp_length;
  uint16_t length;
  struct rte_ether_hdr *eth;
  struct rte_ipv4_hdr *ipv4;
  struct rte_udp_hdr *udp;
  struct rte_eth_dev_tx_buffer *buffer;

  struct nettlp_hdr *nh;
  struct tlp_mr_hdr *mh;

  tx_queueid = lcore_id;
  DEBUG_SDPLANE_LOG (NETTLP, "send DMA write: to port: %d queue %d.",
                     tx_portid, tx_queueid);
  m = rte_pktmbuf_alloc (l2fwd_pktmbuf_pool);
  udp_length = sizeof (struct rte_udp_hdr) + sizeof (struct nettlp_hdr) +
               sizeof (struct tlp_mr_hdr) + sizeof (uint64_t) + size;
  length = sizeof (struct rte_ipv4_hdr) + udp_length;
  rte_pktmbuf_append (m, sizeof (struct rte_ether_hdr) + length);

  eth = rte_pktmbuf_mtod (m, struct rte_ether_hdr *);
  memset (eth, 0, length);
  ipv4 = (struct rte_ipv4_hdr *) (eth + 1);
  udp = (struct rte_udp_hdr *) (ipv4 + 1);

  eth->ether_type = rte_cpu_to_be_16 (RTE_ETHER_TYPE_IPV4);
  rte_ether_addr_copy (&local_ether, &eth->src_addr);
  rte_ether_addr_copy (&remote_ether, &eth->dst_addr);

  ipv4->version_ihl = 0x45;
  ipv4->total_length = rte_cpu_to_be_16 (length);
  ipv4->src_addr = local_addr.s_addr;
  ipv4->dst_addr = remote_addr.s_addr;
  ipv4->next_proto_id = IPPROTO_UDP;
  ipv4->hdr_checksum = rte_ipv4_cksum (ipv4);

  udp->src_port = rte_cpu_to_be_16 (src_port);
  udp->dst_port = rte_cpu_to_be_16 (dst_port);
  udp->dgram_len = rte_cpu_to_be_16 (udp_length);

  nh = (struct nettlp_hdr *) (udp + 1);
  mh = (struct tlp_mr_hdr *) (nh + 1);

#define TLP_TYPE_MASK           0x1F
#define TLP_TYPE_MRd            0x00
#define TLP_TYPE_MRdLk          0x01
#define TLP_TYPE_MWr            0x00
#define TLP_TYPE_Cpl            0x0A

#define tlp_set_type(ft, v) ft = ((ft & ~TLP_TYPE_MASK) | (v & TLP_TYPE_MASK))

#define TLP_FMT_DW_MASK         0x20
#define TLP_FMT_3DW             0x00
#define TLP_FMT_4DW             0x20

#define TLP_FMT_DATA_MASK       0x40
#define TLP_FMT_WO_DATA         0x00
#define TLP_FMT_W_DATA          0x40

#define tlp_set_fmt(ft, dw, wd)                                         \
        (ft) = (((ft) & ~(TLP_FMT_DW_MASK | TLP_FMT_DATA_MASK)) |       \
                ((dw) & TLP_FMT_DW_MASK) | ((wd) & TLP_FMT_DATA_MASK))

  tlp_set_type(mh->tlp.fmt_type, TLP_TYPE_MWr);
  tlp_set_fmt(mh->tlp.fmt_type, TLP_FMT_4DW, TLP_FMT_W_DATA);

  uint64_t *dst_addr64;
  uint32_t *dst_addr32;
  dst_addr64 = (uint64_t *) (mh + 1);
  *dst_addr64 = rte_cpu_to_be_64 (memory_addr & 0xFFFFFFFFFFFFFFFC);

  uint8_t *data;
  data = (uint8_t *) (dst_addr64 + 1);
  memcpy (data, payload_string, size);

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

  rte_ring_free (msg_queue_nettlp);

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

CLI_COMMAND2 (show_nettlp,
              "show nettlp",
              SHOW_HELP,
              "NetTLP information.\n")
{
  struct shell *shell = (struct shell *) context;
  char local[32], remote[32];

  fprintf (shell->terminal, "TX port-id: %d.%s", tx_portid, shell->NL);

  rte_ether_format_addr (local, sizeof (local), &local_ether);
  rte_ether_format_addr (remote, sizeof (remote), &remote_ether);
  fprintf (shell->terminal, "local-ether: %s%s", local, shell->NL);
  fprintf (shell->terminal, "remote-ether: %s%s", remote, shell->NL);

  inet_ntop (AF_INET, &local_addr, local, sizeof (local));
  inet_ntop (AF_INET, &remote_addr, remote, sizeof (remote));
  fprintf (shell->terminal, "local-addr: %s%s", local, shell->NL);
  fprintf (shell->terminal, "remote-addr: %s%s", remote, shell->NL);

  fprintf (shell->terminal, "UDP src-port: %hu.%s", src_port, shell->NL);
  fprintf (shell->terminal, "UDP dst-port: %hu.%s", dst_port, shell->NL);

  fprintf (shell->terminal, "bus_number: %hx:%hx.%s",
           bus_number, dev_number, shell->NL);
  fprintf (shell->terminal, "pci-tag: %d.%s", pci_tag, shell->NL);
  fprintf (shell->terminal, "memory-addr: %p.%s",
           (void *) memory_addr, shell->NL);
  fprintf (shell->terminal, "size: %d.%s", size, shell->NL);
  fprintf (shell->terminal, "MaxPayloadSize: %d.%s",
           max_payload_size, shell->NL);
  fprintf (shell->terminal, "payload-string: %s.%s",
           payload_string, shell->NL);
}

CLI_COMMAND2 (set_nettlp_ether_local_remote,
              "set nettlp ether (local-addr|remote-addr) <WORD>",
              SET_HELP,
              "NetTLP information.\n",
              "local ethernet source address.\n",
              "remote ethernet destination address.\n",
              "Specify ethernet address.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  struct rte_ether_addr *dst;
  int ret;

  if (! strcmp (argv[3], "local-addr"))
    dst = &local_ether;
  else if (! strcmp (argv[3], "remote-addr"))
    dst = &remote_ether;

  ret = rte_ether_unformat_addr (argv[4], dst);
  if (ret < 0)
    {
      fprintf (shell->terminal, "invalid address format: %s.%s",
               argv[4], shell->NL);
      return;
    }

  fprintf (shell->terminal, "set %s: %s.%s",
           argv[3], argv[4], shell->NL);
}


CLI_COMMAND2 (set_nettlp_local_remote,
              "set nettlp (local-addr|remote-addr) A.B.C.D",
              SET_HELP,
              "NetTLP information.\n",
              "local IPv4 source address.\n",
              "remote IPv4 destination address.\n",
              "Specify IPv4 address.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  struct in_addr *dst;
  int ret;

  if (! strcmp (argv[2], "local-addr"))
    dst = &local_addr;
  else if (! strcmp (argv[2], "remote-addr"))
    dst = &remote_addr;

  ret = inet_pton (AF_INET, argv[3], dst);
  if (ret == 0)
    {
      fprintf (shell->terminal, "invalid address format: %s.%s",
               argv[3], shell->NL);
      return;
    }
  if (ret < 0)
    {
      fprintf (shell->terminal, "inet_pton() failed: %s.%s",
               strerror (errno), shell->NL);
      return;
    }

  fprintf (shell->terminal, "set %s: %s.%s",
           argv[2], argv[3], shell->NL);
}

CLI_COMMAND2 (set_nettlp_bus_number,
              "set nettlp bus-number <0-65535> device-number <0-65535>",
              SET_HELP,
              "NetTLP information.\n",
              "Set bus-number.\n",
              "Specify bus-number (i.e. 0x1234 in Hex).\n",
              "Set device-number.\n",
              "Specify device-number (i.e. 0x1234 in Hex).\n"
              )
{
  struct shell *shell = (struct shell *) context;
  int ret;

  bus_number = strtol (argv[3], NULL, 0);
  dev_number = strtol (argv[5], NULL, 0);

  fprintf (shell->terminal, "bus_number %hx:%hx.%s",
           bus_number, dev_number, shell->NL);
}

CLI_COMMAND2 (set_nettlp_pci_tag,
              "set nettlp pci-tag <0-255>",
              SET_HELP,
              "NetTLP information.\n",
              "Set PCI tag.\n",
              "Specify PCI tag.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  pci_tag = strtol (argv[3], NULL, 0);
  fprintf (shell->terminal, "pci-tag: %d.%s", pci_tag, shell->NL);
}

CLI_COMMAND2 (set_nettlp_txportid,
              "set nettlp tx-portid <0-128>",
              SET_HELP,
              "NetTLP information.\n",
              "Set transmission port-id.\n",
              "Specify transmission port-id.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  tx_portid = strtol (argv[3], NULL, 0);
  fprintf (shell->terminal, "tx-portid: %d.%s", tx_portid, shell->NL);
}

CLI_COMMAND2 (set_nettlp_udp_port,
              "set nettlp udp (src-port|dst-port) <0-65535>",
              SET_HELP,
              "NetTLP information.\n",
              "UDP information.\n",
              "Set UDP src port.\n",
              "Set UDP dst port.\n",
              "Specify UDP port.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  uint16_t *dst;

  if (! strcmp (argv[3], "src-port"))
    dst = &src_port;
  else if (! strcmp (argv[3], "dst-port"))
    dst = &dst_port;

  *dst = strtol (argv[4], NULL, 0);
  fprintf (shell->terminal, "UDP %s: %hu.%s", argv[3], *dst, shell->NL);
}

CLI_COMMAND2 (set_nettlp_memory_addr,
              "set nettlp memory-address <WORD>",
              SET_HELP,
              "NetTLP information.\n",
              "Set core memory address.\n",
              "Specify core memory address.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  memory_addr = strtoul (argv[3], NULL, 0);
  fprintf (shell->terminal, "memory-addr: %p.%s",
           (void *) memory_addr, shell->NL);
}

CLI_COMMAND2 (set_nettlp_size,
              "set nettlp size <0-4096>",
              SET_HELP,
              "NetTLP information.\n",
              "Set size.\n",
              "Specify size.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  size = strtoul (argv[3], NULL, 0);
  fprintf (shell->terminal, "size: %d.%s", size, shell->NL);
}

CLI_COMMAND2 (set_nettlp_max_payload_size,
              "set nettlp max-payload-size <0-4096>",
              SET_HELP,
              "NetTLP information.\n",
              "Set Max Payload Size (mps).\n",
              "Specify Max Payload Size.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  max_payload_size = strtoul (argv[3], NULL, 0);
  fprintf (shell->terminal, "MaxPayloadSize: %d.%s",
           max_payload_size, shell->NL);
}

CLI_COMMAND2 (set_nettlp_payload_string,
              "set nettlp payload-string <LINE>",
              SET_HELP,
              "NetTLP information.\n",
              "Set payload string.\n",
              "Specify payload string.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  snprintf (payload_string, sizeof (payload_string),
            "%s", argv[3]);
  fprintf (shell->terminal, "payload-string: %s.%s",
           payload_string, shell->NL);
}

void
nettlp_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, nettlp_send_dma_write);
  INSTALL_COMMAND2 (cmdset, show_nettlp);
  INSTALL_COMMAND2 (cmdset, set_nettlp_ether_local_remote);
  INSTALL_COMMAND2 (cmdset, set_nettlp_local_remote);
  INSTALL_COMMAND2 (cmdset, set_nettlp_bus_number);
  INSTALL_COMMAND2 (cmdset, set_nettlp_pci_tag);
  INSTALL_COMMAND2 (cmdset, set_nettlp_txportid);
  INSTALL_COMMAND2 (cmdset, set_nettlp_udp_port);
  INSTALL_COMMAND2 (cmdset, set_nettlp_memory_addr);
  INSTALL_COMMAND2 (cmdset, set_nettlp_size);
  INSTALL_COMMAND2 (cmdset, set_nettlp_max_payload_size);
  INSTALL_COMMAND2 (cmdset, set_nettlp_payload_string);
}

