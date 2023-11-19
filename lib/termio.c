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
#include <stdio.h>
#include <string.h>
#include <termios.h>

struct termios oterm;

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
termio_init ()
{
  struct termios t;

  /* save original terminal settings */
  tcgetattr (0, &oterm);

  /* disable canonical input */
  memcpy (&t, &oterm, sizeof (t));
  t.c_lflag &= ~(ICANON | ECHO | IEXTEN);
  //t.c_oflag |= ONOCR;

  printf ("termios: c_lflag: old: ");
  termio_print_lflags (oterm.c_lflag);
  printf ("termios: c_lflag: new: ");
  termio_print_lflags (t.c_lflag);

  /* change terminal settings */
  tcsetattr (0, TCSANOW, &t);
}

void
termio_finish ()
{
  /* restore terminal settings */
  tcsetattr (0, TCSANOW, &oterm);
}


