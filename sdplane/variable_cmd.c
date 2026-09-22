#include "include.h"

#include <rte_pci.h>

#include <sdplane/vector.h>
#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>

#include "sdplane.h"

#ifdef REGVAR_SUPPORT
int
regvar_is_spec_ipv4_addr (char *spec)
{
  return ! strcmp (spec, "<IPV4-ADDR>");
}

int
regvar_is_match_ipv4_addr (char *spec, char *word)
{
  int ret;
  struct in_addr in_addr;
  ret = inet_pton (AF_INET, word, &in_addr);
  if (ret == 1)
    return 1;
  return 0;
}

int
regvar_is_matching_ipv4_addr (char *spec, char *word)
{
  char *p;
  int ndots = 0;
  int ndigits = 0;
  char digitstr[4] = { 0 };
  char *endptr;
  int ret;
  for (p = &word[0]; *p != '\0'; p++)
    {
      if ('0' <= *p && *p <= '9')
        {
          digitstr[ndigits] = *p;
          ndigits++;
          if (ndigits > 3)
            return 0;
          digitstr[ndigits] = '\0';
          ret = strtol (digitstr, &endptr, 0);
          if (*endptr != '\0')
            return 0;
          if (ret < 0 || 255 < ret)
            return 0;
        }
      else if (*p == '.')
        {
          /* if '.' appeared without any digit, it's malformed. */
          if (ndigits == 0)
            return 0;
          /* reset ndigits. */
          ndigits = 0;
          memset (digitstr, 0, sizeof (digitstr));
          ndots++;
          if (ndots > 3)
            return 0;
        }
      else
        {
          return 0;
        }
    }
  if (ndots <= 3 && ndigits <= 3)
    return 1;
  return 0;
}

char *
regvar_convert_cmdstr_ipv4_addr (char *spec, char *word)
{
  char cmdstr[32];
  snprintf (cmdstr, sizeof (cmdstr), "<A.B.C.D>");
  return strdup (cmdstr);
}

char *
regvar_convert_helpstr_ipv4_addr (char *spec, char *orig_helpstr, char *word)
{
  char helpstr[128];
  snprintf (helpstr, sizeof (helpstr),
            "%s: %s", orig_helpstr, word);
  return strdup (helpstr);
}

int
regvar_is_spec_ipv4_prefix (char *spec)
{
  return ! strcmp (spec, "<IPV4-PREFIX>");
}

int
regvar_is_match_ipv4_prefix (char *spec, char *word)
{
  int ret;
  char *p;
  char prefix[64];
  struct in_addr in_addr;
  char *endptr;
  snprintf (prefix, sizeof (prefix), "%s", word);
  p = strchr (prefix, '/');
  if (! p)
    return 0;
  *p++ = '\0';
  ret = inet_pton (AF_INET, prefix, &in_addr);
  if (ret != 1)
    return 0;
  if (strlen (p) == 0 || strlen (p) > 2)
    return 0;
  ret = strtol (p, &endptr, 10);
  if (*endptr != '\0')
    return 0;
  if (ret < 0 || 32 < ret)
    return 0;
  return 1;
}

int
regvar_is_matching_ipv4_prefix (char *spec, char *word)
{
  char *p;
  int ndots = 0;
  int ndigits = 0;
  char digitstr[4] = { 0 };
  char *endptr;
  int ret;
  char prefix[24];
  char *slash = NULL;
  struct in_addr in_addr;
  for (p = &word[0]; *p != '\0'; p++)
    {
      if ('0' <= *p && *p <= '9')
        {
          digitstr[ndigits] = *p;
          ndigits++;
          if ((slash && ndigits > 2) ||
              (ndigits > 3))
            return 0;
          digitstr[ndigits] = '\0';
          ret = strtol (digitstr, &endptr, 0);
          if (*endptr != '\0')
            return 0;
          if ((slash && (ret < 0 || 32 < ret)) ||
              (ret < 0 || 255 < ret))
            return 0;
        }
      else if (*p == '.')
        {
          /* if '.' appeared without any digit, it's malformed. */
          if (ndigits == 0)
            return 0;
          /* reset ndigits. */
          ndigits = 0;
          memset (digitstr, 0, sizeof (digitstr));
          ndots++;
          if (ndots > 3)
            return 0;
        }
      else if (*p == '/')
        {
          snprintf (prefix, sizeof (prefix), "%s", word);
          slash = strchr (prefix, '/');
          assert (slash);
          *slash = '\0';
          ret = inet_pton (AF_INET, prefix, &in_addr);
          if (ret != 1)
            return 0;
          slash = p;
          ndigits = 0;
          memset (digitstr, 0, sizeof (digitstr));
        }
      else
        {
          return 0;
        }
    }
  if (ndots <= 3 && ndigits <= 3 && (!slash || ndigits <= 2))
    return 1;
  return 0;
}

char *
regvar_convert_cmdstr_ipv4_prefix (char *spec, char *word)
{
  char cmdstr[32];
  snprintf (cmdstr, sizeof (cmdstr), "<A.B.C.D/M>");
  return strdup (cmdstr);
}

char *
regvar_convert_helpstr_ipv4_prefix (char *spec, char *orig_helpstr, char *word)
{
  char helpstr[128];
  snprintf (helpstr, sizeof (helpstr),
            "%s: %s", orig_helpstr, word);
  return strdup (helpstr);
}

/* IPv6 */
int
regvar_is_spec_ipv6_addr (char *spec)
{
  return ! strcmp (spec, "<IPV6-ADDR>");
}

int
regvar_is_match_ipv6_addr (char *spec, char *word)
{
  int ret;
  struct in6_addr in6_addr;
  ret = inet_pton (AF_INET6, word, &in6_addr);
  if (ret == 1)
    return 1;
  return 0;
}

int
regvar_is_matching_ipv6_addr (char *spec, char *word)
{
  char *p;
  int ncolon = 0;
  int ndigits = 0;
  char digitstr[5] = { 0 };
  char *endptr;
  int ret;
  int coloncolon = 0;
  for (p = &word[0]; *p != '\0'; p++)
    {
      if (('0' <= *p && *p <= '9') ||
          ('a' <= *p && *p <= 'f') ||
          ('A' <= *p && *p <= 'F'))
        {
          digitstr[ndigits] = *p;
          ndigits++;
          if (ndigits > 4)
            return 0;
          digitstr[ndigits] = '\0';
          ret = strtol (digitstr, &endptr, 16);
          if (*endptr != '\0')
            return 0;
          if (ret < 0 || 0xffff < ret)
            return 0;
        }
      else if (*p == ':')
        {
          /* "::" is allowed only one time. */
          if (ndigits == 0)
            {
              if (! coloncolon)
                coloncolon++;
              else
                return 0;
            }
          /* reset ndigits. */
          ndigits = 0;
          memset (digitstr, 0, sizeof (digitstr));
          ncolon++;
          if (ncolon > 15)
            return 0;
        }
      else
        {
          return 0;
        }
    }
  if (ncolon <= 15 && ndigits <= 4)
    return 1;
  return 0;
}

char *
regvar_convert_cmdstr_ipv6_addr (char *spec, char *word)
{
  char cmdstr[32];
  snprintf (cmdstr, sizeof (cmdstr), "<X:X::X:X>");
  return strdup (cmdstr);
}

char *
regvar_convert_helpstr_ipv6_addr (char *spec, char *orig_helpstr, char *word)
{
  char helpstr[128];
  snprintf (helpstr, sizeof (helpstr),
            "%s: %s", orig_helpstr, word);
  return strdup (helpstr);
}

int
regvar_is_spec_ipv6_prefix (char *spec)
{
  return ! strcmp (spec, "<IPV6-PREFIX>");
}

int
regvar_is_match_ipv6_prefix (char *spec, char *word)
{
  int ret;
  char *p;
  char prefix[64];
  struct in6_addr in6_addr;
  char *endptr;
  snprintf (prefix, sizeof (prefix), "%s", word);
  p = strchr (prefix, '/');
  if (! p)
    return 0;
  *p++ = '\0';
  ret = inet_pton (AF_INET6, prefix, &in6_addr);
  if (ret != 1)
    return 0;
  if (strlen (p) == 0 || strlen (p) > 3)
    return 0;
  ret = strtol (p, &endptr, 10);
  if (*endptr != '\0')
    return 0;
  if (ret < 0 || 128 < ret)
    return 0;
  return 1;
}

int
regvar_is_matching_ipv6_prefix (char *spec, char *word)
{
  char *p;
  int ncolon = 0;
  int ndigits = 0;
  char digitstr[5] = { 0 };
  char *endptr;
  int ret;
  char prefix[24];
  char *slash = NULL;
  struct in6_addr in6_addr;
  int coloncolon = 0;
  for (p = &word[0]; *p != '\0'; p++)
    {
      if (('0' <= *p && *p <= '9') ||
          ('a' <= *p && *p <= 'f') ||
          ('A' <= *p && *p <= 'F'))
        {
          digitstr[ndigits] = *p;
          ndigits++;
          if ((slash && ndigits > 3) ||
              (ndigits > 4))
            return 0;
          digitstr[ndigits] = '\0';
          ret = strtol (digitstr, &endptr, 16);
          if (*endptr != '\0')
            return 0;
          if ((slash && (ret < 0 || 128 < ret)) ||
              (ret < 0 || 0xffff < ret))
            return 0;
        }
      else if (*p == ':')
        {
          /* "::" is allowed only one time. */
          if (ndigits == 0)
            {
              if (! coloncolon)
                coloncolon++;
              else
                return 0;
            }
          /* reset ndigits. */
          ndigits = 0;
          memset (digitstr, 0, sizeof (digitstr));
          ncolon++;
          if (ncolon > 15)
            return 0;
        }
      else if (*p == '/')
        {
          snprintf (prefix, sizeof (prefix), "%s", word);
          slash = strchr (prefix, '/');
          assert (slash);
          *slash = '\0';
          ret = inet_pton (AF_INET6, prefix, &in6_addr);
          if (ret != 1)
            return 0;
          slash = p;
          ndigits = 0;
          memset (digitstr, 0, sizeof (digitstr));
        }
      else
        {
          return 0;
        }
    }
  if (ncolon <= 15 && ndigits <= 4 && (!slash || ndigits <= 3))
    return 1;
  return 0;
}

char *
regvar_convert_cmdstr_ipv6_prefix (char *spec, char *word)
{
  char cmdstr[32];
  snprintf (cmdstr, sizeof (cmdstr), "<X:X::X:X/M>");
  return strdup (cmdstr);
}

char *
regvar_convert_helpstr_ipv6_prefix (char *spec, char *orig_helpstr, char *word)
{
  char helpstr[128];
  snprintf (helpstr, sizeof (helpstr),
            "%s: %s", orig_helpstr, word);
  return strdup (helpstr);
}

/* <ETHER-ADDR> */
int
regvar_is_spec_ether_addr (char *spec)
{
  return ! strcmp (spec, "<ETHER-ADDR>");
}

int
regvar_is_match_ether_addr (char *spec, char *word)
{
  int ret;
  char addr_str[64];
  struct rte_ether_addr eaddr;
  snprintf (addr_str, sizeof (addr_str), "%s", word);
  ret = rte_ether_unformat_addr (addr_str, &eaddr);
  if (ret != 0)
    return 0;
  return 1;
}

int
regvar_is_matching_ether_addr (char *spec, char *word)
{
  char *p;
  int ncolon = 0;
  int ndots = 0;
  int nhyphen = 0;
  int ndigits = 0;
  char digitstr[5] = { 0 };
  char *endptr;
  int ret;

  for (p = &word[0]; *p != '\0'; p++)
    {
      if (('0' <= *p && *p <= '9') ||
          ('a' <= *p && *p <= 'f') ||
          ('A' <= *p && *p <= 'F'))
        {
          digitstr[ndigits] = *p;
          ndigits++;
          if (ndigits > 4)
            return 0;
          if ((nhyphen || ncolon) && ndigits > 2)
            return 0;
          digitstr[ndigits] = '\0';
          ret = strtol (digitstr, &endptr, 16);
          if (*endptr != '\0')
            return 0;
          if (ret < 0 || 0xffff < ret)
            return 0;
          if ((nhyphen || ncolon) &&
              (ret < 0 || 0xff < ret))
            return 0;
        }
      else if (*p == ':' || *p == '-')
        {
          if (ndots)
            return 0;
          if (*p == ':' && nhyphen)
            return 0;
          if (*p == '-' && ncolon)
            return 0;
          /* "::" or "--" is not allowed. */
          if (ndigits == 0)
            return 0;
          /* reset ndigits. */
          ndigits = 0;
          memset (digitstr, 0, sizeof (digitstr));
          if (*p == ':')
            {
              ncolon++;
              if (ncolon > 5)
                return 0;
            }
          else if (*p == '-')
            {
              nhyphen++;
              if (nhyphen > 5)
                return 0;
            }
        }
      else if (*p == '.')
        {
          if (nhyphen || ncolon)
            return 0;
          /* ".." is not allowed. */
          if (ndigits == 0)
            return 0;
          /* "0011.2233.4455" does not seem to allow less digits */
          if (ndigits != 4)
            return 0;
          /* reset ndigits. */
          ndigits = 0;
          memset (digitstr, 0, sizeof (digitstr));
          if (ndots > 2)
            return 0;
        }
      else
        {
          return 0;
        }
      if ((!! ncolon) + (!! nhyphen) + (!! ndots) > 2)
        return 0;
    }
  if ((ncolon <= 5 && ndigits <= 2) ||
      (nhyphen <= 5 && ndigits <= 2) ||
      (ndots <= 2 && ndigits <= 4))
    return 1;
  return 0;
}

char *
regvar_convert_cmdstr_ether_addr (char *spec, char *word)
{
  char cmdstr[32];
  snprintf (cmdstr, sizeof (cmdstr), "<AA:BB:CC:DD:EE:FF>");
  return strdup (cmdstr);
}

char *
regvar_convert_helpstr_ether_addr (char *spec, char *orig_helpstr, char *word)
{
  char helpstr[128];
  snprintf (helpstr, sizeof (helpstr),
            "%s: %s", orig_helpstr, word);
  return strdup (helpstr);
}

/* <PCI-BUS-ADDR> */
int
regvar_is_spec_pci_bus_addr (char *spec)
{
  return ! strcmp (spec, "<PCI-BUS-ADDR>");
}

int
regvar_is_match_pci_bus_addr (char *spec, char *word)
{
  int i;
  int ret;
  char addr_str[64];
  struct rte_pci_addr pci_addr;
  snprintf (addr_str, sizeof (addr_str), "%s", word);
  ret = rte_pci_addr_parse (addr_str, &pci_addr);
  if (ret != 0)
    return 0;
  return 1;
}

int
regvar_is_matching_pci_bus_addr (char *spec, char *word)
{
  char *p;
  int ncolon = 0;
  int ndots = 0;
  int ndigits = 0;
  char digitstr[5] = { 0 };
  char *endptr;
  int ret;

  int domain;
  int bus;
  int dev;
  int func;

  for (p = &word[0]; *p != '\0'; p++)
    {
      if (('0' <= *p && *p <= '9') ||
          ('a' <= *p && *p <= 'f') ||
          ('A' <= *p && *p <= 'F'))
        {
          digitstr[ndigits] = *p;
          ndigits++;
          if (ndigits > 4)
            return 0;
          if (ncolon && ndigits > 2)
            return 0;
          if (ndots && ndigits > 1)
            return 0;
          digitstr[ndigits] = '\0';
          ret = strtol (digitstr, &endptr, 16);
          if (*endptr != '\0')
            return 0;
          if (ncolon == 0)
            {
              /* domain or bus part */
              if (ret < 0 || 0xffff < ret)
                return 0;
            }
          else if (ncolon == 1)
            {
              /* bus or device part */
              if (ret < 0 || 0xff < ret)
                return 0;
            }
          else if (ncolon == 2)
            {
              /* device part */
              if (ret < 0 || 0x1f < ret)
                return 0;
            }
          else if (ndots == 1)
            {
              /* function part */
              if (ret < 0 || 0x8 < ret)
                return 0;
            }
        }
      else if (*p == ':')
        {
          if (ndots)
            return 0;
          if (ncolon == 0)
            {
              if (ndigits != 2 && ndigits != 4)
                return 0;
            }
          if (ncolon == 1)
            {
              if (ndigits != 2)
                return 0;
            }
          if (ndigits == 4)
            {
              domain = strtol (digitstr, &endptr, 16);
              if (domain < 0 || 0xffff < domain)
                return 0;
            }
          else if (ndigits == 2)
            {
              if (ncolon == 0)
                domain = 0;
              bus = strtol (digitstr, &endptr, 16);
              if (bus < 0 ||  0xff < bus)
                return 0;
            }
          /* reset ndigits. */
          ndigits = 0;
          memset (digitstr, 0, sizeof (digitstr));

          ncolon++;
          if (ncolon > 2)
            return 0;
        }
      else if (*p == '.')
        {
          if (ndots)
            return 0;
          if (ndigits != 2)
            return 0;
          dev = strtol (digitstr, &endptr, 16);
          if (dev < 0 ||  0x1f < dev)
            return 0;
          /* reset ndigits. */
          ndigits = 0;
          memset (digitstr, 0, sizeof (digitstr));
          ndots++;
        }
      else
        {
          return 0;
        }
      if (ncolon > 2 || ndots > 1)
        return 0;
    }
  if (ndots <= 1 && ncolon <= 2)
    return 1;
  return 0;
}

char *
regvar_convert_cmdstr_pci_bus_addr (char *spec, char *word)
{
  char cmdstr[32];
  snprintf (cmdstr, sizeof (cmdstr), "[XXXX:]XX:XX.X");
  return strdup (cmdstr);
}

char *
regvar_convert_helpstr_pci_bus_addr (char *spec, char *orig_helpstr, char *word)
{
  char helpstr[128];
  snprintf (helpstr, sizeof (helpstr),
            "%s: [domain:]bus:dev.func: %s", orig_helpstr, word);
  return strdup (helpstr);
}

/* registry for variables */

struct regvar regvar_ipv4_addr =
{
  "<IPV4-ADDR>",
  regvar_is_spec_ipv4_addr,
  regvar_is_match_ipv4_addr,
#ifdef REGVAR_MATCHING_SUPPORT
  regvar_is_matching_ipv4_addr,
#endif
  regvar_convert_cmdstr_ipv4_addr,
  regvar_convert_helpstr_ipv4_addr
};

struct regvar regvar_ipv4_prefix =
{
  "<IPV4-PREFIX>",
  regvar_is_spec_ipv4_prefix,
  regvar_is_match_ipv4_prefix,
#ifdef REGVAR_MATCHING_SUPPORT
  regvar_is_matching_ipv4_prefix,
#endif
  regvar_convert_cmdstr_ipv4_prefix,
  regvar_convert_helpstr_ipv4_prefix
};

struct regvar regvar_ipv6_addr =
{
  "<IPV6-ADDR>",
  regvar_is_spec_ipv6_addr,
  regvar_is_match_ipv6_addr,
#ifdef REGVAR_MATCHING_SUPPORT
  regvar_is_matching_ipv6_addr,
#endif
  regvar_convert_cmdstr_ipv6_addr,
  regvar_convert_helpstr_ipv6_addr
};

struct regvar regvar_ipv6_prefix =
{
  "<IPV6-PREFIX>",
  regvar_is_spec_ipv6_prefix,
  regvar_is_match_ipv6_prefix,
#ifdef REGVAR_MATCHING_SUPPORT
  regvar_is_matching_ipv6_prefix,
#endif
  regvar_convert_cmdstr_ipv6_prefix,
  regvar_convert_helpstr_ipv6_prefix
};

struct regvar regvar_ether_addr =
{
  "<ETHER-ADDR>",
  regvar_is_spec_ether_addr,
  regvar_is_match_ether_addr,
#ifdef REGVAR_MATCHING_SUPPORT
  regvar_is_matching_ether_addr,
#endif
  regvar_convert_cmdstr_ether_addr,
  regvar_convert_helpstr_ether_addr
};

struct regvar regvar_pci_bus_addr =
{
  "<PCI-BUS-ADDR>",
  regvar_is_spec_pci_bus_addr,
  regvar_is_match_pci_bus_addr,
#ifdef REGVAR_MATCHING_SUPPORT
  regvar_is_matching_pci_bus_addr,
#endif
  regvar_convert_cmdstr_pci_bus_addr,
  regvar_convert_helpstr_pci_bus_addr
};

#endif /*REGVAR_SUPPORT*/

CLI_COMMAND2 (echo_ipv4_addr,
              "echo ipv4-addr <IPV4-ADDR>",
              "echo command\n",
              "echo ipv4 addr\n",
              "Specify IPv4 Address.\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "echoing ipv4-addr: %s%s",
           argv[2], shell->NL);
  return CMD_SUCCESS;
}

CLI_COMMAND2 (echo_ipv4_prefix,
              "echo ipv4-prefix <IPV4-PREFIX>",
              "echo command\n",
              "echo ipv4 prefix\n",
              "Specify IPv4 Address Prefix.\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "echoing ipv4-prefix: %s%s",
           argv[2], shell->NL);
  return CMD_SUCCESS;
}

CLI_COMMAND2 (echo_ipv4_addr_or_prefix,
              "echo ipv4-addr-or-prefix (<IPV4-ADDR>|<IPV4-PREFIX>)",
              "echo command\n",
              "echo ipv4 addr or prefix\n",
              "Specify IPv4 Address.\n",
              "Specify IPv4 Prefix.\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "echoing ipv4-addr-or-prefix: %s%s",
           argv[2], shell->NL);
  if (argc > 3)
    fprintf (shell->terminal, "%s is a %s%s",
             argv[2], argv[3], shell->NL);
  return CMD_SUCCESS;
}

ALIAS_COMMAND (echo_ipv4_branch_address,
               echo_ipv4_addr_or_prefix,
              "echo ipv4-addr-or-prefix <IPV4-ADDR> address",
              "echo command\n"
              "echo ipv4 addr or prefix\n"
              "Specify IPv4 Address.\n"
              "Branch is a address.\n");

ALIAS_COMMAND (echo_ipv4_branch_prefix,
               echo_ipv4_addr_or_prefix,
              "echo ipv4-addr-or-prefix <IPV4-PREFIX> prefix",
              "echo command\n"
              "echo ipv4 addr or prefix\n"
              "Specify IPv4 Prefix.\n"
              "Branch is a prefix.\n");

CLI_COMMAND2 (echo_ipv6_addr_or_prefix,
              "echo ipv6-addr-or-prefix (<IPV6-ADDR>|<IPV6-PREFIX>)",
              "echo command\n",
              "echo ipv6 addr or prefix\n",
              "Specify IPv6 Address.\n",
              "Specify IPv6 Prefix.\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "echoing ipv6-addr-or-prefix: %s%s",
           argv[2], shell->NL);
  if (argc > 3)
    fprintf (shell->terminal, "%s is a %s%s",
             argv[2], argv[3], shell->NL);
  return CMD_SUCCESS;
}

ALIAS_COMMAND (echo_ipv6_branch_address,
               echo_ipv6_addr_or_prefix,
              "echo ipv6-addr-or-prefix <IPV6-ADDR> address",
              "echo command\n"
              "echo ipv6 addr or prefix\n"
              "Specify IPv6 Address.\n"
              "Branch is a address.\n");

ALIAS_COMMAND (echo_ipv6_branch_prefix,
               echo_ipv6_addr_or_prefix,
              "echo ipv6-addr-or-prefix <IPV6-PREFIX> prefix",
              "echo command\n"
              "echo ipv6 addr or prefix\n"
              "Specify IPv6 Prefix.\n"
              "Branch is a prefix.\n");

CLI_COMMAND2 (echo_ether_addr,
              "echo ether-addr <ETHER-ADDR>",
              "echo command\n",
              "echo ether address\n",
              "Specify Ether Address.\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "echoing ether-addr: %s%s",
           argv[2], shell->NL);
  return CMD_SUCCESS;
}

CLI_COMMAND2 (echo_pci_bus_addr,
              "echo pci-bus-addr <PCI-BUS-ADDR>",
              "echo command\n",
              "echo PCI Bus Address\n",
              "Specify PCI Bus Address.\n")
{
  struct shell *shell = (struct shell *) context;
  fprintf (shell->terminal, "echoing pci-bus-addr: %s%s",
           argv[2], shell->NL);
  return CMD_SUCCESS;
}

void
variable_cmd_init (struct command_set *cmdset)
{
#ifdef REGVAR_SUPPORT
  if (! cmdset->regvars)
    cmdset->regvars = vector_create ();
  vector_add (&regvar_ipv4_addr, cmdset->regvars);
  vector_add (&regvar_ipv4_prefix, cmdset->regvars);
  vector_add (&regvar_ipv6_addr, cmdset->regvars);
  vector_add (&regvar_ipv6_prefix, cmdset->regvars);
  vector_add (&regvar_ether_addr, cmdset->regvars);
  vector_add (&regvar_pci_bus_addr, cmdset->regvars);
#endif /*REGVAR_SUPPORT*/

  INSTALL_COMMAND2 (cmdset, echo_ipv4_addr);
  INSTALL_COMMAND2 (cmdset, echo_ipv4_prefix);
  INSTALL_COMMAND2 (cmdset, echo_ipv4_addr_or_prefix);
  INSTALL_COMMAND2 (cmdset, echo_ipv4_branch_address);
  INSTALL_COMMAND2 (cmdset, echo_ipv4_branch_prefix);

  INSTALL_COMMAND2 (cmdset, echo_ipv6_addr_or_prefix);
  INSTALL_COMMAND2 (cmdset, echo_ipv6_branch_address);
  INSTALL_COMMAND2 (cmdset, echo_ipv6_branch_prefix);

  INSTALL_COMMAND2 (cmdset, echo_ether_addr);
  INSTALL_COMMAND2 (cmdset, echo_pci_bus_addr);
}
