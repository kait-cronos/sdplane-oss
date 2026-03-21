// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2023-2026 Yasuhiro Ohara

#ifndef __SNPRINTF_FLAGS__
#define __SNPRINTF_FLAGS__

#include <stdint.h>

struct flag_name
{
  char *name;
  uint64_t val;
};

int snprintf_flags (char *buf, int size, uint64_t flags,
                    struct flag_name *flag_names, char *delim,
                    int flag_names_size);

#endif /*__SNPRINTF_FLAGS__*/
