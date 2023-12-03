/*
 * Copyright (C) 2007-2023 Yasuhiro Ohara. All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <termios.h>

#include "flag.h"
#include "debug.h"

struct termios termios_old;
struct termios termios_new;

struct flag_name {
  int val;
  char *name;
} c_lflag_name [] =
{
  { ECHOKE,       "ECHOKE" },
  { ECHOE,        "ECHOE" },
  { ECHO,         "ECHO" },
  { ECHONL,       "ECHONL" },
  { ECHOPRT,      "ECHOPRT" },
  { ECHOCTL,      "ECHOCTL" },
  { ISIG,         "ISIG" },
  { ICANON,       "ICANON" },
  { ALTWERASE,    "ALTWERASE" },
  { IEXTEN,       "IEXTEN" },
  { EXTPROC,      "EXTPROC" },
  { TOSTOP,       "TOSTOP" },
  { FLUSHO,       "FLUSHO" },
  { NOKERNINFO,   "NOKERNINFO" },
  { PENDIN,       "PENDIN" },
  { NOFLSH,       "NOFLSH" },
};

void
termio_print_lflags (int c_lflag)
{
  int i;
  int size = sizeof (c_lflag_name) / sizeof (struct flag_name);
  printf ("[");
  for (i = 0; i < size; i++)
    {
      if (c_lflag & c_lflag_name[i].val)
        printf ("%s", c_lflag_name[i].name);
      printf ("|");
    }
  printf ("]\n");
}

void
termio_start ()
{
  if (FLAG_CHECK (debug_config, DEBUG_TERMIO))
    {
      printf ("termios_new: c_lflag: ");
      termio_print_lflags (termios_new.c_lflag);
    }

  /* change terminal settings */
  tcsetattr (0, TCSANOW, &termios_new);
}

void
termio_reset ()
{
  if (FLAG_CHECK (debug_config, DEBUG_TERMIO))
    {
      printf ("termios_old: c_lflag: ");
      termio_print_lflags (termios_old.c_lflag);
    }

  /* change terminal settings */
  tcsetattr (0, TCSANOW, &termios_old);
}

void
termio_init ()
{
  /* save original terminal settings */
  tcgetattr (0, &termios_old);

  if (FLAG_CHECK (debug_config, DEBUG_TERMIO))
    {
      printf ("termios_old: c_lflag: ");
      termio_print_lflags (termios_old.c_lflag);
    }

  /* disable canonical input */
  memcpy (&termios_new, &termios_old, sizeof (termios_new));
  termios_new.c_lflag &= ~(ICANON | ECHO | IEXTEN);
  //termios_new.c_oflag |= ONOCR;

  termio_start ();
}

void
termio_finish ()
{
  termio_reset ();
}

