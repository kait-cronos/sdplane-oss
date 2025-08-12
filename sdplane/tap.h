/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef __TAP_H__
#define __TAP_H__

int tap_open (char *ifname);
void tap_admin_up (char *ifname);

#endif /*__TAP_H__*/
