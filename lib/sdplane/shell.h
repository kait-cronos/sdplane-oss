/*
 * Copyright (C) 2007-2023 Yasuhiro Ohara. All rights reserved.
 */

#ifndef __SHELL_H__
#define __SHELL_H__

#include <sys/types.h>
#include <unistd.h>

#include <stdbool.h>
#include <sys/ioctl.h>

#include <sdplane/flag.h>

#define SHELL_WORD_DELIMITERS     " "
#define SHELL_WORD_DELIMITERS_SUB " /"

#define CONTROL(X) ((X) - '@')

struct shell;
typedef int (*shell_keyfunc_t) (struct shell *shell);

struct shell
{
  int readfd;
  int writefd;
  FILE *terminal;

  uint64_t flag;

  /* new line character */
  char *NL;

  char *prompt;
  int prompt_size;

  char *command_line;
  int cursor; /* cursor index, offset in command_line[] */
  int end;    /* end-of-string index, offset in command_line[] */
  int size;   /* buffer size of command_line[] */
  char *cut_buffer;

  char inputch;
  shell_keyfunc_t *keymap;
  shell_keyfunc_t keymap_normal[256];

  int cmd_status; /* command.h: CMD_SUCCESS, CMD_NOT_FOUND, etc. */

  void *cmdset;
  void *history;

  uint64_t debug_zcmdsh;

  struct winsize winsize;

  unsigned char telnet_cmd;
  unsigned char telnet_opt;
  int subnego_size;
  char subnego_buf[256];

  bool pager;
  bool is_paging;
  char *pager_command;
  int pager_saved_readfd;
  int pager_saved_writefd;
  FILE *pager_saved_terminal;
  pid_t pager_pid;
  int pipefd[2];
  int pty_master;
  int pty_slave;
};

#define SHELL_FLAG_ESCAPE      0x01
#define SHELL_FLAG_EXIT        0x02
#define SHELL_FLAG_CLOSE       0x04
#define SHELL_FLAG_DEBUG       0x08
#define SHELL_FLAG_INTERACTIVE 0x10

#define FUNC_TABLE_SIZE 512
struct funcp_str_map
{
  shell_keyfunc_t ptr;
  char *str;
};
extern struct funcp_str_map func2str[];
#define FUNC_STR_REGISTER(x)                                                  \
  do                                                                          \
    {                                                                         \
      int i;                                                                  \
      for (i = 0; i < FUNC_TABLE_SIZE; i++)                                   \
        {                                                                     \
          if (func2str[i].ptr == (void *) x)                                  \
            {                                                                 \
              func2str[i].str = #x;                                           \
              break;                                                          \
            }                                                                 \
          if (! func2str[i].ptr)                                              \
            {                                                                 \
              func2str[i].ptr = (void *) x;                                   \
              func2str[i].str = #x;                                           \
              break;                                                          \
            }                                                                 \
        }                                                                     \
    }                                                                         \
  while (0)

int func_table_lookup (shell_keyfunc_t ptr);

int shell_terminate (struct shell *shell);
int shell_format (struct shell *shell);
int shell_format2 (struct shell *shell);

int shell_linefeed (struct shell *shell);
int shell_clear (struct shell *shell);
void shell_set_prompt (struct shell *shell, char *prompt);
void shell_prompt (struct shell *shell);
int shell_refresh (struct shell *shell);

void shell_insert (struct shell *shell, char *s);
int shell_input_char (struct shell *shell);
void shell_insert_char (struct shell *shell, char ch);
void shell_delete_string (struct shell *shell, int start, int end);
void shell_cut (struct shell *shell, int start, int end);

void shell_forward (struct shell *shell, int num);
void shell_backward (struct shell *shell, int num);

void shell_moveto (struct shell *shell, int index);
char *shell_word_dup (struct shell *shell, int point);
int shell_word_head (struct shell *shell, int point);
int shell_word_end (struct shell *shell, int point);

int shell_word_prev_head (struct shell *shell, int point);
int shell_subword_prev_head (struct shell *shell, int point);

int shell_subword_head (struct shell *shell, int point);
int shell_delete_word_backward (struct shell *shell);

int shell_move_word_backward (struct shell *shell);
int shell_move_word_forward (struct shell *shell);

void shell_close (struct shell *shell);
int shell_read (struct shell *shell);
int shell_read_nowait (struct shell *shell);

struct shell *shell_create ();
void shell_delete (struct shell *shell);

void shell_set_terminal (struct shell *shell, int readfd, int writefd);
void shell_install (struct shell *shell, unsigned char key,
                    shell_keyfunc_t func);

int shell_running (struct shell *shell);

#endif /*__SHELL_H__*/
