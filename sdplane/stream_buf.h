/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef __STREAM_BUF_H__
#define __STREAM_BUF_H__

struct stream_buf
{
  uint8_t *data;
  unsigned long size;

  unsigned long putp; // put position.
  unsigned long getp; // get position.
  unsigned long endp; // end posision.
};

void stream_get (void *dst, struct stream_buf *s, size_t size);
uint8_t stream_getc (struct stream_buf *s);
uint16_t stream_getw (struct stream_buf *s);
uint32_t stream_getl (struct stream_buf *s);

void stream_get_at (unsigned long pos, void *dst, struct stream_buf *s,
                    size_t size);
uint8_t stream_getc_at (unsigned long pos, struct stream_buf *s);
uint16_t stream_getw_at (unsigned long pos, struct stream_buf *s);
uint32_t stream_getl_at (unsigned long pos, struct stream_buf *s);

#endif /*__STREAM_BUF_H__*/
