#include "support.h"
#include <linux/seg6_local.h>

int
main (void)
{
  setup ();
  const char *sid = "2001:db8::1";
  struct internal_msg_srv6_local_sid msg = { 0 };
  assert (inet_pton (AF_INET6, sid, &msg.srv6_local_sid_addr) == 1);
  msg.srv6_local_sid_addr_index = UINT8_MAX;
  msg.action = SEG6_LOCAL_ACTION_END;
  msg.oif = 11;
  process (internal_msg_create (INTERNAL_MSG_TYPE_SRV6_LOCAL_SID_ADD, &msg,
                                sizeof (msg)));
  check_route (AF_INET6, sid, 128, RT_TABLE_MAIN, RTN_UNICAST, sid);
  msg.oif = 22;
  process (internal_msg_create (INTERNAL_MSG_TYPE_SRV6_LOCAL_SID_ADD, &msg,
                                sizeof (msg)));
  check_route (AF_INET6, sid, 128, RT_TABLE_MAIN, RTN_UNICAST, sid);
  cleanup ();
  puts ("PASS: SRv6 local SID route type on registration and replacement");
  return 0;
}
