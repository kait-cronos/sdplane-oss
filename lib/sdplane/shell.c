/*
 * Copyright (C) 2007-2023 Yasuhiro Ohara. All rights reserved.
 */

#include <includes.h>

#include "debug.h"
#include "flag.h"
#include "shell.h"
#include "shell_keyfunc.h"
#include "vector.h"

#include "debug_category.h"
#include "debug_log.h"
#include "debug_zcmdsh.h"

// static unsigned char inputch = 0;

int
writec (int fd, char c)
{
  return write (fd, &c, 1);
}

void
shell_terminate (struct shell *shell)
{
  if (shell->end < 0 || shell->size <= shell->end)
    return;
  shell->command_line[shell->end] = '\0';
}

void
shell_format (struct shell *shell)
{
  char *command_line;
  int i, cursor, end;
  int count = 0;
  int ret;

  end = 0;
  cursor = 0;
  command_line = (char *) malloc (shell->size);
  if (command_line == NULL)
    return;
  memset (command_line, 0, shell->size);

  /* filter out the duplicated consecutive spaces. */
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

#if 0
  fprintf (shell->terminal, "back from %d to 0%s",
           shell->cursor, shell->NL);
  fprintf (shell->terminal, "write new command_line (%d bytes)%s",
           strlen (command_line), shell->NL);
  fprintf (shell->terminal, "write extra space (%d bytes)%s",
           shell->end - end, shell->NL);
  fprintf (shell->terminal, "go back from %d to %d%s",
           shell->end,  cursor, shell->NL);
#endif

  /* move to the beginning. */
  for (i = shell->cursor; 0 < i; i--)
    writec (shell->writefd, CONTROL ('H'));

  /* re-write the new command-line. */
  ret = write (shell->writefd, command_line, strlen (command_line));
  if (ret < 0)
    DEBUG_ZCMDSH_LOG (SHELL, "write(): failed: %s", strerror (errno));

  /* erase the last part. */
  for (i = end; i < shell->end; i++)
    writec (shell->writefd, ' ');

  /* move back to the cursor. */
  for (i = shell->end; cursor < i; i--)
    writec (shell->writefd, CONTROL ('H'));

  free (shell->command_line);
  shell->command_line = command_line;
  shell->cursor = cursor;
  shell->end = end;
}

void
shell_linefeed (struct shell *shell)
{
  fprintf (shell->terminal, "%s", shell->NL);
}

void
shell_clear (struct shell *shell)
{
  shell->cursor = 0;
  shell->end = 0;
  shell_terminate (shell);
}

void
shell_set_prompt (struct shell *shell, char *prompt)
{
  int newsize;
  char *newprompt;
  newsize = (strlen (prompt) / 128) + 128;
  if (shell->prompt_size < newsize)
    {
      newprompt = realloc (shell->prompt, newsize);
      if (! newprompt)
        {
          fprintf (stderr, "can't realloc new prompt.\n");
          return;
        }
      shell->prompt = newprompt;
      shell->prompt_size = newsize;
    }
  snprintf (shell->prompt, shell->prompt_size, "%s", prompt);
}

void
shell_prompt (struct shell *shell)
{
  int ret;
  if (shell->writefd < 0)
    return;

  /* there may be some left-over in the FILE buf. */
  if (shell->terminal)
    fflush (shell->terminal);

  /* move cursor to beginning */
  if (FLAG_CHECK (shell->flag, SHELL_FLAG_INTERACTIVE))
    writec (shell->writefd, '\r');

  /* print prompt */
  ret = write (shell->writefd, shell->prompt, strlen (shell->prompt));
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
  int ret;

  /* expand command_line */
  while (shell->end + strlen (s) + 1 > shell->size)
    shell_expand (shell);

  memmove (&shell->command_line[shell->cursor + strlen (s)],
           &shell->command_line[shell->cursor], shell->end - shell->cursor);
  memcpy (&shell->command_line[shell->cursor], s, strlen (s));
  shell->end += strlen (s);
  shell_terminate (shell);

  if (shell->writefd >= 0)
    {
      ret = write (shell->writefd, &shell->command_line[shell->cursor],
                   strlen (&shell->command_line[shell->cursor]));
      shell->cursor += strlen (s);
      for (i = shell->end; shell->cursor < i; i--)
        writec (shell->writefd, CONTROL ('H'));
    }
}

void
shell_input_char (struct shell *shell)
{
  char str[2];
  str[0] = shell->inputch;
  str[1] = '\0';
  shell_insert (shell, str);
}

void
shell_insert_char (struct shell *shell, char ch)
{
  char str[2];
  str[0] = ch;
  str[1] = '\0';
  // fprintf (shell->terminal, "%s: ch: %#x\n", __func__, (u_char)ch);
  shell_insert (shell, str);
}

/* delete string from start to end */
void
shell_delete_string (struct shell *shell, int start, int end)
{
  int i, movesize;
  int size;
  int ret;

  assert (0 <= start && start <= end);
  assert (start <= end && end <= shell->end);
  size = end - start;

  /* move the string after the deleted part */
  movesize = shell->end - end;
  if (movesize)
    memmove (&shell->command_line[start], &shell->command_line[end], movesize);

  /* erase the not-needed-anymore part */
  for (i = start + movesize; i < shell->end; i++)
    shell->command_line[i] = ' ';

  /* terminate the string at the previous end */
  shell_terminate (shell);

  /* go back to the place the deletion starts */
  for (i = shell->cursor; start < i; i--)
    writec (shell->writefd, CONTROL ('H'));

  /* redraw the part related to this deletion */
  ret = write (shell->writefd, &shell->command_line[start],
               strlen (&shell->command_line[start]));

  /* adjust the cursor pointer and go back to it */
  shell->cursor = start;
  for (i = shell->end; shell->cursor < i; i--)
    writec (shell->writefd, CONTROL ('H'));

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
    writec (shell->writefd, CONTROL ('H'));
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
shell_word_next_head (struct shell *shell, int point)
{
  char *start;
  char *stringp;
  char *next, *word;
  int head = 0;
  int passed = 0;

  assert (0 <= point && point <= shell->end);

  shell_terminate (shell);
  start = strdup (shell->command_line);
  stringp = start;

  next = NULL;
  while ((word = strsep (&stringp, SHELL_WORD_DELIMITERS)) != NULL)
    {
      if (passed)
        {
          if (*word != '\0')
            next = word;
          break;
        }
      if (word - start <= point && point <= word - start + strlen (word))
        passed++;
    }

  if (next)
    head = next - start;
  else
    head = shell->end;
  free (start);
  return head;
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

void
shell_move_word_backward (struct shell *shell)
{
  int start;

  start = shell_subword_head (shell, shell->cursor);
  if (start == shell->cursor)
    start = shell_subword_prev_head (shell, shell->cursor);

  shell_moveto (shell, start);
}

void
shell_move_word_forward (struct shell *shell)
{
  int start;

  start = shell_word_next_head (shell, shell->cursor);
  shell_moveto (shell, start);
}

#define DEBUG_POS 82
void
shell_debug (struct shell *shell)
{
  int i;
  char debug[64];
  int ret;

  shell_terminate (shell);

  fprintf (shell->terminal, "cmd: [");
  for (i = 0; i < shell->end; i++)
    fprintf (shell->terminal, " %#02x", shell->command_line[i]);
  fprintf (shell->terminal, " ]%s", shell->NL);
  fprintf (shell->terminal, "size: %d cursor: %d end: %d%s", shell->size,
           shell->cursor, shell->end, shell->NL);
  fflush (shell->terminal);

  snprintf (
      debug, sizeof (debug),
      "prevhead=%d whead=%d wend=%d cursor=%d end=%d inputch=%#02x size=%d",
      shell_word_prev_head (shell, shell->cursor),
      shell_word_head (shell, shell->cursor),
      shell_word_end (shell, shell->cursor), shell->cursor, shell->end,
      shell->inputch, shell->size);
  fprintf (shell->terminal, "%s%s", debug, shell->NL);
  fflush (shell->terminal);
  // ret = write (shell->writefd, debug, strlen (debug));

#if 0
  for (i = 0; i < shell->end; i++)
    {
      int cmdch;
      cmdch = shell->command_line[i];
      if (isascii (cmdch))
        write (shell->writefd, (char)cmdch, 1);
      else
        {
          snprintf (debug, sizeof (debug), "(%02x|%d)", cmdch, cmdch);
          write (shell->writefd, debug, strlen (debug));
        }
    }
  snprintf (debug, sizeof (debug), "%s", shell->NL);
  write (shell->writefd, debug, strlen (debug));
#endif

#if 0
  ret = write (shell->writefd, &shell->command_line[shell->cursor],
               strlen (&shell->command_line[shell->cursor]));
#endif
}

void
shell_refresh (struct shell *shell)
{
  int i;
  int ret;

  if (FLAG_CHECK (shell->flag, SHELL_FLAG_DEBUG) ||
      FLAG_CHECK (DEBUG_CONFIG (ZCMDSH), DEBUG_TYPE (ZCMDSH, SHELL)))
    shell_debug (shell);

  /* print prompt */
  shell_prompt (shell);

  /* print current command line */
  shell_terminate (shell);
  ret = write (shell->writefd, shell->command_line,
               strlen (shell->command_line));

  /* move cursor back to its position */
  for (i = shell->end; shell->cursor < i; i--)
    writec (shell->writefd, CONTROL ('H'));
}

void
shell_input (struct shell *shell, unsigned char ch)
{
  int escaped = 0;
  if (FLAG_CHECK (shell->flag, SHELL_FLAG_ESCAPE))
    escaped++;

  /* save input char */
  shell->inputch = ch;

#if 1
  if (FLAG_CHECK (DEBUG_CONFIG (ZCMDSH), DEBUG_TYPE (ZCMDSH, SHELL)))
    {
      fprintf (shell->terminal, "%s", shell->NL);
      fprintf (shell->terminal, "%s: inputch: %d/%#o/%#x", __func__, ch, ch,
               ch);
      if (CONTROL ('@') <= ch && ch <= CONTROL ('_'))
        fprintf (shell->terminal, " CONTROL('%c')%s", ch + '@', shell->NL);
      else if (ch == 127)
        fprintf (shell->terminal, " DEL%s", shell->NL);
      else if (isascii (ch))
        fprintf (shell->terminal, " '%c'%s", ch, shell->NL);
      else
        fprintf (shell->terminal, "%s", shell->NL);
      fprintf (shell->terminal, "keymap: %p, keymap[%d]: %p%s",
               (void *) shell->keymap, ch, (void *) shell->keymap[ch],
               shell->NL);
      shell_refresh (shell);
    }
#endif

  if (shell->keymap[ch])
    (*shell->keymap[ch]) (shell);

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

#include <poll.h>

int
shell_read_nowait (struct shell *shell)
{
  int ret;
  struct pollfd pollfds[1];
  pollfds[0].fd = shell->readfd;
  pollfds[0].events = POLLIN;
  pollfds[0].revents = 0;
  ret = poll (pollfds, 1, 0);
  if (ret > 0)
    return shell_read (shell);
  return ret;
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

  memcpy (shell->keymap_normal, default_keymap, sizeof (shell->keymap_normal));
  shell->keymap = shell->keymap_normal;

  shell_set_prompt (shell, "prompt> ");
  FLAG_SET (shell->flag, SHELL_FLAG_INTERACTIVE);

  shell->NL = "\n";

  shell->readfd = -1;
  shell->writefd = -1;

  return shell;
}

void
shell_delete (struct shell *shell)
{
  if (! FLAG_CHECK (shell->flag, SHELL_FLAG_CLOSE))
    shell_close (shell);
  if (shell->prompt)
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
shell_install (struct shell *shell, unsigned char key, shell_keyfunc_t func)
{
  shell->keymap[key] = func;
}

int
shell_running (struct shell *shell)
{
  if (FLAG_CHECK (shell->flag, SHELL_FLAG_EXIT) ||
      FLAG_CHECK (shell->flag, SHELL_FLAG_CLOSE))
    return 0;
  return 1;
}
