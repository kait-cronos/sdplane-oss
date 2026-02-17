#include "include.h"

#include <sdplane/debug_cmd.h>
#include <sdplane/debug_log.h>
#include "debug_sdplane.h"

#include <sdplane/vector.h>
#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>

/* for netlink */
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

#include <pthread.h>

#include "netlink_hook.h"
#include "argv_list.h"
#include "sdplane.h"

pthread_mutex_t nlhook_mutex;
struct nlhook_entry nlhook_entries[NLHOOK_SIZE];

CLI_COMMAND2 (nlhook_ifaddr_ifname,
              "set netlink-hook <0-3> (ipv4|ipv6) ifaddr (new|del) "
              "ifname <WORD> argv-list <0-7>",
              SET_HELP)
{
  struct shell *shell = (struct shell *) context;
  int ret;

  int index;
  index = strtol (argv[2], NULL, 0);

  uint8_t family = 0;
  if (! strcmp (argv[3], "ipv4"))
    family = AF_INET;
  else if (! strcmp (argv[3], "ipv6"))
    family = AF_INET6;

  uint16_t nlmsg_type = 0;
  if (! strcmp (argv[5], "new"))
    nlmsg_type = RTM_NEWADDR;
  else
    nlmsg_type = RTM_DELADDR;

  char *ifname;
  ifname = strdup (argv[7]);

  int argv_list_index;
  argv_list_index = strtol (argv[9], NULL, 0);

  pthread_mutex_lock (&nlhook_mutex);
  memset (&nlhook_entries[index], 0, sizeof (struct nlhook_entry));
  nlhook_entries[index].flag = 0;
  nlhook_entries[index].nlmsg_type = nlmsg_type;
  nlhook_entries[index].family = family;
  nlhook_entries[index].ifname = ifname;
  nlhook_entries[index].argv_list_index = argv_list_index;
  pthread_mutex_unlock (&nlhook_mutex);

  return 0;
}

CLI_COMMAND2 (show_netlink_hook,
              "show netlink-hook (|<0-3>)",
              SHOW_HELP,
              "show netlink-hook.\n",
              "specify netlink-hok index.\n")
{
  struct shell *shell = (struct shell *) context;
  int i;

  int index = -1;
  if (argc > 2)
    index = strtol (argv[2], NULL, 0);

  pthread_mutex_lock (&nlhook_mutex);
  for (i = 0; i < NLHOOK_SIZE; i++)
    {
      struct nlhook_entry *nlhook;
      nlhook = &nlhook_entries[i];

      if (index >= 0 && i != index)
        continue;

      fprintf (shell->terminal, "netlink_hook[%d]:%s", i, shell->NL);
      fprintf (shell->terminal, "    shed: %d flag: %hx%s",
               nlhook->sched, nlhook->flag, shell->NL);
      fprintf (shell->terminal,
               "    nlmsg_type: %d family: %d prefix_len: %d%s",
               nlhook->nlmsg_type, nlhook->family, nlhook->prefix_len,
               shell->NL);
      fprintf (shell->terminal, "    ifindex: %d ifname: %s%s",
               nlhook->ifindex, nlhook->ifname, shell->NL);
      fprintf (shell->terminal, "    addr: not yet.%s", shell->NL);
      fprintf (shell->terminal, "    exec argv-list: %d%s",
               nlhook->argv_list_index, shell->NL);
    }
  pthread_mutex_unlock (&nlhook_mutex);

  return 0;
}

void
nlhook_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, nlhook_ifaddr_ifname);
  INSTALL_COMMAND2 (cmdset, show_netlink_hook);
}

void
nlhook_init ()
{
  pthread_mutex_init (&nlhook_mutex, NULL);
}
