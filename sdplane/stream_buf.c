
#include "include.h"
#include "stream_buf.h"

struct stream_buf *
stream_new (size_t size)
{
  struct stream_buf *s;
  s = calloc (1, sizeof (struct stream_buf));
  if (! s)
    {
      return NULL;
    }

  s->data = calloc (1, size);
  if (! s->data)
    {
      free (s);
      return NULL;
    }

  s->size = size;
  return s;
}

void
stream_free (struct stream_buf *s)
{
  free (s->data);
  free (s);
}

void
stream_get (void *dst, struct stream_buf *s, size_t size)
{
  size_t len = size;
  if (len > s->size - s->getp)
    len = s->size - s->getp;
  memcpy (dst, s->data + s->getp, len);
  s->getp += len;
}

uint8_t
stream_getc (struct stream_buf *s)
{
  uint8_t c = 0;
  if (s->size - s->getp >= 1)
    {
      c = s->data[s->getp++];
    }
  return c;
}

uint16_t
stream_getw (struct stream_buf *s)
{
  uint16_t w = 0;
  if (s->size - s->getp >= sizeof (uint16_t))
    {
      w = s->data[s->getp++] << 8;
      w |= s->data[s->getp++];
    }
  return w;
}

uint32_t
stream_getl (struct stream_buf *s)
{
  uint32_t l = 0;
  if (s->size - s->getp >= sizeof (uint32_t))
    {
      l = s->data[s->getp++] << 24;
      l |= s->data[s->getp++] << 16;
      l |= s->data[s->getp++] << 8;
      l |= s->data[s->getp++];
    }
  return l;
}

/* get data from a particular position. do not proceed the getp */

void
stream_get_at (unsigned long pos, void *dst, struct stream_buf *s, size_t size)
{
  size_t len = size;
  if (len > s->size - pos)
    len = s->size - pos;
  memcpy (dst, s->data + pos, len);
}

uint8_t
stream_getc_at (unsigned long pos, struct stream_buf *s)
{
  uint8_t c = 0;
  if (s->size - pos > 0)
    {
      c = s->data[pos];
    }
  return c;
}

uint16_t
stream_getw_at (unsigned long pos, struct stream_buf *s)
{
  uint16_t w = 0;
  if (s->size - pos >= sizeof (uint16_t))
    {
      w = s->data[pos] << 8;
      w |= s->data[pos + 1];
    }
  return w;
}

uint32_t
stream_getl_at (unsigned long pos, struct stream_buf *s)
{
  uint32_t l = 0;
  if (s->size - pos >= sizeof (uint32_t))
    {
      l = s->data[pos] << 24;
      l |= s->data[pos + 1] << 16;
      l |= s->data[pos + 2] << 8;
      l |= s->data[pos + 3];
    }
  return l;
}
