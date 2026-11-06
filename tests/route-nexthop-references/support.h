#ifndef ROUTE_REFERENCES_TEST_SUPPORT_H
#define ROUTE_REFERENCES_TEST_SUPPORT_H

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

static struct rib_info *
published_info (void)
{
  return ((struct rib *) rcu_global_ptr_rib)->rib_info;
}

static void
process (struct internal_msg_header *message)
{
  assert (message);
  rib_manager_process_message (message);
  rib = rcu_global_ptr_rib;
}

static void
setup (void)
{
  nexthop_init ();
  size_t size = rte_ring_get_memsize (16);
  msg_queue_rib = aligned_alloc (RTE_CACHE_LINE_SIZE, size);
  assert (msg_queue_rib);
  assert (rte_ring_init (msg_queue_rib, "route-references-test", 16, 0) == 0);
  /* Publish an initial RIB without configuring ports. */
  process (internal_msg_create (UINT16_MAX, NULL, 0));
}

static void
check_group (int id, int references)
{
  assert (id >= 0 && id < MAX_NEXTHOP_GRP_SIZE);
  struct nh_group *group = &published_info ()->nexthop.groups[id];
  assert (group->refcnt == references);
  assert (group->nhcnt == (references ? 1 : 0));
}

static void
check_group_count (int expected)
{
  int count = 0;
  for (int i = 0; i < MAX_NEXTHOP_GRP_SIZE; i++)
    {
      struct nh_group *group = &published_info ()->nexthop.groups[i];
      if (group->nhcnt)
        {
          assert (group->refcnt > 0);
          count++;
        }
      else
        assert (group->refcnt == 0);
    }
  assert (count == expected);
}

static struct fib_node *
lookup (int family, const char *address)
{
  uint8_t key[16] = { 0 };
  assert (inet_pton (family, address, key) == 1);
  for (int i = 0; i < ROUTE_TREE_SIZE; i++)
    {
      struct fib_tree *tree = published_info ()->fib_tree[i];
      if (tree && tree->family == family && tree->table_id == RT_TABLE_MAIN)
        return fib_route_lookup (tree, key);
    }
  return NULL;
}

static int
check_route (int family, const char *address, int plen, int oif)
{
  struct fib_node *node = lookup (family, address);
  assert (node && node->keylen == plen && node->table_id == RT_TABLE_MAIN);
  struct nh_info *info =
      nexthop_get_info_by_index (published_info (), node->sdplane_nh_id);
  assert (info && info->oif == oif);
  return node->sdplane_nh_id;
}

static void
cleanup (void)
{
  check_group_count (0);
  for (int i = 0; i < MAX_NEXTHOP_INFO_SIZE; i++)
    assert (published_info ()->nexthop.info_pool[i].refcnt == 0);

  struct rib *current = rcu_global_ptr_rib;
  for (int i = 0; i < ROUTE_TREE_SIZE; i++)
    {
      fib_free (current->rib_info->fib_tree[i]);
      rib_free (rib_tree_master[i]);
      rib_tree_master[i] = NULL;
    }
  fdb_table_destroy (&current->rib_info->fdb_table);
  mdb_table_destroy (&current->rib_info->mdb_table);
  free (current->rib_info);
  free (current);
  rcu_global_ptr_rib = NULL;
  rib = NULL;
  nexthop_cleanup ();
  void *message;
  while ((message = internal_msg_recv (msg_queue_rib)))
    internal_msg_delete (message);
  free (msg_queue_rib);
  msg_queue_rib = NULL;
}

#endif
