// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2023-2026 Yasuhiro Ohara

#ifndef __JHASH_H__
#define __JHASH_H__

/**
 * jhash.h -- Jenkins hash function.
 * reference: http://www.burtleburtle.net/bob/c/lookup3.c
 */

#include "include.h"

#define JHASH_SEED 0xdeadbeef

/* 32-bit left rotation */
#define jhash_rot(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

// clang-format off

#define jhash_mix(a,b,c) \
{ \
  a -= c; a ^= jhash_rot(c, 4); c += b; \
  b -= a; b ^= jhash_rot(a, 6); a += c; \
  c -= b; c ^= jhash_rot(b, 8); b += a; \
  a -= c; a ^= jhash_rot(c,16); c += b; \
  b -= a; b ^= jhash_rot(a,19); a += c; \
  c -= b; c ^= jhash_rot(b, 4); b += a; \
}

#define jhash_final(a,b,c) \
{ \
  c ^= b; c -= jhash_rot(b,14); \
  a ^= c; a -= jhash_rot(c,11); \
  b ^= a; b -= jhash_rot(a,25); \
  c ^= b; c -= jhash_rot(b,16); \
  a ^= c; a -= jhash_rot(c,4); \
  b ^= a; b -= jhash_rot(a,14); \
  c ^= b; c -= jhash_rot(b,24); \
}

// clang-format on

/**
 * @brief hash a variable-length key into a 32-bit value.
 *
 * @param key     : the key, an array of uint32_t values.
 * @param length  : the length of the key, in uint32_ts.
 * @param initval : the previous hash, or an arbitrary value.
 * @return a 32-bit value.
 */
static inline __attribute__ ((always_inline)) uint32_t
jhash (const int32_t *key, int length, uint32_t initval)
{
  uint32_t a, b, c;

  a = b = c = JHASH_SEED + ((uint32_t) length << 2) + initval;
  while (length > 3)
    {
      a += key[0];
      b += key[1];
      c += key[2];
      jhash_mix (a, b, c);
      length -= 3;
      key += 3;
    }

  /* handle the last 3 uint32_t's */
  switch (length) /* all the case statements fall through */
    {
    case 3:
      c += key[2];
    case 2:
      b += key[1];
    case 1:
      a += key[0];
      jhash_final (a, b, c);
    case 0: /* nothing left to add */
      break;
    }

  return c;
}

#endif /* __JHASH_H__ */
