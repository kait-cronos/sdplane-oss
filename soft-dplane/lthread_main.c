#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <string.h>
#include <sys/queue.h>
#include <stdarg.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>

#include <rte_common.h>
#include <rte_vect.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_random.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_udp.h>
#include <rte_string_fns.h>
#include <rte_cpuflags.h>

#include <cmdline_parse.h>
#include <cmdline_parse_etheraddr.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <lthread.h>

#include "debug.h"
#include "termio.h"
#include "vector.h"
#include "shell.h"
#include "command.h"
#include "command_shell.h"
#include "debug_cmd.h"
//#include "shell_fselect.h"

#include "l3fwd.h"
#include "l3fwd_event.h"
#include "l3fwd_route.h"

#include "l2fwd.h"

#include "soft_dplane.h"

int lthread_main (__rte_unused void *dummy);
int tap_handler (__rte_unused void *dummy);

extern uint32_t l2fwd_dst_ports[RTE_MAX_ETHPORTS];
int l2fwd_launch_one_lcore (__rte_unused void *dummy);

struct lcore_worker lcore_workers[RTE_MAX_LCORE];

void
start_lcore (struct shell *shell, int lcore_id)
{
  fprintf (shell->terminal, "starting worker on lcore: %d\n", lcore_id);
  if (lcore_workers[lcore_id].func == NULL)
    {
      fprintf (shell->terminal,
               "can't start a null worker on lcore: %d\n", lcore_id);
      return;
    }
  force_stop[lcore_id] = false;
  rte_eal_remote_launch (lcore_workers[lcore_id].func,
                         lcore_workers[lcore_id].arg, lcore_id);
  fprintf (shell->terminal, "started worker on lcore: %d\n", lcore_id);
}

void
stop_lcore (struct shell *shell, int lcore_id)
{
  fprintf (shell->terminal, "stopping worker on lcore: %d\n", lcore_id);
  force_stop[lcore_id] = true;

  if (lcore_id == rte_lcore_id ())
    {
      fprintf (shell->terminal, "can't stop lthread lcore: %d\n", lcore_id);
    }
  else
    {
      rte_eal_wait_lcore (lcore_id);
      fprintf (shell->terminal, "stopped worker on lcore: %d\n", lcore_id);
    }
}

DEFINE_COMMAND (set_worker,
                "(set|reset|start|restart) worker lcore <0-16> "
                "(|none|l2fwd|l3fwd|l3fwd-lpm|tap-handler)",
                SET_HELP
                RESET_HELP
                START_HELP
                RESTART_HELP
                WORKER_HELP
                LCORE_HELP
                LCORE_NUMBER_HELP
                "set lcore not to launch anything\n"
                "set lcore to launch l2fwd\n"
                "set lcore to launch l3fwd (default: lpm)\n"
                "set lcore to launch l3fwd-lpm\n"
                "set lcore to launch tap-handler\n"
               )
{
  struct shell *shell = (struct shell *) context;
  int lcore_id;
  lcore_id = strtol (argv[3], NULL, 0);
  lcore_function_t *func;
  void *arg = NULL;

  if (argc == 4)
    func = lcore_workers[lcore_id].func;
  else if (! strcmp (argv[4], "none"))
    func = NULL;
  else if (! strcmp (argv[4], "l2fwd"))
    func = l2fwd_launch_one_lcore;
  else if (! strcmp (argv[4], "tap-handler"))
    func = tap_handler;
  else /* if (! strcmp (argv[4], "l3fwd")) */
    func = lpm_main_loop;

  if (lcore_workers[lcore_id].func == lthread_main)
    {
      fprintf (shell->terminal, "cannot override lthread: lcore[%d].\n",
               lcore_id);
      return;
    }

  char *func_name;
  if (func == lpm_main_loop)
    func_name = "l3fwd-lpm";
  else if (func == l2fwd_launch_one_lcore)
    func_name = "l2fwd";
  else if (func == lthread_main)
    func_name = "lthread_main";
  else if (func == tap_handler)
    func_name = "tap-handler";
  else
    func_name = "none";

  lcore_workers[lcore_id].func = func;
  lcore_workers[lcore_id].arg = arg;
  lcore_workers[lcore_id].func_name = func_name;

  fprintf (shell->terminal, "worker set to lcore[%d]: func: %s\n",
           lcore_id, func_name);

  if (! strcmp (argv[0], "reset") ||
      ! strcmp (argv[0], "start") ||
      ! strcmp (argv[0], "restart"))
    {
      stop_lcore (shell, lcore_id);
      start_lcore (shell, lcore_id);
      fprintf (shell->terminal, "worker[%d]: restarted.\n", lcore_id);
    }
  else if (! strcmp (argv[0], "set") && argc == 4)
    fprintf (shell->terminal, "nothing changed.\n");
  else
    fprintf (shell->terminal,
             "workers need to be restarted for changes to take effect.\n");
}

DEFINE_COMMAND (start_stop_worker,
                "(start|stop|reset|restart) worker lcore (<0-16>|all)",
                START_HELP
                STOP_HELP
                RESET_HELP
                RESTART_HELP
                WORKER_HELP
                LCORE_HELP
                LCORE_NUMBER_HELP
                LCORE_ALL_HELP
               )
{
  struct shell *shell = (struct shell *) context;
  uint32_t nb_lcores;
  unsigned int lcore_id;
  unsigned int lcore_spec = -1;

  if (! strcmp (argv[3], "all"))
    {
      if (! strcmp (argv[0], "stop"))
        force_quit = true;
      else
        force_quit = false;
    }
  else
    lcore_spec = strtol (argv[3], NULL, 0);

  nb_lcores = rte_lcore_count ();
  for (lcore_id = 0; lcore_id < nb_lcores; lcore_id++)
    {
      if (lcore_spec != -1 && lcore_spec != lcore_id)
        continue;
      if (! strcmp (argv[0], "start"))
        start_lcore (shell, lcore_id);
      else if (! strcmp (argv[0], "stop"))
        stop_lcore (shell, lcore_id);
      else if (! strcmp (argv[0], "reset") ||
               ! strcmp (argv[0], "restart"))
        {
          stop_lcore (shell, lcore_id);
          start_lcore (shell, lcore_id);
        }
    }
}

DEFINE_COMMAND (show_worker,
                "show worker",
                SHOW_HELP
                WORKER_HELP
               )
{
  struct shell *shell = (struct shell *) context;
  unsigned int lcore_id;
  uint32_t nb_lcores;
  unsigned int main_lcore_id;
  char *state;
  char flags[16];
  char lcore_name[16];
  nb_lcores = rte_lcore_count ();
  main_lcore_id = rte_get_main_lcore ();
  fprintf (shell->terminal, "%-9s: %-12s %-8s %s\n",
           "lcore", "flags", "state", "func_name");
  for (lcore_id = 0; lcore_id < nb_lcores; lcore_id++)
    {
      snprintf (flags, sizeof (flags), "%s%s",
                (rte_lcore_is_enabled (lcore_id) ? "enabled" : "disabled"),
                (lcore_id == main_lcore_id ? ",main" : ""));
      state = (rte_eal_get_lcore_state (lcore_id) == RUNNING ?
               "running" : "wait");
      snprintf (lcore_name, sizeof (lcore_name),
                "lcore[%d]", lcore_id);
      fprintf (shell->terminal, "%-9s: %-12s %-8s %s\n",
               lcore_name, flags, state, lcore_workers[lcore_id].func_name);
    }
}

DEFINE_COMMAND (exit_cmd,
                "(exit|quit|logout)",
                "exit\n"
                "quite\n"
                "logout\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "exit !\n");
  FLAG_SET (shell->flag, SHELL_FLAG_EXIT);
  /* don't shell_close(): this closes stdout. */
  //shell_close (shell);

  int nb_lcores = rte_lcore_count ();
  for (int lcore_id = 0; lcore_id < nb_lcores; lcore_id++)
    stop_lcore (shell, lcore_id);
}

DEFINE_COMMAND (l2fwd_init,
               "l2fwd init",
               "l2fwd\n"
               "init\n")
{
  struct shell *shell = (struct shell *) context;
  l2fwd_init (0, NULL);
}


void
get_winsize (struct shell *shell)
{
  ioctl (shell->writefd, TIOCGWINSZ, &shell->winsize);
  fprintf (shell->terminal, "row: %d col: %d\n",
           shell->winsize.ws_row, shell->winsize.ws_col);
}


void l2fwd_cmd_init (struct command_set *cmdset);

void
lthread_shell (void *arg)
{
  struct shell *shell = NULL;

  printf ("%s[%d]: %s: enter.\n", __FILE__, __LINE__, __func__);

  /* library initialization. */
  debug_cmd_init ();
  command_shell_init ();

  shell = command_shell_create ();
  //shell_set_prompt_cwd (shell);
  shell_set_terminal (shell, 0, 1);
  get_winsize (shell);

  INSTALL_COMMAND2 (shell->cmdset, exit_cmd);

  INSTALL_COMMAND2 (shell->cmdset, show_worker);
  INSTALL_COMMAND2 (shell->cmdset, set_worker);
  INSTALL_COMMAND2 (shell->cmdset, start_stop_worker);

  //INSTALL_COMMAND2 (shell->cmdset, show_version);

  //INSTALL_COMMAND2 (shell->cmdset, chdir);
  //INSTALL_COMMAND2 (shell->cmdset, list);

  INSTALL_COMMAND2 (shell->cmdset, debug);
  INSTALL_COMMAND2 (shell->cmdset, show_debug);

  INSTALL_COMMAND2 (shell->cmdset, l2fwd_init);

  //INSTALL_COMMAND (shell->cmdset, pwd);
  //INSTALL_COMMAND (shell->cmdset, open);
  //INSTALL_COMMAND2 (shell->cmdset, terminal);
  //INSTALL_COMMAND2 (shell->cmdset, launch_shell);
  //INSTALL_COMMAND2 (shell->cmdset, edit_vi);

  //shell_install (shell, '>', fselect_keyfunc_start);
  //shell_install (shell, CONTROL ('D'), opensh_shell_keyfunc_ctrl_d);

  l2fwd_cmd_init (shell->cmdset);
  soft_dplane_cmd_init (shell->cmdset);

  termio_init ();
  //shell_fselect_init ();

  shell_clear (shell);
  shell_prompt (shell);

  while (shell_running (shell))
    {
      lthread_sleep (0); // yield.
      shell_read (shell);
    }

  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);

  termio_finish ();
}

#include <linux/if.h>
#include <linux/if_tun.h>

extern struct rte_ring *tap_ring_by_lcore[RTE_MAX_LCORE];

int
tap_handler (__rte_unused void *dummy)
{
  int fd;
  struct ifreq ifr;
  int ret;

  printf ("%s[%d]: %s: enter.\n", __FILE__, __LINE__, __func__);

  fd = open ("/dev/net/tun", O_RDWR);
  if (fd < 0)
    {
      printf ("%s: can't open /dev/net/tun. quit", __func__);
      return -1;
    }

  memset (&ifr, 0, sizeof (ifr));
  snprintf (ifr.ifr_name, IFNAMSIZ, "peek0");
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

  ret = ioctl (fd, TUNSETIFF, (void *) &ifr);
  if (ret < 0)
    {
      printf ("%s: ioctl (TUNSETIFF) failed: %s\n",
              __func__, strerror (errno));
      close (fd);
      return -1;
    }

#if 1
  int sockfd;
  sockfd = socket (AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
    {
      printf ("%s: socket() failed: %s\n",
              __func__, strerror (errno));
    }
  else
    {
      memset (&ifr, 0, sizeof (ifr));
      snprintf (ifr.ifr_name, IFNAMSIZ, "peek0");

      ret = ioctl (sockfd, SIOCGIFFLAGS, &ifr);
      if (ret < 0)
        {
          printf ("%s: ioctl (SOICGIFFLAG) failed: %s\n",
                  __func__, strerror (errno));
          close (sockfd);
          sockfd = -1;
        }
    }

  if (sockfd > 0)
    {
      ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
      ret = ioctl (sockfd, SIOCSIFFLAGS, &ifr);
      if (ret < 0)
        {
          printf ("%s: ioctl (SOICSIFFLAG) failed: %s\n",
                  __func__, strerror (errno));
          close (sockfd);
          sockfd = -1;
        }
    }
#endif

  printf ("%s on lcore[%d]: started.\n",
          __func__, rte_lcore_id ());

  unsigned tap_manager_id = rte_lcore_id ();
  while (! force_quit && ! force_stop[tap_manager_id])
    {
      unsigned lcore_id;
      //lthread_sleep (0); // yield.
      for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++)
        {
          struct rte_ring *tap_ring;
          struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
          unsigned int dequeued = 0, avail = 0;
          int i;

          tap_ring = tap_ring_by_lcore[lcore_id];
          do {
            if (tap_ring)
              dequeued = rte_ring_dequeue_burst (tap_ring, (void **) pkts_burst,
                                                 MAX_PKT_BURST, &avail);
            for (i = 0; i < dequeued; i++)
              {
                struct rte_mbuf *m;
                uint32_t pkt_len;
                uint16_t data_len;
                char *pkt;

                m = pkts_burst[i];
                pkt_len = rte_pktmbuf_pkt_len (m);
                data_len = rte_pktmbuf_data_len (m);
                pkt = rte_pktmbuf_mtod (m, char *);
                if (data_len < pkt_len)
                  printf ("%s: warning: multi-seg mbuf: %u < %u\n",
                          __func__, data_len, pkt_len);
                ret = write (fd, pkt, data_len);
                if (ret < 0)
                  printf ("%s: warning: write () failed: %s\n",
                          __func__, strerror (errno));
                else
                  printf ("%s: capture pkt: len: %d(%d) from lcore[%d]\n",
                          __func__, pkt_len, data_len, lcore_id);
                rte_pktmbuf_free (m);
              }
          } while (avail);
        }
    }

  close (fd);
  printf ("%s on lcore[%d]: finished.\n",
          __func__, rte_lcore_id ());
  return 0;
}

int
lthread_main (__rte_unused void *dummy)
{
  lthread_t *lt = NULL;

  /* timer set */
  timer_init (60 * 60, "2025/03/31 23:59:59");

  /* initialize workers */
  int lcore_id;
  for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++)
    {
      lcore_workers[lcore_id].func = NULL;
      lcore_workers[lcore_id].arg = NULL;
      lcore_workers[lcore_id].func_name = NULL;
    }

  lcore_id = rte_lcore_id ();
  lcore_workers[lcore_id].func = lthread_main;
  lcore_workers[lcore_id].func_name = "lthread_main";

  printf ("%s[%d]: %s: enter.\n", __FILE__, __LINE__, __func__);
  lthread_create (&lt, lthread_shell, NULL);
  //lthread_create (&lt, lthread_tap_manager, NULL);
  lthread_run ();
}

