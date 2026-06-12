#include "include.h"
#include <linux/seg6.h>
#include <linux/seg6_iptunnel.h>
#include <linux/seg6_local.h>
#include <netinet/in.h>
#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>
#include <sdplane/debug_log.h>

#include <net/if.h>
#include <stdint.h>
#include <sys/socket.h>

#include "nexthop.h"
#include "sdplane.h"
#include "rib.h"
#include "rib_manager.h"
#include "internal_message.h"
#include "netlink_seg6.h"
#include "srv6.h"

// clang-format off

static const char *
srv6_local_action_str (uint32_t action)
{
  switch (action)
    {
    case SEG6_LOCAL_ACTION_END:      return "End";
    case SEG6_LOCAL_ACTION_END_X:    return "End.X";
    case SEG6_LOCAL_ACTION_END_T:    return "End.T";
    case SEG6_LOCAL_ACTION_END_DX2:  return "End.DX2";
    case SEG6_LOCAL_ACTION_END_DX6:  return "End.DX6";
    case SEG6_LOCAL_ACTION_END_DX4:  return "End.DX4";
    case SEG6_LOCAL_ACTION_END_DT6:  return "End.DT6";
    case SEG6_LOCAL_ACTION_END_DT4:  return "End.DT4";
    case SEG6_LOCAL_ACTION_END_DT46: return "End.DT46";
    case SEG6_LOCAL_ACTION_UNSPEC:   return "(unspec)";
    default:                         return "Unknown";
    }
}

static void
srv6_local_sid_params_str (struct srv6_local_sid_entry *entry,
                           char *buf, size_t buflen)
{
  switch (entry->action)
    {
      case SEG6_LOCAL_ACTION_END:
        snprintf (buf, buflen, "-");
        break;
      case SEG6_LOCAL_ACTION_END_X:
        {
          char nh6[INET6_ADDRSTRLEN];
          char oif_name[IF_NAMESIZE] = { 0 };
          inet_ntop (AF_INET6, &entry->param.nh6, nh6, sizeof (nh6));
          if_indextoname (entry->oif, oif_name);
          snprintf (buf, buflen, "nh6 %s oif %s", nh6, oif_name);
          break;
        }
      case SEG6_LOCAL_ACTION_END_DX4:
        {
          char nh4[INET_ADDRSTRLEN];
          char oif_name[IF_NAMESIZE] = { 0 };
          inet_ntop (AF_INET, &entry->param.nh4, nh4, sizeof (nh4));
          if_indextoname (entry->oif, oif_name);
          snprintf (buf, buflen, "nh4 %s oif %s", nh4, oif_name);
          break;
        }
      case SEG6_LOCAL_ACTION_END_DX6:
        {
          char nh6[INET6_ADDRSTRLEN];
          char oif_name[IF_NAMESIZE] = { 0 };
          inet_ntop (AF_INET6, &entry->param.nh6, nh6, sizeof (nh6));
          if_indextoname (entry->oif, oif_name);
          snprintf (buf, buflen, "nh6 %s oif %s", nh6, oif_name);
          break;
        }
      case SEG6_LOCAL_ACTION_END_T:
      case SEG6_LOCAL_ACTION_END_DT4:
      case SEG6_LOCAL_ACTION_END_DT6:
      case SEG6_LOCAL_ACTION_END_DT46:
        snprintf (buf, buflen, "table %u", entry->param.table_id);
        break;
      default:
        snprintf (buf, buflen, "-");
        break;
    }
}

CLI_COMMAND2 (set_srv6_local_sid,
              "set srv6 local-sid <WORD> (|<0-15>)",
              SET_HELP,
              "set SRv6 information\n",
              "set SRv6 local-SID information\n",
              "specify SRv6 local-SID address\n",
              "specify SRv6 local-SID address index\n"
              )
{
  struct shell *shell = (struct shell *) context;
  int ret;
  void *msgp;
  struct internal_msg_srv6_local_sid srv6_local_sid;
  struct rib *rib = rib_tlocal;

  memset (&srv6_local_sid, 0, sizeof (srv6_local_sid));

  int index;
  if (argc > 4)
    index = strtol (argv[4], NULL, 0);
  else if (! rib || ! rib->rib_info)
    index = 0;
  else
    index = rib->rib_info->srv6_local_sid_addr_num;
  inet_pton (AF_INET6, argv[3],
             &srv6_local_sid.srv6_local_sid_addr);
  srv6_local_sid.action = SEG6_LOCAL_ACTION_END;
  srv6_local_sid.srv6_local_sid_addr_index = index;

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_SRV6_LOCAL_SID_ADD,
                              &srv6_local_sid, sizeof (srv6_local_sid));
  ret = internal_msg_send_to (msg_queue_rib, msgp, shell);
  if (ret < 0)
    {
      return CMD_FAILURE;
    }

  return CMD_SUCCESS;
}

CLI_COMMAND2 (no_set_srv6_local_sid,
              "no set srv6 local-sid <WORD>",
              NO_HELP,
              SET_HELP,
              "set SRv6 information\n",
              "delete SRv6 local-SID entry\n",
              "specify SRv6 local-SID address to delete\n"
              )
{
  struct shell *shell = (struct shell *) context;
  int ret;
  void *msgp;
  struct internal_msg_srv6_local_sid msg;

  memset (&msg, 0, sizeof (msg));
  if (inet_pton (AF_INET6, argv[4], &msg.srv6_local_sid_addr) != 1)
    {
      fprintf (shell->terminal, "Invalid IPv6 address: %s%s", argv[4], shell->NL);
      return CMD_FAILURE;
    }

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_SRV6_LOCAL_SID_DEL,
                              &msg, sizeof (msg));
  ret = internal_msg_send_to (msg_queue_rib, msgp, shell);
  if (ret < 0)
    return CMD_FAILURE;

  return CMD_SUCCESS;
}

CLI_COMMAND2 (show_srv6_local_sid,
              "show srv6 local-sid",
              SHOW_HELP,
              "show SRv6 information\n",
              "show SRv6 local-SID information\n")
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;

  if (! rib || ! rib->rib_info)
    return CMD_SUCCESS;

  int i;
  int limit = rib->rib_info->srv6_local_sid_addr_num;

  fprintf (shell->terminal, "SRv6 Local SID Table:%s", shell->NL);
  fprintf (shell->terminal, "  Total: %d %s%s",
           limit, limit == 1 ? "entry" : "entries", shell->NL);

  if (limit == 0)
    return CMD_SUCCESS;

  fprintf (shell->terminal, "  %-44s %-10s %s%s",
           "SID", "Behavior", "Parameters", shell->NL);
  fprintf (shell->terminal,
           "  -------------------------------------------"
           " ---------- ---------------------------%s", shell->NL);

  for (i = 0; i < limit; i++)
    {
      struct srv6_local_sid_entry *entry = &rib->rib_info->srv6_local_sid[i];
      char sid_str[INET6_ADDRSTRLEN];
      char params_str[64];

      inet_ntop (AF_INET6, &entry->sid, sid_str, sizeof (sid_str));
      srv6_local_sid_params_str (entry, params_str, sizeof (params_str));

      fprintf (shell->terminal, "  %-44s %-10s %s%s",
               sid_str, srv6_local_action_str (entry->action),
               params_str, shell->NL);
    }

  return CMD_SUCCESS;
}


bool enable_srv6_encap_mode = false;

CLI_COMMAND2 (set_srv6_encap_enable,
              "set srv6 encap enable",
              SET_HELP,
              "set SRv6 information\n",
              "enable SRv6 encapsulation mode\n"
              )
{
  struct shell *shell = (struct shell *) context;

  enable_srv6_encap_mode = true;
  fprintf (shell->terminal, "SRv6 encapsulation mode enabled.%s", shell->NL);
  return CMD_SUCCESS;
}

CLI_COMMAND2 (set_srv6_encap_local_sid,
              "set srv6 encap local-sid <WORD> "
              "(|End|End.X|End.T|End.DX2|End.DX6|End.DX4|End.DT6|End.DT4|End.DT46)",
              SET_HELP,
              "set SRv6 information\n",
              "set SRv6 encapsulation information\n",
              "specify SRv6 encap local-SID address\n",
              "specify SRv6 encap local-SID behavior\n"
              )
{
  struct shell *shell = (struct shell *) context;
  int ret;
  void *msgp;
  struct rib *rib = rib_tlocal;
  struct in6_addr tmp_addr;
  struct internal_msg_route_entry* srv6_local_sid;
  int action;

  if (! enable_srv6_encap_mode)
    {
      fprintf (shell->terminal,
        "SRv6 encapsulation mode is not enabled. "
              "Please enable it first with 'set srv6 encap enable'.%s",
               shell->NL);
      return CMD_FAILURE;
    }

  if(inet_pton (AF_INET6, argv[3],
             &tmp_addr) <= 0)
    {
      fprintf (shell->terminal, "Invalid IPv6 address: %s%s", argv[3], shell->NL);
      return CMD_FAILURE;
    }

  if (argc > 4 && strcmp (argv[4], "End") == 0)
    action = SEG6_LOCAL_ACTION_END;
  else
    {
      fprintf (shell->terminal, "Unsupported SRv6 local-SID behavior: %s%s", argv[4], shell->NL);
      return CMD_FAILURE;
    }

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_ROUTE_ENTRY_ADD,
                              NULL, sizeof (*srv6_local_sid));

  srv6_local_sid = internal_msg_body(msgp);

  srv6_local_sid->dst.family = AF_INET6;
  srv6_local_sid->dst.dst_ip_addr.ipv6_addr = tmp_addr;
  srv6_local_sid->dst.plen = 128;
  srv6_local_sid->family = AF_INET6;
  srv6_local_sid->is_nhid = false;
  srv6_local_sid->nh.nhcnt = 1;
  srv6_local_sid->nh.members[0].info.family = AF_INET6;
  srv6_local_sid->nh.members[0].info.type = NEXTHOP_TYPE_SRV6_END;
  srv6_local_sid->nh.members[0].info.srv6_local_sid.action = action;
  srv6_local_sid->nh.members[0].info.srv6_local_sid.sid =
                          srv6_local_sid->dst.dst_ip_addr.ipv6_addr;
  srv6_local_sid->nh.members[0].weight = 1;

  ret = internal_msg_send_to (msg_queue_rib, msgp, shell);
  if (ret < 0)
    {
      return CMD_FAILURE;
    }

  return CMD_SUCCESS;
}


CLI_COMMAND2 (no_set_srv6_encap_local_sid,
              "no set srv6 encap local-sid <WORD>",
              NO_HELP,
              SET_HELP,
              "set SRv6 information\n",
              "set SRv6 encapsulation information\n",
              "delete SRv6 local-SID entry\n",
              "specify SRv6 local-SID address to delete\n"
              )
{
  struct shell *shell = (struct shell *) context;
  int ret;
  void *msgp;
  struct in6_addr tmp_addr;
  struct internal_msg_route_entry* msg;

  if (! enable_srv6_encap_mode)
    {
      fprintf (shell->terminal,
        "SRv6 encapsulation mode is not enabled. "
              "Please enable it first with 'set srv6 encap enable'.%s",
               shell->NL);
      return CMD_FAILURE;
    }

  if (inet_pton (AF_INET6, argv[4], &tmp_addr) != 1)
    {
      fprintf (shell->terminal, "Invalid IPv6 address: %s%s", argv[4], shell->NL);
      return CMD_FAILURE;
    }

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_ROUTE_ENTRY_DEL,
                              NULL, sizeof (*msg));

  msg = internal_msg_body (msgp);
  msg->family = AF_INET6;
  msg->dst.family = AF_INET6;
  msg->dst.dst_ip_addr.ipv6_addr = tmp_addr;
  msg->dst.plen = 128;
  msg->is_nhid = false;
  msg->nh.nhcnt = 1;
  msg->nh.members[0].info.family = AF_INET6;
  msg->nh.members[0].info.type = NEXTHOP_TYPE_SRV6_END;
  msg->nh.members[0].info.srv6_local_sid.action = SEG6_LOCAL_ACTION_END;
  msg->nh.members[0].info.srv6_local_sid.sid = msg->dst.dst_ip_addr.ipv6_addr;
  msg->nh.members[0].weight = 1;

  ret = internal_msg_send_to (msg_queue_rib, msgp, shell);
  if (ret < 0)
    return CMD_FAILURE;

  return CMD_SUCCESS;
}


CLI_COMMAND2(set_srv6_encap_src,"set srv6 encap src <WORD>",
              SET_HELP,
              "set SRv6 information\n",
              "set SRv6 encapsulation information\n",
             "set SRv6 encapsulation source address\n",
             "specify SRv6 encapsulation source address\n"
             )
{
  struct shell *shell = (struct shell *) context;
  int ret;
  void *msgp;
  struct internal_msg_srv6_encap_src* srv6_encap_src;
  struct rib *rib = rib_tlocal;
  char* addr_ptr = argv[4];
  struct in6_addr tmp_addr;

  if(inet_pton (AF_INET6,addr_ptr,&tmp_addr) <= 0)
    {
      fprintf (shell->terminal,"Invalid IPv6 address: %s%s", addr_ptr,
               shell->NL);
      return CMD_FAILURE;
    }

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_SRV6_ENCAP_SRC_SET, NULL,
                sizeof (struct internal_msg_srv6_encap_src));
  srv6_encap_src = internal_msg_body (msgp);
  memcpy(&srv6_encap_src->src_addr, &tmp_addr, sizeof(struct in6_addr));

  ret = internal_msg_send_to (msg_queue_rib, msgp, shell);
  if (ret < 0)
    {
      return CMD_FAILURE;
    }

  return CMD_SUCCESS;
}

CLI_COMMAND2(show_srv6_encap_src,"show srv6 encap src",
              SHOW_HELP,
              "show SRv6 information\n",
              "show SRv6 encapsulation information\n",
              "show SRv6 encapsulation source address\n"
             )
{
  struct shell *shell = (struct shell *) context;
  char ip_str[INET6_ADDRSTRLEN];
  inet_ntop (AF_INET6, &rib_tlocal->rib_info->srv6_tunnel_src_addr,
          ip_str, sizeof (ip_str));
  fprintf (shell->terminal,
    "SRv6 encapsulation source address: %s%s", ip_str, shell->NL);
  return CMD_SUCCESS;
}

/*
 * Parse arguments shared by "set srv6 encap head" and
 * "no set srv6 encap head" into a route entry message body.
 * Returns 0 on success, -1 on error (an error message is printed).
 */
static int
srv6_encap_head_parse_route (struct shell *shell, char *prefix_arg,
                             char *segments_arg, char *action_arg,
                             struct internal_msg_route_entry *route)
{
  char prefix_str[INET6_ADDRSTRLEN + 8];
  char segments_str[MAX_SEGMENT_LIST_SIZE * (INET6_ADDRSTRLEN + 1)];
  char *plen_str, *seg_ptr, *comma;
  struct in6_addr dst_addr;
  struct in6_addr segments[MAX_SEGMENT_LIST_SIZE];
  int segment_count = 0;
  int plen = 128;
  int i;

  /* only H.Encaps is implemented in the data plane for now */
  if (action_arg && strcmp (action_arg, "H.Encaps"))
    {
      fprintf (shell->terminal, "Unsupported SRv6 encap action: %s%s",
               action_arg, shell->NL);
      return -1;
    }

  /* destination prefix: ADDR or ADDR/PLEN (default /128) */
  snprintf (prefix_str, sizeof (prefix_str), "%s", prefix_arg);
  plen_str = strchr (prefix_str, '/');
  if (plen_str)
    {
      *plen_str++ = '\0';
      plen = strtol (plen_str, NULL, 10);
      if (plen < 0 || plen > 128)
        {
          fprintf (shell->terminal, "Invalid prefix length: %s%s",
                   plen_str, shell->NL);
          return -1;
        }
    }
  if (inet_pton (AF_INET6, prefix_str, &dst_addr) != 1)
    {
      fprintf (shell->terminal, "Invalid IPv6 address: %s%s",
               prefix_str, shell->NL);
      return -1;
    }

  /* comma-separated segment list, given in path order */
  if (snprintf (segments_str, sizeof (segments_str), "%s", segments_arg)
      >= (int) sizeof (segments_str))
    {
      fprintf (shell->terminal, "Too many SRv6 segments (max: %d)%s",
               MAX_SEGMENT_LIST_SIZE, shell->NL);
      return -1;
    }
  seg_ptr = segments_str;
  while (seg_ptr)
    {
      comma = strchr (seg_ptr, ',');
      if (comma)
        *comma++ = '\0';
      if (segment_count >= MAX_SEGMENT_LIST_SIZE)
        {
          fprintf (shell->terminal, "Too many SRv6 segments (max: %d)%s",
                   MAX_SEGMENT_LIST_SIZE, shell->NL);
          return -1;
        }
      if (inet_pton (AF_INET6, seg_ptr, &segments[segment_count]) != 1)
        {
          fprintf (shell->terminal, "Invalid IPv6 address: %s%s",
                   seg_ptr, shell->NL);
          return -1;
        }
      segment_count++;
      seg_ptr = comma;
    }

  memset (route, 0, sizeof (*route));
  route->family = AF_INET6;
  route->table_id = 0;
  route->dst.family = AF_INET6;
  route->dst.dst_ip_addr.ipv6_addr = dst_addr;
  route->dst.plen = plen;
  route->is_nhid = false;
  route->nh.nhcnt = 1;
  route->nh.members[0].info.family = AF_INET6;
  route->nh.members[0].info.type = NEXTHOP_TYPE_SRV6_HEAD;
  route->nh.members[0].info.srv6_encap_sid.action = SEG6_IPTUN_MODE_ENCAP;
  route->nh.members[0].info.srv6_encap_sid.segment_list_size = segment_count;
  /* sids[] is in SRH order: sids[segment_count - 1] is the first segment
   * and becomes the outer destination address. */
  for (i = 0; i < segment_count; i++)
    route->nh.members[0].info.srv6_encap_sid.sids[segment_count - 1 - i] =
      segments[i];
  route->nh.members[0].weight = 1;

  return 0;
}

CLI_COMMAND2 (set_srv6_encap_head,
              "set srv6 encap head <WORD> <WORD> (|H.Encaps|H.Insert)",
              SET_HELP,
              "set SRv6 information\n",
              "set SRv6 encapsulation information\n",
              "set SRv6 encap head-end route\n",
              "specify destination prefix (e.g. 2001:db8::/64)\n",
              "specify SRv6 segments in path order (comma-separated)\n",
              "specify SRv6 encap action (default: H.Encaps)\n"
              )
{
  struct shell *shell = (struct shell *) context;
  int ret;
  void *msgp;
  struct internal_msg_route_entry route;

  if (! enable_srv6_encap_mode)
    {
      fprintf (shell->terminal,
        "SRv6 encapsulation mode is not enabled. "
              "Please enable it first with 'set srv6 encap enable'.%s",
               shell->NL);
      return CMD_FAILURE;
    }

  if (srv6_encap_head_parse_route (shell, argv[4], argv[5],
                                   argc > 6 ? argv[6] : NULL, &route) < 0)
    return CMD_FAILURE;

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_ROUTE_ENTRY_ADD,
                              &route, sizeof (route));
  ret = internal_msg_send_to (msg_queue_rib, msgp, shell);
  if (ret < 0)
    return CMD_FAILURE;

  return CMD_SUCCESS;
}

CLI_COMMAND2 (no_set_srv6_encap_head,
              "no set srv6 encap head <WORD> <WORD> (|H.Encaps|H.Insert)",
              NO_HELP,
              SET_HELP,
              "set SRv6 information\n",
              "set SRv6 encapsulation information\n",
              "delete SRv6 encap head-end route\n",
              "specify destination prefix (e.g. 2001:db8::/64)\n",
              "specify SRv6 segments in path order (comma-separated)\n",
              "specify SRv6 encap action (default: H.Encaps)\n"
              )
{
  struct shell *shell = (struct shell *) context;
  int ret;
  void *msgp;
  struct internal_msg_route_entry route;

  if (! enable_srv6_encap_mode)
    {
      fprintf (shell->terminal,
        "SRv6 encapsulation mode is not enabled. "
              "Please enable it first with 'set srv6 encap enable'.%s",
               shell->NL);
      return CMD_FAILURE;
    }

  if (srv6_encap_head_parse_route (shell, argv[5], argv[6],
                                   argc > 7 ? argv[7] : NULL, &route) < 0)
    return CMD_FAILURE;

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_ROUTE_ENTRY_DEL,
                              &route, sizeof (route));
  ret = internal_msg_send_to (msg_queue_rib, msgp, shell);
  if (ret < 0)
    return CMD_FAILURE;

  return CMD_SUCCESS;
}

void
srv6_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, set_srv6_local_sid);
  INSTALL_COMMAND2 (cmdset, no_set_srv6_local_sid);
  INSTALL_COMMAND2 (cmdset, show_srv6_local_sid);
  INSTALL_COMMAND2 (cmdset, set_srv6_encap_enable);
  INSTALL_COMMAND2 (cmdset, set_srv6_encap_local_sid);
  INSTALL_COMMAND2 (cmdset, no_set_srv6_encap_local_sid);
  INSTALL_COMMAND2 (cmdset, set_srv6_encap_head);
  INSTALL_COMMAND2 (cmdset, no_set_srv6_encap_head);
  INSTALL_COMMAND2 (cmdset, set_srv6_encap_src);
  INSTALL_COMMAND2 (cmdset, show_srv6_encap_src);
}
