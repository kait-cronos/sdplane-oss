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

#include "sdplane.h"
#include "rib.h"
#include "rib_manager.h"
#include "internal_message.h"
#include "netlink_seg6.h"

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

/*
CLI_COMMAND2(set_srv6_encap_sid,"set srv6 encap <WORD> <WORD> (|<0-15>)",
              SET_HELP, 
              "set SRv6 information\n",
              "set SRv6 encap information\n",
              "specify SRv6 Encaps addresses\n",
              "specify SRv6 action e.g. H.Encaps",
              "specify SRv6 policy id\n",
            )
{
  struct shell *shell = (struct shell *) context;
  int ret;
  void *msgp;
  struct internal_msg_srv6_encap_sid* srv6_encap;
  struct rib *rib = rib_tlocal;
  char* addr_ptr = argv[3];  
  struct in6_addr tmp_addr;
  size_t addr_count = 0;
  uint32_t action = SEG6_IPTUN_MODE_ENCAP;

  if(argc > 4) 
    {
      if (!strcmp (argv[4],"H.Encaps"))
        action = SEG6_IPTUN_MODE_ENCAP;
      else if (!strcmp (argv[4],"H.Insert"))
        action = SEG6_IPTUN_MODE_INLINE;
      else
        {
          fprintf (shell->terminal, "Invalid action: %s%s", argv[4], shell->NL);
          return CMD_FAILURE;
        }
    }

  while ((addr_ptr = strchr (addr_ptr, ',')) != NULL) 
    {
        addr_count++;
        addr_ptr++;
    }
  addr_count++; // number of addresses is number of commas + 1

  // because 
  // hdrlen = segment_count * 16 / 8 = segment_count * 2
  // and hdrlen is stored in 8 bits, the maximum segment_count is 127.
  if (addr_count > 127) 
    {
      fprintf (shell->terminal, "Too many SRv6 Encaps addresses: %zu (max: 127)%s", addr_count, shell->NL);
      return CMD_FAILURE;
    }

  

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_SRV6_ENCAP_SRH_ADD, NULL,sizeof (struct internal_msg_srv6_encap_sid) + sizeof (struct ipv6_sr_hdr) + sizeof(struct in6_addr) * addr_count);

  struct internal_msg_srv6_encap_sid* body = internal_msg_body (msgp);
  body->action = action;
  body->hdr->first_segment = addr_count - 1;
  body->hdr->segments_left = addr_count - 1;
  body->hdr->hdrlen = body->hdr->first_segment * 2; // in 8-octet unit
  body->hdr->type = 4; // TODO: replace with constant definition
  body->hdr->flags = 0; // TODO: support flags if needed
  body->hdr->tag = 0; // unused

  size_t i = 0;
  addr_ptr = argv[3]; // reset

  do {
    char* last_ptr = strchr (addr_ptr,',');
    if(last_ptr) 
      {
        if (i == addr_count - 1) 
          {
            fprintf (shell->terminal, "Address count mismatch. Expected %zu addresses but got more.%s", addr_count, shell->NL);
            internal_msg_delete (msgp);
            return CMD_FAILURE;
          }
        *last_ptr = '\0';
        if(inet_pton (AF_INET6,addr_ptr,&body->hdr->segments[addr_count - 1 - i]) < 0) 
          {
            fprintf (shell->terminal,"Invalid IPv6 address: %s%s", addr_ptr, shell->NL);
            internal_msg_delete (msgp);
            return CMD_FAILURE;
          }
        i++;
        addr_ptr = last_ptr + 1;
      }
    else 
      {
        if(i != addr_count - 1) 
          {
            fprintf (shell->terminal, "Address count mismatch. Expected %zu addresses but got %zu.%s", addr_count, i + 1, shell->NL);
            internal_msg_delete (msgp);
            return CMD_FAILURE;
          }
        if(inet_pton (AF_INET6,addr_ptr,&body->hdr->segments[addr_count - 1 - i]) < 0) 
          {
            fprintf (shell->terminal,"Invalid IPv6 address: %s%s", addr_ptr, shell->NL);
            internal_msg_delete (msgp);
            return CMD_FAILURE;
          }
        break;
      }
  }while(addr_ptr);

  ret = internal_msg_send_to (msg_queue_rib, msgp, shell);
  if (ret < 0)
    {
      return CMD_FAILURE;
    }

  return CMD_SUCCESS;
}

CLI_COMMAND2 (set_srv6_local_sid,
              "set srv6 local-sid <WORD> <WORD> (|<0-15>)",
              SET_HELP,
              "set SRv6 information\n",
              "set SRv6 local-SID information\n",
              "specify SRv6 local-SID address\n",
              "specify SRv6 action e.g. End,End.X\n",
              "specify SRv6 local-SID address index\n",
              )
{
  struct shell *shell = (struct shell *) context;
  int ret;
  void *msgp;
  struct internal_msg_srv6_local_sid srv6_local_sid;
  struct rib *rib = rib_tlocal;

  memset (&srv6_local_sid, 0, sizeof (srv6_local_sid));

  int index;
  if (argc > 5)
    index = strtol (argv[5], NULL, 0);
  else if (! rib || ! rib->rib_info)
    index = 0;
  else
    index = rib->rib_info->srv6_local_sid_addr_num;
  inet_pton (AF_INET6, argv[3],
             &srv6_local_sid.srv6_local_sid_addr);
  
  int action = SEG6_LOCAL_ACTION_END;
  if (argc > 4) 
    {
      if (!strcmp(argv[4],"End"))
        action = SEG6_LOCAL_ACTION_END;
      else if (!strcmp(argv[4],"End.X"))
        action = SEG6_LOCAL_ACTION_END_X;
      else if (!strcmp(argv[4],"End.T"))
        action = SEG6_LOCAL_ACTION_END_T;
      else if (!strcmp(argv[4],"End.DX4"))
        action = SEG6_LOCAL_ACTION_END_DX4;
      else if (!strcmp(argv[4],"End.DX6"))
        action = SEG6_LOCAL_ACTION_END_DX6;
      else if (!strcmp(argv[4],"End.DT4"))
        action = SEG6_LOCAL_ACTION_END_DT4;
      else if (!strcmp(argv[4],"End.DT6"))
        action = SEG6_LOCAL_ACTION_END_DT6;
      else
        {
          fprintf (shell->terminal, "Invalid action: %s%s", argv[4], shell->NL);
          return CMD_FAILURE;
        }
    }

  
  srv6_local_sid.action = action;
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
*/
void
srv6_cmd_init (struct command_set *cmdset)
{
  //INSTALL_COMMAND2 (cmdset, set_srv6_local_sid);
  //INSTALL_COMMAND2 (cmdset, no_set_srv6_local_sid);
  //INSTALL_COMMAND2 (cmdset, show_srv6_local_sid);
  //INSTALL_COMMAND2(cmdset, set_srv6_encap_sid);
}
