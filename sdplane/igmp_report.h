#ifndef __IGMP_REPORT_H__
#define __IGMP_REPORT_H__

struct rib;
struct router_if;

#define MAX_IGMP_JOIN_GROUPS         32
#define IGMP_REPORT_INTERVAL_DEFAULT 30 // Default query interval in seconds

struct igmp_join
{
  bool used;
  int vswitch_id;
  struct in_addr group_addr;
  int ifindex;
  int fd;
};

extern struct igmp_join igmp_join_table[MAX_IGMP_JOIN_GROUPS];
extern int igmp_report_interval;

struct router_if *igmp_router_if_lookup (struct rib *rib, int vswitch_id);
int igmp_report (void *dummy);

#endif /* __IGMP_REPORT_H__ */
