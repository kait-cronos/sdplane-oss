#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <arpa/telnet.h>

#include "sdplane/command.h"
#include "sdplane/command_shell.h"
#include "sdplane/shell.h"

#include "sdplane/debug.h"
#include "sdplane/debug_cmd.h"

#include "sdplane/debug_category.h"
#include "sdplane/debug_log.h"
#include "sdplane/debug_zcmdsh.h"

shell_keyfunc_t key_func_iac_1[256];
shell_keyfunc_t key_func_iac_2[256];
shell_keyfunc_t key_func_subnego[256];

char *
telcmds2str (u_char telnet_cmd)
{
  char *telnet_cmd_str;
  switch (telnet_cmd)
    {
    case IAC:
      telnet_cmd_str = "IAC";
      break;
    case DONT:
      telnet_cmd_str = "DONT";
      break;
    case DO:
      telnet_cmd_str = "DO";
      break;
    case WONT:
      telnet_cmd_str = "WONT";
      break;
    case WILL:
      telnet_cmd_str = "WILL";
      break;
    case SB:
      telnet_cmd_str = "SB";
      break;
    case GA:
      telnet_cmd_str = "GA";
      break;
    case EL:
      telnet_cmd_str = "EL";
      break;
    case EC:
      telnet_cmd_str = "EC";
      break;
    case AYT:
      telnet_cmd_str = "AYT";
      break;
    case AO:
      telnet_cmd_str = "AO";
      break;
    case IP:
      telnet_cmd_str = "IP";
      break;
    case BREAK:
      telnet_cmd_str = "BREAK";
      break;
    case DM:
      telnet_cmd_str = "DM";
      break;
    case NOP:
      telnet_cmd_str = "NOP";
      break;
    case SE:
      telnet_cmd_str = "SE";
      break;
    case EOR:
      telnet_cmd_str = "EOR";
      break;
    case ABORT:
      telnet_cmd_str = "ABORT";
      break;
    case SUSP:
      telnet_cmd_str = "SUSP";
      break;
    case xEOF:
      telnet_cmd_str = "xEOF";
      break;
    default:
      telnet_cmd_str = "";
      break;
    }
  return telnet_cmd_str;
}

char *
telopts2str (u_char telnet_opt)
{
  char *telnet_opt_str;
  switch (telnet_opt)
    {
    case TELOPT_BINARY:
      telnet_opt_str = "BINARY";
      break;
    case TELOPT_ECHO:
      telnet_opt_str = "ECHO";
      break;
    case TELOPT_RCP:
      telnet_opt_str = "RCP";
      break;
    case TELOPT_SGA:
      telnet_opt_str = "SGA";
      break;
    case TELOPT_NAMS:
      telnet_opt_str = "NAMS";
      break;
    case TELOPT_STATUS:
      telnet_opt_str = "STATUS";
      break;
    case TELOPT_TM:
      telnet_opt_str = "TM";
      break;
    case TELOPT_RCTE:
      telnet_opt_str = "RCTE";
      break;
    case TELOPT_NAOL:
      telnet_opt_str = "NAOL";
      break;
    case TELOPT_NAOP:
      telnet_opt_str = "NAOP";
      break;
    case TELOPT_NAOCRD:
      telnet_opt_str = "NAOCRD";
      break;
    case TELOPT_NAOHTS:
      telnet_opt_str = "NAOHTS";
      break;
    case TELOPT_NAOHTD:
      telnet_opt_str = "NAOHTD";
      break;
    case TELOPT_NAOFFD:
      telnet_opt_str = "NAOFFD";
      break;
    case TELOPT_NAOVTS:
      telnet_opt_str = "NAOVTS";
      break;
    case TELOPT_NAOVTD:
      telnet_opt_str = "NAOVTD";
      break;
    case TELOPT_NAOLFD:
      telnet_opt_str = "NAOLFD";
      break;
    case TELOPT_XASCII:
      telnet_opt_str = "XASCII";
      break;
    case TELOPT_LOGOUT:
      telnet_opt_str = "LOGOUT";
      break;
    case TELOPT_BM:
      telnet_opt_str = "BM";
      break;
    case TELOPT_DET:
      telnet_opt_str = "DET";
      break;
    case TELOPT_SUPDUP:
      telnet_opt_str = "SUPDUP";
      break;
    case TELOPT_SUPDUPOUTPUT:
      telnet_opt_str = "SUPDUPOUTPUT";
      break;
    case TELOPT_SNDLOC:
      telnet_opt_str = "SNDLOC";
      break;
    case TELOPT_TTYPE:
      telnet_opt_str = "TTYPE";
      break;
    case TELOPT_EOR:
      telnet_opt_str = "EOR";
      break;
    case TELOPT_TUID:
      telnet_opt_str = "TUID";
      break;
    case TELOPT_OUTMRK:
      telnet_opt_str = "OUTMRK";
      break;
    case TELOPT_TTYLOC:
      telnet_opt_str = "TTYLOC";
      break;
    case TELOPT_3270REGIME:
      telnet_opt_str = "3270REGIME";
      break;
    case TELOPT_X3PAD:
      telnet_opt_str = "X3PAD";
      break;
    case TELOPT_NAWS:
      telnet_opt_str = "NAWS";
      break;
    case TELOPT_TSPEED:
      telnet_opt_str = "TSPEED";
      break;
    case TELOPT_LFLOW:
      telnet_opt_str = "LFLOW";
      break;
    case TELOPT_LINEMODE:
      telnet_opt_str = "LINEMODE";
      break;
    case TELOPT_OLD_ENVIRON:
      telnet_opt_str = "OLD_ENVIRON";
      break;
    case TELOPT_AUTHENTICATION:
      telnet_opt_str = "AUTHENTICATION";
      break;
    case TELOPT_ENCRYPT:
      telnet_opt_str = "ENCRYPT";
      break;
    case TELOPT_NEW_ENVIRON:
      telnet_opt_str = "NEW_ENVIRON";
      break;
    case TELOPT_EXOPL:
      telnet_opt_str = "EXOPL";
      break;
    default:
      telnet_opt_str = "";
      break;
    }
  return telnet_opt_str;
}

/* Send WILL TELOPT_ECHO to remote server. */
void
vty_will_echo (struct shell *shell)
{
  u_char cmd[] = { IAC, WILL, TELOPT_ECHO, '\0' };
  fprintf (shell->terminal, "%s", cmd);
  if (FLAG_CHECK (shell->debug_zcmdsh, DEBUG_TYPE (ZCMDSH, TELNET)))
    fprintf (shell->terminal, "IAC WILL TELOPT_ECHO%s", shell->NL);
  fflush (shell->terminal);
}

/* Make suppress Go-Ahead telnet option. */
void
vty_will_suppress_go_ahead (struct shell *shell)
{
  u_char cmd[] = { IAC, WILL, TELOPT_SGA, '\0' };
  fprintf (shell->terminal, "%s", cmd);
  if (FLAG_CHECK (shell->debug_zcmdsh, DEBUG_TYPE (ZCMDSH, TELNET)))
    fprintf (shell->terminal, "IAC WILL TELOPT_SGA%s", shell->NL);
  fflush (shell->terminal);
}

/* Make don't use linemode over telnet. */
void
vty_dont_linemode (struct shell *shell)
{
  u_char cmd[] = { IAC, DONT, TELOPT_LINEMODE, '\0' };
  fprintf (shell->terminal, "%s", cmd);
  if (FLAG_CHECK (shell->debug_zcmdsh, DEBUG_TYPE (ZCMDSH, TELNET)))
    fprintf (shell->terminal, "IAC DONT TELOPT_LINEMODE%s", shell->NL);
  fflush (shell->terminal);
}

/* Use window size. */
void
vty_do_window_size (struct shell *shell)
{
  u_char cmd[] = { IAC, DO, TELOPT_NAWS, '\0' };
  fprintf (shell->terminal, "%s", cmd);
  if (FLAG_CHECK (shell->debug_zcmdsh, DEBUG_TYPE (ZCMDSH, TELNET)))
    fprintf (shell->terminal, "IAC DO TELOPT_NAWS%s", shell->NL);
  fflush (shell->terminal);
}

int
vty_shell_keyfunc_iac_start (struct shell *shell)
{
  shell->keymap = key_func_iac_1;
  shell->telnet_cmd = 0;
#if 0
  DEBUG_ZCMDSH_LOG (TELNET, "IAC received.");
  if (FLAG_CHECK (shell->debug_zcmdsh, DEBUG_TYPE (ZCMDSH, TELNET)))
    fprintf (shell->terminal, "IAC received.%s", shell->NL);
  fflush (shell->terminal);
#endif
  return 0;
}

int
vty_shell_keyfunc_telnet_opt (struct shell *shell)
{
  shell->keymap = shell->keymap_normal;
  shell->telnet_opt = (u_char) shell->inputch;

  char *telnet_cmd_str;
  char telnet_cmd_strbuf[32];

  telnet_cmd_str = telcmds2str (shell->telnet_cmd);
  snprintf (telnet_cmd_strbuf, sizeof (telnet_cmd_strbuf), "%s(%d|%#02x)",
            telnet_cmd_str, (u_char) shell->telnet_cmd,
            (u_char) shell->telnet_cmd);

  char *telnet_opt_str;
  char telnet_opt_strbuf[32];

  telnet_opt_str = telopts2str (shell->telnet_opt);
  snprintf (telnet_opt_strbuf, sizeof (telnet_opt_strbuf), "%s(%d|%#02x)",
            telnet_opt_str, (u_char) shell->telnet_opt,
            (u_char) shell->telnet_opt);

  DEBUG_ZCMDSH_LOG (TELNET, "IAC %s %s.", telnet_cmd_strbuf,
                    telnet_opt_strbuf);
  if (FLAG_CHECK (shell->debug_zcmdsh, DEBUG_TYPE (ZCMDSH, TELNET)))
    fprintf (shell->terminal, "IAC %s %s.%s", telnet_cmd_strbuf,
             telnet_opt_strbuf, shell->NL);
  fflush (shell->terminal);
  return 0;
}

int
vty_shell_keyfunc_telnet_cmd (struct shell *shell)
{
  shell->keymap = key_func_iac_2;
  shell->telnet_cmd = (u_char) shell->inputch;

#if 0 // omit debug logging on telnet_cmd (done in telnet_opt instead).
  char *telnet_cmd_str;
  char telnet_cmd_strbuf[32];

  telnet_cmd_str = telcmds2str (shell->telnet_cmd);
  snprintf (telnet_cmd_strbuf, sizeof (telnet_cmd_strbuf),
            "%s(%d|%#02x)", telnet_cmd_str,
            (u_char)shell->inputch, (u_char)shell->inputch);

  DEBUG_ZCMDSH_LOG (TELNET, "IAC %s.", telnet_cmd_strbuf);
  if (FLAG_CHECK (shell->debug_zcmdsh,
                  DEBUG_TYPE (ZCMDSH, TELNET)))
    fprintf (shell->terminal, "IAC %s.%s",
             telnet_cmd_strbuf, shell->NL);

  fflush (shell->terminal);
#endif
  return 0;
}

int
vty_shell_keyfunc_subnego (struct shell *shell)
{
#if 0
  DEBUG_ZCMDSH_LOG (TELNET, "subnego[%d] %#hhx.",
                    shell->subnego_size, shell->inputch);
  if (FLAG_CHECK (shell->debug_zcmdsh, DEBUG_TYPE (ZCMDSH, TELNET)))
    fprintf (shell->terminal, "subnego[%d] %#hhx.%s",
             shell->subnego_size, shell->inputch, shell->NL);
#endif

  if (shell->subnego_size < sizeof (shell->subnego_buf))
    {
      shell->subnego_buf[shell->subnego_size] = shell->inputch;
      shell->subnego_size++;
    }
  fflush (shell->terminal);
  return 0;
}

int
vty_shell_keyfunc_sb_start (struct shell *shell)
{
  shell->keymap = key_func_subnego;
  shell->subnego_size = 0;

  DEBUG_ZCMDSH_LOG (TELNET, "IAC SB (%#hhx).", shell->inputch);
  if (FLAG_CHECK (shell->debug_zcmdsh, DEBUG_TYPE (ZCMDSH, TELNET)))
    fprintf (shell->terminal, "IAC SB (%#hhx).%s", shell->inputch, shell->NL);
  fflush (shell->terminal);
  return 0;
}

int
vty_shell_keyfunc_sb_end (struct shell *shell)
{
  shell->keymap = shell->keymap_normal;

  DEBUG_ZCMDSH_LOG (TELNET, "IAC SE (%#hhx).", shell->inputch);
  if (FLAG_CHECK (shell->debug_zcmdsh, DEBUG_TYPE (ZCMDSH, TELNET)))
    fprintf (shell->terminal, "IAC SE (%#hhx).%s", shell->inputch, shell->NL);

  char subnego_str[64];
  char *p = &subnego_str[0];
  char *end = &subnego_str[63];
  int i;
  int ret;

  ret = snprintf (p, end - p, "len: %d [", shell->subnego_size);
  p += ret;
  for (i = 0; i < shell->subnego_size; i++)
    {
      ret = snprintf (p, end - p, " %#hhx", shell->subnego_buf[i]);
      p += ret;
    }
  ret = snprintf (p, end - p, "]");
  p += ret;

  DEBUG_ZCMDSH_LOG (TELNET, "telnet_sb: %s", subnego_str);
  if (FLAG_CHECK (shell->debug_zcmdsh, DEBUG_TYPE (ZCMDSH, TELNET)))
    fprintf (shell->terminal, "telnet_sb: %s%s", subnego_str, shell->NL);

  if (shell->subnego_size > 0)
    {
      switch (shell->subnego_buf[0])
        {
        case TELOPT_NAWS:
          if (shell->subnego_size == 5)
            {
              unsigned short width;
              unsigned short height;
              width = ((u_short) shell->subnego_buf[1] << 8) +
                      shell->subnego_buf[2];
              height = ((u_short) shell->subnego_buf[3] << 8) +
                       shell->subnego_buf[4];
              shell->winsize.ws_row = height;
              shell->winsize.ws_col = width;
              DEBUG_ZCMDSH_LOG (TELNET, "width: %hu height: %hu",
                                shell->winsize.ws_col, shell->winsize.ws_row);
              if (FLAG_CHECK (shell->debug_zcmdsh,
                              DEBUG_TYPE (ZCMDSH, TELNET)))
                fprintf (shell->terminal, "width: %hu height: %hu%s",
                         shell->winsize.ws_col, shell->winsize.ws_row,
                         shell->NL);
            }
          break;
        }
    }
  fflush (shell->terminal);
  shell_refresh (shell);
  return 0;
}

void
shell_telnet_keyfunc_init (struct shell *shell)
{
  int i;
  memset (key_func_iac_1, 0, sizeof (key_func_iac_1));
  memset (key_func_iac_2, 0, sizeof (key_func_iac_2));
  memset (key_func_subnego, 0, sizeof (key_func_subnego));
  for (i = 0; i < 256; i++)
    {
      key_func_iac_1[i] = vty_shell_keyfunc_telnet_opt;
      key_func_iac_2[i] = vty_shell_keyfunc_telnet_opt;
      key_func_subnego[i] = vty_shell_keyfunc_subnego;
    }

  key_func_iac_1[SB] = vty_shell_keyfunc_sb_start;
  key_func_iac_1[SE] = vty_shell_keyfunc_sb_end;
  key_func_iac_1[DO] = vty_shell_keyfunc_telnet_cmd;
  key_func_iac_1[WILL] = vty_shell_keyfunc_telnet_cmd;
  key_func_iac_1[DONT] = vty_shell_keyfunc_telnet_cmd;
  key_func_iac_1[WONT] = vty_shell_keyfunc_telnet_cmd;
  key_func_subnego[IAC] = vty_shell_keyfunc_iac_start;

  shell->keymap[IAC] = vty_shell_keyfunc_iac_start;
}
