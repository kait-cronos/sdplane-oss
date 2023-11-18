#include <includes.h>

#include "vector.h"
#include "file.h"
#include "shell.h"
#include "command.h"
#include "command_shell.h"

shell_keyfunc_t key_func_fselect[256];

int fselect_index = 0;
int fselect_ncolumn = 1;
int fselect_nentry = 0;
int fselect_maxlen = 0;
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

  fprintf (terminal, "\n");
  fprintf (terminal, "  maxlen: %d ncol: %d\n",
           fselect_maxlen, fselect_ncolumn);
  fprintf (terminal, "\n");

  char dirent_name[1024];
  int dirent_len;

  while ((dirent = readdir (dir)) != NULL)
    {
      if (dirent->d_name[0] == '.')
        continue;

      if (strncmp (dirent->d_name, fselect_filename,
                   strlen (fselect_filename)))
        continue;

      snprintf (dirent_name, sizeof (dirent_name),
                "%s%s",
                dirent->d_name,
                (dirent->d_type == DT_DIR ? "/" : ""));
      dirent_len = strlen (dirent_name);

      if (num % fselect_ncolumn == 0)
        fprintf (terminal, "  ");

      if (num == fselect_index)
        fprintf (terminal, "%s", "\033[7m");

      fprintf (terminal, "%s", dirent_name);

      if (num == fselect_index)
        fprintf (terminal, "%s", "\033[0m");

      if (fselect_ncolumn > 1)
        {
          if (fselect_maxlen + 2 - dirent_len > 0)
            fprintf (terminal, "%-*s",
                     fselect_maxlen + 2 - dirent_len, " ");

          if (num % fselect_ncolumn == fselect_ncolumn - 1)
            fprintf (terminal, "\n");
        }
      else
        fprintf (terminal, "\n");

      num++;
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

      if (! strncmp (dirent->d_name, fselect_filename,
                     strlen (fselect_filename)))
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

  struct command_set *cmdset = shell->cmdset;
  struct command_node *match = NULL;
  if (shell->end)
    match = command_match_node (shell->command_line, cmdset);
  if (! match || match == cmdset->root)
    {
      fprintf (shell->terminal, "\n");
      //fprintf (shell->terminal, "no command node matched.\n");
      shell_refresh (shell);
      return;
    }

  if (! file_spec (match->cmdstr))
    {
      int is_file_spec;
      struct vector_node *vn;
      struct command_node *node;

      is_file_spec = 0;
      for (vn = vector_head (match->cmdvec); vn; vn = vector_next (vn))
        {
          node = (struct command_node *) vn->data;
          if (file_spec (node->cmdstr))
            is_file_spec++;
        }

      if (! is_file_spec)
        {
          fprintf (shell->terminal, "\n");
          //fprintf (shell->terminal, "not in file spec.\n");
          shell_refresh (shell);
          return;
        }
    }

  fprintf (shell->terminal, "debug...\n");

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

  fselect_maxlen = 0;
  fselect_nentry = 0;
  while ((dirent = readdir (dir)) != NULL)
    {
      if (dirent->d_name[0] == '.')
        continue;
      if (strncmp (dirent->d_name, fselect_filename,
                   strlen (fselect_filename)))
        continue;

      fselect_maxlen = (fselect_maxlen < dirent->d_namlen ?
                        dirent->d_namlen : fselect_maxlen);
      fselect_nentry++;
    }
  closedir (dir);

  fselect_ncolumn = (80 - 2) / (fselect_maxlen + 2);
  if (fselect_ncolumn == 0)
    fselect_ncolumn = 1;

#if 0
  fprintf (shell->terminal, "\n");
  fprintf (shell->terminal, "  maxlen: %d ncol: %d\n",
           fselect_maxlen, fselect_ncolumn);
  fprintf (shell->terminal, "\n");
#endif

  fselect_index = 0;
  fselect_ls_candidate (shell->terminal);
}

void
fselect_keyfunc_quit (struct shell *shell)
{
  free (fselect_path);
  shell->key_func = key_func_orig;
}

void
fselect_keyfunc_enter (struct shell *shell)
{
  char *completion;

  //fprintf (shell->terminal, "%s: called.\n", __func__);

  shell_moveto (shell, shell_word_end (shell, shell->cursor));
  completion = fselect_completion ();
  if (completion)
    {
      shell_insert (shell, completion);
      free (completion);
    }

  fselect_keyfunc_quit (shell);
}

void
fselect_keyfunc_left (struct shell *shell)
{
  if (fselect_ncolumn > 1 &&
      fselect_index % fselect_ncolumn > 0)
    fselect_index--;
  fselect_ls_candidate (shell->terminal);
}

void
fselect_keyfunc_right (struct shell *shell)
{
  if (fselect_ncolumn > 1 &&
      fselect_index % fselect_ncolumn < fselect_ncolumn - 1)
    fselect_index++;
  fselect_ls_candidate (shell->terminal);
}

void
fselect_keyfunc_up (struct shell *shell)
{
  if (fselect_index - fselect_ncolumn >= 0)
    fselect_index -= fselect_ncolumn;
  fselect_ls_candidate (shell->terminal);
}

void
fselect_keyfunc_down (struct shell *shell)
{
  if (fselect_index + fselect_ncolumn < fselect_nentry)
    fselect_index += fselect_ncolumn;
  fselect_ls_candidate (shell->terminal);
}

void
fselect_keyfunc_none (struct shell *shell)
{
  fselect_ls_candidate (shell->terminal);
}

void
fselect_keyfunc_leftmost (struct shell *shell)
{
  int leftmost;
  leftmost = (fselect_index / fselect_ncolumn) * fselect_ncolumn;
  fselect_index = leftmost;
  fselect_ls_candidate (shell->terminal);
}

void
fselect_keyfunc_rightmost (struct shell *shell)
{
  int rightmost;
  rightmost = (fselect_index / fselect_ncolumn) * fselect_ncolumn +
              (fselect_ncolumn - 1);
  if (rightmost > fselect_nentry - 1)
    rightmost = fselect_nentry - 1;
  fselect_index = rightmost;
  fselect_ls_candidate (shell->terminal);
}

void
fselect_keyfunc_first (struct shell *shell)
{
  fselect_index = 0;
  fselect_ls_candidate (shell->terminal);
}

void
fselect_keyfunc_end (struct shell *shell)
{
  fselect_index = fselect_nentry;
  fselect_ls_candidate (shell->terminal);
}

void
shell_fselect_init ()
{
  int i;
  memset (key_func_fselect, 0, sizeof (key_func_fselect));
  for (i = 0; i < 256; i++)
    key_func_fselect[i] = fselect_keyfunc_none;

  key_func_fselect[CONTROL('A')] = fselect_keyfunc_leftmost;
  key_func_fselect[CONTROL('E')] = fselect_keyfunc_rightmost;
  key_func_fselect[CONTROL('F')] = fselect_keyfunc_right;
  key_func_fselect[CONTROL('B')] = fselect_keyfunc_left;
  key_func_fselect[CONTROL('J')] = fselect_keyfunc_enter;
  key_func_fselect[CONTROL('M')] = fselect_keyfunc_enter;
  key_func_fselect[CONTROL('N')] = fselect_keyfunc_down;
  key_func_fselect[CONTROL('P')] = fselect_keyfunc_up;

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

  key_func_fselect['<'] = fselect_keyfunc_first;
  key_func_fselect['>'] = fselect_keyfunc_end;

  key_func_fselect['j'] = fselect_keyfunc_down;
  key_func_fselect['k'] = fselect_keyfunc_up;
  key_func_fselect['l'] = fselect_keyfunc_right;
  key_func_fselect['h'] = fselect_keyfunc_left;

  key_func_fselect['q'] = fselect_keyfunc_quit;
}



