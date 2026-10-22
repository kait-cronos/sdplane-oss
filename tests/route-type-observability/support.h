/* Test setup only; route decoding, publication and lookups use production
 * code. */
#ifndef ROUTE_TYPE_TEST_SUPPORT_H
#define ROUTE_TYPE_TEST_SUPPORT_H
#include "include.h"
#include <assert.h>
#include <linux/rtnetlink.h>
#include "sdplane.h"
#include "rib_manager.h"
#include "internal_message.h"
#include "radix.h"
#include "fib.h"
#include "fdb.h"
#include "mdb.h"

char *pid_path;
struct sdplane_process_status process_status;
extern void rib_manager_process_message (void *);
extern struct rib *rib;
struct netlink_sock;
extern struct netlink_sock netlink_cmd;
extern int netlink_read_nlmsg_route (struct netlink_sock *, struct nlmsghdr *);

static struct rib_info *
published_info (void)
{
  return ((struct rib *) rcu_global_ptr_rib)->rib_info;
}

static struct rte_ring *
test_ring (const char *name)
{
  size_t size = rte_ring_get_memsize (16);
  struct rte_ring *r = aligned_alloc (RTE_CACHE_LINE_SIZE, size);
  assert (r && rte_ring_init (r, name, 16, 0) == 0);
  return r;
}

static void
setup (void)
{
  nexthop_init ();
  msg_queue_rib = test_ring ("route-type-test");
  rib_manager_process_message (internal_msg_create (UINT16_MAX, NULL, 0));
  rib = rcu_global_ptr_rib;
  published_info ()->vswitch_size = 1;
  published_info ()->vswitch[0].router_if.ifindex = 11;
}

static void
cleanup (void)
{
  struct rib *current = rcu_global_ptr_rib;
  if (current)
    {
      for (int i = 0; i < ROUTE_TREE_SIZE; i++)
        fib_free (current->rib_info->fib_tree[i]);
      fdb_table_destroy (&current->rib_info->fdb_table);
      mdb_table_destroy (&current->rib_info->mdb_table);
      free (current->rib_info);
      free (current);
    }
  for (int i = 0; i < ROUTE_TREE_SIZE; i++)
    {
      rib_free (rib_tree_master[i]);
      rib_tree_master[i] = NULL;
    }
  rcu_global_ptr_rib = NULL;
  rib = NULL;
  nexthop_cleanup ();
  void *msg;
  while ((msg = internal_msg_recv (msg_queue_rib)))
    internal_msg_delete (msg);
  free (msg_queue_rib);
  msg_queue_rib = NULL;
}

struct route_wire
{
  struct nlmsghdr h;
  struct rtmsg r;
  uint8_t attrs[128];
};

static void
wire_attr (struct route_wire *w, uint16_t type, const void *data, size_t len)
{
  struct rtattr *a = (void *) ((uint8_t *) w + w->h.nlmsg_len);
  assert (w->h.nlmsg_len + RTA_SPACE (len) <= sizeof (*w));
  a->rta_type = type;
  a->rta_len = RTA_LENGTH (len);
  memcpy (RTA_DATA (a), data, len);
  w->h.nlmsg_len += RTA_ALIGN (a->rta_len);
}

static struct route_wire
route_wire (int family, const char *prefix, int plen, uint8_t table,
            uint8_t type)
{
  struct route_wire w = { 0 };
  uint8_t dst[16] = { 0 };
  int oif = 11;
  w.h.nlmsg_len = NLMSG_LENGTH (sizeof (w.r));
  w.h.nlmsg_type = RTM_NEWROUTE;
  w.r.rtm_family = family;
  w.r.rtm_table = table;
  w.r.rtm_type = type;
  w.r.rtm_dst_len = plen;
  assert (inet_pton (family, prefix, dst) == 1);
  wire_attr (&w, RTA_DST, dst, family == AF_INET ? 4 : 16);
  wire_attr (&w, RTA_OIF, &oif, sizeof (oif));
  return w;
}

static void
process (struct internal_msg_header *message)
{
  assert (message);
  rib_manager_process_message (message);
  rib = rcu_global_ptr_rib;
}

static void
import_route (struct route_wire *w)
{
  rib = rcu_global_ptr_rib;
  assert (netlink_read_nlmsg_route (&netlink_cmd, &w->h) == 0);
  struct internal_msg_header *msg = internal_msg_recv (msg_queue_rib);
  assert (msg);
  struct internal_msg_route_entry *entry = internal_msg_body (msg);
  assert (entry->route_type == w->r.rtm_type);
  process (msg);
}

struct rib_route_check
{
  const uint8_t *key;
  int plen;
  uint32_t table;
  uint8_t type;
  int matches;
};

static int
check_rib_route (struct rib_node *n, void *arg)
{
  struct rib_route_check *check = arg;
  struct route_entry *entry = &n->entry;
  size_t key_size = entry->dst.family == AF_INET ? 4 : 16;
  if (entry->dst.plen == check->plen && entry->table_id == check->table &&
      ! memcmp (&entry->dst.dst_ip_addr, check->key, key_size))
    {
      assert (entry->route_type == check->type);
      check->matches++;
    }
  return 0;
}

static void
check_route (int family, const char *prefix, int plen, uint32_t table,
             uint8_t type, const char *address)
{
  uint8_t key[16] = { 0 }, dst[16] = { 0 };
  struct rib_route_check check = { key, plen, table, type, 0 };
  int fib_matches = 0;
  assert (inet_pton (family, prefix, key) == 1);
  assert (inet_pton (family, address, dst) == 1);
  for (int i = 0; i < ROUTE_TREE_SIZE; i++)
    {
      struct rib_tree *r = rib_tree_master[i];
      if (r && r->family == family)
        {
          assert (rib_traverse (r, check_rib_route, &check) == 0);
        }
      struct fib_tree *f = published_info ()->fib_tree[i];
      if (f && f->family == family)
        {
          struct fib_node *n = fib_route_lookup (f, dst);
          if (n && n->table_id == table)
            {
              assert (n->keylen == plen && n->route_type == type);
              fib_matches++;
            }
        }
    }
  assert (check.matches == 1 && fib_matches == 1);
}
#endif
