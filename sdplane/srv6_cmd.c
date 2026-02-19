#include "include.h"

#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>

#include <sdplane/debug_log.h>
#include "debug_sdplane.h"

#include "sdplane.h"

#include "rib.h"
#include "internal_message.h"

// clang-format off

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
  srv6_local_sid.srv6_local_sid_addr_index = index;

  msgp = internal_msg_create (INTERNAL_MSG_TYPE_SRV6_LOCAL_SID,
                              &srv6_local_sid, sizeof (srv6_local_sid));
  ret = internal_msg_send_to (msg_queue_rib, msgp, shell);
  if (ret < 0)
    {
      return CMD_FAILURE;
    }

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
  char addr_str[64];

  if (! rib || ! rib->rib_info)
    return CMD_SUCCESS;

  int i;
  int limit = rib->rib_info->srv6_local_sid_addr_num;

  for (i = 0; i < limit; i++)
    {
  inet_ntop (AF_INET6, &rib->rib_info->srv6_local_sid_addr[i],
             addr_str, sizeof (addr_str));
  fprintf (shell->terminal,
           "srv6 local-sid addr[%d/%d]: %s%s",
           i, limit, addr_str, shell->NL);
    }

  return CMD_SUCCESS;
}

void
srv6_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, set_srv6_local_sid);
  INSTALL_COMMAND2 (cmdset, show_srv6_local_sid);
}
