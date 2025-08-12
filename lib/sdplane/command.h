/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef __COMMAND_H__
#define __COMMAND_H__

#define COMMAND_WORD_DELIMITERS " "
#define COMMAND_HELP_DELIMITERS "\n"

#define CMD_NONE      0
#define CMD_SUCCESS   1
#define CMD_FAILURE   2
#define CMD_NOT_FOUND 3

typedef int (*command_func_t) (void *context, int argc, char **argv);

struct command_node
{
  char *cmdstr;
  char *helpstr;
  command_func_t func;
  char *cmdmem;
  char *helpmem;

  struct vector *cmdvec;
};

struct command_set
{
  struct command_node *root;
};

struct command_header
{
  char *cmdstr;
  char *helpstr;
  command_func_t cmdfunc;
};

struct command_func_name_entry
{
  command_func_t command_func;
  char *command_name;
};

void command_func_name_init ();
void command_func_name_register (command_func_t command_func,
                                 char *command_name);
char *command_func_name_lookup (command_func_t command_func);
void command_func_name_finish ();

#define DEFINE_COMMAND(cmdname, cmdstr, helpstr)                              \
  int cmdname##_func (void *context, int argc, char **argv);                  \
  struct command_header cmdname##_cmd = { cmdstr, helpstr, cmdname##_func };  \
  int cmdname##_func (void *context, int argc, char **argv)

#define ALIAS_COMMAND(aliasname, cmdname, cmdstr, helpstr)                    \
  struct command_header aliasname##_cmd = { cmdstr, helpstr, cmdname##_func };

#define INSTALL_COMMAND(cmdset, cmdname)                                      \
  do                                                                          \
    {                                                                         \
      command_install (cmdset, cmdname##_cmd.cmdstr, cmdname##_cmd.helpstr,   \
                       cmdname##_cmd.cmdfunc);                                \
    }                                                                         \
  while (0)

#define INSTALL_COMMAND2(cmdset, cmdname)                                     \
  do                                                                          \
    {                                                                         \
      if (! cmdname##_cmd.cmdstr || ! cmdname##_cmd.helpstr ||                \
          ! cmdname##_cmd.cmdfunc)                                            \
        fprintf (stderr, "%s:%d: %s: null commands. forgot init?\n",          \
                 __FILE__, __LINE__, __func__);                               \
      else                                                                    \
        {                                                                     \
          command_func_name_register (cmdname##_cmd.cmdfunc, #cmdname);       \
          command_install2 (cmdset, cmdname##_cmd.cmdstr,                     \
                          cmdname##_cmd.helpstr, cmdname##_cmd.cmdfunc);      \
        }                                                                     \
    }                                                                         \
  while (0)

#define EXTERN_COMMAND(cmdname) extern struct command_header cmdname##_cmd

#if 0
#define INSTALL_COMMAND3(cmdset, cmdname, index)                              \
  do                                                                          \
    {                                                                         \
      command_install2 (cmdset, cmdname##_cmd[index].cmdstr,                  \
                        cmdname##_cmd[index].helpstr,                         \
                        cmdname##_cmd[index].cmdfunc);                        \
    }                                                                         \
  while (0)

#define EXTERN_COMMAND3(cmdname) extern struct command_header cmdname##_cmd[]
#endif /*0*/

struct command_set *command_set_create ();
void command_set_delete (struct command_set *cmdset);

int is_command_match (char *spec, char *word);

void command_install (struct command_set *cmdset, char *command_line,
                      char *help_string, command_func_t func);
void command_install2 (struct command_set *cmdset, char *command_line,
                       char *help_string, command_func_t func);

char *shell_format3 (char *command_line_orig);

int command_argv_parse (char *command_line_dup,
                     int *argc_ptr, char ***argv_ptr);
int command_matched_nodes (int argc, char **argv, char *command_line,
                       struct command_set *cmdset,
                       struct command_node ***cmdnodes_ptr);

int command_execute (char *command_line, struct command_set *cmdset,
                     void *context);

char *command_complete (char *command_line, int point,
                        struct command_set *cmdset);

struct command_node *command_match_node (char *command_line,
                                         struct command_set *cmdset);

int file_spec (char *spec);

void command_config_add (struct vector *config, int argc, char **argv);
void command_config_clear (struct vector *config);
void command_config_write (struct vector *config, FILE *fp);

#define NODE_SPEC          "<0-4294967295>"
#define NODE_SPEC_HELP_STR "specify node id.\n"

#define ROUTING_ALGORITHM_HELP_STR "calculate routing using algorithm.\n"

#define IMPORT_HELP "import information\n"
#define EXPORT_HELP "export information\n"

#define SHOW_HELP "show information.\n"
#define NO_HELP   "disable information.\n"

#define ZCMDSH_MKSTR_AUX(x) #x
#define ZCMDSH_MKSTR(x) ZCMDSH_MKSTR_AUX(x)

#define ZCMDSH_CONCAT_AUX(x,y) x##y
#define ZCMDSH_CONCAT(x,y) ZCMDSH_CONCAT_AUX(x,y)

#define ZCMDSH_LIST_AUX(x,y) x y
#define ZCMDSH_LIST(x,y) ZCMDSH_LIST_AUX(x,y)

/* The following part is to compute the number of args:
   e.g., number_arg[] = concat(arg[1,2,3,4], reverse[8,7,6,5,4,3,2,1]),
   then number_arg[8] == 4.
   below implementation is using 64 instead of 8, meaning
   we support up to 64 args. */
#define ZCMDSH_INDEX_REVERSE \
  63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, \
      44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, \
      26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9,  \
      8, 7, 6, 5, 4, 3, 2, 1, 0
#define ZCMDSH_INDEX_VARNAME() \
  __0, __1, __2, __3, __4, __5, __6, __7, __8, __9, _10,   \
  _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21,   \
  _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32,   \
  _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43,   \
  _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54,   \
  _55, _56, _57, _58, _59, _60, _61, _62, _63

#define ZCMDSH_NARG(...) \
  ZCMDSH_NARG_AUX (0, ##__VA_ARGS__, ZCMDSH_INDEX_REVERSE())
#define ZCMDSH_NARG_AUX(...) ZCMDSH_ARG_N (__VA_ARGS__)
#define ZCMDSH_ARG_N(\
  __0, __1, __2, __3, __4, __5, __6, __7, __8, __9, _10,   \
  _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21,   \
  _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32,   \
  _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43,   \
  _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54,   \
  _55, _56, _57, _58, _59, _60, _61, _62, _63, \
  NUM, ...) NUM

/* The following part is to change the listing by commas to
   the listing by spaces: e.g., (x, y) to (x y).
   it decreases by one arg for each substitution. */
#define LIST ZCMDSH_LIST
#define NCLI1(n, s, h1, ...) DEFINE_COMMAND (n, s, h1)
#define NCLI2(n, s, h1, h2, ...) NCLI1 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI3(n, s, h1, h2, ...) NCLI2 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI4(n, s, h1, h2, ...) NCLI3 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI5(n, s, h1, h2, ...) NCLI4 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI6(n, s, h1, h2, ...) NCLI5 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI7(n, s, h1, h2, ...) NCLI6 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI8(n, s, h1, h2, ...) NCLI7 (n, s, LIST (h1, h2), __VA_ARGS__)

#define NCLI9(n, s, h1, h2, ...) NCLI8 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI10(n, s, h1, h2, ...) NCLI9 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI11(n, s, h1, h2, ...) NCLI10 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI12(n, s, h1, h2, ...) NCLI11 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI13(n, s, h1, h2, ...) NCLI12 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI14(n, s, h1, h2, ...) NCLI13 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI15(n, s, h1, h2, ...) NCLI14 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI16(n, s, h1, h2, ...) NCLI15 (n, s, LIST (h1, h2), __VA_ARGS__)

#define NCLI17(n, s, h1, h2, ...) NCLI16 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI18(n, s, h1, h2, ...) NCLI17 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI19(n, s, h1, h2, ...) NCLI18 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI20(n, s, h1, h2, ...) NCLI19 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI21(n, s, h1, h2, ...) NCLI20 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI22(n, s, h1, h2, ...) NCLI21 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI23(n, s, h1, h2, ...) NCLI22 (n, s, LIST (h1, h2), __VA_ARGS__)
#define NCLI24(n, s, h1, h2, ...) NCLI23 (n, s, LIST (h1, h2), __VA_ARGS__)

/* CLI_COMMAND2() computes the number of args, 'X',
   and calls NCLI'X' */
#define ZCMDSH_VARARG_COMMAND(...)  \
  ZCMDSH_CONCAT(NCLI, ZCMDSH_NARG(__VA_ARGS__))
#define CLI_COMMAND2(cmdname, cmdstr, ...)  \
  ZCMDSH_VARARG_COMMAND (__VA_ARGS__) (cmdname, cmdstr, __VA_ARGS__)

#endif /*__COMMAND_H__*/
