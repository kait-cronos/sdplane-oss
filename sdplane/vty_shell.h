/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef __VTY_SHELL__
#define __VTY_SHELL__


int shell_keyfunc_clear_terminal (struct shell *shell);
EXTERN_COMMAND (clear_cmd);

void vty_shell (void *arg);

#endif /*__VTY_SHELL__*/
