/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef __L3_TAP_HANDLER_H__
#define __L3_TAP_HANDLER_H__

#include <sdplane/debug.h>
#include <sdplane/command.h>

#include "sdplane.h"
#include "rib_manager.h"

extern int capture_fd;
extern char capture_ifname[64];
extern int capture_if_persistent;

int l3_tap_handler (__rte_unused void *dummy);

#endif /*__L3_TAP_HANDLER_H__*/
