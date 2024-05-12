
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>

#include <rte_common.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_random.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_string_fns.h>

#include <zcmdsh/command.h>
#include <zcmdsh/shell.h>

#include "l2fwd.h"

#include "soft_dplane.h"

int l2fwd_launch_one_lcore (__rte_unused void *dummy);

#define rte_exit(x, ...) \
  do { printf (__VA_ARGS__); return -1; } while (0)

#define rte_warn(x, ...) \
  do { printf (__VA_ARGS__); } while (0)

int
l2fwd_init (int argc, char **argv)
{
  struct lcore_queue_conf *qconf;
  int ret;
  uint16_t nb_ports;
  uint16_t nb_ports_available = 0;
  uint16_t portid, last_port;
  unsigned lcore_id, rx_lcore_id;
  unsigned nb_ports_in_mask = 0;
  unsigned int nb_lcores = 0;
  unsigned int nb_mbufs;

#if 0
  /* Init EAL. 8< */
  ret = rte_eal_init (argc, argv);
  if (ret < 0)
    rte_exit (EXIT_FAILURE, "Invalid EAL arguments\n");
  argc -= ret;
  argv += ret;

  force_quit = false;
  signal (SIGINT, signal_handler);
  signal (SIGTERM, signal_handler);

  /* parse application arguments (after the EAL ones) */
  ret = l2fwd_parse_args (argc, argv);
  if (ret < 0)
    rte_exit (EXIT_FAILURE, "Invalid L2FWD arguments\n");
  /* >8 End of init EAL. */

  printf ("MAC updating %s\n", mac_updating ? "enabled" : "disabled");
#endif

  printf ("debug: %s: line %d\n", __func__, __LINE__);

  /* convert to number of cycles */
  timer_period *= rte_get_timer_hz ();

  nb_ports = rte_eth_dev_count_avail ();
  if (nb_ports == 0)
    rte_exit (EXIT_FAILURE, "No Ethernet ports - bye\n");

  printf ("debug: %s: line %d\n", __func__, __LINE__);

  if (port_pair_params != NULL)
    {
      if (check_port_pair_config () < 0)
        rte_exit (EXIT_FAILURE, "Invalid port pair config\n");
    }

  printf ("debug: %s: line %d\n", __func__, __LINE__);

  /* check port mask to possible port mask */
  if (l2fwd_enabled_port_mask & ~((1 << nb_ports) - 1))
    rte_exit (EXIT_FAILURE, "Invalid portmask; possible (0x%x)\n",
              (1 << nb_ports) - 1);

  /* Initialization of the driver. 8< */

  printf ("debug: %s: line %d\n", __func__, __LINE__);

  /* reset l2fwd_dst_ports */
  for (portid = 0; portid < RTE_MAX_ETHPORTS; portid++)
    l2fwd_dst_ports[portid] = 0;
  last_port = 0;

  printf ("debug: %s: line %d\n", __func__, __LINE__);

  /* populate destination port details */
  if (port_pair_params != NULL)
    {
      uint16_t idx, p;

      for (idx = 0; idx < (nb_port_pair_params << 1); idx++)
        {
          p = idx & 1;
          portid = port_pair_params[idx >> 1].port[p];
          l2fwd_dst_ports[portid] = port_pair_params[idx >> 1].port[p ^ 1];
        }
    }
  else
    {
      RTE_ETH_FOREACH_DEV (portid)
      {
        /* skip ports that are not enabled */
        if ((l2fwd_enabled_port_mask & (1 << portid)) == 0)
          continue;

        if (nb_ports_in_mask % 2)
          {
            l2fwd_dst_ports[portid] = last_port;
            l2fwd_dst_ports[last_port] = portid;
          }
        else
          {
            last_port = portid;
          }

        nb_ports_in_mask++;
      }
      if (nb_ports_in_mask % 2)
        {
          printf ("Notice: odd number of ports in portmask.\n");
          l2fwd_dst_ports[last_port] = last_port;
        }
    }
  /* >8 End of initialization of the driver. */

  printf ("debug: %s: line %d\n", __func__, __LINE__);

  for (rx_lcore_id = 0; rx_lcore_id < RTE_MAX_LCORE; rx_lcore_id++)
    {
      lcore_queue_conf[rx_lcore_id].n_rx_port = 0;
    }


  printf ("debug: %s: line %d\n", __func__, __LINE__); fflush (stdout);

  rx_lcore_id = 0;
  qconf = NULL;

  /* Initialize the port/queue configuration of each logical core */
  RTE_ETH_FOREACH_DEV (portid)
  {
    /* skip ports that are not enabled */
    if ((l2fwd_enabled_port_mask & (1 << portid)) == 0)
      continue;

    /* get the lcore_id for this port */
    while (rte_lcore_is_enabled (rx_lcore_id) == 0 ||
           lcore_workers[rx_lcore_id].func != l2fwd_launch_one_lcore ||
           lcore_queue_conf[rx_lcore_id].n_rx_port == l2fwd_rx_queue_per_lcore)
      {
        rx_lcore_id++;
        if (rx_lcore_id >= RTE_MAX_LCORE)
          rte_exit (EXIT_FAILURE, "Not enough cores\n");
        printf ("try lcore %d for rx_lcore of port %d...\n",
                rx_lcore_id, portid);
      }

    if (qconf != &lcore_queue_conf[rx_lcore_id])
      {
        /* Assigned a new logical core in the loop above. */
        qconf = &lcore_queue_conf[rx_lcore_id];
        nb_lcores++;
      }

    qconf->rx_port_list[qconf->n_rx_port] = portid;
    qconf->n_rx_port++;
    printf ("Lcore %u: RX port %u TX port %u\n", rx_lcore_id, portid,
            l2fwd_dst_ports[portid]);
  }

  printf ("debug: %s: line %d\n", __func__, __LINE__); fflush (stdout);

  nb_mbufs = RTE_MAX (nb_ports * (nb_rxd + nb_txd + MAX_PKT_BURST +
                                  nb_lcores * MEMPOOL_CACHE_SIZE),
                      8192U);

  /* Create the mbuf pool. 8< */
  if (l2fwd_pktmbuf_pool == NULL)
    l2fwd_pktmbuf_pool =
      rte_pktmbuf_pool_create ("mbuf_pool", nb_mbufs, MEMPOOL_CACHE_SIZE, 0,
                               RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id ());
  if (l2fwd_pktmbuf_pool == NULL)
    rte_exit (EXIT_FAILURE, "Cannot init mbuf pool\n");
  /* >8 End of create the mbuf pool. */

  printf ("debug: %s: line %d\n", __func__, __LINE__); fflush (stdout);

  /* Initialise each port */
  RTE_ETH_FOREACH_DEV (portid)
  {
    struct rte_eth_rxconf rxq_conf;
    struct rte_eth_txconf txq_conf;
    struct rte_eth_conf local_port_conf = port_conf;
    struct rte_eth_dev_info dev_info;

    /* skip ports that are not enabled */
    if ((l2fwd_enabled_port_mask & (1 << portid)) == 0)
      {
        printf ("Skipping disabled port %u\n", portid);
        continue;
      }
    nb_ports_available++;

    /* init port */
    printf ("Initializing port %u... ", portid);
    fflush (stdout);

    ret = rte_eth_dev_info_get (portid, &dev_info);
    if (ret != 0)
      rte_exit (EXIT_FAILURE,
                "Error during getting device (port %u) info: %s\n", portid,
                strerror (-ret));

    if (dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE)
      local_port_conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE;
    /* Configure the number of queues for a port. */
    ret = rte_eth_dev_configure (portid, 1, 1, &local_port_conf);
    if (ret < 0)
      rte_warn (EXIT_FAILURE, "Cannot configure device: err=%d, port=%u\n",
                ret, portid);
    /* >8 End of configuration of the number of queues for a port. */

    ret = rte_eth_dev_adjust_nb_rx_tx_desc (portid, &nb_rxd, &nb_txd);
    if (ret < 0)
      rte_warn (EXIT_FAILURE,
                "Cannot adjust number of descriptors: err=%d, port=%u\n", ret,
                portid);

    ret = rte_eth_macaddr_get (portid, &l2fwd_ports_eth_addr[portid]);
    if (ret < 0)
      rte_warn (EXIT_FAILURE, "Cannot get MAC address: err=%d, port=%u\n", ret,
                portid);

    /* init one RX queue */
    fflush (stdout);
    rxq_conf = dev_info.default_rxconf;
    rxq_conf.offloads = local_port_conf.rxmode.offloads;
    /* RX queue setup. 8< */
    ret = rte_eth_rx_queue_setup (portid, 0, nb_rxd,
                                  rte_eth_dev_socket_id (portid), &rxq_conf,
                                  l2fwd_pktmbuf_pool);
    if (ret < 0)
      rte_warn (EXIT_FAILURE, "rte_eth_rx_queue_setup:err=%d, port=%u\n", ret,
                portid);
    /* >8 End of RX queue setup. */

    /* Init one TX queue on each port. 8< */
    fflush (stdout);
    txq_conf = dev_info.default_txconf;
    txq_conf.offloads = local_port_conf.txmode.offloads;
    ret = rte_eth_tx_queue_setup (portid, 0, nb_txd,
                                  rte_eth_dev_socket_id (portid), &txq_conf);
    if (ret < 0)
      rte_warn (EXIT_FAILURE, "rte_eth_tx_queue_setup:err=%d, port=%u\n", ret,
                portid);
    /* >8 End of init one TX queue on each port. */

    /* Initialize TX buffers */
    if (tx_buffer[portid] == NULL)
      tx_buffer[portid] = rte_zmalloc_socket (
        "tx_buffer", RTE_ETH_TX_BUFFER_SIZE (MAX_PKT_BURST), 0,
        rte_eth_dev_socket_id (portid));
    if (tx_buffer[portid] == NULL)
      rte_warn (EXIT_FAILURE, "Cannot allocate buffer for tx on port %u\n",
                portid);

    rte_eth_tx_buffer_init (tx_buffer[portid], MAX_PKT_BURST);

    ret = rte_eth_tx_buffer_set_err_callback (
        tx_buffer[portid], rte_eth_tx_buffer_count_callback,
        &port_statistics[portid].dropped);
    if (ret < 0)
      rte_warn (EXIT_FAILURE,
                "Cannot set error callback for tx buffer on port %u\n",
                portid);

    ret = rte_eth_dev_set_ptypes (portid, RTE_PTYPE_UNKNOWN, NULL, 0);
    if (ret < 0)
      printf ("Port %u, Failed to disable Ptype parsing\n", portid);
    /* Start device */
    ret = rte_eth_dev_start (portid);
    if (ret < 0)
      rte_warn (EXIT_FAILURE, "rte_eth_dev_start:err=%d, port=%u\n", ret,
                portid);

    printf ("done: \n");
    if (promiscuous_on)
      {
        ret = rte_eth_promiscuous_enable (portid);
        if (ret != 0)
          rte_warn (EXIT_FAILURE,
                    "rte_eth_promiscuous_enable:err=%s, port=%u\n",
                    rte_strerror (-ret), portid);
      }

    printf ("Port %u, MAC address: " RTE_ETHER_ADDR_PRT_FMT "\n\n", portid,
            RTE_ETHER_ADDR_BYTES (&l2fwd_ports_eth_addr[portid]));

    /* initialize port stats */
    memset (&port_statistics, 0, sizeof (port_statistics));
  }

  printf ("debug: %s: line %d\n", __func__, __LINE__); fflush (stdout);

  if (! nb_ports_available)
    {
      rte_warn (EXIT_FAILURE,
                "All available ports are disabled. Please set portmask.\n");
    }

  printf ("debug: %s: line %d\n", __func__, __LINE__); fflush (stdout);

#if 0
  check_all_ports_link_status (l2fwd_enabled_port_mask);

  ret = 0;
  /* launch per-lcore init on every lcore */
  rte_eal_mp_remote_launch (l2fwd_launch_one_lcore, NULL, CALL_MAIN);
  RTE_LCORE_FOREACH_WORKER (lcore_id)
  {
    if (rte_eal_wait_lcore (lcore_id) < 0)
      {
        ret = -1;
        break;
      }
  }

  RTE_ETH_FOREACH_DEV (portid)
  {
    if ((l2fwd_enabled_port_mask & (1 << portid)) == 0)
      continue;
    printf ("Closing port %d...", portid);
    ret = rte_eth_dev_stop (portid);
    if (ret != 0)
      printf ("rte_eth_dev_stop: err=%d, port=%d\n", ret, portid);
    rte_eth_dev_close (portid);
    printf (" Done\n");
  }

  /* clean up the EAL */
  rte_eal_cleanup ();
  printf ("Bye...\n");
#endif

  return ret;
}
