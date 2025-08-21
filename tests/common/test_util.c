#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <limits.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/time.h>

#include <lthread.h>

#include <rte_common.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include <rte_mempool.h>
#include <rte_eal.h>
#include <rte_eth_ring.h>
#include <rte_mempool.h>

#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>
#include <sdplane/debug_log.h>
#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/log_cmd.h>

#include "l3fwd.h"
#include "l3fwd_cmd.h"
#include "l2fwd_cmd.h"

#include "sdplane.h"
#include "thread_info.h"
#include "debug_sdplane.h"

#include "test_util.h"
#include "sdplane_version_for_test.h"
#include "test_assert.h"

char *pid_path = "/var/run/sdplane.pid";

struct rte_mempool *test_mbuf_pool = NULL;

int test_ports[TEST_PORT_NUM];
struct rte_ring *test_rings[TEST_RING_NUM];
char *test_rte_eal_argv[4] = { "sdplane", "-c", "0xf", "--no-pci" };
int test_rte_eal_argc = 4;

int test_ret;

static inline uint64_t
now_ms (void)
{
  struct timeval tv;
  if (gettimeofday (&tv, NULL) != 0)
    return 0;
  return ((uint64_t) tv.tv_sec) * 1000ULL + (tv.tv_usec / 1000ULL);
}

struct rte_mbuf *
test_mbuf_from_bytes (const void *data, uint16_t len)
{
  struct rte_mbuf *m = rte_pktmbuf_alloc (test_mbuf_pool);
  SDPLANE_ASSERT (m);

  void *dst = rte_pktmbuf_mtod (m, void *);
  memcpy (dst, data, len);
  m->data_len = len;
  m->pkt_len = len;
  return m;
}

int
test_enqueue_packet (struct rte_mbuf *m, int port_id, int rx_queue_idx)
{
  int ring_idx = RXQ_TO_RING_IDX (port_id, rx_queue_idx);
  return rte_ring_enqueue (test_rings[ring_idx], m);
}

int
test_dequeue_packet (int port_id, int tx_queue_idx, struct rte_mbuf **out,
                     int timeout_ms)
{
  int ring_idx = TXQ_TO_RING_IDX (port_id, tx_queue_idx);
  uint64_t deadline = now_ms () + (timeout_ms < 0 ? 0 : (uint64_t) timeout_ms);

  while (1)
    {
      int ret = rte_ring_dequeue (test_rings[ring_idx], (void **) out);
      if (ret == 0)
        return 0;
      if (timeout_ms >= 0 && now_ms () >= deadline)
        return -ETIMEDOUT;
      rte_pause ();
    }
}

int
test_mbuf_compare (struct rte_mbuf *a, struct rte_mbuf *b)
{
  if (! a || ! b)
    return -EINVAL;

  if (rte_pktmbuf_pkt_len (a) != rte_pktmbuf_pkt_len (b))
    return -1;

  uint16_t len = rte_pktmbuf_pkt_len (a);
  void *pa = rte_pktmbuf_mtod (a, void *);
  void *pb = rte_pktmbuf_mtod (b, void *);
  if (memcmp (pa, pb, len) != 0)
    return -1;
  return 0;
}

int
test_expect_packet_equal (struct rte_mbuf *expected, int port_id,
                          int tx_queue_idx, int timeout_ms)
{
  struct rte_mbuf *got = NULL;
  int ret = test_dequeue_packet (port_id, tx_queue_idx, &got, timeout_ms);
  if (ret < 0)
    return ret;
  int cmp = test_mbuf_compare (expected, got);
  rte_pktmbuf_free (got);
  return cmp;
}

int
test_enqueue_bytes (const void *data, uint16_t len, int port_id,
                    int rx_queue_idx)
{
  struct rte_mbuf *m = test_mbuf_from_bytes (data, len);
  if (! m)
    return -ENOMEM;
  int ret = test_enqueue_packet (m, port_id, rx_queue_idx);
  if (ret < 0)
    rte_pktmbuf_free (m);
  return ret;
}

int
test_expect_bytes_equal (const void *data, uint16_t len, int port_id,
                         int tx_queue_idx, int timeout_ms)
{
  struct rte_mbuf *expected = test_mbuf_from_bytes (data, len);
  if (! expected)
    return -ENOMEM;
  int ret =
      test_expect_packet_equal (expected, port_id, tx_queue_idx, timeout_ms);
  rte_pktmbuf_free (expected);
  return ret;
}

int
prepare_test (struct test_config *config)
{
  int dev_count = rte_eth_dev_count_avail ();

  test_mbuf_pool =
      rte_pktmbuf_pool_create ("test_mbuf_pool", 1024 * 8, 32, 0,
                               RTE_MBUF_DEFAULT_BUF_SIZE, SOCKET_ID_ANY);
  if (! test_mbuf_pool)
    {
      printf ("Failed to create mbuf pool: %s\n", rte_strerror (rte_errno));
      return 1;
    }


  for (int i = 0; i < TEST_RING_NUM; i++)
    {
      char ring_name[4];
      snprintf (ring_name, sizeof (ring_name), "R%d", i);
      test_rings[i] = rte_ring_create (ring_name, 256, SOCKET_ID_ANY,
                                       RING_F_SP_ENQ | RING_F_SC_DEQ);
      if (! test_rings[i])
        {
          printf ("Failed to create ring %s: %s\n", ring_name,
                  rte_strerror (rte_errno));
          return 1;
        }
    }

  for (int i = 0; i < TEST_PORT_NUM; i++)
    {
      char port_name[2];
      snprintf (port_name, sizeof (port_name), "%d", i);
      test_ports[i] = rte_eth_from_rings (
          port_name, &test_rings[RXQ_TO_RING_IDX (i, 0)], TEST_NRXQ_NUM,
          &test_rings[TXQ_TO_RING_IDX (i, 0)], TEST_NTXQ_NUM, SOCKET_ID_ANY);
      if (test_ports[i] < 0)
        {
          printf ("Failed to create port %d: %s\n", i,
                  rte_strerror (rte_errno));
          return 1;
        }
      printf ("Created port %d with rx_ring=%d and tx_ring=%d-%d\n", i,
              RXQ_TO_RING_IDX (i, 0), TXQ_TO_RING_IDX (i, 0),
              TXQ_TO_RING_IDX (i, TEST_NTXQ_NUM - 1));
    }

  if (config->config_path)
    apply_config (config->config_path);
}

void
test_lthread_main (void *arg)
{
  struct test_config *config = (struct test_config *) arg;
  prepare_test (config);
  test_ret = config->test_func ();
  force_quit = true;
}

void
signal_handler (int signum)
{
  force_quit = true;
}

int
apply_config (const char *config_path)
{
  struct shell *shell = NULL;

  shell = command_shell_create ();
  shell_set_prompt (shell, "startup-config> ");
  shell->pager = false;
  FLAG_UNSET (shell->flag, SHELL_FLAG_INTERACTIVE);

  // INSTALL_COMMAND2 (shell->cmdset, show_worker);
  INSTALL_COMMAND2 (shell->cmdset, start_stop_worker);

  INSTALL_COMMAND2 (shell->cmdset, debug_zcmdsh);
  // INSTALL_COMMAND2 (shell->cmdset, show_debug_zcmdsh);

  INSTALL_COMMAND2 (shell->cmdset, debug_sdplane);
  // INSTALL_COMMAND2 (shell->cmdset, show_debug_sdplane);

  INSTALL_COMMAND2 (shell->cmdset, l2fwd_init);

  log_cmd_init (shell->cmdset);
  l2fwd_cmd_init (shell->cmdset);
  l3fwd_cmd_init (shell->cmdset);
  sdplane_cmd_init (shell->cmdset);

  printf ("%s[%d]: %s: command set initialized.\n", __FILE__, __LINE__,
          __func__);

  // termio_init ();

  shell_clear (shell);
  shell_prompt (shell);

  printf ("%s[%d]: %s: opening %s.\n", __FILE__, __LINE__, __func__,
          config_path);
  int fd;
  int ret = 0;
  fd = open (config_path, O_RDONLY);
  printf ("%s[%d]: %s: opened %s.\n", __FILE__, __LINE__, __func__,
          config_path);
  if (fd >= 0)
    {
      shell_set_terminal (shell, fd, 1);
      while (shell_running (shell))
        {
          lthread_sleep (10); // yield.

          ret = shell_read_nowait (shell);
          if (ret < 0)
            {
              FLAG_SET (shell->flag, SHELL_FLAG_EXIT);
              DEBUG_SDPLANE_LOG (RIB, "shell_read_nowait: %d", ret);
              printf ("shell_read_nowait: %d\n", ret);
            }
        }
    }
  else
    printf ("%s[%d]: %s: opening %s: failed: %s.\n", __FILE__, __LINE__,
            __func__, config_path, strerror (errno));

  printf ("%s[%d]: %s: terminating.\n", __FILE__, __LINE__, __func__);
  fflush (stdout);

  // termio_finish ();
  if (ret < 0)
    return ret;
  return 0;
}

static int
run_test (struct test_config *config)
{
  lthread_t *lt = NULL;

  signal (SIGINT, signal_handler);
  signal (SIGTERM, signal_handler);
  signal (SIGHUP, signal_handler);

  debug_log_init (config->name);
  debug_zcmdsh_cmd_init ();
  command_shell_init ();
  sdplane_init ();

  printf ("%s[%d]: %s: started.\n", __FILE__, __LINE__, __func__);

  int ret = rte_eal_init (test_rte_eal_argc, test_rte_eal_argv);
  if (ret < 0)
    rte_panic ("Cannot init EAL\n");

  lthread_create (&lt, (lthread_func) test_lthread_main, config);
  thread_register (-1, lt, (lthread_func) test_lthread_main,
                   "test_lthread_main", config);
  lthread_run ();

  // l3fwd_terminate (argc, argv);
  return test_ret;
}

static int
open_logfile (const char *test_name, int flags)
{
  if (! test_name)
    {
      errno = EINVAL;
      return -1;
    }

  const char *logs_dir = "logs";
  if (mkdir (logs_dir, 0755) < 0 && errno != EEXIST)
    return -1;

  char name_sanitized[256];
  size_t ns = 0;
  const char *n = test_name;
  for (; *n && ns + 1 < sizeof (name_sanitized); n++)
    {
      unsigned char c = (unsigned char) *n;
      if (isalnum (c) || c == '-' || c == '_')
        name_sanitized[ns++] = c;
      else if (c == ' ')
        name_sanitized[ns++] = '_';
      else
        name_sanitized[ns++] = '_';
    }
  name_sanitized[ns] = '\0';
  if (ns == 0)
    snprintf (name_sanitized, sizeof (name_sanitized), "test");

  char pathbuf[4096];
  if (snprintf (pathbuf, sizeof (pathbuf), "%s/%s.log", logs_dir,
                name_sanitized) >= (int) sizeof (pathbuf))
    {
      errno = ENAMETOOLONG;
      return -1;
    }

  int fd;
  if (flags & O_CREAT)
    fd = open (pathbuf, flags, 0644);
  else
    fd = open (pathbuf, flags);
  return fd;
}

static void
print_test_log (const char *test_name)
{
  printf ("The log is following:\n");
  int rfd = open_logfile (test_name, O_RDONLY);
  if (rfd >= 0)
    {
      char buf[4096];
      ssize_t n;
      int any = 0;
      while ((n = read (rfd, buf, sizeof (buf))) > 0)
        {
          ssize_t w = write (STDOUT_FILENO, buf, n);
          (void) w;
          any = 1;
        }
      close (rfd);
      if (! any)
        printf ("(no captured log)\n");
    }
  else
    {
      printf ("(no captured log available)\n");
      perror ("open log for read");
    }
}

int
run_tests (struct test_config configs[], int count)
{
  int pass_count = 0;
  int fail_count = 0;
  uint64_t total_start = now_ms ();

  for (int i = 0; i < count; i++)
    {
      printf ("=== RUN   %s\n", configs[i].name);
      uint64_t start = now_ms ();

      int logfd = open_logfile (configs[i].name, O_WRONLY | O_CREAT | O_TRUNC);
      if (logfd < 0)
        {
          perror ("open_logfile");
          return -1;
        }

      pid_t pid = fork ();
      if (pid < 0)
        {
          perror ("fork");
          close (logfd);
          return -1;
        }

      if (pid == 0)
        {
          if (dup2 (logfd, STDOUT_FILENO) < 0)
            {
              perror ("dup2 stdout");
              _exit (127);
            }
          if (dup2 (logfd, STDERR_FILENO) < 0)
            {
              perror ("dup2 stderr");
              _exit (127);
            }

          if (logfd > STDERR_FILENO)
            close (logfd);

          int rc = run_test (&configs[i]);
          _exit (rc & 0xff);
        }
      else
        {
          close (logfd);

          int status = 0;
          if (waitpid (pid, &status, 0) < 0)
            {
              perror ("waitpid");
              return -1;
            }

          uint64_t end = now_ms ();
          uint64_t dur = (end >= start) ? (end - start) : 0;

          if (WIFEXITED (status))
            {
              int code = WEXITSTATUS (status);
              if (code == 0)
                {
                  printf ("--- PASS: %s (%llums)\n", configs[i].name,
                          (unsigned long long) dur);
                  pass_count++;
                }
              else
                {
                  printf ("--- FAIL: %s (%llums) exit code=%d\n",
                          configs[i].name, (unsigned long long) dur, code);
                  fail_count++;

                  print_test_log (configs[i].name);

                  printf ("FAIL\n");
                  return code;
                }
            }
          else if (WIFSIGNALED (status))
            {
              int sig = WTERMSIG (status);
              printf ("--- FAIL: %s (%llums) terminated by signal %d\n",
                      configs[i].name, (unsigned long long) dur, sig);
              fail_count++;

              print_test_log (configs[i].name);

              printf ("FAIL\n");
              return -1;
            }
          else
            {
              printf ("--- FAIL: %s (%llums) unknown status\n",
                      configs[i].name, (unsigned long long) dur);
              fail_count++;
              return -1;
            }
        }
    }

  uint64_t total_end = now_ms ();
  uint64_t total_dur =
      (total_end >= total_start) ? (total_end - total_start) : 0;
  printf ("PASS: %d tests, %d failures, elapsed %llums\n", pass_count,
          fail_count, (unsigned long long) total_dur);

  return 0;
}
