#include "include.h"

#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>
#include <sdplane/debug_log.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "sdplane.h"
#include "rib.h"
#include "rib_manager.h"
#include "igmp_report.h"

// clang-format off

/* IGMP Join via Kernel command */

struct igmp_join igmp_join_table[MAX_IGMP_JOIN_GROUPS];

static struct igmp_join *
igmp_join_lookup (int vswitch_id, struct in_addr *group_addr)
{
  int i;
  for (i = 0; i < MAX_IGMP_JOIN_GROUPS; i++)
    {
      struct igmp_join *entry = &igmp_join_table[i];
        if (entry->used && entry->vswitch_id == vswitch_id &&
            entry->group_addr.s_addr == group_addr->s_addr)
        {
          return entry;
        }
    }
  return NULL;
}

struct router_if *
igmp_router_if_lookup (struct rib *rib, int vswitch_id)
{
  int i;
  if (! rib || ! rib->rib_info)
    return NULL;
  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vswitch = &rib->rib_info->vswitch[i];
      if (vswitch->vswitch_id == vswitch_id && ! vswitch->is_deleted)
        {
          return &vswitch->router_if;
        }
    }
  return NULL;
}

CLI_COMMAND2 (set_vswitch_igmp_join,
              "set vswitch igmp join <1-4094> "IPV4_ADDR_SPEC,
              SET_HELP,
              "set vswitch information\n",
              "set vswitch IGMP information\n",
              "join IGMP group for the vswitch\n",
              "specify vswitch ID\n",
              "specify IPv4 multicast group address\n"
              )
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;
  struct router_if *rif;
  struct igmp_join *entry = NULL;
  struct in_addr group_addr;
  struct ip_mreqn mreq;
  int vswitch_id, fd, i;

  vswitch_id = atoi (argv[4]);
  if (inet_pton (AF_INET, argv[5], &group_addr) != 1 ||
   ! IN_MULTICAST (ntohl (group_addr.s_addr)))
    {
      fprintf (shell->terminal, "Invalid IPv4 multicast address: %s%s",
               argv[5], shell->NL);
      return CMD_FAILURE;
    }
  if (igmp_join_lookup (vswitch_id, &group_addr))
    {
      fprintf (shell->terminal,
               "IGMP join already exists for vswitch %d group %s%s",
               vswitch_id, argv[5], shell->NL);
      return CMD_FAILURE;
    }
    rif = igmp_router_if_lookup (rib, vswitch_id);
    if (! rif || rif->sockfd < 0 || rif->ifindex <= 0)
    {
      fprintf (shell->terminal,
               "Cannot find router interface for vswitch %d%s",
               vswitch_id, shell->NL);
      return CMD_FAILURE;
    }
    for (i = 0; i < MAX_IGMP_JOIN_GROUPS; i++)
      if (! igmp_join_table[i].used)
        {
          entry = &igmp_join_table[i];
          break;
        }
    if (! entry)
        {
          fprintf (shell->terminal,
                   "IGMP join table is full%s", shell->NL);
          return CMD_FAILURE;
        }
    fd = socket (AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        {
            fprintf (shell->terminal,
                     "Failed to create socket: %s%s", strerror (errno),
                     shell->NL);
            return CMD_FAILURE;
        }
    memset (&mreq, 0, sizeof (mreq));
    mreq.imr_multiaddr = group_addr;
    mreq.imr_ifindex = rif->ifindex;
    if (setsockopt (fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                    &mreq, sizeof (mreq)) < 0)
        {
            fprintf (shell->terminal,
                     "Failed to join IGMP group %s on vswitch %d: %s%s",
                     argv[5], vswitch_id, strerror (errno), shell->NL);
            close (fd);
            return CMD_FAILURE;
        }
    entry->used = true;
    entry->vswitch_id = vswitch_id;
    entry->group_addr = group_addr;
    entry->ifindex = rif->ifindex;
    entry->fd = fd;
    return CMD_SUCCESS;
}

CLI_COMMAND2 (no_set_vswitch_igmp_join,
              "no set vswitch igmp join <1-4094> "IPV4_ADDR_SPEC,
              NO_HELP,
              SET_HELP,
              "set vswitch information\n",
              "set vswitch IGMP information\n",
              "leave IGMP group for the vswitch\n",
              "specify vswitch ID\n",
              "specify IPv4 multicast group address\n"
              )
{
  struct shell *shell = (struct shell *) context;
  struct igmp_join *entry;
  struct in_addr group_addr;
  int vswitch_id;

  vswitch_id = atoi (argv[5]);
  if (inet_pton (AF_INET, argv[6], &group_addr) != 1 ||
      ! IN_MULTICAST (ntohl (group_addr.s_addr)))
    {
      fprintf (shell->terminal, "Invalid IPv4 multicast address: %s%s",
               argv[6], shell->NL);
      return CMD_FAILURE;
    }
  entry = igmp_join_lookup (vswitch_id, &group_addr);
  if (! entry)
    {
      fprintf (shell->terminal,
               "IGMP join does not exist for vswitch %d group %s%s",
               vswitch_id, argv[6], shell->NL);
      return CMD_FAILURE;
    }
    close (entry->fd);
    memset (entry, 0, sizeof (*entry));
    return CMD_SUCCESS;
}

CLI_COMMAND2 (show_igmp_join,
              "show igmp join",
              SHOW_HELP,
              "show IGMP information\n",
              "show IGMP joins\n"
              )
{
  struct shell *shell = (struct shell *) context;
  int i;

  fprintf (shell->terminal, "IGMP report interval: %d sec%s",
           igmp_report_interval, shell->NL);
  fprintf (shell->terminal, "IGMP Joins:%s", shell->NL);
  fprintf (shell->terminal, "%-10s %-15s %-10s%s",
           "VSwitchID", "GroupAddr", "IfIndex", shell->NL);
  for (i = 0; i < MAX_IGMP_JOIN_GROUPS; i++)
    {
      struct igmp_join *entry = &igmp_join_table[i];
      if (entry->used)
        {
          char group_addr_str[INET_ADDRSTRLEN];
          inet_ntop (AF_INET, &entry->group_addr, group_addr_str,
                        sizeof (group_addr_str));
          fprintf (shell->terminal, "%-10d %-15s %-10d%s",
                   entry->vswitch_id, group_addr_str, entry->ifindex,
                   shell->NL);
        }
    }
  return CMD_SUCCESS;
}

CLI_COMMAND2 (set_igmp_report_interval,
              "set igmp report interval <1-3600>",
              SET_HELP,
              "set IGMP information\n",
              "set IGMP report information\n",
              "set the interval to have the kernel send IGMP reports\n",
              "specify the interval in seconds\n"
              )
{
  igmp_report_interval = atoi (argv[4]);
  return CMD_SUCCESS;
}

CLI_COMMAND2 (no_set_igmp_report_interval,
              "no set igmp report interval",
              NO_HELP,
              SET_HELP,
              "set IGMP information\n",
              "set IGMP report information\n",
              "reset the interval to the default\n"
              )
{
  igmp_report_interval = IGMP_REPORT_INTERVAL_DEFAULT;
  return CMD_SUCCESS;
}

static int
igmp_version_set (const char *tap_name, int version)
{
  char path[128];
  FILE *fp;

  snprintf (path, sizeof (path),
            "/proc/sys/net/ipv4/conf/%s/force_igmp_version", tap_name);
  fp = fopen (path, "w");
  if (! fp)
    return -1;
  fprintf (fp, "%d\n", version);
  if (fclose (fp) != 0)
    return -1;
  return 0;
}

static int
igmp_version_get (const char *tap_name)
{
  char path[128];
  FILE *fp;
  int version = -1;

  snprintf (path, sizeof (path),
            "/proc/sys/net/ipv4/conf/%s/force_igmp_version", tap_name);
  fp = fopen (path, "r");
  if (! fp)
    return -1;
  if (fscanf (fp, "%d", &version) != 1)
    version = -1;
  fclose (fp);
  return version;
}

CLI_COMMAND2 (set_vswitch_igmp_version,
              "set vswitch igmp version <1-4094> <2-3>",
              SET_HELP,
              "set vswitch information\n",
              "set vswitch IGMP information\n",
              "set IGMP version of the kernel for the vswitch\n",
              "specify vswitch ID\n",
              "specify IGMP version\n"
              )
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;
  struct router_if *rif;
  int vswitch_id, version;

  vswitch_id = atoi (argv[4]);
  version = atoi (argv[5]);
  rif = igmp_router_if_lookup (rib, vswitch_id);
  if (! rif || rif->sockfd < 0)
    {
      fprintf (shell->terminal,
               "Cannot find router interface for vswitch %d%s",
               vswitch_id, shell->NL);
      return CMD_FAILURE;
    }
  if (igmp_version_set (rif->tap_name, version) < 0)
    {
      fprintf (shell->terminal,
               "Failed to set IGMP version %d on %s: %s%s",
               version, rif->tap_name, strerror (errno), shell->NL);
      return CMD_FAILURE;
    }
  return CMD_SUCCESS;
}

CLI_COMMAND2 (no_set_vswitch_igmp_version,
              "no set vswitch igmp version <1-4094>",
              NO_HELP,
              SET_HELP,
              "set vswitch information\n",
              "set vswitch IGMP information\n",
              "reset IGMP version of the kernel for the vswitch\n",
              "specify vswitch ID\n"
              )
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;
  struct router_if *rif;
  int vswitch_id;

  vswitch_id = atoi (argv[5]);
  rif = igmp_router_if_lookup (rib, vswitch_id);
  if (! rif || rif->sockfd < 0)
    {
      fprintf (shell->terminal,
               "Cannot find router interface for vswitch %d%s",
               vswitch_id, shell->NL);
      return CMD_FAILURE;
    }
  if (igmp_version_set (rif->tap_name, 0) < 0)
    {
      fprintf (shell->terminal,
               "Failed to reset IGMP version on %s: %s%s",
               rif->tap_name, strerror (errno), shell->NL);
      return CMD_FAILURE;
    }
  return CMD_SUCCESS;
}

CLI_COMMAND2 (show_igmp_version,
              "show igmp version",
              SHOW_HELP,
              "show IGMP information\n",
              "show IGMP version of the kernel\n"
              )
{
  struct shell *shell = (struct shell *) context;
  struct rib *rib = rib_tlocal;
  int i, version;

  if (! rib || ! rib->rib_info)
    return CMD_SUCCESS;

  fprintf (shell->terminal, "%-10s %-16s %-10s%s",
           "VSwitchID", "TapName", "Version", shell->NL);
  for (i = 0; i < rib->rib_info->vswitch_size; i++)
    {
      struct vswitch_conf *vswitch = &rib->rib_info->vswitch[i];
      struct router_if *rif = &vswitch->router_if;
      char version_str[16];

      if (vswitch->is_deleted || rif->sockfd < 0)
        continue;
      version = igmp_version_get (rif->tap_name);
      if (version < 0)
        snprintf (version_str, sizeof (version_str), "unknown");
      else if (version == 0)
        snprintf (version_str, sizeof (version_str), "auto");
      else
        snprintf (version_str, sizeof (version_str), "%d", version);
      fprintf (shell->terminal, "%-10d %-16s %-10s%s",
               vswitch->vswitch_id, rif->tap_name, version_str, shell->NL);
    }
  return CMD_SUCCESS;
}

void igmp_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, set_vswitch_igmp_join);
  INSTALL_COMMAND2 (cmdset, no_set_vswitch_igmp_join);
  INSTALL_COMMAND2 (cmdset, show_igmp_join);
  INSTALL_COMMAND2 (cmdset, set_igmp_report_interval);
  INSTALL_COMMAND2 (cmdset, no_set_igmp_report_interval);
  INSTALL_COMMAND2 (cmdset, set_vswitch_igmp_version);
  INSTALL_COMMAND2 (cmdset, no_set_vswitch_igmp_version);
  INSTALL_COMMAND2 (cmdset, show_igmp_version);
}
