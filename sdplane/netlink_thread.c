#include "include.h"

#include <lthread.h>

/* for sdplane debug log */
#include <sdplane/debug.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "sdplane.h"
#include "thread_info.h"
#include "internal_message.h"
#include "neigh_manager.h"

/* for netlink */
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

/* for SRv6 */
#include "netlink_seg6.h"

#include "netlink_hook.h"

extern int lthread_core;
extern volatile bool force_stop[RTE_MAX_LCORE];
static __thread uint64_t loop_counter = 0;

struct netlink_sock
{
  int sockfd;
  int seq;
  struct sockaddr_nl snl;
  char *name;
};

struct netlink_sock netlink_kernel = { -1, 0, { 0 }, "netlink-kernel" };
struct netlink_sock netlink_cmd = { -1, 0, { 0 }, "netlink-cmd" };

int
netlink_socket (struct netlink_sock *nlsock, unsigned long groups)
{
  int sockfd;
  int ret;
  struct sockaddr_nl snl;
  socklen_t namelen;

  sockfd = socket (AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sockfd < 0)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "can't open %s socket: %s", nlsock->name,
                         strerror (errno));
      return -1;
    }

#if 1
  ret = fcntl (sockfd, F_SETFL, O_NONBLOCK);
  if (ret < 0)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "can't set %s socket O_NONBLOCK: %s",
                         nlsock->name, strerror (errno));
      close (sockfd);
      return -1;
    }
#endif

  memset (&snl, 0, sizeof (snl));
  snl.nl_family = AF_NETLINK;
  snl.nl_groups = groups;

  ret = bind (sockfd, (struct sockaddr *) &snl, sizeof (snl));
  if (ret < 0)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "can't bind %s socket to groups 0x%x: %s",
                         nlsock->name, groups, strerror (errno));
      close (sockfd);
      return -1;
    }

  namelen = sizeof (snl);
  ret = getsockname (sockfd, (struct sockaddr *) &snl, &namelen);
  if (ret < 0 || namelen != sizeof (snl))
    {
      DEBUG_SDPLANE_LOG (NETLINK, "can't bind %s socket name: %s",
                         nlsock->name, strerror (errno));
      close (sockfd);
      return -1;
    }

  nlsock->snl = snl;
  nlsock->sockfd = sockfd;

  return 0;
}

int
netlink_request (int family, int type, struct netlink_sock *nlsock)
{
  if (! nlsock || nlsock->sockfd < 0)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "%s socket isn't active.",
                         (nlsock ? nlsock->name : "netlink command"));
      return -1;
    }

  struct
  {
    struct nlmsghdr nlh;
    struct rtgenmsg g;
  } req;

  req.nlh.nlmsg_len = sizeof (req);
  req.nlh.nlmsg_type = type;
  req.nlh.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;
  req.nlh.nlmsg_pid = 0;
  req.nlh.nlmsg_seq = ++nlsock->seq;
  req.g.rtgen_family = family;

  struct sockaddr_nl snl;
  memset (&snl, 0, sizeof (snl));
  snl.nl_family = AF_NETLINK;

  int ret;
  ret = sendto (nlsock->sockfd, (void *) &req, sizeof (req), 0,
                (struct sockaddr *) &snl, sizeof (snl));
  if (ret < 0)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "%s sendto failed: %s.", nlsock->name,
                         strerror (errno));
      return -1;
    }
  else
    {
      DEBUG_SDPLANE_LOG (NETLINK,
                         "%s: sent: "
                         "family: %d type: %d pid: %d seq: %d.",
                         nlsock->name, family, type, req.nlh.nlmsg_pid,
                         req.nlh.nlmsg_seq);
    }

  return 0;
}

static inline const char *
netlink_nlmsg_str (uint16_t nlmsg_type)
{
  switch (nlmsg_type)
    {
    case NLMSG_NOOP:
      return "NLMSG_NOOP";
    case NLMSG_ERROR:
      return "NLMSG_ERROR";
    case NLMSG_DONE:
      return "NLMSG_DONE";
    case NLMSG_OVERRUN:
      return "NLMSG_OVERRUN";
    case RTM_NEWROUTE:
      return "RTM_NEWROUTE";
    case RTM_DELROUTE:
      return "RTM_DELROUTE";
    case RTM_GETROUTE:
      return "RTM_GETROUTE";
    case RTM_NEWNEIGH:
      return "RTM_NEWNEIGH";
    case RTM_DELNEIGH:
      return "RTM_DELNEIGH";
    case RTM_GETNEIGH:
      return "RTM_GETNEIGH";
    case RTM_NEWLINK:
      return "RTM_NEWLINK";
    case RTM_DELLINK:
      return "RTM_DELLINK";
    case RTM_GETLINK:
      return "RTM_GETLINK";
    case RTM_NEWADDR:
      return "RTM_NEWADDR";
    case RTM_DELADDR:
      return "RTM_DELADDR";
    case RTM_GETADDR:
      return "RTM_GETADDR";
    case RTM_NEWNEXTHOP:
      return "RTM_NEWNEXTHOP";
    case RTM_DELNEXTHOP:
      return "RTM_DELNEXTHOP";
    default:
      return "Unknown";
    }
  /* not reached */
  return "Not-reached";
}

static inline __attribute__ ((always_inline)) int
netlink_get_rtattr (struct rtattr **rtas, int max, void *head, int len)
{
  int counter = 0;
  struct rtattr *rta = head;

  for (; RTA_OK (rta, len); rta = RTA_NEXT (rta, len))
    {
      if (rta->rta_type <= max)
        {
          rtas[rta->rta_type] = rta;
          counter++;
        }
    }

  return counter;
}

int
netlink_read_nlmsg_error (struct netlink_sock *nlsock, struct nlmsghdr *h)
{
  struct nlmsgerr *err = (struct nlmsgerr *) NLMSG_DATA (h);

  if (h->nlmsg_len < NLMSG_LENGTH (sizeof (struct nlmsgerr)))
    {
      DEBUG_SDPLANE_LOG (NETLINK,
                         "%s: invalid len: %d < "
                         "NLMSG_LEN(nlmsgerr): %d",
                         nlsock->name, h->nlmsg_len,
                         NLMSG_LENGTH (sizeof (struct nlmsgerr)));
      return -1;
    }

  if (err->error == 0)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "%s: ACK: type: %s(%u) seq: %lu pid: %lu",
                         nlsock->name, netlink_nlmsg_str (err->msg.nlmsg_type),
                         err->msg.nlmsg_type, err->msg.nlmsg_seq,
                         err->msg.nlmsg_pid);
      if (! (h->nlmsg_flags & NLM_F_MULTI))
        {
          DEBUG_SDPLANE_LOG (NETLINK, "%s: ACK: not multipart, should end.",
                             nlsock->name);
          return -1;
        }
      return 0;
    }

  DEBUG_SDPLANE_LOG (NETLINK,
                     "%s: error: %d: %s "
                     "type: %s(%u) seq: %lu pid: %lu",
                     nlsock->name, err->error, strerror (-err->error),
                     netlink_nlmsg_str (err->msg.nlmsg_type),
                     err->msg.nlmsg_type, err->msg.nlmsg_seq,
                     err->msg.nlmsg_pid);

  return -1;
}

int
netlink_read_nlmsg_neigh (struct netlink_sock *nlsock, struct nlmsghdr *h)
{
  struct ndmsg *ndm = (struct ndmsg *) NLMSG_DATA (h);

  if (h->nlmsg_len < NLMSG_LENGTH (sizeof (struct nlmsgerr)))
    {
      DEBUG_SDPLANE_LOG (NETLINK,
                         "%s: invalid len: %d < "
                         "NLMSG_LEN(nlmsgerr): %d",
                         nlsock->name, h->nlmsg_len,
                         NLMSG_LENGTH (sizeof (struct nlmsgerr)));
      return -1;
    }

  /* New/Delete Neighbor Table */
  struct rtattr *rtns[NDA_MAX + 1] = { 0 };
  char ifname[16] = { 0 };
  char addr[64] = { 0 };
  char lladdr[RTE_ETHER_ADDR_FMT_SIZE] = { 0 };
  struct rte_ether_addr *etha = NULL;
  if_indextoname (ndm->ndm_ifindex, ifname);
  struct rtattr *rta = (struct rtattr *) RTM_RTA (ndm);
  netlink_get_rtattr (rtns, RTA_MAX, rta, RTM_PAYLOAD (h));
  void *msgp;
  struct internal_msg_neigh_entry msg_neigh_entry;

  if (! rtns[NDA_DST])
    {
      DEBUG_SDPLANE_LOG (NETLINK, "%s: no destination address in message.",
                         nlsock->name);
      return -1;
    }

  inet_ntop (ndm->ndm_family, RTA_DATA (rtns[NDA_DST]), addr, sizeof (addr));
  msg_neigh_entry.data.family = ndm->ndm_family;
  msg_neigh_entry.data.state = ndm->ndm_state;
  msg_neigh_entry.data.ifindex = ndm->ndm_ifindex;
  switch (ndm->ndm_family)
    {
    case AF_INET:
      msg_neigh_entry.type = NEIGH_ARP_TABLE;
      memcpy (&msg_neigh_entry.data.ip_addr.ipv4_addr,
              RTA_DATA (rtns[NDA_DST]), sizeof (struct in_addr));
      break;

    case AF_INET6:
      msg_neigh_entry.type = NEIGH_ND_TABLE;
      memcpy (&msg_neigh_entry.data.ip_addr.ipv6_addr,
              RTA_DATA (rtns[NDA_DST]), sizeof (struct in6_addr));
      break;

    default:
      return -1;
    }

  if (h->nlmsg_type == RTM_NEWNEIGH)
    {
      if (! rtns[NDA_LLADDR])
        return -1;
      etha = (struct rte_ether_addr *) RTA_DATA (rtns[NDA_LLADDR]);
      rte_ether_format_addr (lladdr, sizeof (lladdr), etha);
      DEBUG_SDPLANE_LOG (NETLINK, "[NEW] dst=%s lladdr=%s state=%s dev=%s", addr,
                         lladdr, neigh_manager_state_str (msg_neigh_entry.data.state),
                         ifname);
      memcpy (&msg_neigh_entry.data.mac_addr, etha,
              sizeof (struct rte_ether_addr));
      msgp = internal_msg_create (INTERNAL_MSG_TYPE_NEIGH_ENTRY_ADD,
                                  &msg_neigh_entry, sizeof (msg_neigh_entry));
    }
  else // RTM_DELNEIGH
    {
      DEBUG_SDPLANE_LOG (NETLINK, "[DEL] dst=%s dev=%s", addr, ifname);
      msgp = internal_msg_create (INTERNAL_MSG_TYPE_NEIGH_ENTRY_DEL,
                                  &msg_neigh_entry, sizeof (msg_neigh_entry));
    }

  if (! msg_queue_neigh)
    DEBUG_SDPLANE_LOG (NETLINK, "error: neigh_manager is not started.");

  int ret;
  ret = internal_msg_send_to (msg_queue_neigh, msgp, NULL);
  if (ret < 0)
    {
      WARNING ("send imsg to msg_queue_neigh (%p) failed.",
               msg_queue_neigh);
    }

  return 0;
}

static inline __attribute__ ((always_inline)) char *
netlink_format_nexthop_object (enum nexthop_type nh_type, void *nh,
                              char *buf, size_t buf_size)
{
  int i, len = 0;
  char nh_ip_str[INET6_ADDRSTRLEN] = { 0 };

  switch (nh_type)
    {
      case NH_TYPE_LEGACY:
        {
          struct nh_legacy *nh_legacy = (struct nh_legacy *) nh;
          switch (nh_legacy->type)
            {
              case NH_OBJ_TYPE_OBJECT:
                {
                  struct nh_info *nh_info = &nh_legacy->nh_info;

                  inet_ntop (nh_info->family, &nh_info->nh_ip_addr,
                             nh_ip_str, sizeof (nh_ip_str));
                  len += snprintf (buf + len, buf_size - len,
                                   "%s(oif=%d)",
                                   nh_ip_str, nh_info->oif);
                  break;
                }

              case NH_OBJ_TYPE_GROUP:
                {
                  struct nh_info_group *nhg = &nh_legacy->nh_grp;

                  len += snprintf (buf + len, buf_size - len, "[");
                  for (i = 0; i < nhg->num; i++)
                    {
                      inet_ntop (nhg->nh_info_list[i].family,
                                 &nhg->nh_info_list[i].nh_ip_addr,
                                 nh_ip_str, sizeof (nh_ip_str));
                      len += snprintf (buf + len, buf_size - len,
                                       "%s(oif=%d)%s",
                                       nh_ip_str,
                                       nhg->nh_info_list[i].oif,
                                       (i == nhg->num - 1) ? "" : ", ");
                    }
                  snprintf (buf + len, buf_size - len, "]");
                  break;
                }

              default:
                return buf;
            }
          break;
        }

      case NH_TYPE_OBJECT_CAP:
        {
          int *nh_id = (int *) nh;
          len += snprintf (buf + len, buf_size - len, "nhid=%d", *nh_id);
          break;
        }

      default:
        return buf;
    }

  return buf;
}

static inline const char *
seg6_mode_str (int mode)
{
  switch (mode)
    {
    case SEG6_IPTUN_MODE_INLINE:
      return "H.Insert";
    case SEG6_IPTUN_MODE_ENCAP:
      return "H.Encaps";
    case SEG6_IPTUN_MODE_L2ENCAP:
      return "H.Encaps.L2";
    case SEG6_IPTUN_MODE_ENCAP_RED:
      return "H.Encaps.Red";
    case SEG6_IPTUN_MODE_L2ENCAP_RED:
      return "H.Encaps.L2.Red";
    default:
      return "Unknown";
    }
  /* not reached */
  return "Not-reached";
}

static inline const char *
seg6local_action_str (uint32_t action)
{
  switch (action)
    {
    case SEG6_LOCAL_ACTION_UNSPEC:
      return "Unspec";
    case SEG6_LOCAL_ACTION_END:
      return "End";
    case SEG6_LOCAL_ACTION_END_X:
      return "End.X";
    case SEG6_LOCAL_ACTION_END_T:
      return "End.T";
    case SEG6_LOCAL_ACTION_END_DX2:
      return "End.DX2";
    case SEG6_LOCAL_ACTION_END_DX6:
      return "End.DX6";
    case SEG6_LOCAL_ACTION_END_DX4:
      return "End.DX4";
    case SEG6_LOCAL_ACTION_END_DT6:
      return "End.DT6";
    case SEG6_LOCAL_ACTION_END_DT4:
      return "End.DT4";
    case SEG6_LOCAL_ACTION_END_B6:
      return "End.B6";
    case SEG6_LOCAL_ACTION_END_B6_ENCAP:
      return "End.B6.Encaps";
    case SEG6_LOCAL_ACTION_END_BM:
      return "End.BM";
    case SEG6_LOCAL_ACTION_END_S:
      return "End.S";
    case SEG6_LOCAL_ACTION_END_AS:
      return "End.AS";
    case SEG6_LOCAL_ACTION_END_AM:
      return "End.AM";
    case SEG6_LOCAL_ACTION_END_BPF:
      return "End.BPF";
    case SEG6_LOCAL_ACTION_END_DT46:
      return "End.DT46";
    default:
      return "Unknown";
    }
  /* not reached */
  return "Not-reached";
}

static inline __attribute__ ((always_inline)) void
netlink_log_seg6 (struct seg6_param *param)
{
  char str[1024] = { 0 };
  int pos = 0, i;
  struct ipv6_sr_hdr *srh = param->seg6_tuninfo->srh;

  pos += snprintf (str + pos, sizeof (str) - pos, "seg6");
  if (param->seg6_tuninfo->mode >= 0)
    pos += snprintf (str + pos, sizeof (str) - pos, " mode %s",
                     seg6_mode_str (param->seg6_tuninfo->mode));

  if (srh->first_segment >= 0)
    {
      pos += snprintf (str + pos, sizeof (str) - pos, " segs ");
      for (i = srh->first_segment; i >= 0; i--)
        {
          char segment_str[INET6_ADDRSTRLEN];
          inet_ntop (AF_INET6, &srh->segments[i], segment_str,
                     sizeof (segment_str));
          pos += snprintf (str + pos, sizeof (str) - pos, "%s",
                           segment_str);
          if (i != 0)
            pos += snprintf (str + pos, sizeof (str) - pos, ",");
        }
    }

  DEBUG_NEW (NETLINK, "%s", str);
}

static inline __attribute__ ((always_inline)) void
netlink_log_seg6local (struct seg6local_param *param)
{
  char str[1024] = { 0 };
  int pos = 0, i;

  pos += snprintf (str + pos, sizeof (str) - pos, "seg6local");
  if (param->action)
    pos += snprintf (str + pos, sizeof (str) - pos, " action %s",
                     seg6local_action_str (param->action));
  if (param->table_id)
    pos += snprintf (str + pos, sizeof (str) - pos, " table %d",
                     param->table_id);
  if (param->vrf_id)
    pos += snprintf (str + pos, sizeof (str) - pos, " vrftable %d",
                     param->vrf_id);
  if (param->nh4.s_addr != 0)
    {
      char nh4_str[INET_ADDRSTRLEN];
      inet_ntop (AF_INET, &param->nh4, nh4_str,
                 sizeof (nh4_str));
      pos += snprintf (str + pos, sizeof (str) - pos, " nh4 %s",
                       nh4_str);
    }
  if (! IN6_IS_ADDR_UNSPECIFIED (&param->nh6))
    {
      char nh6_str[INET6_ADDRSTRLEN];
      inet_ntop (AF_INET6, &param->nh6, nh6_str,
                 sizeof (nh6_str));
      pos += snprintf (str + pos, sizeof (str) - pos, " nh6 %s",
                       nh6_str);
    }
  if (param->iif)
    {
      char iif_str[16];
      if_indextoname (param->iif, iif_str);
      pos += snprintf (str + pos, sizeof (str) - pos, " iif %s",
                       iif_str);
    }
  if (param->oif)
    {
      char oif_str[16];
      if_indextoname (param->oif, oif_str);
      pos += snprintf (str + pos, sizeof (str) - pos, " oif %s",
                       oif_str);
    }
  if (param->srh)
    {
      pos += snprintf (str + pos, sizeof (str) - pos, " srh segs ");
      for (i = param->srh->first_segment; i >= 0; i--)
        {
          char segment_str[INET6_ADDRSTRLEN];
          inet_ntop (AF_INET6, &param->srh->segments[i], segment_str,
                     sizeof (segment_str));
          pos += snprintf (str + pos, sizeof (str) - pos, "%s",
                           segment_str);
          if (i != 0)
            pos += snprintf (str + pos, sizeof (str) - pos, ",");
        }
    }

  DEBUG_NEW (NETLINK, "%s", str);
}

static inline __attribute__ ((always_inline)) void
netlink_parse_seg6 (struct rtattr *rta, struct seg6_param *param)
{
  struct rtattr *rtas[SEG6_IPTUNNEL_MAX + 1] = { 0 };
  netlink_get_rtattr (rtas, SEG6_IPTUNNEL_MAX, RTA_DATA (rta), RTA_PAYLOAD (rta));

  if (rtas[SEG6_IPTUNNEL_SRH])
    param->seg6_tuninfo = RTA_DATA (rtas[SEG6_IPTUNNEL_SRH]);
}

static inline __attribute__ ((always_inline)) void
netlink_parse_seg6local (struct rtattr *rta, struct seg6local_param *param)
{
  int i;
  struct rtattr *rtas[SEG6_LOCAL_MAX + 1] = { 0 };
  netlink_get_rtattr (rtas, SEG6_LOCAL_MAX, RTA_DATA (rta), RTA_PAYLOAD (rta));

  if (rtas[SEG6_LOCAL_ACTION])
    param->action = *(uint32_t *) RTA_DATA (rtas[SEG6_LOCAL_ACTION]);
  if (rtas[SEG6_LOCAL_SRH])
    param->srh = RTA_DATA (rtas[SEG6_LOCAL_SRH]);
  if (rtas[SEG6_LOCAL_TABLE])
    param->table_id = *(uint32_t *) RTA_DATA (rtas[SEG6_LOCAL_TABLE]);
  if (rtas[SEG6_LOCAL_NH4])
    memcpy (&param->nh4, RTA_DATA (rtas[SEG6_LOCAL_NH4]), RTA_PAYLOAD (rtas[SEG6_LOCAL_NH4]));
  if (rtas[SEG6_LOCAL_NH6])
    memcpy (&param->nh6, RTA_DATA (rtas[SEG6_LOCAL_NH6]), RTA_PAYLOAD (rtas[SEG6_LOCAL_NH6]));
  if (rtas[SEG6_LOCAL_IIF])
    param->iif = *(int *) RTA_DATA (rtas[SEG6_LOCAL_IIF]);
  if (rtas[SEG6_LOCAL_OIF])
    param->oif = *(int *) RTA_DATA (rtas[SEG6_LOCAL_OIF]);
  if (rtas[SEG6_LOCAL_VRFTABLE])
    param->vrf_id = *(uint32_t *) RTA_DATA (rtas[SEG6_LOCAL_VRFTABLE]);
#if 0
  if (rtas[SEG6_LOCAL_FLAVORS])
    // not supported yet
  if (rtas[SEG6_LOCAL_COUNTERS])
    // not supported yet
  if (rtas[SEG6_LOCAL_BPF])
    // not supported yet
#endif
}

int
netlink_read_nlmsg_route (struct netlink_sock *nlsock, struct nlmsghdr *h)
{
  struct rtmsg *rtm = (struct rtmsg *) NLMSG_DATA (h);

  if (h->nlmsg_len < NLMSG_LENGTH (sizeof (struct nlmsgerr)))
    {
      DEBUG_SDPLANE_LOG (NETLINK,
                         "%s: invalid len: %d < "
                         "NLMSG_LEN(nlmsgerr): %d",
                         nlsock->name, h->nlmsg_len,
                         NLMSG_LENGTH (sizeof (struct nlmsgerr)));
      return -1;
    }

  /* New/Delete Route Entry */
  struct rtattr *rtns[RTA_MAX + 1] = { 0 };
  struct rtattr *rta = (struct rtattr *) RTM_RTA (rtm);
  netlink_get_rtattr (rtns, RTA_MAX, rta, RTM_PAYLOAD (h));
  int ifindex, *count;
  struct nh_info *nh_info;
  struct nh_legacy *nh_legacy;
  struct nh_info_group *nh_grp;

  char dst_addr[64] = { 0 };
  char gw_addr[512] = { 0 }; // large buffer for multipath nexthops display
  char oif_name[16] = { 0 };

  /* internal message */
  void *msgp = NULL;
  struct internal_msg_route_entry msg_route_entry;
  memset (&msg_route_entry, 0, sizeof (msg_route_entry));
  uint16_t msg_type;
  const char *action_type;

  /* table id and address family */
  if (rtm->rtm_table != RT_TABLE_MAIN) // only main table is supported
    return -1;
  msg_route_entry.table_id = rtm->rtm_table;
  msg_route_entry.family = rtm->rtm_family; // table family

  /* destination */
  if (! rtns[RTA_DST] && rtm->rtm_dst_len != 0)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "%s: no destination address in message.",
                         nlsock->name);
      return -1;
    }
  else if (rtns[RTA_DST])
    {
      msg_route_entry.dst.family = rtm->rtm_family; // destination address family
      msg_route_entry.dst.plen = rtm->rtm_dst_len;
      memcpy (&msg_route_entry.dst.dst_ip_addr,
              RTA_DATA (rtns[RTA_DST]), RTA_PAYLOAD (rtns[RTA_DST]));
      inet_ntop (rtm->rtm_family, RTA_DATA (rtns[RTA_DST]), dst_addr,
                 sizeof (dst_addr));
    }
  else
    {
      /* default route */
      msg_route_entry.dst.family = rtm->rtm_family; // destination address family
      msg_route_entry.dst.plen = 0;
      memset (&msg_route_entry.dst.dst_ip_addr, 0,
              sizeof (msg_route_entry.dst.dst_ip_addr));
      inet_ntop (rtm->rtm_family, &msg_route_entry.dst.dst_ip_addr,
                 dst_addr, sizeof (dst_addr));
    }

  /* multipath */
  if (rtns[RTA_MULTIPATH])
    {
      struct rtattr *rtnh_rtns[RTA_MAX + 1] = { 0 };
      struct rtnexthop *rtnh = RTA_DATA (rtns[RTA_MULTIPATH]);
      int rtnh_len = RTA_PAYLOAD (rtns[RTA_MULTIPATH]);

      nh_legacy = &msg_route_entry.nh.nh_legacy;
      nh_grp = &msg_route_entry.nh.nh_legacy.nh_grp;

      msg_route_entry.nh_type = NH_TYPE_LEGACY;
      nh_legacy->type = NH_OBJ_TYPE_GROUP;
      count = &nh_grp->num;

      for (; RTNH_OK (rtnh, rtnh_len) && *count < MAX_ECMP_ENTRY;
           rtnh = RTNH_NEXT (rtnh), (*count)++)
        {
          nh_info = &nh_grp->nh_info_list[*count];
          nh_info->family = rtm->rtm_family; // nexthop address family
          if (rtnh->rtnh_len > sizeof (*rtnh))
            {
              netlink_get_rtattr (rtnh_rtns, RTA_MAX, RTNH_DATA (rtnh),
                                  rtnh->rtnh_len - sizeof (*rtnh));
              /* gateway (nexthop) */
              if (rtnh_rtns[RTA_GATEWAY])
                {
                  memcpy (&nh_info->nh_ip_addr,
                          RTA_DATA (rtnh_rtns[RTA_GATEWAY]),
                          RTA_PAYLOAD (rtnh_rtns[RTA_GATEWAY]));
                  nh_info->type = ROUTE_TYPE_NEXTHOP;
                }
              else
                {
                  /* no gateway - direct route or link-local */
                  memset (&nh_info->nh_ip_addr, 0, sizeof (nh_info->nh_ip_addr));
                  nh_info->type = ROUTE_TYPE_CONNECTED;
                }
            }
          /* oif */
          nh_info->oif = rtnh->rtnh_ifindex;
        }
    }

  /* nexthop id */
  // TODO: support nexthop object capable routes (RTA_NH_ID)
  if (rtns[RTA_NH_ID])
    {
     msg_route_entry.nh_type = NH_TYPE_OBJECT_CAP;
      DEBUG_SDPLANE_LOG (NETLINK,
                         "%s: nexthop object capable route is not supported yet.",
                         nlsock->name);
      return -1;
    }

  /* gateway (nexthop) */
  if (rtns[RTA_GATEWAY] || rtns[RTA_OIF])
    {
      nh_legacy = &msg_route_entry.nh.nh_legacy;
      nh_info = &msg_route_entry.nh.nh_legacy.nh_info;

      msg_route_entry.nh_type = NH_TYPE_LEGACY;
      nh_legacy->type = NH_OBJ_TYPE_OBJECT;

      nh_info->family = rtm->rtm_family; // nexthop address family

      /* gateway address (if present) */
      if (rtns[RTA_GATEWAY])
        {
          memcpy (&nh_info->nh_ip_addr,
                  RTA_DATA (rtns[RTA_GATEWAY]),
                  RTA_PAYLOAD (rtns[RTA_GATEWAY]));
          nh_info->type = ROUTE_TYPE_NEXTHOP;
        }
      else
        {
          /* no gateway - direct route or link-local */
          memset (&nh_info->nh_ip_addr, 0, sizeof (nh_info->nh_ip_addr));
          nh_info->type = ROUTE_TYPE_CONNECTED;
        }

      /* output interface (if present) */
      if (rtns[RTA_OIF])
        {
          ifindex = *(int *) RTA_DATA (rtns[RTA_OIF]);
          nh_info->oif = ifindex;
          if_indextoname (ifindex, oif_name);
        }
      else
        nh_info->oif = 0; /* or appropriate default */
    }

  if (rtns[RTA_ENCAP])
    {
      uint16_t encap_type = *(uint16_t *) RTA_DATA (rtns[RTA_ENCAP_TYPE]);
      if (encap_type == LWTUNNEL_ENCAP_SEG6)
        {
          struct seg6_param seg6_param;
          memset (&seg6_param, 0, sizeof (seg6_param));
          netlink_parse_seg6 (rtns[RTA_ENCAP], &seg6_param);
          netlink_log_seg6 (&seg6_param);
        }
      else if (encap_type == LWTUNNEL_ENCAP_SEG6_LOCAL)
        {
          struct seg6local_param seg6local_param;
          memset (&seg6local_param, 0, sizeof (seg6local_param));
          netlink_parse_seg6local (rtns[RTA_ENCAP], &seg6local_param);
          netlink_log_seg6local (&seg6local_param);
        }
    }

  /* create and send internal message */
  if (h->nlmsg_type == RTM_NEWROUTE)
    {
      msg_type = INTERNAL_MSG_TYPE_ROUTE_ENTRY_ADD;
      action_type = "NEW";
    }
  else if (h->nlmsg_type == RTM_DELROUTE)
    {
      msg_type = INTERNAL_MSG_TYPE_ROUTE_ENTRY_DEL;
      action_type = "DEL";
    }
  else
    {
      DEBUG_SDPLANE_LOG (NETLINK, "unexpected message type: %s(%u)",
                         netlink_nlmsg_str (h->nlmsg_type), h->nlmsg_type);
      return -1;
    }

  DEBUG_SDPLANE_LOG (NETLINK, "[%s] dst=%s/%u, nexthop=%s",
                     action_type,
                     dst_addr, msg_route_entry.dst.plen,
                     netlink_format_nexthop_object (
                          msg_route_entry.nh_type,
                          &msg_route_entry.nh,
                          gw_addr, sizeof (gw_addr))
                     );
  msgp = internal_msg_create (msg_type, &msg_route_entry,
                              sizeof (msg_route_entry));
  if (! msgp)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "internal_message create failed");
      return -1;
    }

  if (! msg_queue_rib)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "error: msg_queue_rib is not started.");
      internal_msg_delete (msgp);
      return -1;
    }

  int ret;
  ret = internal_msg_send_to (msg_queue_rib, msgp, NULL);
  if (ret < 0)
    {
      WARNING ("send imsg to msg_queue_rib (%p) failed.",
               msg_queue_rib);
    }

  return 0;
}

int
netlink_read_nlmsg_nexthop (struct netlink_sock *nlsock, struct nlmsghdr *h)
{
  if (h->nlmsg_len < NLMSG_LENGTH (sizeof (struct ifinfomsg)))
    {
      DEBUG_SDPLANE_LOG (NETLINK,
                         "%s: invalid len: %d < "
                         "NLMSG_LEN(ifinfomsg): %d",
                         nlsock->name, h->nlmsg_len,
                         NLMSG_LENGTH (sizeof (struct ifinfomsg)));
      return -1;
    }

    // TODO: implement nexthop handling

  DEBUG_SDPLANE_LOG (NETLINK, "%s: nexthop message received: type=%s(%u)",
                     nlsock->name, netlink_nlmsg_str (h->nlmsg_type),
                     h->nlmsg_type);
  return 0;
}

int
netlink_read_nlmsg_link (struct netlink_sock *nlsock, struct nlmsghdr *h)
{
  if (h->nlmsg_len < NLMSG_LENGTH (sizeof (struct ifinfomsg)))
    {
      DEBUG_SDPLANE_LOG (NETLINK,
                         "%s: invalid len: %d < "
                         "NLMSG_LEN(ifinfomsg): %d",
                         nlsock->name, h->nlmsg_len,
                         NLMSG_LENGTH (sizeof (struct ifinfomsg)));
      return -1;
    }

  struct ifinfomsg *ifi = (struct ifinfomsg *) NLMSG_DATA (h);
  struct rtattr *rtas[IFLA_MAX + 1] = { 0 };
  char ifname[16] = { 0 };
  if_indextoname (ifi->ifi_index, ifname);
  struct rtattr *rta = (struct rtattr *) IFLA_RTA (ifi);
  netlink_get_rtattr (rtas, IFLA_MAX, rta, IFLA_PAYLOAD (h));

  void *msgp = NULL;
  struct internal_msg_mac_addr msg_mac_addr;
  memset (&msg_mac_addr, 0, sizeof (msg_mac_addr));

  if (! rtas[IFLA_ADDRESS])
    {
      DEBUG_SDPLANE_LOG (NETLINK, "%s: no MAC address in message.",
                         nlsock->name);
      return -1;
    }

  memcpy (msg_mac_addr.ifname, ifname, sizeof (ifname));
  memcpy (&msg_mac_addr.mac_addr, RTA_DATA (rtas[IFLA_ADDRESS]),
          sizeof (struct rte_ether_addr));

  uint16_t msg_type;

  if (h->nlmsg_type == RTM_NEWLINK)
    msg_type = INTERNAL_MSG_TYPE_MAC_ADDR_ADD;
  else if (h->nlmsg_type == RTM_DELLINK)
    msg_type = INTERNAL_MSG_TYPE_MAC_ADDR_DEL;
  else
    {
      DEBUG_SDPLANE_LOG (NETLINK, "unexpected message type: %s(%u)",
                         netlink_nlmsg_str (h->nlmsg_type), h->nlmsg_type);
      return -1;
    }

  char mac_addr_str[RTE_ETHER_ADDR_FMT_SIZE];
  rte_ether_format_addr (mac_addr_str, sizeof (mac_addr_str), &msg_mac_addr.mac_addr);
  DEBUG_SDPLANE_LOG (NETLINK, "[%s] ifname=%s mac=%s ifindex=%u",
                     (h->nlmsg_type == RTM_NEWLINK ? "NEW" : "DEL"),
                     ifname, mac_addr_str, ifi->ifi_index);

  msgp = internal_msg_create (msg_type, &msg_mac_addr,
                              sizeof (msg_mac_addr));
  if (! msgp)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "internal_message create failed");
      return -1;
    }

  if (! msg_queue_rib)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "error: msg_queue_rib is not started.");
      internal_msg_delete (msgp);
      return -1;
    }

  int ret;
  ret = internal_msg_send_to (msg_queue_rib, msgp, NULL);
  if (ret < 0)
    {
      WARNING ("send imsg to msg_queue_rib (%p) failed.",
               msg_queue_rib);
    }

  return 0;
}

int
netlink_read_nlmsg_addr (struct netlink_sock *nlsock, struct nlmsghdr *h)
{
  if (h->nlmsg_len < NLMSG_LENGTH (sizeof (struct ifaddrmsg)))
    {
      DEBUG_SDPLANE_LOG (NETLINK,
                         "%s: invalid len: %d < "
                         "NLMSG_LEN(ifaddrmsg): %d",
                         nlsock->name, h->nlmsg_len,
                         NLMSG_LENGTH (sizeof (struct ifaddrmsg)));
      return -1;
    }

  struct ifaddrmsg *ifa = (struct ifaddrmsg *) NLMSG_DATA (h);
  struct rtattr *rtas[IFA_MAX + 1] = { 0 };
  char ifname[16] = { 0 };
  if_indextoname (ifa->ifa_index, ifname);
  struct rtattr *rta = (struct rtattr *) IFA_RTA (ifa);
  netlink_get_rtattr (rtas, IFA_MAX, rta, IFA_PAYLOAD (h));

  void *msgp = NULL;
  struct internal_msg_ip_addr msg_ip_addr;
  memset (&msg_ip_addr, 0, sizeof (msg_ip_addr));

  if (! rtas[IFA_ADDRESS])
    {
      DEBUG_SDPLANE_LOG (NETLINK, "%s: no IP address in message.",
                         nlsock->name);
      return -1;
    }
  // if (ifa->ifa_scope == RT_SCOPE_LINK)
  //   {
  //     DEBUG_SDPLANE_LOG (NETLINK, "%s: Link Local address.",
  //                        nlsock->name);
  //     return -1;
  //   }

  char ip_addr_str[INET6_ADDRSTRLEN];
  memcpy (msg_ip_addr.ifname, ifname, sizeof (ifname));
  msg_ip_addr.family = ifa->ifa_family;
  switch (ifa->ifa_family)
    {
      case AF_INET:
        memcpy (&msg_ip_addr.ip_addr.ipv4_addr,
                RTA_DATA (rtas[IFA_ADDRESS]), sizeof (struct in_addr));
        inet_ntop (AF_INET, &msg_ip_addr.ip_addr.ipv4_addr, ip_addr_str, sizeof (ip_addr_str));
        break;

      case AF_INET6:
        if (ifa->ifa_scope == RT_SCOPE_LINK)
          {
            msg_ip_addr.is_ll_addr = true;
            memcpy (&msg_ip_addr.ip_addr.ipv6_addr,
                    RTA_DATA (rtas[IFA_ADDRESS]), sizeof (struct in6_addr));
            inet_ntop (AF_INET6, &msg_ip_addr.ip_addr.ipv6_addr, ip_addr_str, sizeof (ip_addr_str));
          }
        else
          {
            memcpy (&msg_ip_addr.ip_addr.ipv6_addr,
                    RTA_DATA (rtas[IFA_ADDRESS]), sizeof (struct in6_addr));
            inet_ntop (AF_INET6, &msg_ip_addr.ip_addr.ipv6_addr, ip_addr_str, sizeof (ip_addr_str));
          }
        break;

      default:
        return -1;
    }

  uint16_t msg_type;

  if (h->nlmsg_type == RTM_NEWADDR)
    msg_type = INTERNAL_MSG_TYPE_IP_ADDR_ADD;
  else if (h->nlmsg_type == RTM_DELADDR)
    msg_type = INTERNAL_MSG_TYPE_IP_ADDR_DEL;
  else
    {
      DEBUG_SDPLANE_LOG (NETLINK, "unexpected message type: %s(%u)",
                         netlink_nlmsg_str (h->nlmsg_type), h->nlmsg_type);
      return -1;
    }

  DEBUG_SDPLANE_LOG (NETLINK, "[%s] ifname=%s ip_addr=%s ifindex=%u",
                     (h->nlmsg_type == RTM_NEWADDR ? "NEW" : "DEL"),
                     ifname, ip_addr_str, ifa->ifa_index);

  msgp = internal_msg_create (msg_type, &msg_ip_addr,
                              sizeof (msg_ip_addr));
  if (! msgp)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "internal_message create failed");
      return -1;
    }

  if (! msg_queue_rib)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "error: msg_queue_rib is not started.");
      internal_msg_delete (msgp);
      return -1;
    }

  int ret;
  ret = internal_msg_send_to (msg_queue_rib, msgp, NULL);
  if (ret < 0)
    {
      WARNING ("send imsg to msg_queue_rib (%p) failed.",
               msg_queue_rib);
    }

  if (ifa->ifa_family == AF_INET6 &&
      ifa->ifa_scope == RT_SCOPE_LINK)
    {
#if 0
      DEBUG_NEW (NETLINK_HOOK,
                 "omit calling nlhook_check on IPv6 lladdr: %s%%%s",
                 ip_addr_str, ifname);
#endif
    }
  else
    {
      DEBUG_NEW (NETLINK_HOOK,
                 "calling nlhook_check on ifaddr: %s%%%s",
                 ip_addr_str, ifname);
      nlhook_check_ifaddr (h->nlmsg_type, ifa->ifa_index, ifname,
                           ifa->ifa_family, ifa->ifa_prefixlen,
                           (char *)RTA_DATA (rtas[IFA_ADDRESS]));
    }

  return 0;
}

unsigned int msg_count = 0;

int
netlink_read (struct netlink_sock *nlsock)
{
  char buf[8192];
  struct iovec iov[1];
  struct sockaddr_nl snl;
  struct msghdr msg;

  bool netlink_read_done = false;

  iov[0].iov_base = (void *) buf;
  iov[0].iov_len = sizeof (buf);
  msg.msg_name = (void *) &snl;
  msg.msg_namelen = sizeof (snl);
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;

  int retval = 0;

  int ret;
  int flags = 0;

  while (1)
    {
      ret = recvmsg (nlsock->sockfd, &msg, flags);
      if (ret < 0)
        {
          if (errno == EINTR)
            continue;
          if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
#if 0
              DEBUG_NEW (NETLINK, "%s: read end: recvmsg(): ret: %d %s",
                         nlsock->name, ret, strerror (errno));
#endif
              break;
            }
          DEBUG_NEW (NETLINK, "%s: recvmsg(): ret: %d error: %s",
                     nlsock->name, ret, strerror (errno));
          continue;
        }
      msg_count++;

      if (snl.nl_pid != 0)
        {
          DEBUG_NEW (NETLINK, "%s: ignore message from pid %u",
                     nlsock->name, snl.nl_pid);
          continue;
        }

      if (ret == 0)
        {
          DEBUG_NEW (NETLINK, "%s: EOF", nlsock->name);
          return -1;
        }

      if (msg.msg_namelen != sizeof (snl))
        {
          DEBUG_NEW (NETLINK, "%s: sender addr len error: %d",
                     nlsock->name, msg.msg_namelen);
          return -1;
        }

      struct nlmsghdr *h;
      int len;
      len = ret;
      for (h = (struct nlmsghdr *) buf; NLMSG_OK (h, len);
           h = NLMSG_NEXT (h, len))
        {
          DEBUG_NEW (NETLINK, "%s: type: %s(%u) seq: %lu pid: %lu msg: %d",
                     nlsock->name, netlink_nlmsg_str (h->nlmsg_type),
                     h->nlmsg_type, h->nlmsg_seq, h->nlmsg_pid, msg_count);
          switch (h->nlmsg_type)
            {
            case NLMSG_ERROR:
              netlink_read_nlmsg_error (nlsock, h);
              break;
            case NLMSG_DONE:
              netlink_read_done = true;
              break;
            case RTM_NEWNEIGH:
            case RTM_DELNEIGH:
              netlink_read_nlmsg_neigh (nlsock, h);
              break;
            case RTM_NEWROUTE:
            case RTM_DELROUTE:
              netlink_read_nlmsg_route (nlsock, h);
              break;
            case RTM_NEWLINK:
            case RTM_DELLINK:
              netlink_read_nlmsg_link (nlsock, h);
              break;
            case RTM_NEWADDR:
            case RTM_DELADDR:
              netlink_read_nlmsg_addr (nlsock, h);
              break;
            case RTM_NEWNEXTHOP:
            case RTM_DELNEXTHOP:
              netlink_read_nlmsg_nexthop (nlsock, h);
              break;
            default:
              break;
            }
        }

      if (netlink_read_done)
        {
          DEBUG_NEW (NETLINK, "%s: read end: netlink_read_done",
                     nlsock->name);
          break;
        }

      if (1) //always
        {
          /* yield */
          DEBUG_NEW (NETLINK, "%s: yield at msg[%d].",
                     nlsock->name, msg_count);
          urcu_qsbr_quiescent_state ();
          if (IS_LTHREAD ())
            lthread_sleep (0);
        }
    }

  return retval;
}

int
netlink_read_block (struct netlink_sock *nlsock)
{
  int ret;
  int flags;
  int retval;

  ret = fcntl (nlsock->sockfd, F_GETFL, 0);
  if (ret < 0)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "fcntl(): F_GETFL failed: %s.",
                         strerror (errno));
      return -1;
    }

  flags = ret;
  flags &= ~O_NONBLOCK;

  ret = fcntl (nlsock->sockfd, F_SETFL, flags);
  if (ret < 0)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "fcntl(): F_SETFL failed: %s.",
                         strerror (errno));
      return -1;
    }

  retval = netlink_read (nlsock);

  flags |= O_NONBLOCK;
  ret = fcntl (nlsock->sockfd, F_SETFL, flags);
  if (ret < 0)
    {
      DEBUG_SDPLANE_LOG (NETLINK, "fcntl(): F_SETFL failed: %s.",
                         strerror (errno));
    }

  return retval;
}

void neigh_manager_init ();

void
netlink_thread (void *arg)
{
  unsigned lcore_id = rte_lcore_id ();

  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);
  DEBUG_SDPLANE_LOG (NETLINK, "%s: started.", __func__);

  /* In case if the netlink is launched earlier,
     initialize the neigh_manager. */
  neigh_manager_init ();

  int thread_id;
  thread_id = thread_lookup_by_lcore (netlink_thread, lcore_id);
  thread_register_loop_counter (thread_id, &loop_counter);

  if (IS_LTHREAD ())
    DEBUG_NEW (NETLINK, "started as a lthread.");
  else
    DEBUG_NEW (NETLINK, "started on lcore: %d.", lcore_id);

#if HAVE_LIBURCU_QSBR
  if (! IS_LTHREAD ())
    urcu_qsbr_register_thread ();
#endif /*HAVE_LIBURCU_QSBR*/

  unsigned long listen_groups;
  listen_groups = RTMGRP_LINK;
  listen_groups |= RTMGRP_IPV4_ROUTE | RTMGRP_IPV4_IFADDR;
  listen_groups |= RTMGRP_IPV6_ROUTE | RTMGRP_IPV6_IFADDR;
  listen_groups |= RTMGRP_NEIGH;
  netlink_socket (&netlink_kernel, listen_groups);
  netlink_socket (&netlink_cmd, 0);

  /* wait for startup_config to complete before subscribing to routes */
  while (! startup_config_completed &&
         ! force_quit && ! force_stop[lthread_core])
    {
      if (IS_LTHREAD ())
        lthread_sleep (100);
      else
        usleep (100);
      urcu_qsbr_quiescent_state ();
    }

  DEBUG_NEW (NETLINK, "%s: startup_config completed, starting subscription.",
             __func__);

  int ret;
#define NETLINK_REQUEST_CMD(family, type, nlsock)                             \
  do                                                                          \
    {                                                                         \
      ret = netlink_request (family, type, nlsock);                           \
      if (ret < 0)                                                            \
        DEBUG_SDPLANE_LOG (NETLINK, "netlink_request: %s %s failed.",         \
                           #family, #type);                                   \
      else                                                                    \
        DEBUG_SDPLANE_LOG (NETLINK,                                           \
                           "netlink_request: %s %s "                          \
                           "succeeded: %s seq: %d",                           \
                           #family, #type, (nlsock)->name, (nlsock)->seq);    \
    }                                                                         \
  while (0)

  NETLINK_REQUEST_CMD (AF_PACKET, RTM_GETLINK, &netlink_cmd);
  netlink_read_block (&netlink_cmd);

  NETLINK_REQUEST_CMD (AF_INET, RTM_GETADDR, &netlink_cmd);
  netlink_read_block (&netlink_cmd);

  NETLINK_REQUEST_CMD (AF_INET6, RTM_GETADDR, &netlink_cmd);
  netlink_read_block (&netlink_cmd);

  NETLINK_REQUEST_CMD (AF_INET, RTM_GETROUTE, &netlink_cmd);
  netlink_read_block (&netlink_cmd);

  NETLINK_REQUEST_CMD (AF_INET, RTM_GETNEIGH, &netlink_cmd);
  netlink_read_block (&netlink_cmd);

  NETLINK_REQUEST_CMD (AF_INET6, RTM_GETROUTE, &netlink_cmd);
  netlink_read_block (&netlink_cmd);

  NETLINK_REQUEST_CMD (AF_INET6, RTM_GETNEIGH, &netlink_cmd);
  netlink_read_block (&netlink_cmd);

  while (! force_quit && ! force_stop[lthread_core])
    {
      if (IS_LTHREAD ())
        lthread_sleep (0); // yield.
      // DEBUG_SDPLANE_LOG (NETLINK, "%s: schedule.", __func__);

      netlink_read (&netlink_cmd);
      netlink_read (&netlink_kernel);

      netlink_hook_exec ();

      urcu_qsbr_quiescent_state ();
      loop_counter++;
    }

#if HAVE_LIBURCU_QSBR
  if (! IS_LTHREAD ())
    urcu_qsbr_unregister_thread ();
#endif /*HAVE_LIBURCU_QSBR*/

  DEBUG_SDPLANE_LOG (NETLINK, "%s: terminating.", __func__);
  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);
}
