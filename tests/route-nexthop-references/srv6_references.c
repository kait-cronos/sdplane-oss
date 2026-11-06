#include "support.h"
#include <linux/seg6_local.h>

static void
send_sid (const char *sid, int oif, bool add)
{
  struct internal_msg_srv6_local_sid message = { 0 };
  assert (inet_pton (AF_INET6, sid, &message.srv6_local_sid_addr) == 1);
  message.srv6_local_sid_addr_index = UINT8_MAX;
  message.action = SEG6_LOCAL_ACTION_END;
  message.oif = oif;
  process (internal_msg_create (add ? INTERNAL_MSG_TYPE_SRV6_LOCAL_SID_ADD
                                    : INTERNAL_MSG_TYPE_SRV6_LOCAL_SID_DEL,
                                &message, sizeof (message)));
}

static int
check_sid (const char *sid, int oif)
{
  struct in6_addr address;
  assert (inet_pton (AF_INET6, sid, &address) == 1);
  int id = check_route (AF_INET6, sid, 128, oif);
  struct nh_info *info = nexthop_get_info_by_index (published_info (), id);
  assert (info->type == NEXTHOP_TYPE_SRV6_END);
  assert (! memcmp (&info->gw.srv6_local_sid.sid, &address, sizeof (address)));
  return id;
}

int
main (void)
{
  setup ();
  send_sid ("2001:db8::1", 11, true);
  for (int i = 0; i < 16; i++)
    {
      send_sid ("2001:db8::1", 11, true);
      int current = check_sid ("2001:db8::1", 11);
      check_group (current, 1);
      check_group_count (1);
    }
  send_sid ("2001:db8::1", 22, true);
  int replacement = check_sid ("2001:db8::1", 22);
  check_group (replacement, 1);
  check_group_count (1);
  send_sid ("2001:db8::2", 33, true);
  int other = check_sid ("2001:db8::2", 33);
  check_group (replacement, 1);
  check_group (other, 1);
  check_group_count (2);

  send_sid ("2001:db8::1", 999, false);
  assert (! lookup (AF_INET6, "2001:db8::1"));
  check_group (replacement, 0);
  check_group (other, 1);
  assert (published_info ()->srv6_local_sid_addr_num == 1);
  assert (check_sid ("2001:db8::2", 33) == other);
  send_sid ("2001:db8::1", 999, false);
  check_group (other, 1);
  assert (check_sid ("2001:db8::2", 33) == other);
  send_sid ("2001:db8::2", 999, false);
  check_group (other, 0);
  assert (published_info ()->srv6_local_sid_addr_num == 0);
  cleanup ();
  puts ("PASS: SRv6 local SID replacement, deletion and SID array compaction");
  return 0;
}
