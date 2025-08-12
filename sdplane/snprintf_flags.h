/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef __SNPRINTF_FLAGS__
#define __SNPRINTF_FLAGS__

struct flag_name
{
  char *name;
  uint64_t val;
};

int snprintf_flags (char *buf, int size, uint64_t flags,
                    struct flag_name *flag_names, char *delim,
                    int flag_names_size);

#endif /*__SNPRINTF_FLAGS__*/
