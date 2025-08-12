/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#include <includes.h>

#include "debug.h"
#include "flag.h"
#include "shell.h"
#include "shell_keyfunc.h"
#include "vector.h"

#include "command.h"
#include "command_shell.h"

shell_keyfunc_t default_keymap[256] = {
  NULL,                        /* Function for CONTROL('@') */
  shell_keyfunc_move_to_begin, /* Function for CONTROL('A') */
  shell_keyfunc_backward_char, /* Function for CONTROL('B') */
  NULL,                        /* Function for CONTROL('C') */
  shell_keyfunc_delete_char,   /* Function for CONTROL('D') */
  shell_keyfunc_move_to_end,   /* Function for CONTROL('E') */
  shell_keyfunc_forward_char,  /* Function for CONTROL('F') */
  NULL,                        /* Function for CONTROL('G') */
  shell_keyfunc_backspace,     /* Function for CONTROL('H') */
  shell_keyfunc_insert_tab,    /* Function for CONTROL('I') */
  shell_keyfunc_empty_enter,   /* Function for CONTROL('J') */
  shell_keyfunc_kill_line,     /* Function for CONTROL('K') */
  shell_keyfunc_clear_screen,  /* Function for CONTROL('L') */
  shell_keyfunc_empty_enter,   /* Function for CONTROL('M') */
  NULL,                        /* Function for CONTROL('N') */
  NULL,                        /* Function for CONTROL('O') */

  NULL,                               /* Function for CONTROL('P') */
  NULL,                               /* Function for CONTROL('Q') */
  shell_keyfunc_refresh,              /* Function for CONTROL('R') */
  NULL,                               /* Function for CONTROL('S') */
  NULL,                               /* Function for CONTROL('T') */
  shell_keyfunc_kill_all,             /* Function for CONTROL('U') */
  NULL,                               /* Function for CONTROL('V') */
  shell_keyfunc_delete_word_backward, /* Function for CONTROL('W') */
  NULL,                               /* Function for CONTROL('X') */
  shell_keyfunc_yank,                 /* Function for CONTROL('Y') */
  NULL,                               /* Function for CONTROL('Z') */
  shell_keyfunc_escape,               /* Function for CONTROL('[') */
  NULL,                               /* Function for CONTROL('\') */
  NULL,                               /* Function for CONTROL(']') */
  NULL,                               /* Function for CONTROL('^') */
  NULL,                               /* Function for CONTROL('_') */

  shell_input_char, /* Function for Key(' ') */
  shell_input_char, /* Function for Key('!') */
  shell_input_char, /* Function for Key('"') */
  shell_input_char, /* Function for Key('#') */
  shell_input_char, /* Function for Key('$') */
  shell_input_char, /* Function for Key('%') */
  shell_input_char, /* Function for Key('&') */
  shell_input_char, /* Function for Key(''') */
  shell_input_char, /* Function for Key('(') */
  shell_input_char, /* Function for Key(')') */
  shell_input_char, /* Function for Key('*') */
  shell_input_char, /* Function for Key('+') */
  shell_input_char, /* Function for Key(',') */
  shell_input_char, /* Function for Key('-') */
  shell_input_char, /* Function for Key('.') */
  shell_input_char, /* Function for Key('/') */

  shell_input_char, /* Function for Key('0') */
  shell_input_char, /* Function for Key('1') */
  shell_input_char, /* Function for Key('2') */
  shell_input_char, /* Function for Key('3') */
  shell_input_char, /* Function for Key('4') */
  shell_input_char, /* Function for Key('5') */
  shell_input_char, /* Function for Key('6') */
  shell_input_char, /* Function for Key('7') */
  shell_input_char, /* Function for Key('8') */
  shell_input_char, /* Function for Key('9') */
  shell_input_char, /* Function for Key(':') */
  shell_input_char, /* Function for Key(';') */
  shell_input_char, /* Function for Key('<') */
  shell_input_char, /* Function for Key('=') */
  shell_input_char, /* Function for Key('>') */
  shell_input_char, /* Function for Key('?') */

  shell_input_char, /* Function for Key('@') */
  shell_input_char, /* Function for Key('A') */
  shell_input_char, /* Function for Key('B') */
  shell_input_char, /* Function for Key('C') */
  shell_input_char, /* Function for Key('D') */
  shell_input_char, /* Function for Key('E') */
  shell_input_char, /* Function for Key('F') */
  shell_input_char, /* Function for Key('G') */
  shell_input_char, /* Function for Key('H') */
  shell_input_char, /* Function for Key('I') */
  shell_input_char, /* Function for Key('J') */
  shell_input_char, /* Function for Key('K') */
  shell_input_char, /* Function for Key('L') */
  shell_input_char, /* Function for Key('M') */
  shell_input_char, /* Function for Key('N') */
  shell_input_char, /* Function for Key('O') */

  shell_input_char, /* Function for Key('P') */
  shell_input_char, /* Function for Key('Q') */
  shell_input_char, /* Function for Key('R') */
  shell_input_char, /* Function for Key('S') */
  shell_input_char, /* Function for Key('T') */
  shell_input_char, /* Function for Key('U') */
  shell_input_char, /* Function for Key('V') */
  shell_input_char, /* Function for Key('W') */
  shell_input_char, /* Function for Key('X') */
  shell_input_char, /* Function for Key('Y') */
  shell_input_char, /* Function for Key('Z') */
  shell_input_char, /* Function for Key('[') */
  shell_input_char, /* Function for Key('\') */
  shell_input_char, /* Function for Key(']') */
  shell_input_char, /* Function for Key('^') */
  shell_input_char, /* Function for Key('_') */

  shell_input_char, /* Function for Key(',') */
  shell_input_char, /* Function for Key('a') */
  shell_input_char, /* Function for Key('b') */
  shell_input_char, /* Function for Key('c') */
  shell_input_char, /* Function for Key('d') */
  shell_input_char, /* Function for Key('e') */
  shell_input_char, /* Function for Key('f') */
  shell_input_char, /* Function for Key('g') */
  shell_input_char, /* Function for Key('h') */
  shell_input_char, /* Function for Key('i') */
  shell_input_char, /* Function for Key('j') */
  shell_input_char, /* Function for Key('k') */
  shell_input_char, /* Function for Key('l') */
  shell_input_char, /* Function for Key('m') */
  shell_input_char, /* Function for Key('n') */
  shell_input_char, /* Function for Key('o') */

  shell_input_char,                   /* Function for Key('p') */
  shell_input_char,                   /* Function for Key('q') */
  shell_input_char,                   /* Function for Key('r') */
  shell_input_char,                   /* Function for Key('s') */
  shell_input_char,                   /* Function for Key('t') */
  shell_input_char,                   /* Function for Key('u') */
  shell_input_char,                   /* Function for Key('v') */
  shell_input_char,                   /* Function for Key('w') */
  shell_input_char,                   /* Function for Key('x') */
  shell_input_char,                   /* Function for Key('y') */
  shell_input_char,                   /* Function for Key('z') */
  shell_input_char,                   /* Function for Key('{') */
  shell_input_char,                   /* Function for Key('|') */
  shell_input_char,                   /* Function for Key('}') */
  shell_input_char,                   /* Function for Key('~') */
  shell_keyfunc_delete_char_advanced, /* Function for DEL */
};

int
shell_keyfunc_forward_char (struct shell *shell)
{
  /* Move forward */
  shell_forward (shell, 1);
  return 0;
}

int
shell_keyfunc_backward_char (struct shell *shell)
{
  /* Move backward */
  shell_backward (shell, 1);
  return 0;
}

int
shell_keyfunc_move_to_begin (struct shell *shell)
{
  /* Move to beggining-of-line */
  shell_backward (shell, shell->cursor);
  return 0;
}

int
shell_keyfunc_move_to_end (struct shell *shell)
{
  /* Move to end-of-line */
  shell_forward (shell, shell->end - shell->cursor);
  return 0;
}

int
shell_keyfunc_delete_char (struct shell *shell)
{
  /* Delete one character */
  if (shell->cursor < shell->end)
    shell_delete_string (shell, shell->cursor, shell->cursor + 1);
  return 0;
}

int
shell_keyfunc_backspace (struct shell *shell)
{
  /* Backspace */
  if (shell->cursor <= 0)
    return 0;

  if (FLAG_CHECK (shell->flag, SHELL_FLAG_ESCAPE))
    {
      shell_delete_word_backward (shell);
      return 0;
    }

  shell_backward (shell, 1);
  shell_delete_string (shell, shell->cursor, shell->cursor + 1);
  return 0;
}

int
shell_keyfunc_kill_line (struct shell *shell)
{
  /* Kill after the cursor */
  shell_cut (shell, shell->cursor, shell->end);
  return 0;
}

int
shell_keyfunc_kill_all (struct shell *shell)
{
  /* Kill all the line */
  shell_backward (shell, shell->cursor);
  shell_cut (shell, 0, shell->end);
  return 0;
}

int
shell_keyfunc_yank (struct shell *shell)
{
  /* Paste (Yank) */
  if (shell->cut_buffer)
    shell_insert (shell, shell->cut_buffer);
  return 0;
}

int
shell_keyfunc_clear_screen (struct shell *shell)
{
  const char clear[] = { 27, '[', '2', 'J', '\0' };
  const char topleft[] = { 27, '[', '1', ';', '1', 'H', '\0' };
  fprintf (shell->terminal, "%s%s", clear, topleft);
  shell_format (shell);
  // shell_linefeed (shell);
  shell_refresh (shell);
  return 0;
}

int
shell_keyfunc_refresh (struct shell *shell)
{
  /* Refresh and Re-format */
  // shell_linefeed (shell);
  shell_format (shell);
  shell_linefeed (shell);
  shell_refresh (shell);
  return 0;
}

int
shell_keyfunc_empty_enter (struct shell *shell)
{
  shell_linefeed (shell);
  shell_clear (shell);
  shell_prompt (shell);
  fflush (shell->terminal);
  return 0;
}

int
shell_keyfunc_insert_tab (struct shell *shell)
{
  shell_insert (shell, "<tab>");
  return 0;
}

int
shell_keyfunc_escape (struct shell *shell)
{
  FLAG_SET (shell->flag, SHELL_FLAG_ESCAPE);
  return 0;
}

int
shell_keyfunc_delete_word_backward (struct shell *shell)
{
  shell_delete_word_backward (shell);
  return 0;
}

int
shell_keyfunc_delete_char_advanced (struct shell *shell)
{
  /* Delete one character */
  if (shell->cursor < shell->end)
    shell_delete_string (shell, shell->cursor, shell->cursor + 1);
  else if (shell->end > 0)
    shell_delete_string (shell, shell->end - 1, shell->end);
  return 0;
}

shell_keyfunc_t key_func_escape_1[256];
shell_keyfunc_t key_func_escape_2[256];

int
vty_shell_keyfunc_normal (struct shell *shell)
{
  shell->keymap = shell->keymap_normal;
  return 0;
}

int
vty_shell_move_word_backward (struct shell *shell)
{
  shell_move_word_backward (shell);
  vty_shell_keyfunc_normal (shell);
  return 0;
}

int
vty_shell_move_word_forward (struct shell *shell)
{
  shell_move_word_forward (shell);
  vty_shell_keyfunc_normal (shell);
  return 0;
}

int
vty_shell_delete_word_backward (struct shell *shell)
{
  shell_delete_word_backward (shell);
  vty_shell_keyfunc_normal (shell);
  return 0;
}

int
vty_shell_keyfunc_escape_1 (struct shell *shell)
{
  shell->keymap = key_func_escape_1;
  return 0;
}

int
vty_shell_keyfunc_escape_2 (struct shell *shell)
{
  shell->keymap = key_func_escape_2;
  return 0;
}

int command_history_prev (struct shell *shell);
int command_history_next (struct shell *shell);

int
vty_shell_keyfunc_arrow_up (struct shell *shell)
{
  command_history_prev (shell);
  vty_shell_keyfunc_normal (shell);
  return 0;
}

int
vty_shell_keyfunc_arrow_down (struct shell *shell)
{
  command_history_next (shell);
  vty_shell_keyfunc_normal (shell);
  return 0;
}

int
vty_shell_keyfunc_arrow_left (struct shell *shell)
{
  shell_backward (shell, 1);
  vty_shell_keyfunc_normal (shell);
  return 0;
}

int
vty_shell_keyfunc_arrow_right (struct shell *shell)
{
  shell_forward (shell, 1);
  vty_shell_keyfunc_normal (shell);
  return 0;
}

void
shell_escape_keyfunc_init (struct shell *shell)
{
  int i;
  memset (key_func_escape_1, 0, sizeof (key_func_escape_1));
  memset (key_func_escape_2, 0, sizeof (key_func_escape_2));

  for (i = 0; i < 256; i++)
    {
      key_func_escape_1[i] = vty_shell_keyfunc_normal;
      key_func_escape_2[i] = vty_shell_keyfunc_normal;
    }

  FUNC_STR_REGISTER (vty_shell_keyfunc_escape_1);
  FUNC_STR_REGISTER (vty_shell_keyfunc_escape_2);
  FUNC_STR_REGISTER (vty_shell_move_word_backward);
  FUNC_STR_REGISTER (vty_shell_move_word_forward);
  FUNC_STR_REGISTER (vty_shell_delete_word_backward);
  FUNC_STR_REGISTER (vty_shell_keyfunc_arrow_up);
  FUNC_STR_REGISTER (vty_shell_keyfunc_arrow_down);
  FUNC_STR_REGISTER (vty_shell_keyfunc_arrow_left);
  FUNC_STR_REGISTER (vty_shell_keyfunc_arrow_right);

  key_func_escape_1['b'] = vty_shell_move_word_backward;
  key_func_escape_1['f'] = vty_shell_move_word_forward;
  key_func_escape_1[CONTROL ('H')] = vty_shell_delete_word_backward;
  key_func_escape_1[0x7f] = vty_shell_delete_word_backward;
  key_func_escape_1['['] = vty_shell_keyfunc_escape_2;

  key_func_escape_2['A'] = vty_shell_keyfunc_arrow_up;
  key_func_escape_2['B'] = vty_shell_keyfunc_arrow_down;
  key_func_escape_2['C'] = vty_shell_keyfunc_arrow_right;
  key_func_escape_2['D'] = vty_shell_keyfunc_arrow_left;

  shell->keymap[CONTROL ('[')] = vty_shell_keyfunc_escape_1;
}

