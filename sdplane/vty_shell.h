// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2023-2026 Yasuhiro Ohara

#ifndef __VTY_SHELL__
#define __VTY_SHELL__

#include <sdplane/command.h>
#include <sdplane/shell.h>

int shell_keyfunc_clear_terminal (struct shell *shell);
EXTERN_COMMAND (clear_cmd);

void vty_shell (void *arg);

#endif /*__VTY_SHELL__*/
