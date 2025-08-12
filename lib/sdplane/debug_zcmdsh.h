/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef __DEBUG_ZCMDSH_H__
#define __DEBUG_ZCMDSH_H__

/* types in zcmdsh category */
#define DEBUG_ZCMDSH_SHELL   (1ULL << 0)
#define DEBUG_ZCMDSH_COMMAND (1ULL << 1)
#define DEBUG_ZCMDSH_PAGER   (1ULL << 2)
#define DEBUG_ZCMDSH_TIMER   (1ULL << 3)
#define DEBUG_ZCMDSH_UNICODE (1ULL << 4)
#define DEBUG_ZCMDSH_TERMIO  (1ULL << 5)
#define DEBUG_ZCMDSH_TELNET  (1ULL << 6)
#define DEBUG_ZCMDSH_COMMAND_SHELL   (1ULL << 7)
#define DEBUG_ZCMDSH_COMMAND_LOG   (1ULL << 8)
#define DEBUG_ZCMDSH_PAGER_CONTENTS   (1ULL << 9)

#define DEBUG_ZCMDSH_LOG(type, format, ...)                                   \
  DEBUG_LOG (ZCMDSH, type, format, ##__VA_ARGS__)

#if 0
#define DEBUG_ZCMDSH(type, format, ...) \
  DEBUG_LOG(ZCMDSH, type, format, ##__VA_ARGS__)
#endif

#include <sdplane/command.h>

EXTERN_COMMAND (debug_zcmdsh);
EXTERN_COMMAND (show_debug_zcmdsh);
void debug_zcmdsh_cmd_init ();

#endif /*__DEBUG_ZCMDSH_H__*/
