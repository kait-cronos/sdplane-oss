/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#include "include.h"

#include "nettlp_support.h"

int
tlp_calculate_lstdw (uintptr_t addr, size_t count)
{
  uintptr_t end, end_start, start;

  start = (addr >> 2) << 2;
  end = addr + count;
  if ((end & 0x3) == 0)
    end_start = end - 4;
  else
    end_start = (end >> 2) << 2;

  /* corner case. count is smaller than 8 */
  if (end_start <= start)
    end_start = addr + 4;
  if (end < end_start)
    return 0;

  return ~(0xF << (end - end_start)) & 0xF;
}

int
tlp_calculate_fstdw (uintptr_t addr, size_t count)
{
  uint8_t be = 0xF;

  if (count < 4)
    be = ~(0xF << count) & 0xF;

  return (be << (addr & 0x3)) & 0xF;
}

int
tlp_calculate_length (uintptr_t addr, size_t count)
{
  size_t len = 0;
  uintptr_t start, end;

  start = addr & 0xFFFFFFFFFFFFFFFc;
  end = addr + count;

  len = (end - start) >> 2;

  if ((end - start) & 0x3)
    len++;

  return len;
}

uintptr_t
tlp_mr_addr (struct tlp_mr_hdr *mh)
{
  int n;
  uintptr_t addr;
  uint32_t *addr32;
  uint64_t *addr64;

  if (tlp_is_3dw (mh->tlp.fmt_type))
    {
      addr32 = (uint32_t *) (mh + 1);
      addr = be32toh (*addr32);
    }
  else
    {
      addr64 = (uint64_t *) (mh + 1);
      addr = be64toh (*addr64);
    }

  /* move forard the address in accordance with the 1st DW BE */
  if (mh->fstdw && mh->fstdw != 0xF)
    {
      for (n = 0; n < 4; n++)
        {
          if ((mh->fstdw & (0x1 << n)) == 0)
            {
              addr += 1;
            }
          else
            break;
        }
    }

  return addr;
}

int
tlp_mr_data_length (struct tlp_mr_hdr *mh)
{
  int n;
  uint32_t len;

  len = tlp_length (mh->tlp.falen) << 2;

  if (mh->fstdw && mh->fstdw != 0xF)
    {
      for (n = 0; n < 4; n++)
        {
          if ((mh->fstdw & (0x1 << n)) == 0)
            {
              len--;
            }
        }
    }

  if (mh->lstdw && mh->lstdw != 0xF)
    {
      for (n = 0; n < 4; n++)
        {
          if ((mh->lstdw & (0x8 >> n)) == 0)
            {
              len--;
            }
          else
            break;
        }
    }

  return len;
}

void *
tlp_mwr_data (struct tlp_mr_hdr *mh)
{
  int n;
  void *p;

  p = tlp_is_3dw (mh->tlp.fmt_type) ? ((char *) (mh + 1)) + 4
                                    : ((char *) (mh + 1)) + 8;

  if (mh->fstdw && mh->fstdw != 0xF)
    {
      for (n = 0; n < 4; n++)
        {
          if ((mh->fstdw & (0x1 << n)) == 0)
            {
              p++;
            }
          else
            break;
        }
    }

  return p;
}

int
tlp_cpld_data_length (struct tlp_cpl_hdr *ch)
{
  /* if this is last CplD, byte count is actual byte length */
  if (tlp_length (ch->tlp.falen) ==
      ((ch->lowaddr & 0x3) + tlp_cpl_bcnt (ch->stcnt) + 3) >> 2)
    return tlp_cpl_bcnt (ch->stcnt);

  /* if not, length - padding due to 4DW alignment */
  return (tlp_length (ch->tlp.falen) << 2) - (ch->lowaddr & 0x3);
}

void *
tlp_cpld_data (struct tlp_cpl_hdr *ch)
{
  void *p;

  p = tlp_is_3dw (ch->tlp.fmt_type) ? ((char *) (ch + 1)) + 4
                                    : ((char *) (ch + 1)) + 8;

  /* shift for padding due to 4DW alignment */
  p += (ch->lowaddr & 0x3);
  return p;
}
