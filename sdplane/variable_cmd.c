#include "include.h"

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

void
variable_cmd_init (struct command_set *cmdset)
{
#ifdef REGVAR_SUPPORT
  if (! cmdset->regvars)
    cmdset->regvars = vector_create ();
  vector_add (&regvar_ipv4_addr, cmdset->regvars);
  vector_add (&regvar_ipv4_prefix, cmdset->regvars);
#endif /*REGVAR_SUPPORT*/
  INSTALL_COMMAND2 (cmdset, echo_ipv4_addr);
  INSTALL_COMMAND2 (cmdset, echo_ipv4_prefix);
  INSTALL_COMMAND2 (cmdset, echo_ipv4_addr_or_prefix);
  INSTALL_COMMAND2 (cmdset, echo_ipv4_branch_address);
  INSTALL_COMMAND2 (cmdset, echo_ipv4_branch_prefix);
}
