/*
 * Copyright (C) 2007-2023 Yasuhiro Ohara. All rights reserved.
 */

#include <includes.h>

#include "debug.h"
#include "flag.h"

#include "command.h"
#include "file.h"
#include "vector.h"

#include "debug_category.h"
#include "debug_log.h"
#include "debug_zcmdsh.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif /*MIN*/

void
strcommon (char *dest, char *src)
{
  int i;
  for (i = 0; i < strlen (dest); i++)
    {
      if (dest[i] != src[i])
        dest[i] = '\0';
    }
}

int
command_node_cmp (const void *a, const void *b)
{
  struct command_node *ca = (struct command_node *) a;
  struct command_node *cb = (struct command_node *) b;
  return strcmp (ca->cmdstr, cb->cmdstr);
}

int
command_node_cmp_qsort (const void *a, const void *b)
{
  struct command_node *ca = *(struct command_node **) a;
  struct command_node *cb = *(struct command_node **) b;
  return strcmp (ca->cmdstr, cb->cmdstr);
}

struct command_node *
command_node_create ()
{
  struct command_node *node;
  node = (struct command_node *) malloc (sizeof (struct command_node));
  memset (node, 0, sizeof (struct command_node));
  node->cmdvec = vector_create ();
  return node;
}

void
command_node_delete (struct command_node *node)
{
  vector_delete (node->cmdvec);
  free (node);
}

struct command_set *
command_set_create ()
{
  struct command_set *cmdset;
  cmdset = (struct command_set *) malloc (sizeof (struct command_set));
  memset (cmdset, 0, sizeof (struct command_set));
  cmdset->root = command_node_create ();
  return cmdset;
}

void
command_set_delete (struct command_set *cmdset)
{
  command_node_delete (cmdset->root);
  free (cmdset);
}

int
ipv4_spec (char *spec)
{
  return (! strcmp (spec, "A.B.C.D"));
}

int
ipv4_match (char *spec, char *word)
{
  int ret;
  struct in_addr in;
  ret = inet_pton (AF_INET, word, &in);
  if (ret == 0)
    return 0;
  return 1;
}

int
ipv4_prefix_spec (char *spec)
{
  return (! strcmp (spec, "A.B.C.D/M"));
}

int
ipv4_prefix_match (char *spec, char *word)
{
  int ret, len;
  char *p, *endp;
  struct in_addr in;
  char *word2 = strdup (word);
  p = strchr (word2, '/');
  if (p == NULL)
    {
      free (word2);
      return 0;
    }
  *p++ = '\0';
  len = strtol (p, &endp, 10);
  if (*endp != '\0')
    {
      free (word2);
      return 0;
    }
  if (len < 0 || 32 < len)
    {
      free (word2);
      return 0;
    }
  ret = inet_pton (AF_INET, word2, &in);
  free (word2);
  return ret;
}

#define MAX_RANGE_SPEC_STR 10
int
range_spec (char *spec)
{
  unsigned long min, max;
  double dmin, dmax;
  char *s, *e;
  char minstr[MAX_RANGE_SPEC_STR + 1];
  char maxstr[MAX_RANGE_SPEC_STR + 1];
  char *endptr;

  if (spec[0] != '<')
    return 0;
  s = &spec[1];

  e = strchr (spec, '-');
  if (e == NULL)
    return 0;
  if (e - s > MAX_RANGE_SPEC_STR)
    return 0;
  memset (minstr, 0, sizeof (minstr));
  strncpy (minstr, s, e - s);

  s = e + 1;
  e = &spec[strlen (spec) - 1];
  if (*e != '>')
    return 0;
  if (e - s > MAX_RANGE_SPEC_STR)
    return 0;
  memset (maxstr, 0, sizeof (maxstr));
  strncpy (maxstr, s, e - s);

  if (strchr (minstr, '.') || strchr (maxstr, '.'))
    {
      dmin = strtod (minstr, &endptr);
      if (*endptr != '\0')
        return 0;

      dmax = strtod (maxstr, &endptr);
      if (*endptr != '\0')
        return 0;

      if (dmin > dmax)
        return 0;
    }
  else
    {
      min = strtoul (minstr, &endptr, 0);
      if (*endptr != '\0')
        return 0;

      max = strtoul (maxstr, &endptr, 0);
      if (*endptr != '\0')
        return 0;

      if (min > max)
        return 0;
    }

  return 1;
}

int
range_match (char *spec, char *word)
{
  unsigned long min, max, val;
  double dmin, dmax, dval;
  char *s, *e;
  char minstr[MAX_RANGE_SPEC_STR + 1];
  char maxstr[MAX_RANGE_SPEC_STR + 1];
  char *endptr;

  s = &spec[1];
  e = strchr (spec, '-');
  memset (minstr, 0, sizeof (minstr));
  strncpy (minstr, s, e - s);

  s = e + 1;
  e = &spec[strlen (spec) - 1];
  memset (maxstr, 0, sizeof (maxstr));
  strncpy (maxstr, s, e - s);

  if (strchr (minstr, '.') || strchr (maxstr, '.'))
    {
      dmin = strtod (minstr, &endptr);
      dmax = strtod (maxstr, &endptr);
      dval = strtod (word, &endptr);
      if (*endptr != '\0')
        return 0;

      if (dval < dmin || dmax < dval)
        return 0;
    }
  else
    {
      min = strtoul (minstr, &endptr, 0);
      max = strtoul (maxstr, &endptr, 0);
      val = strtoul (word, &endptr, 0);
      if (*endptr != '\0')
        return 0;

      if (val < min || max < val)
        return 0;
    }

  return 1;
}

int
double_spec (char *spec)
{
  if (! strcmp (spec, "<[-]ddd.ddd>"))
    return 1;
  if (! strcmp (spec, "<[-]d.ddde[+-]dd>"))
    return 1;
  return 0;
}

int
double_match (char *spec, char *word)
{
  char *endptr;
  double d;
  d = strtod (word, &endptr);
  if (*endptr != '\0')
    return 0;
  return 1;
}

int
file_spec (char *spec)
{
  return (! strcmp (spec, "<FILENAME>") || ! strcmp (spec, "<FILE>"));
}

int
file_match (char *spec, char *word)
{
  char pathname[FILENAME_MAX + 1];
  char *dirname, *filename;
  struct stat statbuf;
  int ret;

  strncpy (pathname, word, sizeof (pathname) - 1);
  path_disassemble (pathname, &dirname, &filename);

  /* assert directory part */
  ret = stat (dirname, &statbuf);
  if ((ret != 0 && errno != ENOENT) ||
      (ret == 0 && ! S_ISDIR (statbuf.st_mode)))
    {
      fprintf (stderr, "stat(): %s\n", strerror (errno));
      return 0;
    }

  ret = stat (filename, &statbuf);
  if (ret == 0)
    {
      /* existing regular file */
      if (S_ISREG (statbuf.st_mode))
        return 1;

      /* existing directory (for completion) */
      if (S_ISDIR (statbuf.st_mode))
        return 1;

      /* existing special file */
      return 0;
    }

  /* new file in existing or new directory */
  return 1;
}

char *
file_complete (char *word)
{
  char pathname[FILENAME_MAX + 1];
  char *dirname, *filename;
  DIR *dir;
  struct dirent *dirent;
  int nmatch;
  char matched[FILENAME_MAX + 1];
  char common[FILENAME_MAX + 1];
  static char retbuf[FILENAME_MAX + 1];

  memset (retbuf, 0, sizeof (retbuf));

  strncpy (pathname, word, sizeof (pathname) - 1);
  path_disassemble (pathname, &dirname, &filename);

  dir = opendir (dirname);
  if (dir == NULL)
    {
      fprintf (stderr, "opendir(): %s\n", strerror (errno));
      return NULL;
    }

  nmatch = 0;
  while ((dirent = readdir (dir)) != NULL)
    if (! strncmp (dirent->d_name, filename, strlen (filename)))
      {
        strncpy (matched, dirent->d_name, sizeof (matched));
        if (nmatch == 0)
          strncpy (common, dirent->d_name, sizeof (matched));
        else
          strcommon (common, dirent->d_name);
        nmatch++;
      }

  if (nmatch == 1)
    {
      int ret;
      struct stat statbuf;
      char filepath[FILENAME_MAX * 2 + 1];

      if (! strcmp (dirname, "/"))
        snprintf (filepath, sizeof (filepath), "/%s", matched);
      else
        snprintf (filepath, sizeof (filepath), "%s/%s", dirname, matched);

      ret = stat (filepath, &statbuf);
      if (ret == 0 && S_ISDIR (statbuf.st_mode))
        snprintf (retbuf, sizeof (retbuf), "%s/", &matched[strlen (filename)]);
      else
        snprintf (retbuf, sizeof (retbuf), "%s", &matched[strlen (filename)]);
    }
  else if (nmatch > 1)
    snprintf (retbuf, sizeof (retbuf), "%s", &common[strlen (filename)]);

  return retbuf;
}

char *
file_replace (char *word)
{
  static char filename[FILENAME_MAX + 1];
  time_t clock;
  struct tm *tm;
  int ret;

  if (! strchr (word, '%'))
    return word;

  time (&clock);
  tm = localtime (&clock);

  ret = strftime (filename, FILENAME_MAX, word, tm);
  if (ret == 0)
    return word;

  return filename;
}

int
line_spec (char *spec)
{
  return (! strcmp (spec, "<LINE>"));
}

int
word_spec (char *spec)
{
  return (! strcmp (spec, "<WORD>"));
}

int
var_spec (char *spec)
{
  return (! strcmp (spec, "<VAR>"));
}

char *
cmdstr_complete (char *word, struct command_node *match)
{
  return (&match->cmdstr[strlen (word)]);
}

struct command_node *
command_match_exact (struct command_node *parent, char *word)
{
  struct vector_node *vn;
  struct command_node *node, *match = NULL;
  for (vn = vector_head (parent->cmdvec); vn; vn = vector_next (vn))
    {
      node = (struct command_node *) vn->data;
      if (! strcmp (node->cmdstr, word))
        match = node;
    }
  return match;
}

int
is_command_match_variable (char *spec, char *word)
{
  if (ipv4_spec (spec))
    {
      if (ipv4_match (spec, word))
        return 1;
    }
  if (ipv4_prefix_spec (spec))
    {
      if (ipv4_prefix_match (spec, word))
        return 1;
    }
  if (range_spec (spec))
    {
      if (range_match (spec, word))
        return 1;
    }
  if (double_spec (spec))
    {
      if (double_match (spec, word))
        return 1;
    }
  if (file_spec (spec))
    {
      if (file_match (spec, word))
        return 1;
    }
  if (line_spec (spec))
    {
      return 1;
    }
  if (word_spec (spec))
    {
      return 1;
    }
  if (var_spec (spec))
    {
      return 1;
    }

  return 0;
}

int
is_command_match_regular (char *spec, char *word)
{
  int ret;

  ret = strncmp (spec, word, strlen (word));
  if (ret == 0)
    return 1;

  return 0;
}

int
is_command_match (char *spec, char *word)
{
  int ret;

  ret = is_command_match_variable (spec, word);
  if (ret)
    return 1;

  ret = is_command_match_regular (spec, word);
  if (ret)
    return 1;

  return 0;
}

int
varmatch_pri (char *spec)
{
  if (ipv4_prefix_spec (spec))
    return 6;
  if (ipv4_spec (spec))
    return 5;
  if (range_spec (spec))
    return 4;
  if (double_spec (spec))
    return 3;
  if (file_spec (spec))
    return 2;
  if (var_spec (spec))
    return 1;
  if (line_spec (spec))
    return 0;
  return -1;
}

int
is_command_node_variable (struct command_node *node)
{
  if (ipv4_spec (node->cmdstr) || range_spec (node->cmdstr) ||
      double_spec (node->cmdstr) || file_spec (node->cmdstr) ||
      line_spec (node->cmdstr) || var_spec (node->cmdstr))
    return 1;
  return 0;
}

struct command_node *
command_match_unique (struct command_node *parent, char *word)
{
  struct vector_node *vn;
  struct command_node *node, *match = NULL, *varmatch = NULL;
  int match_count = 0, varmatch_count = 0;
  for (vn = vector_head (parent->cmdvec); vn; vn = vector_next (vn))
    {
      node = (struct command_node *) vn->data;
      if (is_command_match_variable (node->cmdstr, word))
        {
          match = node;
          if (! varmatch ||
              varmatch_pri (node->cmdstr) > varmatch_pri (varmatch->cmdstr))
            varmatch = node;
          varmatch_count++;
          match_count++;
        }
      if (is_command_match_regular (node->cmdstr, word))
        {
          match = node;
          match_count++;
        }
    }

  if (! match)
    return NULL;

  /* else if all matches are variable matches, return the best var match */
  if (varmatch_count == match_count)
    return varmatch;

  if (match_count == 1)
    return match;

  return NULL;
}

struct command_node *
command_match_unique_exact (struct command_node *parent, char *word)
{
  struct command_node *match = NULL;

  match = command_match_unique (parent, word);
  if (match)
    return match;

  /* else multiple matches exist. return unique if exact match exists */
  match = command_match_exact (parent, word);

  return match;
}

void
command_install (struct command_set *cmdset, char *command_line,
                 char *help_string, command_func_t func)
{
  struct command_node *parent = cmdset->root;
  struct command_node *node = NULL;
  char *cmd_dup, *help_dup, *stringp, *hstringp;
  char *word, *word_help;

  cmd_dup = strdup (command_line);
  help_dup = strdup (help_string);
  stringp = cmd_dup;
  hstringp = help_dup;

  while ((word = strsep (&stringp, COMMAND_WORD_DELIMITERS)) != NULL)
    {
      word_help = strsep (&hstringp, COMMAND_HELP_DELIMITERS);
      node = command_match_exact (parent, word);
      if (node == NULL)
        {
          node = command_node_create ();
          node->cmdstr = word;
          node->helpstr = word_help;
          vector_add (node, parent->cmdvec);
          vector_sort (command_node_cmp_qsort, parent->cmdvec);
        }
      parent = node;
    }
  node->func = func;
  node->cmdmem = cmd_dup;
  node->helpmem = help_dup;
}

void
command_install2 (struct command_set *cmdset, char *command_line,
                  char *help_string, command_func_t func)
{
  struct command_node *node = NULL;
  char *cmd_dup, *help_dup, *stringp, *hstringp;
  char *word, *subword, *word_help = "\n";

  struct vector *parents, *next_parents;
  struct vector_node *vn;
  struct command_node *parent;

  if (! command_line && ! help_string && ! func)
    {
      fprintf (stderr, "%s: null commands. forgot init?\n", __func__);
      return;
    }

  parents = vector_create ();
  next_parents = vector_create ();

  if (FLAG_CHECK (DEBUG_CONFIG (ZCMDSH), DEBUG_TYPE (ZCMDSH, COMMAND)))
    {
      DEBUG_LOG_MSG ("%s: vector_create: %p\n", __func__, parents);
      DEBUG_LOG_MSG ("%s: vector_create: %p\n", __func__, next_parents);
    }

  vector_add (cmdset->root, parents);

  cmd_dup = strdup (command_line);
  help_dup = strdup (help_string);
  stringp = cmd_dup;
  hstringp = help_dup;

  while ((word = strsep (&stringp, COMMAND_WORD_DELIMITERS)) != NULL)
    {
      char *p;

      DEBUG_ZCMDSH_LOG (COMMAND, "%s: word: %s", __func__, word);

      /* everything before '(' is ignored. */
      p = index (word, '(');
      if (p)
        {
          *p++ = '\0';
          word = p;
        }

      /* everything after ')' is ignored. */
      p = index (word, ')');
      if (p)
        *p = '\0';

      /* for each subword: the word is separated by the '|' delimeter. */
      while ((subword = strsep (&word, "|")) != NULL)
        {
          // printf ("subword: %s\n", subword);
          if (strlen (subword))
            {
              word_help = strsep (&hstringp, COMMAND_HELP_DELIMITERS);
              if (FLAG_CHECK (debug_config, DEBUG_COMMAND))
                printf ("%s: subword: %s help: %s\n", __func__, subword,
                        word_help);
            }

          /* for each parent, add the subword node below it,
             and prepare the next parents */
          for (vn = vector_head (parents); vn; vn = vector_next (vn))
            {
              parent = (struct command_node *) vn->data;

              /* if no subword, just add the parent to the next_parents,
                 allowing omitting this level of subword. */
              if (! strlen (subword))
                {
                  vector_add (parent, next_parents);
                  if (FLAG_CHECK (debug_config, DEBUG_COMMAND))
                    printf ("%s: vector_add: node: %p to vector: %p\n",
                            __func__, (void *) parent, (void *) next_parents);
                  continue;
                }

              node = command_match_exact (parent, subword);
              if (node == NULL)
                {
                  node = command_node_create ();
                  node->cmdstr = subword;
                  node->helpstr = word_help;
                  vector_add (node, parent->cmdvec);
                  vector_sort (command_node_cmp_qsort, parent->cmdvec);
                }

              if (FLAG_CHECK (debug_config, DEBUG_COMMAND))
                printf ("%s: parent: %p (%s) -> child: %p (%s)\n", __func__,
                        (void *) parent, parent->cmdstr,
                        (void *) node, node->cmdstr);
              vector_add (node, next_parents);
              if (FLAG_CHECK (debug_config, DEBUG_COMMAND))
                printf ("%s: vector_add: node: %p to vector: %p\n", __func__,
                        (void *) node, (void *) next_parents);
            }
        }

      if (FLAG_CHECK (debug_config, DEBUG_COMMAND))
        printf ("%s: vector_delete: %p\n", __func__, (void *) parents);
      vector_delete (parents);
      parents = next_parents;
      next_parents = vector_create ();
      if (FLAG_CHECK (debug_config, DEBUG_COMMAND))
        printf ("%s: vector_create: %p\n", __func__, (void *) next_parents);
    }

  for (vn = vector_head (parents); vn; vn = vector_next (vn))
    {
      node = (struct command_node *) vn->data;
      node->func = func;
      if (FLAG_CHECK (debug_config, DEBUG_COMMAND))
        printf ("%s: node: %p (%s) add func %p\n", __func__, (void *) node,
                node->cmdstr, (void *) func);
    }

  /* attache the string memory to free in the last processed node. */
  if (node)
    {
  node->cmdmem = cmd_dup;
  node->helpmem = help_dup;
  if (FLAG_CHECK (debug_config, DEBUG_COMMAND))
    printf ("%s: save memory for free: node: %p\n", __func__, (void *) node);
    }

  if (FLAG_CHECK (debug_config, DEBUG_COMMAND))
    {
      printf ("%s: vector_delete: %p\n", __func__, (void *) parents);
      printf ("%s: vector_delete: %p\n", __func__, (void *) next_parents);
    }
  vector_delete (parents);
  vector_delete (next_parents);
}

char *
command_replace (struct command_node *cmd, char *word)
{
  char *ret = word;

  if (file_spec (cmd->cmdstr))
    ret = file_replace (word);
  else if (! is_command_match_variable (cmd->cmdstr, word))
    ret = cmd->cmdstr;

  return ret;
}

/*
  command_line_dup = strdup (shell->command_line);
  command_argv_parse (command_line_dup, &argc, &argv);
  command_matched_nodes (argc, argv, shell->cmdset, &cmdnodes);
  free (command_line_dup);
  free (argv);
  free (cmdnodes);
 */

#define PTR_ARRAY_MALLOC(size, type) \
    (type*) malloc (sizeof (type) * (size))
#define PTR_ARRAY_CLEAR(ptr, size, type) \
    memset (ptr, 0, sizeof (type) * (size))
#define PTR_ARRAY_REALLOC(ptr, size, type) \
    (type*) realloc (ptr, sizeof (type) * (size))

char *
shell_format3 (char *command_line_orig)
{
  char *command_line;
  int i, cursor, end;
  int count = 0;
  int ret;
  int shell_end;
  int shell_orig_end;

  end = 0;
  cursor = 0;
  command_line = strdup (command_line_orig);
  if (command_line == NULL)
    return 0;
  shell_end = strlen (command_line);
  shell_orig_end = strlen (command_line_orig);
  memset (command_line, 0, shell_end);

  /* filter out the duplicated consecutive spaces. */
  for (i = 0; i < shell_orig_end; i++)
    {
      if (command_line_orig[i] == ' ')
        count++;

      /* omit redundant spaces */
      if (command_line_orig[i] == ' ' && count > 1)
        continue;

      /* omit even first space if it is the beginning of the line */
      if (command_line_orig[i] == ' ' && i == 0)
        continue;

      command_line[end++] = command_line_orig[i];

      if (command_line_orig[i] != ' ')
        count = 0;
    }

  return command_line;
}

int
command_argv_parse (char *command_line_dup,
                     int *argc_ptr, char ***argv_ptr)
{
  char *stringp;
  char *word;

  int argc;
  int argsize;

  /* arrays to be created. */
  char **argv, **argv_new;

  stringp = shell_format3 (command_line_dup);

  argc = 0;
  argsize = 4;

  argv = PTR_ARRAY_MALLOC(argsize, char *);
  if (! argv)
    {
      DEBUG_ZCMDSH_LOG (COMMAND, "malloc failed.");
      return -1;
    }
  PTR_ARRAY_CLEAR(argv, argsize, char *);

  while ((word = strsep (&stringp, COMMAND_WORD_DELIMITERS)) != NULL)
    {
#if 0 /* "" word is allowed here. */
      /* prevent execution completing NULL word */
      if (*word == '\0')
        break;
#endif

      if (argc + 1 >= argsize)
        {
          argv_new = PTR_ARRAY_REALLOC (argv, argsize * 2, char *);
          if (argv_new)
            {
              DEBUG_ZCMDSH_LOG (COMMAND,
                                "expand argv array: %p: %d -> %p: %d.",
                                argv, argsize, argv_new, argsize * 2);
              argv = argv_new;
              argsize *= 2;
            }
          else
            {
              DEBUG_ZCMDSH_LOG (COMMAND, "expand realloc failed.");
              break;
            }
        }

      argv[argc++] = word;
    }

  int i, ret;
  char str_buf[1024];
  char *s, *e;
  s = str_buf;
  e = str_buf + sizeof (str_buf);
  for (i = 0; i < argc; i++)
    {
      ret = snprintf (s, e - s, " \"%s\"", argv[i]);
      s += ret;
    }
  DEBUG_ZCMDSH_LOG (COMMAND, "argc: %d argv: %s", argc, str_buf);

  *argc_ptr = argc;
  *argv_ptr = argv;

  return 0;
}

int
command_matched_nodes (int argc, char **argv, char *command_line,
                       struct command_set *cmdset,
                       struct command_node ***cmdnodes_ptr)
{
  struct command_node **cmdnodes;
  int i;
  struct command_node *parent, *match = NULL;
  char *word;

  cmdnodes = PTR_ARRAY_MALLOC (argc, struct command_node *);
  if (! cmdnodes)
    {
      DEBUG_ZCMDSH_LOG (COMMAND, "malloc failed.");
      return -1;
    }
  PTR_ARRAY_CLEAR (cmdnodes, argc, struct command_node *);

  parent = cmdset->root;

  for (i = 0; i < argc; i++)
    {
      word = argv[i];
      match = command_match_unique_exact (parent, word);
      if (match == NULL)
        break;
      cmdnodes[i] = match;

      /* update argv if necessary */
      if (line_spec (match->cmdstr))
        {
          argv[i] = strstr (command_line, word);
          break;
        }

      argv[i] = command_replace (match, word);

      parent = match;
    }

  *cmdnodes_ptr = cmdnodes;
  return 0;
}

int
command_execute (char *command_line, struct command_set *cmdset, void *context)
{
  char *cmd_dup;
  char *stringp;
  char *word;
  int argc;
  int argsize;
  char **argv, **new;
  struct command_node *parent, *match = NULL;
  int ret = 0;

  cmd_dup = strdup (command_line);
  stringp = cmd_dup;
  parent = cmdset->root;

  argc = 0;
  argsize = 4;
  argv = (char **) malloc (sizeof (char *) * argsize);
  memset (argv, 0, sizeof (char *) * argsize);

  while ((word = strsep (&stringp, COMMAND_WORD_DELIMITERS)) != NULL)
    {
      /* prevent execution completing NULL word */
      if (*word == '\0')
        break;

      match = command_match_unique_exact (parent, word);
      if (match == NULL)
        break;

      if (argc + 1 >= argsize)
        {
          new = (char **) realloc (argv, sizeof (char *) * argsize * 2);
          if (new == NULL)
            exit (-1);
          argv = new;
          argsize *= 2;
        }

      if (line_spec (match->cmdstr) || file_spec (match->cmdstr))
        {
          argv[argc++] = strstr (command_line, word);
          break;
        }

      argv[argc++] = command_replace (match, word);

      parent = match;
    }

  if (match && match->func)
    {
      if (FLAG_CHECK (debug_config, DEBUG_COMMAND))
        {
          printf ("%s[%d]: %s: argv[%d]:", __FILE__, __LINE__, __func__, argc);
          for (int i = 0; i < argc; i++)
            printf (" %s", argv[i]);
          printf ("\n");
        }

      ret = (*match->func) (context, argc, argv);
      if (ret < 0)
        {
          printf ("%s[%d]: %s: argv[%d]:", __FILE__, __LINE__, __func__, argc);
          for (int i = 0; i < argc; i++)
            printf (" %s", argv[i]);
          printf ("\n");
          printf ("func: %s returned %d\n", match->cmdstr, ret);
        }
    }
  else
    ret = CMD_NOT_FOUND;

  free (argv);
  free (cmd_dup);
  return ret;
}

char *
command_complete_common (char *word, struct command_node *parent)
{
  int nmatch = 0;
  struct vector_node *vn;
  struct command_node *node;
  static char common[64];

  memset (common, 0, sizeof (common));
  for (vn = vector_head (parent->cmdvec); vn; vn = vector_next (vn))
    {
      node = (struct command_node *) vn->data;
      if (is_command_node_variable (node))
        continue;
      if (! strncmp (node->cmdstr, word, strlen (word)))
        {
          if (nmatch == 0)
            strncpy (common, node->cmdstr, sizeof (common) - 1);
          else
            strcommon (common, node->cmdstr);
          nmatch++;
        }
    }
  return (&common[strlen (word)]);
}

char *
command_complete (char *command_line, int point, struct command_set *cmdset)
{
  char *cmd_dup;
  char *stringp;
  char *word, *matched;
  struct command_node *parent, *match = NULL;
  static char retbuf[64];

  memset (retbuf, 0, sizeof (retbuf));

  cmd_dup = shell_format3 (command_line);
  point = strlen (cmd_dup);
  cmd_dup[point] = '\0';
  stringp = cmd_dup;
  parent = cmdset->root;

  while ((word = strsep (&stringp, COMMAND_WORD_DELIMITERS)) != NULL)
    {
      match = command_match_unique_exact (parent, word);
      if (match == NULL)
        break;
      matched = word;

      parent = match;
    }

  if (match && file_spec (match->cmdstr))
    {
      char *completion;
      completion = file_complete (matched);
      if (completion)
        snprintf (retbuf, sizeof (retbuf), "%s", completion);
    }

  if (match && ! is_command_node_variable (match))
    snprintf (retbuf, sizeof (retbuf), "%s ",
              &match->cmdstr[strlen (matched)]);

  if (match == NULL && parent)
    snprintf (retbuf, sizeof (retbuf), "%s",
              command_complete_common (word, parent));

  free (cmd_dup);
  return retbuf;
}

struct command_node *
command_match_node (char *command_line, struct command_set *cmdset)
{
  char *cmd_dup;
  char *stringp;
  char *word;
  struct command_node *parent, *match = NULL;

  cmd_dup = strdup (command_line);
  stringp = cmd_dup;
  parent = cmdset->root;
  match = cmdset->root;

  while ((word = strsep (&stringp, COMMAND_WORD_DELIMITERS)) != NULL)
    {
      /* prevent execution completing NULL word */
      if (*word == '\0')
        break;

      match = command_match_unique_exact (parent, word);
      if (match == NULL)
        break;

      if (line_spec (match->cmdstr) || file_spec (match->cmdstr))
        break;

      parent = match;
    }

  free (cmd_dup);
  return match;
}

void
command_config_add (struct vector *config, int argc, char **argv)
{
  int i, left, ret;
  char *p, config_line[128];

  memset (config_line, 0, sizeof (config_line));
  p = config_line;
  left = sizeof (config_line);

  ret = snprintf (p, left, "  %s", argv[0]);
  p += ret;
  left -= ret;

  for (i = 1; i < argc; i++)
    {
      ret = snprintf (p, left, " %s", argv[i]);
      p += ret;
      left -= ret;
    }

  vector_add (strdup (config_line), config);
}

void
command_config_clear (struct vector *config)
{
  struct vector_node *vn;
  for (vn = vector_head (config); vn; vn = vector_next (vn))
    if (vn->data)
      free (vn->data);
  vector_clear (config);
}

void
command_config_write (struct vector *config, FILE *fp)
{
  struct vector_node *vn;
  for (vn = vector_head (config); vn; vn = vector_next (vn))
    if (vn->data)
      fprintf (fp, "%s\n", (char *) vn->data);
}


struct vector *command_func_name;

void
command_func_name_init ()
{
  command_func_name = vector_create ();
}

void
command_func_name_register (command_func_t command_func, char *command_name)
{
  struct command_func_name_entry *entry;
  entry = malloc (sizeof (struct command_func_name_entry));
  entry->command_func = command_func;
  entry->command_name = command_name;
  vector_add (entry, command_func_name);
}

char *
command_func_name_lookup (command_func_t command_func)
{
  struct vector_node *vn;
  struct command_func_name_entry *entry;
  char *match = NULL;
  for (vn = vector_head (command_func_name); vn; vn = vector_next (vn))
    {
      entry = (struct command_func_name_entry *) vn->data;
      if (entry->command_func == command_func)
        match = entry->command_name;
    }
  return match;
}

void
command_func_name_finish ()
{
  struct vector_node *vn;
  struct command_func_name_entry *entry;
  for (vn = vector_head (command_func_name); vn; vn = vector_next (vn))
    {
      entry = (struct command_func_name_entry *) vn->data;
      free (entry);
    }
  vector_delete (command_func_name);
}
