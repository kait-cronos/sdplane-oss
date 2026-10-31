#include "include.h"

#include <lthread.h>

#include <rte_common.h>
#include <rte_ether.h>
#include <rte_ip.h>

#include <sdplane/debug.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>

#include "sdplane.h"
#include "thread_info.h"
#include "rib_manager.h"
#include "igmp.h"

/* IGMP Report via Kernel by injecting uery */

#define IGMP_QUERY_MAX_RESP_CODE 100 // RFC 3376
#define IGMP_QUERY_QRV           2   // RFC 3376
#define IGMP_QUERY_QQIC          125 // RFC 3376

extern int lthread_core;

int igmp_report_interval = IGMP_REPORT_INTERVAL_DEFAULT;

static uint64_t loop_counter_igmp_report = 0;

struct igmpv3_query
{
  uint8_t type;
  uint8_t max_resp_code;
  uint16_t cksum;
  uint32_t group;
  uint8_t s_qrv;
  uint8_t qqic;
  uint16_t nsrcs;
} __attribute__ ((packed));

struct igmp_query_frame
{
  struct rte_ether_hdr eth;
  struct rte_ipv4_hdr ip;
  uint8_t ra[4];
  struct igmpv3_query igmp;
} __attribute__ ((packed));

static struct igmp_query_frame igmp_query;

static void
igmp_query_init (struct igmp_query_frame *f)
{
  struct rte_ether_addr dst = { { 0x01, 0x00, 0x5e, 0x00, 0x00, 0x01 } };
  struct rte_ether_addr src = { { 0x02, 0x00, 0x00, 0x00, 0x00, 0x01 } };
  uint8_t ra[] = { 0x94, 0x04, 0x00, 0x00 }; // RFC 2113

  memset (f, 0, sizeof (*f));

  f->eth.dst_addr = dst;
  f->eth.src_addr = src;
  f->eth.ether_type = rte_cpu_to_be_16 (RTE_ETHER_TYPE_IPV4);

  // the source 0.0.0.0 is accepted by the kernel without rp_filter.
  f->ip.version_ihl = (4 << 4) | ((sizeof (f->ip) + sizeof (f->ra)) / 4);
  f->ip.type_of_service = 0xc0;
  f->ip.total_length =
      rte_cpu_to_be_16 (sizeof (f->ip) + sizeof (f->ra) + sizeof (f->igmp));
  f->ip.time_to_live = 1;
  f->ip.next_proto_id = IPPROTO_IGMP;
  f->ip.src_addr = 0;
  f->ip.dst_addr = rte_cpu_to_be_32 (INADDR_ALLHOSTS_GROUP);
  memcpy (f->ra, ra, sizeof (f->ra));
  f->ip.hdr_checksum =
      ~rte_raw_cksum (&f->ip, sizeof (f->ip) + sizeof (f->ra));

  f->igmp.type = 0x11; // membership query
  f->igmp.max_resp_code = IGMP_QUERY_MAX_RESP_CODE;
  f->igmp.group = 0; // general query
  f->igmp.s_qrv = IGMP_QUERY_QRV;
  f->igmp.qqic = IGMP_QUERY_QQIC;
  f->igmp.cksum = ~rte_raw_cksum (&f->igmp, sizeof (f->igmp));
}

static void
igmp_report_query_all (struct rib *rib)
{
  struct router_if *rif;
  int i, j;

  for (i = 0; i < MAX_IGMP_JOIN_GROUPS; i++)
    {
      struct igmp_join *entry = &igmp_join_table[i];

      if (! entry->used)
        continue;

      // a general query per router-if is enough.
      for (j = 0; j < i; j++)
        if (igmp_join_table[j].used &&
            igmp_join_table[j].vswitch_id == entry->vswitch_id)
          break;
      if (j < i)
        continue;

      // look up every time, since the tap may be recreated.
      rif = igmp_router_if_lookup (rib, entry->vswitch_id);
      if (! rif || rif->sockfd < 0)
        continue;

      if (write (rif->sockfd, &igmp_query, sizeof (igmp_query)) < 0)
        {
          DEBUG_NEW (IGMP, "%s: failed to inject a query: %s", rif->tap_name,
                     strerror (errno));
          continue;
        }
      DEBUG_NEW (IGMP, "%s: injected a general query.", rif->tap_name);
    }
}

int
igmp_report (__rte_unused void *dummy)
{
  struct rib *rib = NULL;
  int elapsed = 0;
  int thread_id;

  DEBUG_NEW (IGMP, "started.");

  thread_id = thread_lookup (igmp_report);
  thread_register_loop_counter (thread_id, &loop_counter_igmp_report);

  igmp_query_init (&igmp_query);

  while (! force_quit && ! force_stop[lthread_core])
    {
      lthread_sleep (1000); // yield.
      loop_counter_igmp_report++;

      if (++elapsed < igmp_report_interval)
        continue;
      elapsed = 0;

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_lock ();
      rib = (struct rib *) rcu_dereference (rcu_global_ptr_rib);
#endif /*HAVE_LIBURCU_QSBR*/

      // must not yield while holding the rib.
      igmp_report_query_all (rib);

#if HAVE_LIBURCU_QSBR
      urcu_qsbr_read_unlock ();
      urcu_qsbr_quiescent_state ();
#endif /*HAVE_LIBURCU_QSBR*/
    }

  printf ("%s: finished.\n", __func__);
  return 0;
}
