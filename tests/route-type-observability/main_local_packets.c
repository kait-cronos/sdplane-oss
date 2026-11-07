/* Exercise forwarding with real mbufs and in-memory rings. */
#define _GNU_SOURCE
#include "support.h"
#include <sched.h>
#include "rte_override.h"
static unsigned lcore_id;
#include "net_subr.h"

static void
packet (struct rte_mempool *pool, int family, const char *dst, bool local,
        struct rte_ring *main_ring, struct rte_ring *local_ring)
{
  struct rib_info *info = published_info ();
  struct rte_mbuf *m = rte_pktmbuf_alloc (pool);
  assert (m);
  size_t iplen = family == AF_INET ? sizeof (struct rte_ipv4_hdr)
                                   : sizeof (struct rte_ipv6_hdr);
  struct rte_ether_hdr *eth =
      (void *) rte_pktmbuf_append (m, sizeof (*eth) + iplen);
  assert (eth);
  memset (eth, 0, sizeof (*eth) + iplen);
  struct rte_ipv4_hdr *v4 = NULL;
  struct rte_ipv6_hdr *v6 = NULL;
  if (family == AF_INET)
    {
      eth->ether_type = htons (RTE_ETHER_TYPE_IPV4);
      v4 = (void *) (eth + 1);
      v4->version_ihl = 0x45;
      v4->time_to_live = 64;
      v4->total_length = htons (sizeof (*v4));
      inet_pton (family, dst, &v4->dst_addr);
      v4->hdr_checksum = rte_ipv4_cksum (v4);
    }
  else
    {
      eth->ether_type = htons (RTE_ETHER_TYPE_IPV6);
      v6 = (void *) (eth + 1);
      v6->vtc_flow = htonl (6U << 28);
      v6->hop_limits = 64;
      inet_pton (family, dst, &v6->dst_addr);
    }
  _fwd_cache_invalidate ();
  assert (
      ! _forwarding (m, 0, 0, eth, v4, v6, &info->vswitch[1], NULL, false));
  struct rte_mbuf *out = NULL;
  assert (rte_ring_dequeue (local ? local_ring : main_ring, (void **) &out) ==
          0);
  assert (out && rte_ring_empty (local ? main_ring : local_ring));
  struct rte_ether_hdr *out_eth =
      rte_pktmbuf_mtod (out, struct rte_ether_hdr *);
  if (family == AF_INET)
    {
      struct rte_ipv4_hdr *out_ip = (void *) (out_eth + 1);
      assert (out_ip->time_to_live == (local ? 64 : 63));
      assert (rte_ipv4_cksum (out_ip) == 0);
    }
  else
    assert (((struct rte_ipv6_hdr *) (out_eth + 1))->hop_limits ==
            (local ? 64 : 63));
  if (! local)
    assert (out_eth->dst_addr.addr_bytes[5] == 99);
  rte_pktmbuf_free (out);
  rte_pktmbuf_free (m);
  printf ("PASS: %s %s packet reached expected ring\n",
          family == AF_INET ? "IPv4" : "IPv6",
          local ? "local punt" : "main forward");
}

int
main (void)
{
  cpu_set_t cpus;
  assert (sched_getaffinity (0, sizeof (cpus), &cpus) == 0);
  int cpu = 0;
  while (! CPU_ISSET (cpu, &cpus))
    cpu++;
  char core[40];
  snprintf (core, sizeof (core), "0@%d", cpu);
  char *argv[] = { "routing-packets", "--lcores", core,
                   "--no-huge",       "--no-pci", "--no-shconf",
                   "--no-telemetry",  "-m",       "64" };
  assert (rte_eal_init (sizeof (argv) / sizeof (argv[0]), argv) >= 0);
  struct rte_mempool *pool = rte_pktmbuf_pool_create (
      "routing-packets", 63, 0, 0, RTE_MBUF_DEFAULT_BUF_SIZE, 0);
  assert (pool);
  setup ();
  int families[] = { AF_INET, AF_INET6 };
  char *prefix[] = { "192.0.2.0", "2001:db8::" };
  char *forward[] = { "192.0.2.9", "2001:db8::9" };
  char *local[] = { "192.0.2.1", "2001:db8::1" };
  for (int i = 0; i < 2; i++)
    {
      struct route_wire w = route_wire (families[i], prefix[i], i ? 64 : 24,
                                        RT_TABLE_MAIN, RTN_UNICAST);
      import_route (&w);
      w = route_wire (families[i], local[i], i ? 128 : 32, RT_TABLE_LOCAL,
                      RTN_LOCAL);
      import_route (&w);
    }
  struct rte_ring *main_ring = test_ring ("main-egress");
  struct rte_ring *local_ring = test_ring ("local-punt");
  struct rib_info *info = published_info ();
  info->vswitch_size = 2;
  info->vswitch[0].router_if.ring_up = main_ring;
  info->vswitch[0].router_if.sockfd = 0;
  info->vswitch[1].router_if.ring_up = local_ring;
  info->vswitch[1].router_if.sockfd = 0;
  info->vswitch[1].router_if.ifindex = 22;
  for (int i = 0; i < 2; i++)
    {
      struct neigh_table *t =
          &info->neigh_tables[AF_TO_NEIGH_TABLE (families[i])];
      t->num_entries = 1;
      t->entries[0].family = families[i];
      t->entries[0].ifindex = 11;
      inet_pton (families[i], forward[i], &t->entries[0].ip_addr);
      t->entries[0].mac_addr.addr_bytes[5] = 99;
      packet (pool, families[i], forward[i], false, main_ring, local_ring);
      packet (pool, families[i], local[i], true, main_ring, local_ring);
    }
  free (main_ring);
  free (local_ring);
  cleanup ();
  rte_mempool_free (pool);
  assert (rte_eal_cleanup () == 0);
  return 0;
}
