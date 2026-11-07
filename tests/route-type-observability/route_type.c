#include "support.h"
#include <sdplane/command.h>

extern int set_srv6_encap_head_func (void *, int, char **);

static void
family_routes (int family, const char *parent, const char *child,
               const char *outside, int plen, int childlen)
{
  struct route_wire p =
      route_wire (family, parent, plen, RT_TABLE_MAIN, RTN_UNICAST);
  import_route (&p);
  check_route (family, parent, plen, RT_TABLE_MAIN, RTN_UNICAST, parent);
  struct route_wire c =
      route_wire (family, child, childlen, RT_TABLE_MAIN, RTN_LOCAL);
  import_route (&c);
  check_route (family, child, childlen, RT_TABLE_MAIN, RTN_LOCAL, child);
  check_route (family, parent, plen, RT_TABLE_MAIN, RTN_UNICAST, outside);
  c.r.rtm_type = RTN_BROADCAST;
  import_route (&c);
  check_route (family, child, childlen, RT_TABLE_MAIN, RTN_BROADCAST, child);
  /* Updating the shorter route must preserve the more specific route. */
  p.r.rtm_type = RTN_MULTICAST;
  import_route (&p);
  check_route (family, parent, plen, RT_TABLE_MAIN, RTN_MULTICAST, outside);
  check_route (family, child, childlen, RT_TABLE_MAIN, RTN_BROADCAST, child);
  c.h.nlmsg_type = RTM_DELROUTE;
  import_route (&c);
  check_route (family, parent, plen, RT_TABLE_MAIN, RTN_MULTICAST, child);
}

int
main (void)
{
  setup ();
  family_routes (AF_INET, "192.0.2.0", "192.0.2.1", "192.0.2.200", 24, 32);
  family_routes (AF_INET6, "2001:db8::", "2001:db8::1", "2001:db8::ffff", 64,
                 128);
  /* Odd prefix lengths also exercise partial K-bit expansion. */
  family_routes (AF_INET, "198.51.100.0", "198.51.100.2", "198.51.100.0", 30,
                 31);
  family_routes (AF_INET6, "2001:db8:1::", "2001:db8:1::2",
                 "2001:db8:1::", 126, 127);
  struct shell shell = { .terminal = stdout, .NL = "\n" };
  char *prefixes[] = { "203.0.113.0/24", "2001:db8:2::/64" };
  char *networks[] = { "203.0.113.0", "2001:db8:2::" };
  char *addresses[] = { "203.0.113.1", "2001:db8:2::1" };
  for (int i = 0; i < 2; i++)
    {
      char *argv[] = { "set",  "srv6",      "encap",
                       "head", prefixes[i], "2001:db8::a" };
      assert (set_srv6_encap_head_func (&shell, 6, argv) == CMD_SUCCESS);
      struct internal_msg_header *msg = internal_msg_recv (msg_queue_rib);
      assert (msg);
      struct internal_msg_route_entry *entry = internal_msg_body (msg);
      assert (entry->route_type == RTN_UNICAST);
      process (msg);
      check_route (i ? AF_INET6 : AF_INET, networks[i], i ? 64 : 24,
                   RT_TABLE_MAIN, RTN_UNICAST, addresses[i]);
    }
  cleanup ();
  puts ("PASS: Netlink/CLI route types, replacement and prefix expansion");
  return 0;
}
