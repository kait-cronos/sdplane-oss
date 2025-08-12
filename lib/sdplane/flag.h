/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef __FLAG_H__
#define __FLAG_H__

#ifndef FLAG_SET
#define FLAG_CHECK(V, F) ((V) & (F))
#define FLAG_SET(V, F)   ((V) |= (F))
#define FLAG_UNSET(V, F) ((V) &= ~(F))
#define FLAG_CLEAR(V, F) ((V) &= ~(F))
#define FLAG_RESET(V)    ((V) = 0)
#define FLAG_ZERO(V)     ((V) = 0)
#endif /*FLAG_SET*/

#endif /*__FLAG_H__*/
