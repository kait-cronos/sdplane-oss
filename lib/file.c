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

void
path_disassemble (char *pathname, char **dirname, char **filename)
{
  char *p;
  p = strrchr (pathname, '/');
  if (p == NULL)
    {
      *dirname = ".";
      *filename = pathname;
    }
  else if (p == &pathname[0])
    {
      *dirname = "/";
      *filename = &pathname[1];
    }
  else
    {
      *p = '\0';
      *dirname = pathname;
      *filename = ++p;
    }
}

FILE *
fopen_create (char *file, char *mode)
{
  FILE *fp = NULL;
  char pathname[FILENAME_MAX + 1];
  char *p, *dirname, *filename;
  int ret;
  struct stat statbuf;

  strncpy (pathname, file, sizeof (pathname));
  path_disassemble (pathname, &dirname, &filename);

  p = dirname;
  while ((p = strchr (p, '/')) != NULL)
    {
      *p = '\0';
      ret = stat (dirname, &statbuf);
      if (ret != 0 && errno == ENOENT)
        {
          ret = mkdir (dirname, 0755);
          if (ret != 0)
            return NULL;
        }
      *p++ = '/';
    }
  ret = stat (dirname, &statbuf);
  if (ret != 0 && errno == ENOENT)
    {
      ret = mkdir (dirname, 0755);
      if (ret != 0)
        return NULL;
    }

  fp = fopen (file, mode);
  return fp;
}

int
file_truncate (char *file)
{
  struct stat statbuf;
  int fail;
  int fd;

  fail = stat (file, &statbuf);
  if (fail)
    return -1;
  if (! S_ISREG (statbuf.st_mode))
    return -1;

  fd = open (file, O_TRUNC, 0);
  if (fd < 0)
    return -1;
  close (fd);

  return 0;
}

FILE *save_stdin;
FILE *save_stdout;
FILE *save_stderr;

int
redirect_stdio (FILE *std, FILE *fp)
{
  if (std == stdin)
    {
      save_stdin = stdin;
      stdin = fp;
    }
  else if (std == stdout)
    {
      save_stdout = stdout;
      stdout = fp;
    }
  else if (std == stderr)
    {
      save_stderr = stderr;
      stderr = fp;
    }

  return 0;
}

int
restore_stdio ()
{
  stdin = save_stdin;
  stdout = save_stdout;
  stderr = save_stderr;
}

