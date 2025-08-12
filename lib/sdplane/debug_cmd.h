/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef __DEBUG_CMD_H__
#define __DEBUG_CMD_H__

void debug_cmdstr_init (char *cate, char *cmdstr, int size,
                        struct debug_type *debug_types, int types_size);
void debug_helpstr_init (char *cate, char *helpstr, int size,
                         struct debug_type *debug_types, int types_size);

#endif /*__DEBUG_CMD_H__*/
