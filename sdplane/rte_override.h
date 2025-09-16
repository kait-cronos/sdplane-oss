/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef __RTE_OVERRIDE_H__
#define __RTE_OVERRIDE_H__

#include <rte_version.h>

/* override rte_exit() so that the whole process is not broken. */
#define rte_exit(x, ...)                                                      \
  do                                                                          \
    {                                                                         \
      printf (__VA_ARGS__);                                                   \
    }                                                                         \
  while (0)
#define rte_warn(x, ...)                                                      \
  do                                                                          \
    {                                                                         \
      printf (__VA_ARGS__);                                                   \
    }                                                                         \
  while (0)

#if RTE_VERSION < RTE_VERSION_NUM(25,0,0,0)
  #define __rte_packed_begin
  #define __rte_packed_end __rte_packed
#endif

#endif /*__RTE_OVERRIDE_H__*/
