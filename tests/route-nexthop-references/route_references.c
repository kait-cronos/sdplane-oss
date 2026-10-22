#include "support.h"

static bool fail_tree;
static bool fail_route;
extern struct rib_tree *__real_rib_new (struct rib_tree *);
extern int __real_rib_route_add (struct rib_tree *, const uint8_t *, int,
                                 struct route_entry *);

struct rib_tree *
__wrap_rib_new (struct rib_tree *tree)
{
  if (fail_tree)
    {
      fail_tree = false;
      return NULL;
    }
  return __real_rib_new (tree);
}

int
__wrap_rib_route_add (struct rib_tree *tree, const uint8_t *key, int keylen,
                      struct route_entry *entry)
{
  if (fail_route)
    {
      fail_route = false;
      return -1;
    }
  return __real_rib_route_add (tree, key, keylen, entry);
}

static void
send_route (int family, const char *dst, int plen, int oif, bool add)
{
  struct internal_msg_route_entry message = { 0 };
  message.family = message.dst.family = family;
  message.table_id = RT_TABLE_MAIN;
  message.dst.plen = plen;
  assert (inet_pton (family, dst, &message.dst.dst_ip_addr) == 1);
  message.nh.nhcnt = 1;
  message.nh.members[0].info.family = family;
  message.nh.members[0].info.type = NEXTHOP_TYPE_CONNECTED;
  message.nh.members[0].info.oif = oif;
  process (internal_msg_create (add ? INTERNAL_MSG_TYPE_ROUTE_ENTRY_ADD
                                    : INTERNAL_MSG_TYPE_ROUTE_ENTRY_DEL,
                                &message, sizeof (message)));
}

static void
check_failures (int family)
{
  const char *dst = family == AF_INET ? "192.0.2.0" : "2001:db8::";
  int plen = family == AF_INET ? 24 : 64;
  setup ();
  fail_tree = true;
  send_route (family, dst, plen, 11, true);
  assert (! fail_tree && ! lookup (family, dst));
  check_group_count (0);

  fail_route = true;
  send_route (family, dst, plen, 11, true);
  assert (! fail_route && ! lookup (family, dst));
  check_group_count (0);

  send_route (family, dst, plen, 11, true);
  int original = check_route (family, dst, plen, 11);
  check_group (original, 1);

  /* Failed replacements must release the acquired reference. */
  for (int oif = 11; oif <= 22; oif += 11)
    {
      fail_route = true;
      send_route (family, dst, plen, oif, true);
      assert (! fail_route);
      assert (check_route (family, dst, plen, 11) == original);
      check_group (original, 1);
      check_group_count (1);
    }

  send_route (family, dst, plen, 22, true);
  int replacement = check_route (family, dst, plen, 22);
  check_group (original, 0);
  check_group (replacement, 1);
  check_group_count (1);
  send_route (family, dst, plen, 22, false);
  check_group (replacement, 0);
  cleanup ();
}

static void
check_updates (int family)
{
  const char *dst = family == AF_INET ? "192.0.2.0" : "2001:db8::";
  const char *wide =
      family == AF_INET ? "192.0.2.200" : "2001:db8:0:0:8000::1";
  const char *host = family == AF_INET ? "192.0.2.1" : "2001:db8::1";
  int plen = family == AF_INET ? 24 : 64;
  int hostlen = family == AF_INET ? 32 : 128;
  setup ();
  send_route (family, dst, plen, 11, true);
  int original = check_route (family, dst, plen, 11);
  for (int i = 0; i < 20; i++)
    {
      send_route (family, dst, plen, 11, true);
      assert (check_route (family, dst, plen, 11) == original);
      check_group (original, 1);
      check_group_count (1);
    }

  /* The prefixes share an address, so LPM would select the narrower route. */
  send_route (family, dst, plen + 1, 22, true);
  int narrower = check_route (family, dst, plen + 1, 22);
  check_group (original, 1);
  check_group (narrower, 1);
  check_group_count (2);

  send_route (family, dst, plen + 2, 11, false);
  check_group (original, 1);
  check_group (narrower, 1);
  assert (check_route (family, wide, plen, 11) == original);
  assert (check_route (family, dst, plen + 1, 22) == narrower);

  send_route (family, dst, plen, 33, true);
  int replacement = check_route (family, wide, plen, 33);
  check_group (original, 0);
  check_group (replacement, 1);
  check_group (narrower, 1);
  check_group_count (2);

  /* The delete message names the other route's nexthop. */
  send_route (family, dst, plen, 22, false);
  assert (! lookup (family, wide));
  check_group (replacement, 0);
  check_group (narrower, 1);
  assert (check_route (family, dst, plen + 1, 22) == narrower);
  send_route (family, dst, plen, 22, false);
  check_group (narrower, 1);
  send_route (family, dst, plen + 1, 999, false);
  assert (! lookup (family, dst));
  check_group (narrower, 0);
  send_route (family, dst, plen + 1, 999, false);
  check_group_count (0);

  /* Two routes retain the same group until the last route is removed. */
  send_route (family, dst, plen, 11, true);
  int shared = check_route (family, dst, plen, 11);
  send_route (family, host, hostlen, 11, true);
  assert (check_route (family, host, hostlen, 11) == shared);
  check_group (shared, 2);
  check_group_count (1);
  send_route (family, dst, plen, 11, true);
  check_group (shared, 2);
  send_route (family, dst, plen, 11, false);
  check_group (shared, 1);
  assert (check_route (family, host, hostlen, 11) == shared);
  send_route (family, host, hostlen, 11, false);
  check_group (shared, 0);
  cleanup ();
}

int
main (void)
{
  check_failures (AF_INET);
  check_failures (AF_INET6);
  check_updates (AF_INET);
  check_updates (AF_INET6);
  puts (
      "PASS: IPv4/IPv6 route replacement, exact deletion, shared groups and registration failures");
  return 0;
}
