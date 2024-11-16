/*
 * Copyright (C) 2007-2023 Yasuhiro Ohara. All rights reserved.
 */

#ifndef __SHELL_H__
#define __SHELL_H__

#include <sys/types.h>
#include <unistd.h>

#include <stdbool.h>
#include <sys/ioctl.h>

#include <zcmdsh/flag.h>

#define SHELL_WORD_DELIMITERS     " "
#define SHELL_WORD_DELIMITERS_SUB " /"

#define CONTROL(X) ((X) - '@')

struct shell;
typedef void (*shell_keyfunc_t) (struct shell *shell);

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

void shell_terminate (struct shell *shell);
void shell_format (struct shell *shell);
void shell_linefeed (struct shell *shell);
void shell_clear (struct shell *shell);
void shell_set_prompt (struct shell *shell, char *prompt);
void shell_prompt (struct shell *shell);
void shell_refresh (struct shell *shell);

void shell_insert (struct shell *shell, char *s);
void shell_input_char (struct shell *shell);
void shell_insert_char (struct shell *shell, char ch);
void shell_delete_string (struct shell *shell, int start, int end);
void shell_cut (struct shell *shell, int start, int end);

void shell_forward (struct shell *shell, int num);
void shell_backward (struct shell *shell, int num);

void shell_moveto (struct shell *shell, int index);
int shell_word_head (struct shell *shell, int point);
int shell_word_end (struct shell *shell, int point);

int shell_subword_head (struct shell *shell, int point);
void shell_delete_word_backward (struct shell *shell);

void shell_move_word_backward (struct shell *shell);
void shell_move_word_forward (struct shell *shell);

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
