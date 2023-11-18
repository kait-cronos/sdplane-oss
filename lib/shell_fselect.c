#include <includes.h>

#include "file.h"
#include "shell.h"

shell_keyfunc_t key_func_fselect[256];

int fselect_index = 0;
int fselect_ncolumn = 1;
char *fselect_path;
char *fselect_dirname;
char *fselect_filename;

shell_keyfunc_t *key_func_orig;

void
fselect_ls_candidate (FILE *terminal)
{
  int num = 0;
  DIR *dir;
  struct dirent *dirent;

  dir = opendir (fselect_dirname);
  if (dir == NULL)
    return;

  int maxlen = 0;
  while ((dirent = readdir (dir)) != NULL)
    {
      if (dirent->d_name[0] == '.')
        continue;
      if (! strncmp (dirent->d_name, fselect_filename, strlen (fselect_filename)))
        maxlen = (maxlen < dirent->d_namlen ? dirent->d_namlen : maxlen);
    }
  rewinddir (dir);

  fselect_ncolumn = (80 - 2) / (maxlen + 2);
  if (fselect_ncolumn == 0)
    fselect_ncolumn = 1;

  fprintf (terminal, "  maxlen: %d ncol: %d\n", maxlen, fselect_ncolumn);

  fprintf (terminal, "\n");

  char dirent_name[1024];

  while ((dirent = readdir (dir)) != NULL)
    {
      if (dirent->d_name[0] == '.')
        continue;

      if (dirent->d_type == DT_DIR)
        snprintf (dirent_name, sizeof (dirent_name), "%s/", dirent->d_name);
      else
        snprintf (dirent_name, sizeof (dirent_name), "%s", dirent->d_name);

      if (! strncmp (dirent->d_name, fselect_filename, strlen (fselect_filename)))
        {
          if (num % fselect_ncolumn == 0)
            fprintf (terminal, "  ");

          if (num == fselect_index)
            fprintf (terminal, "[");
          fprintf (terminal, "%-*s", maxlen + 2, dirent_name);
          if (num == fselect_index)
            fprintf (terminal, "]");

          if (num % fselect_ncolumn == fselect_ncolumn - 1)
            fprintf (terminal, "\n");

          num++;
        }
    }

  closedir (dir);
  fprintf (terminal, "\n");
}

char *
fselect_completion ()
{
  int num = 0;
  DIR *dir;
  struct dirent *dirent;
  char *completion = NULL;

  dir = opendir (fselect_dirname);
  if (dir == NULL)
    return NULL;

  while ((dirent = readdir (dir)) != NULL)
    {
      if (dirent->d_name[0] == '.')
        continue;

      if (! strncmp (dirent->d_name, fselect_filename, strlen (fselect_filename)))
        {
          if (num == fselect_index)
            completion = strdup (dirent->d_name);

          num++;
        }
    }

  closedir (dir);
  return completion;
}

void
fselect_keyfunc_start (struct shell *shell)
{
  int last_head;
  DIR *dir;
  struct dirent *dirent;

  key_func_orig = shell->key_func;
  shell->key_func = key_func_fselect;

  last_head = shell_word_head (shell, shell->cursor);
  fselect_path = strdup (&shell->command_line[last_head]);

  path_disassemble (fselect_path, &fselect_dirname, &fselect_filename);
  fprintf (shell->terminal, "  path: %s dir: %s filename: %s\n",
           fselect_path, fselect_dirname, fselect_filename);

  dir = opendir (fselect_dirname);
  if (dir == NULL)
    return;

  int maxlen = 0;
  while ((dirent = readdir (dir)) != NULL)
    {
      if (dirent->d_name[0] == '.')
        continue;
      if (! strncmp (dirent->d_name, fselect_filename, strlen (fselect_filename)))
        maxlen = (maxlen < dirent->d_namlen ? dirent->d_namlen : maxlen);
    }
  closedir (dir);

  fselect_ncolumn = (80 - 2) / (maxlen + 2);
  if (fselect_ncolumn == 0)
    fselect_ncolumn = 1;

  fprintf (shell->terminal, "  maxlen: %d ncol: %d\n", maxlen, fselect_ncolumn);
  fprintf (shell->terminal, "\n");

  fselect_index = 0;
  fselect_ls_candidate (shell->terminal);
}

void
fselect_keyfunc_enter (struct shell *shell)
{
  char *completion;

  fprintf (shell->terminal, "%s: called.\n", __func__);

  shell_moveto (shell, shell_word_end (shell, shell->cursor));
  completion = fselect_completion ();
  if (completion)
    {
      shell_insert (shell, completion);
      free (completion);
    }

  free (fselect_path);

  shell->key_func = key_func_orig;
}

void
fselect_keyfunc_left (struct shell *shell)
{
  fselect_ls_candidate (shell->terminal);
}

void
shell_fselect_init ()
{
  memset (key_func_fselect, 0, sizeof (key_func_fselect));

  //key_func_fselect[CONTROL('A')] = fselect_keyfunc_leftmost;
  //key_func_fselect[CONTROL('E')] = fselect_keyfunc_rightmost;
  //key_func_fselect[CONTROL('F')] = fselect_keyfunc_right;
  //key_func_fselect[CONTROL('B')] = fselect_keyfunc_left;
  key_func_fselect[CONTROL('J')] = fselect_keyfunc_enter;
  key_func_fselect[CONTROL('M')] = fselect_keyfunc_enter;
  //key_func_fselect[CONTROL('N')] = fselect_keyfunc_down;
  //key_func_fselect[CONTROL('P')] = fselect_keyfunc_up;

  //key_func_fselect['0'] = fselect_keyfunc_0;
  //key_func_fselect['1'] = fselect_keyfunc_1;
  //key_func_fselect['2'] = fselect_keyfunc_2;
  //key_func_fselect['3'] = fselect_keyfunc_3;
  //key_func_fselect['4'] = fselect_keyfunc_4;
  //key_func_fselect['5'] = fselect_keyfunc_5;
  //key_func_fselect['6'] = fselect_keyfunc_6;
  //key_func_fselect['7'] = fselect_keyfunc_7;
  //key_func_fselect['8'] = fselect_keyfunc_8;
  //key_func_fselect['9'] = fselect_keyfunc_9;

  //key_func_fselect['<'] = fselect_keyfunc_first;
  //key_func_fselect['>'] = fselect_keyfunc_end;

  //key_func_fselect['j'] = fselect_keyfunc_down;
  //key_func_fselect['k'] = fselect_keyfunc_up;
  //key_func_fselect['l'] = fselect_keyfunc_right;
  //key_func_fselect['h'] = fselect_keyfunc_left;
}



