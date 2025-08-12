/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

extern shell_keyfunc_t default_keymap[];

int shell_keyfunc_forward_char (struct shell *shell);
int shell_keyfunc_backward_char (struct shell *shell);
int shell_keyfunc_move_to_begin (struct shell *shell);
int shell_keyfunc_move_to_end (struct shell *shell);
int shell_keyfunc_delete_char (struct shell *shell);
int shell_keyfunc_backspace (struct shell *shell);
int shell_keyfunc_kill_line (struct shell *shell);
int shell_keyfunc_kill_all (struct shell *shell);
int shell_keyfunc_yank (struct shell *shell);
int shell_keyfunc_clear_screen (struct shell *shell);
int shell_keyfunc_refresh (struct shell *shell);
int shell_keyfunc_empty_enter (struct shell *shell);
int shell_keyfunc_insert_tab (struct shell *shell);
int shell_keyfunc_escape (struct shell *shell);
int shell_keyfunc_delete_word_backward (struct shell *shell);
int shell_keyfunc_delete_char_advanced (struct shell *shell);

void shell_escape_keyfunc_init (struct shell *shell);
