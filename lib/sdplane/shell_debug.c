#include <includes.h>

#include "vector.h"
#include "file.h"
#include "shell.h"
#include "shell_keyfunc.h"
#include "termio.h"
#include "command.h"
#include "command_shell.h"

void
shell_debug (struct shell *shell)
{
  int i;
  char debug[64];

  shell_terminate (shell);

  char *command_line_dup;
  int argc;
  char **argv;
  struct command_node **cmdnodes;

  command_line_dup = strdup (shell->command_line);

  /* parse current command-line's argv. */
  command_argv_parse (command_line_dup, &argc, &argv);

  /* display current command-line's argv. */
  fprintf (shell->terminal, "debug: argc: %d argv:", argc);
  for (i = 0; i < argc; i++)
    {
      fprintf (shell->terminal, " %s", argv[i]);
    }
  fprintf (shell->terminal, "%s", shell->NL);

  /* find matching command nodes. */
  command_matched_nodes (argc, argv, shell->command_line,
                         shell->cmdset, &cmdnodes);

  /* display matched command nodes. */
  char *command_name;
  char *complete_str;
  for (i = 0; i < argc; i++)
    {
      command_name = "unknown";
      if (! cmdnodes[i] || ! cmdnodes[i]->func)
        command_name = "null";
      else
        {
          command_name = command_func_name_lookup (cmdnodes[i]->func);
          if (! command_name)
            command_name = "null";
        }
      complete_str = NULL;
      if (cmdnodes[i])
        complete_str = cmdnodes[i]->cmdstr;
      fprintf (shell->terminal, "cmdnode[%d]: %p %s %s (func: %s)%s",
               i, cmdnodes[i], argv[i], complete_str,
               command_name, shell->NL);
    }
  fflush (shell->terminal);

  /* need to free allocated memory. */
  free (cmdnodes);
  free (argv);
  free (command_line_dup);

  /* the shell status when the input was received. */
  fprintf (shell->terminal, "size: %d cursor: %d end: %d%s",
           shell->size, shell->cursor, shell->end, shell->NL);

  /* display the last input char. */
    {
      char ch = shell->inputch;
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

      int index;
      char *name = "unknown";
      shell_keyfunc_t ptr;
      ptr = shell->keymap[ch];
      index = func_table_lookup (ptr);
      if (index >= 0)
        name = func2str[index].str;

      fprintf (shell->terminal, "keymap: %p, keymap[%d]: %p (%s)%s",
               (void *) shell->keymap, ch, (void *) ptr, name,
               shell->NL);
    }

  /* history */
  int floor, start;
  struct command_history *history = shell->history;
  floor = HISTORY_NEXT (history->last);
  start = HISTORY_NEXT (floor);
  fprintf (shell->terminal,
           "history: start: %d last: %d floor: %d current: %d%s",
           start, history->last, floor, history->current, shell->NL);
#if 0
  /* ring buffer. from start to floor, possibly wrapping. */
  for (i = start; i != floor; i = HISTORY_NEXT (i))
    if (history->array[i])
      fprintf (shell->terminal, "history[%3d] %s%s",
               i, history->array[i], shell->NL);
#endif

  fflush (shell->terminal);
}


