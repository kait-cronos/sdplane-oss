#include "include.h"

#include <lthread.h>

#include <rte_ethdev.h>
#include <rte_bus_pci.h>

#include <sdplane/debug.h>
#include <sdplane/termio.h>
#include <sdplane/vector.h>
#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>
#include <sdplane/debug_cmd.h>
#ifdef HAVE_SDPLANE_LIBSDPLANE_VERSION_H
#include <sdplane/libsdplane_version.h>
#endif

#include "l2fwd_export.h"
#include "internal_message.h"
#include "sdplane.h"
#include "sdplane_version.h"
#include "stat_collector.h"
#include "debug_sdplane.h"
#include "thread_info.h"
#include "queue_config.h"
#include "rib.h"
#include "tap_cmd.h"
#include "dpdk_port_cmd.h"
#include "rte_flow_support.h"
#include "netlink_hook.h"

#include "argv_list.h"

CLI_COMMAND2 (show_version, "show version", SHOW_HELP, "version\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
#ifdef HAVE_SDPLANE_LIBSDPLANE_VERSION_H
  fprintf (t, "libsdplane version: %s%s", libsdplane_version, shell->NL);
#endif
  fprintf (t, "sdplane version: %s%s", sdplane_version, shell->NL);
  return 0;
}

CLI_COMMAND2 (set_locale, "set locale (C|C.utf8|en_US.utf8|POSIX)", SET_HELP,
              "locale information\n", "C\n", "C.utf8\n", "en_US.utf8\n",
              "POSIX")
{
  struct shell *shell = (struct shell *) context;
  char *ret;
  ret = setlocale (LC_ALL, argv[2]);
  if (! ret)
    fprintf (shell->terminal, "setlocale(): failed.\n");
  else
    fprintf (shell->terminal, "setlocale(): %s.\n", ret);
  return 0;
}

CLI_COMMAND2 (show_loop_count,
              "show loop-count (console|vty-shell|l2fwd) (pps|total)",
              SHOW_HELP, "loop count\n", "console shell\n", "vty shell\n",
              "l2fwd loop\n", "pps\n", "total count\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;

  char name[32];

  if (! strcmp (argv[2], "console"))
    {
      snprintf (name, sizeof (name), "console:");
      if (! strcmp (argv[3], "pps"))
        {
          fprintf (t, "%16s %'8lu\n", name, loop_console_pps);
        }
      else if (! strcmp (argv[3], "total"))
        {
          fprintf (t, "%16s %'8lu\n", name, loop_console_current);
        }
    }
  else if (! strcmp (argv[2], "vty-shell"))
    {
      snprintf (name, sizeof (name), "vty-shell:");
      if (! strcmp (argv[3], "pps"))
        {
          fprintf (t, "%16s %'8lu\n", name, loop_vty_shell_pps);
        }
      else if (! strcmp (argv[3], "total"))
        {
          fprintf (t, "%16s %'8lu\n", name, loop_vty_shell_current);
        }
    }
  else if (! strcmp (argv[2], "l2fwd"))
    {
      int i;
      for (i = 0; i < RTE_MAX_LCORE; i++)
        {
          snprintf (name, sizeof (name), "l2fwd: core[%d]:", i);
          if (loop_l2fwd_current[i])
            {
              if (! strcmp (argv[3], "pps"))
                {
                  fprintf (t, "%24s %'8lu\n", name, loop_l2fwd_pps[i]);
                }
              else if (! strcmp (argv[3], "total"))
                {
                  fprintf (t, "%24s %'8lu\n", name, loop_l2fwd_current[i]);
                }
            }
        }
    }
  return 0;
}

extern uint64_t rib_rcu_replace;
CLI_COMMAND2 (show_rcu, "show rcu", SHOW_HELP, "show rcu-information.\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  fprintf (t, "rib_rcu_replace: %'lu%s", rib_rcu_replace, shell->NL);
  return 0;
}

CLI_COMMAND2 (sleep_cmd, "sleep <0-300>", "sleep command\n",
              "specify seconds to sleep.\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  int sec;
  sec = strtol (argv[1], NULL, 0);
  if (sec > 0)
    {
      while (sec > 0)
        {
          fprintf (t, " %d", sec);
          fflush (t);
          lthread_sleep (1000);
          sec--;
        }
    }
  else
    lthread_sleep (0);
  fprintf (t, " 0.%s", shell->NL);
  fflush (t);
  return 0;
}

CLI_COMMAND2 (show_mempool, "show mempool", SHOW_HELP, "show mempool.\n")
{
  struct shell *shell = (struct shell *) context;
  FILE *t = shell->terminal;
  struct rte_mempool *mp;
  unsigned int count;
  int is_full;

  mp = l2fwd_pktmbuf_pool;
  count = rte_mempool_avail_count (mp);
  is_full = rte_mempool_full (mp);

  fprintf (t, "mempool: size: %u count: %u is_full: %d%s", mp->size, count,
           is_full, shell->NL);

  return 0;
}

#if 0
$ rg router_mode router.c net_subr.h
router.c
37:volatile int router_mode = 0; /* the normal/default mode. */
58:  if (router_mode == 1) /* process_rx_packet: the beginning. */
248:  if (router_mode == 2) /* process_rx_packet: just before the forwarding. */

net_subr.h
140:extern volatile int router_mode;
361:  if (router_mode == 11) /* flooding: the beginning. */
387:  if (router_mode == 12) /* flooding: after sending all ports, without sending router-if. */
427:  if (router_mode == 8) /* switching: just before the fdb-lookup */
438:  if (router_mode == 9) /* switching: right after the fdb-lookup */
465:  if (router_mode == 10) /* switching: just before the flooding */
509:  if (router_mode == 3) /* forwarding: just before the routing-lookup. */
530:  if (router_mode == 4) /* forwarding: right after the routing-lookup. */
591:  if (router_mode == 5) /* forwarding: just before the neighbor-lookup. */
602:  if (router_mode == 6) /* forwarding: right after the neighbor-lookup. */
711:  if (router_mode == 7) /* forwarding: just before the switching. */
#endif

extern volatile int router_mode;

CLI_COMMAND2 (set_router_mode,
              "set router mode (0|1|2|3|4|5|6|7|8|9|10|11|12)",
              SET_HELP,
              "set router mode.\n",
              "set router mode.\n",
              "the normal/default mode.\n",
              "process_rx_packet: the beginning.\n",
              "process_rx_packet: just before the forwarding.\n",
              "forwarding: just before the routing-lookup.\n",
              "forwarding: right after the routing-lookup.\n",
              "forwarding: just before the neighbor-lookup.\n",
              "forwarding: right after the neighbor-lookup.\n",
              "forwarding: just before the switching.\n",
              "switching: just before the fdb-lookup\n",
              "switching: right after the fdb-lookup\n",
              "switching: just before the flooding\n",
              "flooding: the beginning.\n",
              "flooding: after sending all ports, without sending router-if.\n"
              )
{
  struct shell *shell = (struct shell *) context;
  router_mode = strtol (argv[3], NULL, 0);
  return 0;
}

CLI_COMMAND2 (show_router_mode,
              "show router mode",
              SHOW_HELP,
              "show router mode.\n",
              "show router mode.\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "router-mode: %d%s", router_mode, shell->NL);
  return 0;
}

int
snprintf_time (char *buf, int size, uint32_t tv_sec)
{
  int ret = 0;
  struct tm *tm;
  time_t clock;
  if (tv_sec == 0)
    {
      ret = snprintf (buf, size, "--/--/-- --:--:--");
      return ret;
    }
  clock = tv_sec;
  tm = localtime (&clock);
  ret = strftime (buf, size, "%Y/%m/%d %H:%M:%S", tm);
  return ret;
}

int
snprintf_uptime (char *buf, int size, uint32_t tv_sec)
{
  int ret = 0;
  struct timeval current_time;
  gettimeofday (&current_time, NULL);
  uint32_t seconds;
  int day, hour, min, sec;
  seconds = 0;
  if (tv_sec)
    seconds = current_time.tv_sec - tv_sec;
  sec = seconds % 60;
  min = (seconds / 60 ) % 60;
  hour = (seconds / 60 / 60) % 24;
  day = (seconds / 60 / 60 / 24);
  if (tv_sec == 0)
    ret = snprintf (buf, size, "%2s:%2s:%2s",
                    "--", "--", "--");
  else if (day)
    ret = snprintf (buf, size, "%'d days %02d:%02d:%02d",
                    day, hour, min, sec);
  else
    ret = snprintf (buf, size, "%02d:%02d:%02d",
                    hour, min, sec);
  return ret;
}

CLI_COMMAND2 (show_process_status,
              "show process status",
              SHOW_HELP,
              "show process information.\n",
              "show process status.\n")
{
  struct shell *shell = (struct shell *) context;
  struct sdplane_process_status *p = &process_status;
  fprintf (shell->terminal, "process status:%s", shell->NL);
  fprintf (shell->terminal, "  pid: %d%s", p->pid, shell->NL);
  fprintf (shell->terminal, "  config-file: %s%s",
           p->config_file_path, shell->NL);
  fprintf (shell->terminal, "  pid-file: %s%s",
           p->pid_file_path, shell->NL);

  char timebuf[32];
  snprintf_time (timebuf, sizeof (timebuf), p->start_time.tv_sec);
  fprintf (shell->terminal, "  start-time: %s%s", timebuf, shell->NL);

  char uptimebuf[32];
  snprintf_uptime (uptimebuf, sizeof (uptimebuf), p->start_time.tv_sec);
  fprintf (shell->terminal, "  uptime: %s%s", uptimebuf, shell->NL);

  fprintf (shell->terminal, "  version: libsdplane: %s%s",
           p->version_libsdplane, shell->NL);
  fprintf (shell->terminal, "  version: sdplane: %s%s",
           p->version_sdplane, shell->NL);

  struct rusage rusage_self, rusage_children;
  getrusage (RUSAGE_SELF, &rusage_self);
  getrusage (RUSAGE_CHILDREN, &rusage_children);
  fprintf (shell->terminal, "  rusage_self.ru_maxrss: "
           "%'ld KB (now %'ld KB)%s",
           p->rusage_self.ru_maxrss,
           rusage_self.ru_maxrss, shell->NL);
  fprintf (shell->terminal, "  rusage_children.ru_maxrss: "
           "%'ld KB (now %'ld KB)%s",
           p->rusage_children.ru_maxrss,
           rusage_children.ru_maxrss, shell->NL);
  return 0;
}

void
shell_read_response_port_get (struct shell *shell,
                              struct internal_msg_header *imsghdr)
{
  struct internal_msg_port_info *port_info;
  struct rte_eth_dev_info *dev;
  char link_capa[32];
  char devname[32];
  char drivername[32];

  port_info = internal_msg_body (imsghdr);
  dev = &port_info->dev_info;

  snprintf (devname, sizeof (devname), "%s", rte_dev_name (dev->device));
  snprintf_flags (link_capa, sizeof (link_capa), dev->speed_capa,
                  link_speeds, "|", LINK_SPEEDS_SIZE);
  snprintf (drivername, sizeof (drivername), "%s", dev->driver_name);

  FILE *t = shell->terminal;
  fprintf (t, "resp: %p type: %d port_info: port_id: %d%s",
           imsghdr, imsghdr->type, port_info->port_id, shell->NL);
  fprintf (t, "dev_info.device: %s%s", devname, shell->NL);
  fprintf (t, "dev_info.speed_capa: %s%s", link_capa, shell->NL);
  fprintf (t, "dev_info.driver_name: %s%s", drivername, shell->NL);
  fprintf (t, "  min_mtu: %'u max_mtu: %'u%s",
           dev->min_mtu, dev->max_mtu, shell->NL);
  fflush (t);
}

void
shell_read_response_cmd_success (struct shell *shell,
                                 struct internal_msg_header *imsghdr)
{
  FILE *t = shell->terminal;
  struct internal_msg_cmd_success *cmd_success;
  cmd_success = internal_msg_body (imsghdr);
  if (strlen (cmd_success->message))
    fprintf (t, "cmd success: %s", cmd_success->message);
  else
    fprintf (t, "cmd success.");
  fflush (t);
}

void
shell_read_response_cmd_error (struct shell *shell,
                               struct internal_msg_header *imsghdr)
{
  FILE *t = shell->terminal;
  struct internal_msg_cmd_error *cmd_error;
  cmd_error = internal_msg_body (imsghdr);
  if (strlen (cmd_error->message))
    fprintf (t, "cmd error: %s", cmd_error->message);
  else
    fprintf (t, "cmd error.");
  fflush (t);
}

void
shell_read_response (struct shell *shell, struct rte_ring *ring_resp)
{
  void *msgp;
  struct internal_msg_header *imsghdr;

  msgp = internal_msg_recv (ring_resp);
  imsghdr = msgp;

  switch (imsghdr->type)
    {
    case INTERNAL_MSG_TYPE_PORT_GET_RESPONSE:
      shell_read_response_port_get (shell, imsghdr);
      break;
    case INTERNAL_MSG_TYPE_CMD_SUCCESS:
      shell_read_response_cmd_success (shell, imsghdr);
      break;
    case INTERNAL_MSG_TYPE_CMD_ERROR:
      shell_read_response_cmd_error (shell, imsghdr);
      break;
    }
}

void variable_cmd_init (struct command_set *cmdset);

void dpdk_lcore_cmd_init (struct command_set *cmdset);
void dpdk_port_cmd_init (struct command_set *cmdset);
void lthread_cmd_init (struct command_set *cmdset);
void queue_config_cmd_init (struct command_set *cmdset);
void dpdk_devbind_cmd_init (struct command_set *cmdset);
void pktgen_cmd_init (struct command_set *cmdset);
void dhcp_cmd_init (struct command_set *cmdset);
void srv6_cmd_init (struct command_set *cmdset);
void igmp_cmd_init (struct command_set *cmdset);
void netlink_cmd_init (struct command_set *cmdset);

void
sdplane_cmd_init (struct command_set *cmdset)
{
  setlocale (LC_ALL, "en_US.utf8");
  variable_cmd_init (cmdset);
  dpdk_lcore_cmd_init (cmdset);
  dpdk_port_cmd_init (cmdset);
  argv_list_cmd_init (cmdset);
  nlhook_cmd_init (cmdset);

  INSTALL_COMMAND2 (cmdset, show_loop_count);
  INSTALL_COMMAND2 (cmdset, show_version);
  INSTALL_COMMAND2 (cmdset, show_rcu);
  INSTALL_COMMAND2 (cmdset, sleep_cmd);
  INSTALL_COMMAND2 (cmdset, set_locale);
  INSTALL_COMMAND2 (cmdset, show_mempool);
  thread_info_cmd_init (cmdset);
  queue_config_cmd_init (cmdset);
  lthread_cmd_init (cmdset);
  tap_cmd_init (cmdset);
  rib_cmd_init (cmdset);
  dpdk_devbind_cmd_init (cmdset);
#ifdef ENABLE_PKTGEN
  pktgen_cmd_init (cmdset);
#endif

  dhcp_cmd_init (cmdset);

  rte_flow_cmd_init (cmdset);

  srv6_cmd_init (cmdset);

  igmp_cmd_init (cmdset);
  netlink_cmd_init (cmdset);

  INSTALL_COMMAND2 (cmdset, set_router_mode);
  INSTALL_COMMAND2 (cmdset, show_router_mode);
  INSTALL_COMMAND2 (cmdset, show_process_status);
}

void
sdplane_init ()
{
  argv_list_init ();
  nlhook_init ();
  debug_sdplane_cmd_init ();
  thread_info_init ();
}
