#include "support.h"
#include <sdplane/command.h>

extern int show_fib_ip_route_func (void *, int, char **);
extern int set_srv6_encap_head_func (void *, int, char **);

static void
check_row (const char *output, const char *prefix, const char *type)
{
  const char *header = strstr (output, "Type");
  const char *row = strstr (output, prefix);
  assert (header && row);
  while (row > output && row[-1] != '\n')
    row--;
  const char *end = strchr (row, '\n');
  size_t column = header - output;
  size_t length = strlen (type);
  assert (end && end - row > column + length);
  assert (! strncmp (row + column, type, length));
  assert (row[column + length] == ' ');
}

int
main (void)
{
  const struct
  {
    uint8_t type;
    const char *name;
  } cases[] = { { RTN_UNSPEC, "unspec" },
                { RTN_UNICAST, "unicast" },
                { RTN_LOCAL, "local" },
                { RTN_BROADCAST, "broadcast" },
                { RTN_ANYCAST, "anycast" },
                { RTN_MULTICAST, "multicast" },
                { RTN_BLACKHOLE, "blackhole" },
                { RTN_UNREACHABLE, "unreachable" },
                { RTN_PROHIBIT, "prohibit" },
                { RTN_THROW, "throw" },
                { RTN_NAT, "nat" },
                { RTN_XRESOLVE, "xresolve" },
                { 255, "unknown(255)" } };
  setup ();
  for (int f = 0; f < 2; f++)
    {
      char prefixes[sizeof (cases) / sizeof (cases[0])][64];
      for (int i = 0; i < sizeof (cases) / sizeof (cases[0]); i++)
        {
          char dst[48];
          snprintf (dst, sizeof (dst), f ? "2001:db8::%x" : "192.0.2.%d",
                    i + 1);
          snprintf (prefixes[i], sizeof (prefixes[i]), "%s/%d", dst,
                    f ? 128 : 32);
          struct route_wire w = route_wire (
              f ? AF_INET6 : AF_INET, dst, f ? 128 : 32,
              cases[i].type == RTN_LOCAL ? RT_TABLE_LOCAL : RT_TABLE_MAIN,
              cases[i].type);
          import_route (&w);
        }
      struct shell shell = { .terminal = stdout, .NL = "\n" };
      char *add[] = { "set",
                      "srv6",
                      "encap",
                      "head",
                      f ? "2001:db8:1::/64" : "198.51.100.0/24",
                      "2001:db8::a" };
      assert (set_srv6_encap_head_func (&shell, 6, add) == CMD_SUCCESS);
      process (internal_msg_recv (msg_queue_rib));
      char *output = NULL;
      size_t len = 0;
      shell.terminal = open_memstream (&output, &len);
      assert (shell.terminal);
      rib_tlocal = rcu_global_ptr_rib;
      char *argv[] = { "show", "fib", f ? "ipv6" : "ipv4", "route" };
      assert (show_fib_ip_route_func (&shell, 4, argv) == 0);
      assert (fclose (shell.terminal) == 0);
      assert (strstr (output, "NH-ID") && strstr (output, "Interface"));
      for (int i = 0; i < sizeof (cases) / sizeof (cases[0]); i++)
        check_row (output, prefixes[i], cases[i].name);
      check_row (output, add[4], "unicast");
      assert (strstr (output, "srv6-head"));
      fputs (output, stdout);
      free (output);
    }
  rib_tlocal = NULL;
  cleanup ();
  puts ("PASS: show fib route types and column alignment (IPv4/IPv6)");
  return 0;
}
