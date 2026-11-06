#include <assert.h>
#include "netlink_thread.c"

static bool fail_set;
static bool fail_get;
extern int __real_setsockopt (int, int, int, const void *, socklen_t);
extern int __real_getsockopt (int, int, int, void *, socklen_t *);

int
__wrap_setsockopt (int fd, int level, int option, const void *value,
                   socklen_t size)
{
  if (fail_set && level == SOL_SOCKET && option == SO_RCVBUF)
    {
      errno = EPERM;
      return -1;
    }
  return __real_setsockopt (fd, level, option, value, size);
}

int
__wrap_getsockopt (int fd, int level, int option, void *value, socklen_t *size)
{
  if (fail_get && level == SOL_SOCKET && option == SO_RCVBUF)
    {
      errno = EIO;
      return -1;
    }
  return __real_getsockopt (fd, level, option, value, size);
}

static int
buffer_size (int fd)
{
  int size;
  socklen_t len = sizeof (size);
  assert (getsockopt (fd, SOL_SOCKET, SO_RCVBUF, &size, &len) == 0);
  return size;
}

static int
reference_size (int fd, int requested)
{
  assert (setsockopt (fd, SOL_SOCKET, SO_RCVBUF, &requested,
                      sizeof (requested)) == 0);
  return buffer_size (fd);
}

static int
set_size (struct shell *shell, char *size)
{
  char *argv[] = { "set", "netlink", "receive-buffer", size };
  return set_netlink_receive_buffer_func (shell, 4, argv);
}

int
main (void)
{
  char *output = NULL;
  size_t length = 0;
  struct shell shell = { 0 };
  shell.terminal = open_memstream (&output, &length);
  shell.NL = "\n";
  assert (shell.terminal);

  int reference = socket (AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  assert (reference >= 0);
  int dump_size = buffer_size (reference);

  assert (show_netlink_receive_buffer_func (&shell, 0, NULL) == CMD_SUCCESS);
  fflush (shell.terminal);
  assert (strstr (output, "requested 1048576 bytes (socket not open)"));
  assert (set_size (&shell, "32768") == CMD_SUCCESS);
  assert (netlink_socket (&netlink_kernel, RTMGRP_LINK) == 0);
  assert (buffer_size (netlink_kernel.sockfd) ==
          reference_size (reference, 32768));

  assert (netlink_socket (&netlink_cmd, 0) == 0);
  assert (buffer_size (netlink_cmd.sockfd) == dump_size);

  assert (set_size (&shell, "65536") == CMD_SUCCESS);
  int expected = reference_size (reference, 65536);
  assert (buffer_size (netlink_kernel.sockfd) == expected);
  assert (buffer_size (netlink_cmd.sockfd) == dump_size);
  assert (show_netlink_receive_buffer_func (&shell, 0, NULL) == CMD_SUCCESS);
  fflush (shell.terminal);
  char shown[128];
  snprintf (shown, sizeof (shown), "requested 65536 bytes, actual %d bytes",
            expected);
  assert (strstr (output, shown));

  char *invalid[] = { "0",      "-1", "1073741824", "9999999999999999999999",
                      "32768x", "" };
  for (size_t i = 0; i < sizeof (invalid) / sizeof (*invalid); i++)
    assert (set_size (&shell, invalid[i]) == CMD_FAILURE);
  assert (netlink_rcvbuf_size == 65536);
  assert (buffer_size (netlink_kernel.sockfd) == expected);

  fail_set = true;
  assert (set_size (&shell, "32768") == CMD_FAILURE);
  fail_set = false;
  assert (netlink_rcvbuf_size == 65536);
  assert (buffer_size (netlink_kernel.sockfd) == expected);
  assert (show_netlink_receive_buffer_func (&shell, 0, NULL) == CMD_SUCCESS);
  fail_get = true;
  assert (show_netlink_receive_buffer_func (&shell, 0, NULL) == CMD_FAILURE);
  fail_get = false;
  fflush (shell.terminal);
  assert (strstr (output, "can't set netlink-kernel socket SO_RCVBUF"));
  assert (strstr (output, "can't get netlink-kernel socket SO_RCVBUF"));

  size_t before_limit = length;
  assert (set_size (&shell, "1073741823") == CMD_SUCCESS);
  assert (show_netlink_receive_buffer_func (&shell, 0, NULL) == CMD_SUCCESS);
  if (buffer_size (netlink_kernel.sockfd) < 2147483646)
    {
      fflush (shell.terminal);
      assert (strstr (output + before_limit, "check net.core.rmem_max"));
    }

  close (netlink_kernel.sockfd);
  netlink_kernel.sockfd = -1;
  fail_set = fail_get = true;
  assert (netlink_socket (&netlink_kernel, RTMGRP_LINK) == 0);
  fail_set = fail_get = false;
  close (netlink_kernel.sockfd);
  close (netlink_cmd.sockfd);
  close (reference);
  fclose (shell.terminal);
  free (output);
  puts (
      "PASS: Netlink buffer configuration, live changes, dump socket isolation, invalid values and socket errors");
  return 0;
}
