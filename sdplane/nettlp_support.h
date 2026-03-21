// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2023-2026 Yasuhiro Ohara

#ifndef __NETTLP_SUPPORT_H__
#define __NETTLP_SUPPORT_H__

#include <stdint.h>

/*
 * NetTLP specific header
 */
struct nettlp_hdr
{
  uint16_t seq;
  uint32_t tstamp;
} __attribute__ ((packed));

#endif /*__NETTLP_SUPPORT_H__*/
