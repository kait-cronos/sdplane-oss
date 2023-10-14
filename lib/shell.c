/*
 * Copyright (C) 2007  Yasuhiro Ohara
 * 
 * This file is part of SimRouting.
 * 
 * SimRouting is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * SimRouting is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <includes.h>

#include "vector.h"
#include "shell.h"

static unsigned char inputch = 0;

int
writec (int fd, char c)
{
  return write (fd, &c, 1);
}

void
shell_terminate (struct shell *shell)
{
  shell->command_line[shell->end] = '\0';
}

void
shell_format (struct shell *shell)
{
  char *command_line;
  int i, cursor, end;
  int count = 0;

  end = 0;
  cursor = 0;
  command_line = (char *) malloc (shell->size);
  if (command_line == NULL)
    return;
  memset (command_line, 0, shell->size);

  for (i = 0; i < shell->end; i++)
    {
      if (shell->command_line[i] == ' ')
        count++;

      /* omit redundant spaces */
      if (shell->command_line[i] == ' ' && count > 1)
        continue;

      /* omit even first space if it is the beginning of the line */
      if (shell->command_line[i] == ' ' && i == 0)
        continue;

      command_line[end++] = shell->command_line[i];
      if (i < shell->cursor)
        cursor++;

      if (shell->command_line[i] != ' ')
        count = 0;
    }

  free (shell->command_line);
  shell->command_line = command_line;
  shell->cursor = cursor;
  shell->end = end;
}

void
shell_linefeed (struct shell *shell)
{
  writec (shell->writefd, '\n');
}

void
shell_clear (struct shell *shell)
{
#if 0
  if (shell->interactive)
    writec (shell->writefd, '\r');
#endif
  shell->cursor = 0;
  shell->end = 0;
  shell_terminate (shell);
}

void
shell_prompt (struct shell *shell)
{
  if (shell->writefd < 0)
    return;

  /* move cursor to beginning */
  if (shell->interactive)
    writec (shell->writefd, '\r');

  /* print prompt */
  write (shell->writefd, shell->prompt, strlen (shell->prompt));
}

static void
shell_expand (struct shell *shell)
{
  int retry = 3;
  char *new = NULL;

  while (new == NULL && retry-- != 0)
    new = realloc (shell->command_line, shell->size * 2);

  if (new == NULL)
    {
      /* malloc failed */
      assert (0);
      exit (-1);
    }

  shell->command_line = new;
  shell->size *= 2;
}

void
shell_insert (struct shell *shell, char *s)
{
  int i;

  /* expand command_line */
  while (shell->end + strlen (s) + 1 > shell->size)
    shell_expand (shell);

  memmove (&shell->command_line[shell->cursor + strlen (s)],
           &shell->command_line[shell->cursor],
           shell->end - shell->cursor);
  memcpy (&shell->command_line[shell->cursor], s, strlen (s));
  shell->end += strlen (s);
  shell_terminate (shell);

  if (shell->writefd >= 0)
    {
      write (shell->writefd, &shell->command_line[shell->cursor],
             strlen (&shell->command_line[shell->cursor]));
      shell->cursor += strlen (s);
      for (i = shell->end; shell->cursor < i; i--)
        writec (shell->writefd, CONTROL('H'));
    }
}

void
shell_insert_char (struct shell *shell, char ch)
{
  char str[2];
  str[0] = ch;
  str[1] = '\0';
  shell_insert (shell, str);
}

void
shell_delete_string (struct shell *shell, int start, int end)
{
  int i, movesize;
  int size;

  assert (0 <= start && start <= end);
  assert (start <= end && end <= shell->end);
  size = end - start;

  /* update the string of the deleted part */
  movesize = shell->end - start - size;
  if (movesize)
    memmove (&shell->command_line[start],
             &shell->command_line[end], movesize);

  /* erase the not-needed-anymore part */
  for (i = start + movesize; i < shell->end; i++)
    shell->command_line[i] = ' ';

  /* terminate the string at the previous end */
  shell_terminate (shell);

  /* go back to the place the deletion starts */
  for (i = shell->cursor; start < i; i--)
    writec (shell->writefd, CONTROL('H'));

  /* redraw the part related to this deletion */
  write (shell->writefd, &shell->command_line[start],
         strlen (&shell->command_line[start]));

  /* adjust the cursor pointer and go back to it */
  shell->cursor = start;
  for (i = shell->end; shell->cursor < i; i--)
    writec (shell->writefd, CONTROL('H'));

  /* adjust the ending */
  shell->end -= size;
  shell_terminate (shell);
}

void
shell_cut (struct shell *shell, int start, int end)
{
  int size;

  assert (0 <= start && start <= end);
  assert (start <= end && end <= shell->end);
  size = end - start;

  if (shell->cut_buffer)
    {
      free (shell->cut_buffer);
      shell->cut_buffer = NULL;
    }

  if (size == 0)
    return;

  shell->cut_buffer = (char *) malloc (size + 1);
  if (shell->cut_buffer)
    {
      memcpy (shell->cut_buffer, &shell->command_line[start], size);
      shell->cut_buffer[size] = '\0';
    }

  shell_delete_string (shell, start, end);
}

void
shell_forward (struct shell *shell, int num)
{
  int i;

  if (shell->cursor + num > shell->end)
    return;

  for (i = shell->cursor; i < shell->cursor + num; i++)
    writec (shell->writefd, shell->command_line[i]);
  shell->cursor += num;
}

void
shell_backward (struct shell *shell, int num)
{
  int i;

  if (shell->cursor - num < 0)
    return;

  shell->cursor -= num;
  for (i = num; i > 0; i--)
    writec (shell->writefd, CONTROL('H'));
}

void
shell_moveto (struct shell *shell, int index)
{
  assert (0 <= index && index <= shell->end);
  if (shell->cursor < index)
    shell_forward (shell, index - shell->cursor);
  else if (index < shell->cursor)
    shell_backward (shell, shell->cursor - index);
}

/* used for command line tokenize */
int
shell_word_head (struct shell *shell, int point)
{
  char *start;
  char *stringp;
  char *word, *next;
  int head = 0;

  assert (0 <= point && point <= shell->end);

  shell_terminate (shell);
  start = strdup (shell->command_line);
  stringp = start;

  word = start;
  while ((next = strsep (&stringp, SHELL_WORD_DELIMITERS)) != NULL)
    {
      if (word - start <= point && point < next - start)
        break;
      if (*next != '\0')
        word = next;
    }

  if (word - start + strlen (word) < point)
    head = point;
  else
    head = word - start;

  free (start);
  return head;
}

/* used for command line word deletion */
int
shell_subword_head (struct shell *shell, int point)
{
  char *start;
  char *stringp;
  char *word, *next;
  int head = 0;

  assert (0 <= point && point <= shell->end);

  shell_terminate (shell);
  start = strdup (shell->command_line);
  stringp = start;

  word = start;
  while ((next = strsep (&stringp, SHELL_WORD_DELIMITERS_SUB)) != NULL)
    {
      if (word - start <= point && point < next - start)
        break;
      if (*next != '\0')
        word = next;
    }

  if (word - start + strlen (word) < point)
    head = point;
  else
    head = word - start;

  free (start);
  return head;
}

int
shell_word_end (struct shell *shell, int point)
{
  char *start;
  char *stringp;
  char *word, *next;
  int end = 0;

  assert (0 <= point && point <= shell->end);

  shell_terminate (shell);
  start = strdup (shell->command_line);
  stringp = start;

  word = start;
  while ((next = strsep (&stringp, SHELL_WORD_DELIMITERS)) != NULL)
    {
      if (word - start <= point && point < next - start)
        break;
      if (*next != '\0')
        word = next;
    }

  if (word - start + strlen (word) < point)
    end = point;
  else
    end = word - start + strlen (word);

  free (start);
  return end;
}

int
shell_word_prev_head (struct shell *shell, int point)
{
  char *start;
  char *stringp;
  char *prev, *word;
  int head = 0;

  assert (0 <= point && point <= shell->end);

  shell_terminate (shell);
  start = strdup (shell->command_line);
  stringp = start;

  prev = start;
  while ((word = strsep (&stringp, SHELL_WORD_DELIMITERS)) != NULL)
    {
      if (word - start <= point && point <= word - start + strlen (word))
        break;
      if (*word != '\0')
        prev = word;
    }

  head = prev - start;
  free (start);
  return head;
}

int
shell_subword_prev_head (struct shell *shell, int point)
{
  char *start;
  char *stringp;
  char *prev, *word;
  int head = 0;

  assert (0 <= point && point <= shell->end);

  shell_terminate (shell);
  start = strdup (shell->command_line);
  stringp = start;

  prev = start;
  while ((word = strsep (&stringp, SHELL_WORD_DELIMITERS_SUB)) != NULL)
    {
      if (word - start <= point && point <= word - start + strlen (word))
        break;
      if (*word != '\0')
        prev = word;
    }

  head = prev - start;
  free (start);
  return head;
}

void
shell_delete_word_backward (struct shell *shell)
{
  int start;

  start = shell_subword_head (shell, shell->cursor);
  if (start == shell->cursor)
    start = shell_subword_prev_head (shell, shell->cursor);

  shell_cut (shell, start, shell->cursor);
}

#define DEBUG_POS 82
void
shell_debug (struct shell *shell)
{
  int i;
  char debug[64];

  shell_terminate (shell);
  write (shell->writefd, &shell->command_line[shell->cursor],
         strlen (&shell->command_line[shell->cursor]));

  /* Go to the position where the debugging info will be out */
  if (shell->end < DEBUG_POS)
    {
      /* go forward */
      for (i = shell->end; i < DEBUG_POS; i++)
        writec (shell->writefd, ' ');
    }
  else if (shell->end > DEBUG_POS)
    {
      /* go backward */
      for (i = shell->end; i > DEBUG_POS; i--)
        writec (shell->writefd, CONTROL('H'));
    }

  snprintf (debug, sizeof (debug),
            "prevhead=%d whead=%d wend=%d cursor=%d end=%d inputch=%0x('%c') size=%d",
            shell_word_prev_head (shell, shell->cursor),
            shell_word_head (shell, shell->cursor),
            shell_word_end (shell, shell->cursor),
            shell->cursor, shell->end,
            inputch, inputch, shell->size);
  write (shell->writefd, debug, strlen (debug));

  /* Go back to the position where the cursor should be */
  if (shell->end < DEBUG_POS + strlen (debug))
    {
      /* go backward */
      for (i = DEBUG_POS + strlen (debug); i > shell->cursor; i--)
        writec (shell->writefd, CONTROL('H'));
    }
  else if (DEBUG_POS + strlen (debug) < shell->end)
    {
      /* go forward */
      i = shell->end - DEBUG_POS - strlen (debug);
      write (shell->writefd,
             &shell->command_line[DEBUG_POS + strlen (debug)], i);
    }
}

void
shell_refresh (struct shell *shell)
{
  int i;

  shell_prompt (shell);

  /* print current command line */
  shell_terminate (shell);
  write (shell->writefd, shell->command_line, strlen (shell->command_line));

  /* move cursor back to its position */
  for (i = shell->end; shell->cursor < i; i--)
    writec (shell->writefd, CONTROL('H'));

  if (FLAG_CHECK (shell->flag, SHELL_FLAG_DEBUG))
    shell_debug (shell);
}


void
shell_keyfunc_ctrl_f (struct shell *shell)
{
  /* Move forward */
  shell_forward (shell, 1);
}

void
shell_keyfunc_ctrl_b (struct shell *shell)
{
  /* Move backward */
  shell_backward (shell, 1);
}

void
shell_keyfunc_ctrl_a (struct shell *shell)
{
  /* Move to beggining-of-line */
  shell_backward (shell, shell->cursor);
}

void
shell_keyfunc_ctrl_e (struct shell *shell)
{
  /* Move to end-of-line */
  shell_forward (shell, shell->end - shell->cursor);
}

void
shell_keyfunc_ctrl_d (struct shell *shell)
{
  /* Delete one character */
  if (shell->cursor < shell->end)
    shell_delete_string (shell, shell->cursor, shell->cursor + 1);
}

void
shell_keyfunc_ctrl_h (struct shell *shell)
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
shell_keyfunc_ctrl_k (struct shell *shell)
{
  /* Kill after the cursor */
  shell_cut (shell, shell->cursor, shell->end);
}

void
shell_keyfunc_ctrl_u (struct shell *shell)
{
  /* Kill all the line */
  shell_backward (shell, shell->cursor);
  shell_cut (shell, 0, shell->end);
}

void
shell_keyfunc_ctrl_y (struct shell *shell)
{
  /* Paste (Yank) */
  if (shell->cut_buffer)
    shell_insert (shell, shell->cut_buffer);
}

void
shell_keyfunc_ctrl_r (struct shell *shell)
{
  /* Refresh and Re-format */
  shell_linefeed (shell);
  shell_format (shell);
  shell_refresh (shell);
}

void
shell_keyfunc_ctrl_j (struct shell *shell)
{
  shell_linefeed (shell);
  shell_clear (shell);
  shell_prompt (shell);
}

void
shell_keyfunc_ctrl_m (struct shell *shell)
{
  shell_linefeed (shell);
  shell_clear (shell);
  shell_prompt (shell);
}

void
shell_keyfunc_ctrl_i (struct shell *shell)
{
  shell_insert (shell, "<tab>");
}

void
shell_keyfunc_ctrl_lb (struct shell *shell)
{
  FLAG_SET (shell->flag, SHELL_FLAG_ESCAPE);
}

void
shell_keyfunc_ctrl_w (struct shell *shell)
{
  shell_delete_word_backward (shell);
}

shell_keyfunc_t default_key_func[] =
{
  NULL,                    /* Function for CONTROL('@') */
  shell_keyfunc_ctrl_a,    /* Function for CONTROL('A') */
  shell_keyfunc_ctrl_b,    /* Function for CONTROL('B') */
  NULL,                    /* Function for CONTROL('C') */
  shell_keyfunc_ctrl_d,    /* Function for CONTROL('D') */
  shell_keyfunc_ctrl_e,    /* Function for CONTROL('E') */
  shell_keyfunc_ctrl_f,    /* Function for CONTROL('F') */
  NULL,                    /* Function for CONTROL('G') */
  shell_keyfunc_ctrl_h,    /* Function for CONTROL('H') */
  shell_keyfunc_ctrl_i,    /* Function for CONTROL('I') */
  shell_keyfunc_ctrl_j,    /* Function for CONTROL('J') */
  shell_keyfunc_ctrl_k,    /* Function for CONTROL('K') */
  NULL,                    /* Function for CONTROL('L') */
  shell_keyfunc_ctrl_m,    /* Function for CONTROL('M') */
  NULL,                    /* Function for CONTROL('N') */
  NULL,                    /* Function for CONTROL('O') */

  NULL,                    /* Function for CONTROL('P') */
  NULL,                    /* Function for CONTROL('Q') */
  shell_keyfunc_ctrl_r,    /* Function for CONTROL('R') */
  NULL,                    /* Function for CONTROL('S') */
  NULL,                    /* Function for CONTROL('T') */
  shell_keyfunc_ctrl_u,    /* Function for CONTROL('U') */
  NULL,                    /* Function for CONTROL('V') */
  shell_keyfunc_ctrl_w,    /* Function for CONTROL('W') */
  NULL,                    /* Function for CONTROL('X') */
  shell_keyfunc_ctrl_y,    /* Function for CONTROL('Y') */
  NULL,                    /* Function for CONTROL('Z') */
  shell_keyfunc_ctrl_lb,   /* Function for CONTROL('[') */
  NULL,                    /* Function for CONTROL('\') */
  NULL,                    /* Function for CONTROL(']') */
  NULL,                    /* Function for CONTROL('^') */
  NULL,                    /* Function for CONTROL('_') */

  NULL,                    /* Function for Key(' ') */
  NULL,                    /* Function for Key('!') */
  NULL,                    /* Function for Key('"') */
  NULL,                    /* Function for Key('#') */
  NULL,                    /* Function for Key('$') */
  NULL,                    /* Function for Key('%') */
  NULL,                    /* Function for Key('&') */
  NULL,                    /* Function for Key(''') */
  NULL,                    /* Function for Key('(') */
  NULL,                    /* Function for Key(')') */
  NULL,                    /* Function for Key('*') */
  NULL,                    /* Function for Key('+') */
  NULL,                    /* Function for Key(',') */
  NULL,                    /* Function for Key('-') */
  NULL,                    /* Function for Key('.') */
  NULL,                    /* Function for Key('/') */

  NULL,                    /* Function for Key('0') */
  NULL,                    /* Function for Key('1') */
  NULL,                    /* Function for Key('2') */
  NULL,                    /* Function for Key('3') */
  NULL,                    /* Function for Key('4') */
  NULL,                    /* Function for Key('5') */
  NULL,                    /* Function for Key('6') */
  NULL,                    /* Function for Key('7') */
  NULL,                    /* Function for Key('8') */
  NULL,                    /* Function for Key('9') */
  NULL,                    /* Function for Key(':') */
  NULL,                    /* Function for Key(';') */
  NULL,                    /* Function for Key('<') */
  NULL,                    /* Function for Key('=') */
  NULL,                    /* Function for Key('>') */
  NULL,                    /* Function for Key('?') */

  NULL,                    /* Function for Key('@') */
  NULL,                    /* Function for Key('A') */
  NULL,                    /* Function for Key('B') */
  NULL,                    /* Function for Key('C') */
  NULL,                    /* Function for Key('D') */
  NULL,                    /* Function for Key('E') */
  NULL,                    /* Function for Key('F') */
  NULL,                    /* Function for Key('G') */
  NULL,                    /* Function for Key('H') */
  NULL,                    /* Function for Key('I') */
  NULL,                    /* Function for Key('J') */
  NULL,                    /* Function for Key('K') */
  NULL,                    /* Function for Key('L') */
  NULL,                    /* Function for Key('M') */
  NULL,                    /* Function for Key('N') */
  NULL,                    /* Function for Key('O') */

  NULL,                    /* Function for Key('P') */
  NULL,                    /* Function for Key('Q') */
  NULL,                    /* Function for Key('R') */
  NULL,                    /* Function for Key('S') */
  NULL,                    /* Function for Key('T') */
  NULL,                    /* Function for Key('U') */
  NULL,                    /* Function for Key('V') */
  NULL,                    /* Function for Key('W') */
  NULL,                    /* Function for Key('X') */
  NULL,                    /* Function for Key('Y') */
  NULL,                    /* Function for Key('Z') */
  NULL,                    /* Function for Key('[') */
  NULL,                    /* Function for Key('\') */
  NULL,                    /* Function for Key(']') */
  NULL,                    /* Function for Key('^') */
  NULL,                    /* Function for Key('_') */

  NULL,                    /* Function for Key(',') */
  NULL,                    /* Function for Key('a') */
  NULL,                    /* Function for Key('b') */
  NULL,                    /* Function for Key('c') */
  NULL,                    /* Function for Key('d') */
  NULL,                    /* Function for Key('e') */
  NULL,                    /* Function for Key('f') */
  NULL,                    /* Function for Key('g') */
  NULL,                    /* Function for Key('h') */
  NULL,                    /* Function for Key('i') */
  NULL,                    /* Function for Key('j') */
  NULL,                    /* Function for Key('k') */
  NULL,                    /* Function for Key('l') */
  NULL,                    /* Function for Key('m') */
  NULL,                    /* Function for Key('n') */
  NULL,                    /* Function for Key('o') */

  NULL,                    /* Function for Key('p') */
  NULL,                    /* Function for Key('q') */
  NULL,                    /* Function for Key('r') */
  NULL,                    /* Function for Key('s') */
  NULL,                    /* Function for Key('t') */
  NULL,                    /* Function for Key('u') */
  NULL,                    /* Function for Key('v') */
  NULL,                    /* Function for Key('w') */
  NULL,                    /* Function for Key('x') */
  NULL,                    /* Function for Key('y') */
  NULL,                    /* Function for Key('z') */
  NULL,                    /* Function for Key('{') */
  NULL,                    /* Function for Key('|') */
  NULL,                    /* Function for Key('}') */
  NULL,                    /* Function for Key('~') */
  shell_keyfunc_ctrl_d,    /* Function for DEL */
};


void
shell_input (struct shell *shell, unsigned char ch)
{
  int escaped = 0;
  if (FLAG_CHECK (shell->flag, SHELL_FLAG_ESCAPE))
    escaped++;

  /* for debug */
  inputch = ch;

  if (shell->key_func[ch])
    (*shell->key_func[ch]) (shell);
  else if (' ' <= ch && ch <= '~')
    shell_insert_char (shell, ch);

  if (escaped)
    FLAG_CLEAR (shell->flag, SHELL_FLAG_ESCAPE);

  if (FLAG_CHECK (shell->flag, SHELL_FLAG_DEBUG))
    shell_refresh (shell);
}

void
shell_read_close (struct shell *shell)
{
  FLAG_SET (shell->flag, SHELL_FLAG_CLOSE);
  if (shell->readfd >= 0)
    close (shell->readfd);
  shell->readfd = -1;
}

void
shell_write_close (struct shell *shell)
{
  fclose (shell->terminal);
  shell->terminal = NULL;
  if (shell->writefd >= 0)
    close (shell->writefd);
  shell->writefd = -1;
}

void
shell_close (struct shell *shell)
{
  FLAG_SET (shell->flag, SHELL_FLAG_CLOSE);

  if (shell->terminal)
    fclose (shell->terminal);
  shell->terminal = NULL;

  if (shell->readfd == shell->writefd)
    {
      if (shell->readfd >= 0)
        close (shell->readfd);
      shell->readfd = -1;
      shell->writefd = -1;
    }
  else
    {
      if (shell->readfd >= 0)
        close (shell->readfd);
      if (shell->writefd >= 0)
        close (shell->writefd);
      shell->readfd = -1;
      shell->writefd = -1;
    }
}

int
shell_read (struct shell *shell)
{
#define SHELL_READ_BUF_SIZE 256
  unsigned char buf[SHELL_READ_BUF_SIZE];
  int i, ret;

  memset (buf, 0, sizeof (buf));

  ret = read (shell->readfd, buf, sizeof (buf));
  if (ret < 0)
    return ret;

  if (ret == 0)
    {
      shell_linefeed (shell);
      shell_read_close (shell);
      return ret;
    }

  for (i = 0; i < ret; i++)
    shell_input (shell, buf[i]);

  return 0;
}

struct shell *
shell_create ()
{
  struct shell *shell;
  shell = (struct shell *) malloc (sizeof (struct shell));
  if (shell == NULL)
    return NULL;
  memset (shell, 0, sizeof (struct shell));

#define INITIAL_COMMAND_LINE_SIZE 32
  shell->command_line = (char *) malloc (INITIAL_COMMAND_LINE_SIZE);
  shell->size = INITIAL_COMMAND_LINE_SIZE;
  memset (shell->command_line, 0, shell->size);

  memcpy (shell->key_func, default_key_func,
          sizeof (shell->key_func));
  shell->prompt = strdup ("prompt> ");
  shell->interactive = 1;

  shell->readfd = -1;
  shell->writefd = -1;

  return shell;
}

void
shell_delete (struct shell *shell)
{
  if (! FLAG_CHECK (shell->flag, SHELL_FLAG_CLOSE))
    shell_close (shell);
  free (shell->prompt);
  free (shell->command_line);
  if (shell->cut_buffer)
    free (shell->cut_buffer);
  free (shell);
}

void
shell_set_terminal (struct shell *shell, int readfd, int writefd)
{
  if (shell->terminal)
    fflush (shell->terminal);

  FLAG_CLEAR (shell->flag, SHELL_FLAG_CLOSE);
  shell->readfd = readfd;
  shell->writefd = writefd;
  shell->terminal = fdopen (writefd, "w");
}

void
shell_set_prompt (struct shell *shell, char *prompt)
{
  free (shell->prompt);
  shell->prompt = strdup (prompt);
}

void
shell_install (struct shell *shell, unsigned char key,
               shell_keyfunc_t func)
{
  shell->key_func[key] = func;
}

int
shell_running (struct shell *shell)
{
  if (FLAG_CHECK (shell->flag, SHELL_FLAG_EXIT) ||
      FLAG_CHECK (shell->flag, SHELL_FLAG_CLOSE))
    return 0;
  return 1;
}

