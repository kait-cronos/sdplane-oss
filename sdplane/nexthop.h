#ifndef __NEXTHOP_H__
#define __NEXTHOP_H__

#ifndef IF_NAMESIZE
#define IF_NAMESIZE 16
#endif

#define MAX_NEXTHOP_INFO_SIZE     4096
#define MAX_NEXTHOP_GRP_SIZE      4096
#define MAX_ECMP_ENTRY            32

#define NEXTHOP_HASH_BUCKETS_SIZE 1024  /* must be power of 2 */

struct rib_info;
struct internal_msg_nh_entry;
struct hash_table;

enum nexthop_type
{
  NEXTHOP_TYPE_CONNECTED,
  NEXTHOP_TYPE_GATEWAY,
};

struct nh_info
{
  int family;
  enum nexthop_type type;
  union
  {
    struct in_addr ipv4_addr;
    struct in6_addr ipv6_addr;
  } gw;
  uint32_t oif;
  char oif_name[IF_NAMESIZE];
  int refcnt; /* reference count from groups */
};

struct nh_member
{
  int ref_sdplane_nh_id;
  int info_index;
  uint32_t weight;
};

struct nh_group
{
  int sdplane_nh_id; /* index in nexthop.groups[] */
  int kernel_nh_id; /* kernel nexthop id */
  int refcnt; /* reference count from routes or other groups */
  int nhcnt; /* number of members */
  struct nh_member members[MAX_ECMP_ENTRY];
};

struct nexthop
{
  struct nh_group groups[MAX_NEXTHOP_GRP_SIZE];
  struct nh_info info_pool[MAX_NEXTHOP_INFO_SIZE];
  int groups_top; /* next-free-slot hint for groups[] */
  int info_top; /* next-free-slot hint for info_pool[] */
};

void nexthop_init (void);
void nexthop_cleanup (void);

char *nexthop_format (struct internal_msg_nh_entry *nh, char *buf, size_t buf_size);
void nexthop_show_group_table (struct shell *shell, struct rib_info *rib_info);
void nexthop_show_pool (struct shell *shell, struct rib_info *rib_info);

int nexthop_add_entry (struct rib_info *rib_info, struct internal_msg_nh_entry *nh);
int nexthop_del_entry (struct rib_info *rib_info, struct internal_msg_nh_entry *nh);

void nexthop_increment_refcnt (struct rib_info *rib_info, int sdplane_nh_id);
void nexthop_decrement_refcnt (struct rib_info *rib_info, int sdplane_nh_id);

struct nh_info *nexthop_get_info_by_index (struct rib_info *rib_info,
                                           int sdplane_nh_id);
int nexthop_get_index_by_nhid (struct rib_info *rib_info, int kernel_nh_id);

#endif /* __NEXTHOP_H__ */
