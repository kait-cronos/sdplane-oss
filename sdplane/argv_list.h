// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2023-2026 Yasuhiro Ohara

#ifndef __ARGV_LIST_H__
#define __ARGV_LIST_H__

extern pthread_mutex_t argv_list_mutex;

#define ARGV_LIST_MAX      8
#define ARGV_LIST_ARGV_MAX 32
extern char *argv_list[ARGV_LIST_MAX][ARGV_LIST_ARGV_MAX];
extern int argv_list_argc[ARGV_LIST_MAX];

void argv_list_cmd_init (struct command_set *cmdset);

void argv_list_init ();

#endif /*__ARGV_LIST_H__*/
