/*
 * Copyright (C) 2024 Yasuhiro Ohara. All rights reserved.
 */

#include <includes.h>

#include "flag.h"
#include "debug.h"
#include "vector.h"
#include "shell.h"
#include "shell_keyfunc.h"

shell_keyfunc_t default_keymap[256] =
{
  NULL,                           /* Function for CONTROL('@') */
  shell_keyfunc_move_to_begin,    /* Function for CONTROL('A') */
  shell_keyfunc_backward_char,    /* Function for CONTROL('B') */
  NULL,                           /* Function for CONTROL('C') */
  shell_keyfunc_delete_char,      /* Function for CONTROL('D') */
  shell_keyfunc_move_to_end,      /* Function for CONTROL('E') */
  shell_keyfunc_forward_char,     /* Function for CONTROL('F') */
  NULL,                           /* Function for CONTROL('G') */
  shell_keyfunc_backspace,        /* Function for CONTROL('H') */
  shell_keyfunc_insert_tab,       /* Function for CONTROL('I') */
  shell_keyfunc_empty_enter,      /* Function for CONTROL('J') */
  shell_keyfunc_kill_line,        /* Function for CONTROL('K') */
  shell_keyfunc_clear_screen,     /* Function for CONTROL('L') */
  shell_keyfunc_empty_enter,      /* Function for CONTROL('M') */
  NULL,                           /* Function for CONTROL('N') */
  NULL,                           /* Function for CONTROL('O') */

  NULL,                           /* Function for CONTROL('P') */
  NULL,                           /* Function for CONTROL('Q') */
  shell_keyfunc_refresh,          /* Function for CONTROL('R') */
  NULL,                           /* Function for CONTROL('S') */
  NULL,                           /* Function for CONTROL('T') */
  shell_keyfunc_kill_all,         /* Function for CONTROL('U') */
  NULL,                           /* Function for CONTROL('V') */
  shell_keyfunc_delete_word_backward, /* Function for CONTROL('W') */
  NULL,                           /* Function for CONTROL('X') */
  shell_keyfunc_yank,             /* Function for CONTROL('Y') */
  NULL,                           /* Function for CONTROL('Z') */
  shell_keyfunc_escape,           /* Function for CONTROL('[') */
  NULL,                           /* Function for CONTROL('\') */
  NULL,                           /* Function for CONTROL(']') */
  NULL,                           /* Function for CONTROL('^') */
  NULL,                           /* Function for CONTROL('_') */

  NULL,                           /* Function for Key(' ') */
  NULL,                           /* Function for Key('!') */
  NULL,                           /* Function for Key('"') */
  NULL,                           /* Function for Key('#') */
  NULL,                           /* Function for Key('$') */
  NULL,                           /* Function for Key('%') */
  NULL,                           /* Function for Key('&') */
  NULL,                           /* Function for Key(''') */
  NULL,                           /* Function for Key('(') */
  NULL,                           /* Function for Key(')') */
  NULL,                           /* Function for Key('*') */
  NULL,                           /* Function for Key('+') */
  NULL,                           /* Function for Key(',') */
  NULL,                           /* Function for Key('-') */
  NULL,                           /* Function for Key('.') */
  NULL,                           /* Function for Key('/') */

  NULL,                           /* Function for Key('0') */
  NULL,                           /* Function for Key('1') */
  NULL,                           /* Function for Key('2') */
  NULL,                           /* Function for Key('3') */
  NULL,                           /* Function for Key('4') */
  NULL,                           /* Function for Key('5') */
  NULL,                           /* Function for Key('6') */
  NULL,                           /* Function for Key('7') */
  NULL,                           /* Function for Key('8') */
  NULL,                           /* Function for Key('9') */
  NULL,                           /* Function for Key(':') */
  NULL,                           /* Function for Key(';') */
  NULL,                           /* Function for Key('<') */
  NULL,                           /* Function for Key('=') */
  NULL,                           /* Function for Key('>') */
  NULL,                           /* Function for Key('?') */

  NULL,                           /* Function for Key('@') */
  NULL,                           /* Function for Key('A') */
  NULL,                           /* Function for Key('B') */
  NULL,                           /* Function for Key('C') */
  NULL,                           /* Function for Key('D') */
  NULL,                           /* Function for Key('E') */
  NULL,                           /* Function for Key('F') */
  NULL,                           /* Function for Key('G') */
  NULL,                           /* Function for Key('H') */
  NULL,                           /* Function for Key('I') */
  NULL,                           /* Function for Key('J') */
  NULL,                           /* Function for Key('K') */
  NULL,                           /* Function for Key('L') */
  NULL,                           /* Function for Key('M') */
  NULL,                           /* Function for Key('N') */
  NULL,                           /* Function for Key('O') */

  NULL,                           /* Function for Key('P') */
  NULL,                           /* Function for Key('Q') */
  NULL,                           /* Function for Key('R') */
  NULL,                           /* Function for Key('S') */
  NULL,                           /* Function for Key('T') */
  NULL,                           /* Function for Key('U') */
  NULL,                           /* Function for Key('V') */
  NULL,                           /* Function for Key('W') */
  NULL,                           /* Function for Key('X') */
  NULL,                           /* Function for Key('Y') */
  NULL,                           /* Function for Key('Z') */
  NULL,                           /* Function for Key('[') */
  NULL,                           /* Function for Key('\') */
  NULL,                           /* Function for Key(']') */
  NULL,                           /* Function for Key('^') */
  NULL,                           /* Function for Key('_') */

  NULL,                           /* Function for Key(',') */
  NULL,                           /* Function for Key('a') */
  NULL,                           /* Function for Key('b') */
  NULL,                           /* Function for Key('c') */
  NULL,                           /* Function for Key('d') */
  NULL,                           /* Function for Key('e') */
  NULL,                           /* Function for Key('f') */
  NULL,                           /* Function for Key('g') */
  NULL,                           /* Function for Key('h') */
  NULL,                           /* Function for Key('i') */
  NULL,                           /* Function for Key('j') */
  NULL,                           /* Function for Key('k') */
  NULL,                           /* Function for Key('l') */
  NULL,                           /* Function for Key('m') */
  NULL,                           /* Function for Key('n') */
  NULL,                           /* Function for Key('o') */

  NULL,                           /* Function for Key('p') */
  NULL,                           /* Function for Key('q') */
  NULL,                           /* Function for Key('r') */
  NULL,                           /* Function for Key('s') */
  NULL,                           /* Function for Key('t') */
  NULL,                           /* Function for Key('u') */
  NULL,                           /* Function for Key('v') */
  NULL,                           /* Function for Key('w') */
  NULL,                           /* Function for Key('x') */
  NULL,                           /* Function for Key('y') */
  NULL,                           /* Function for Key('z') */
  NULL,                           /* Function for Key('{') */
  NULL,                           /* Function for Key('|') */
  NULL,                           /* Function for Key('}') */
  NULL,                           /* Function for Key('~') */
  shell_keyfunc_delete_char,      /* Function for DEL */
};

void
shell_keyfunc_forward_char (struct shell *shell)
{
  /* Move forward */
  shell_forward (shell, 1);
}

void
shell_keyfunc_backward_char (struct shell *shell)
{
  /* Move backward */
  shell_backward (shell, 1);
}

void
shell_keyfunc_move_to_begin (struct shell *shell)
{
  /* Move to beggining-of-line */
  shell_backward (shell, shell->cursor);
}

void
shell_keyfunc_move_to_end (struct shell *shell)
{
  /* Move to end-of-line */
  shell_forward (shell, shell->end - shell->cursor);
}

void
shell_keyfunc_delete_char (struct shell *shell)
{
  /* Delete one character */
  if (shell->cursor < shell->end)
    shell_delete_string (shell, shell->cursor, shell->cursor + 1);
}

void
shell_keyfunc_backspace (struct shell *shell)
{
  /* Backspace */
  if (shell->cursor <= 0)
    return;

  if (FLAG_CHECK (shell->flag, SHELL_FLAG_ESCAPE))
    {
      shell_delete_word_backward (shell);
      return;
    }

  shell_backward (shell, 1);
  shell_delete_string (shell, shell->cursor, shell->cursor + 1);
}

void
shell_keyfunc_kill_line (struct shell *shell)
{
  /* Kill after the cursor */
  shell_cut (shell, shell->cursor, shell->end);
}

void
shell_keyfunc_kill_all (struct shell *shell)
{
  /* Kill all the line */
  shell_backward (shell, shell->cursor);
  shell_cut (shell, 0, shell->end);
}

void
shell_keyfunc_yank (struct shell *shell)
{
  /* Paste (Yank) */
  if (shell->cut_buffer)
    shell_insert (shell, shell->cut_buffer);
}

void
shell_keyfunc_clear_screen (struct shell *shell)
{
  const char clear[] = { 27, '[', '2', 'J', '\0' };
  const char topleft[] = { 27, '[', '1', ';', '1', 'H', '\0' };
  fprintf (shell->terminal, "%s%s", clear, topleft);
  shell_format (shell);
  //shell_linefeed (shell);
  shell_refresh (shell);
}

void
shell_keyfunc_refresh (struct shell *shell)
{
  /* Refresh and Re-format */
  //shell_linefeed (shell);
  shell_format (shell);
  shell_linefeed (shell);
  shell_refresh (shell);
}

void
shell_keyfunc_empty_enter (struct shell *shell)
{
  shell_linefeed (shell);
  shell_clear (shell);
  shell_prompt (shell);
  fflush (shell->terminal);
}

void
shell_keyfunc_insert_tab (struct shell *shell)
{
  shell_insert (shell, "<tab>");
}

void
shell_keyfunc_escape (struct shell *shell)
{
  FLAG_SET (shell->flag, SHELL_FLAG_ESCAPE);
}

void
shell_keyfunc_delete_word_backward (struct shell *shell)
{
  shell_delete_word_backward (shell);
}

