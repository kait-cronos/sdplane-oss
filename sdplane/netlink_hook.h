#ifndef __NETLINK_HOOK_H__
#define __NETLINK_HOOK_H__

#define NLHOOK_SIZE 4

struct nlhook_entry
{
  uint8_t sched;
  uint16_t flag;
#define NLHOOK_FLAG_MATCH_IFADDR (1UL << 0)
#define NLHOOK_FLAG_MATCH_ROUTE  (1UL << 1)
  uint16_t nlmsg_type;
  uint8_t family;
  uint8_t prefix_len;
  int ifindex;
  char *ifname;
  union {
    struct in_addr addr4;
    struct in6_addr addr6;
    //struct rte_ether_addr lladdr;
  } a;
  int argv_list_index;
};

extern pthread_mutex_t nlhook_mutex;
extern struct nlhook_entry nlhook_entries[NLHOOK_SIZE];

int nlhook_check_ifaddr (uint16_t nlmsg_type, int ifindex, char *ifname,
                     uint8_t family, uint8_t prefix_len, char *ifaddr);
int netlink_hook_exec ();

void nlhook_cmd_init (struct command_set *cmdset);
void nlhook_init ();

#endif /*__NETLINK_HOOK_H__*/
