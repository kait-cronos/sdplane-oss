/*
 * Copyright (C) 2007-2023 Yasuhiro Ohara. All rights reserved.
 */

#include <includes.h>

#include "vector.h"

/* the initial size of the array */
#define VECTOR_DEFSIZ 1

/* the number of retry allowed to do vector_expand () in one
   add/set manipulation. you can set a node to a empty vector
   whose index is limited to less than 2^{VECTOR_RETRY}.
   otherwise the addition/set is given up. */
#define VECTOR_RETRY 16

/* sample compare function which compares memory address */
int
vector_compare (const void *a, const void *b)
{
  if (a == b)
    return 0;
  if (a == NULL)
    return 1;
  if (b == NULL)
    return -1;
  return (a < b ? -1 : 1);
}

void
vector_sort (vector_cmp_t cmp, struct vector *v)
{
  int retval;

  assert (cmp);
  retval = 0;

  /* sort */
#ifdef HAVE_HEAPSORT
  retval = heapsort (v->array, v->size, sizeof (void *), cmp);
  if (retval != 0)
    fprintf (stderr, "heapsort failed: %s\n", strerror (errno));
#else  /*HAVE_HEAPSORT*/
  qsort (v->array, v->size, sizeof (void *), cmp);
#endif /*HAVE_HEAPSORT*/
}

/* returns index for data using binary search (recursive function) */
static int
vector_binsearch (void *data, int index, int size, vector_cmp_t cmp,
                  struct vector *v)
{
  int middle = index + size / 2;
  int shift = (size % 2 == 1 ? 1 : 0);
  int ret;

  if (size < 0)
    assert (0);

  ret = (*cmp) (&data, &v->array[middle]);
  if (ret == 0)
    return middle;
  else if (size == 0 || size == 1)
    return -1; /* search failed */
  else if (ret < 0)
    return vector_binsearch (data, index, size / 2, cmp, v);
  else
    return vector_binsearch (data, middle + shift, size / 2, cmp, v);

  /* not reached */
  return -1;
}

int
vector_lookup_index_bsearch (void *data, vector_cmp_t cmp, struct vector *v)
{
  int index;
  if (v->size == 0)
    return -1;
  index = vector_binsearch (data, 0, v->size, cmp, v);
  if (index < 0)
    return -1;
  if (cmp (&data, &v->array[index]) == 0)
    return index;
  return -1;
}

int
vector_lookup_index (void *data, struct vector *v)
{
  int index;

  if (v->size == 0)
    return -1;

  for (index = 0; index < v->size; index++)
    if (v->array[index] == data)
      return index;

  return -1;
}

void *
vector_lookup_bsearch (void *data, vector_cmp_t cmp, struct vector *v)
{
  int index;
  index = vector_lookup_index_bsearch (data, cmp, v);
  if (index < 0)
    return NULL;
  return v->array[index];
}

void *
vector_lookup (void *data, struct vector *v)
{
  int index;
  index = vector_lookup_index (data, v);
  if (index < 0)
    return NULL;
  return v->array[index];
}

static void
vector_expand (struct vector *v)
{
  void *newarray;

  newarray = (void **) realloc (v->array, v->limit * 2 * sizeof (void *));
  if (newarray == NULL)
    return;
  memset ((caddr_t) newarray + v->limit * sizeof (void *), 0,
          v->limit * sizeof (void *));

  v->array = newarray;
  v->limit *= 2;
}

void
vector_add (void *data, struct vector *v)
{
  int retry = VECTOR_RETRY;

  if (vector_lookup (data, v))
    {
      fprintf (stderr, "Can't add to vector: data %p already exists\n", data);
      assert (0);
      return;
    }

  if (v->size == v->limit)
    {
      while (retry-- && v->size == v->limit)
        vector_expand (v);
      if (v->size == v->limit)
        {
          fprintf (stderr, "Cannot double vector size from %d\n", v->limit);
          fprintf (stderr, "Give up to add data %p\n", data);
          return;
        }
    }

  /* add */
  v->array[v->size++] = data;
}

void
vector_add_allow_dup (void *data, struct vector *v)
{
  int retry = VECTOR_RETRY;

  if (v->size == v->limit)
    {
      while (retry-- && v->size == v->limit)
        vector_expand (v);
      if (v->size == v->limit)
        {
          fprintf (stderr, "Cannot double vector size from %d\n", v->limit);
          fprintf (stderr, "Give up to add data %p\n", data);
          return;
        }
    }

  /* add */
  v->array[v->size++] = data;
}

void
vector_add_sort (void *data, vector_cmp_t cmp, struct vector *v)
{
  vector_add (data, v);
  vector_sort (cmp, v);
}

void
vector_remove (void *data, struct vector *v)
{
  int index;

  index = vector_lookup_index (data, v);
  if (index < 0)
    {
      fprintf (stderr, "Can't remove from vector: no such data: %p\n", data);
      assert (0);
      return;
    }

  /* remove */
  v->array[index] = NULL;
  v->size--;

  /* shift */
  if (index + 1 < v->limit)
    memmove (&v->array[index], &v->array[index + 1],
             (v->limit - index - 1) * sizeof (void *));
}

void
vector_remove_index (int index, struct vector *v)
{
  assert (index >= 0);

  /* remove */
  v->array[index] = NULL;
  v->size--;

  /* shift */
  if (index + 1 < v->limit)
    memmove (&v->array[index], &v->array[index + 1],
             (v->limit - index - 1) * sizeof (void *));
}

void
vector_clear (struct vector *v)
{
  memset (v->array, 0, v->limit * sizeof (void *));
  v->size = 0;
}

/* You will not want to sort when you use below functions */
void
vector_set (struct vector *v, int index, void *data)
{
  int retry = VECTOR_RETRY;

  if (v->limit <= index)
    {
      while (retry-- && v->limit <= index)
        vector_expand (v);
      if (v->limit <= index)
        {
          fprintf (stderr, "Cannot double vector size from %d\n", v->limit);
          fprintf (stderr, "Give up to set data %p at index %d\n", data,
                   index);
          return;
        }
    }

  /* add */
  v->array[index] = data;
  if (v->size <= index)
    v->size = index + 1;
}

void *
vector_get (struct vector *v, int index)
{
  if (v->size <= index)
    return NULL;
  return v->array[index];
}

struct vector_node *
vector_head (struct vector *vector)
{
  struct vector_node *node;

  if (vector->size == 0)
    return NULL;

  node = (struct vector_node *) malloc (sizeof (struct vector_node));
  if (node == NULL)
    return NULL;
  memset (node, 0, sizeof (struct vector_node));

  node->vector = vector;
  node->index = 0;
  node->data = node->vector->array[node->index];

  return node;
}

struct vector_node *
vector_next (struct vector_node *node)
{
  /* vector might be deleted. in the case, reload data only */
  if (node->data == node->vector->array[node->index] &&
      node->index < node->vector->size)
    node->index++;

  /* if index reaches end, return NULL */
  if (node->index == node->vector->size)
    {
      free (node);
      return NULL;
    }

  node->data = node->vector->array[node->index];

  return node;
}

/* for break */
void
vector_break (struct vector_node *node)
{
  free (node);
}

struct vector *
vector_create ()
{
  struct vector *v;

  v = (struct vector *) malloc (sizeof (struct vector));
  if (v == NULL)
    return NULL;
  memset (v, 0, sizeof (struct vector));
  v->size = 0;
  v->limit = VECTOR_DEFSIZ;
  v->array = (void **) malloc (v->limit * sizeof (void *));
  if (v->array == NULL)
    {
      free (v);
      return NULL;
    }
  memset (v->array, 0, v->limit * sizeof (void *));

#if 0
  v->cmp = vector_compare;
#endif

  return v;
}

void
vector_delete (struct vector *v)
{
  free (v->array);
  free (v);
}

void
vector_assert (struct vector *v)
{
  int index;

  for (index = 0; index < v->size; index++)
    assert (v->array[index]);

  for (index = v->size; index < v->limit; index++)
    assert (v->array[index] == NULL);
}

void
vector_debug (struct vector *v)
{
  struct vector_node *node;

  fprintf (stderr, "vector: debug: size = %d limit = %d\n", v->size, v->limit);
  fprintf (stderr, "vector: debug: vector = %p array = %p\n", v, v->array);
  for (node = vector_head (v); node; node = vector_next (node))
    fprintf (stderr, "data: %p index: %d\n", node->data, node->index);

  vector_assert (v);
}

struct vector *
vector_copy (struct vector *v)
{
  struct vector *vector;
  int index;
  vector = vector_create ();
  for (index = 0; index < v->size; index++)
    vector_set (vector, index, vector_get (v, index));
  return vector;
}

int
vector_is_same (struct vector *va, struct vector *vb)
{
  int i;
  if (va->size != vb->size)
    return 0;

  for (i = 0; i < va->size; i++)
    if (va->array[i] != vb->array[i])
      return 0;

  return 1;
}

struct vector *
vector_cap (struct vector *va, struct vector *vb)
{
  struct vector *cap = vector_create ();
  struct vector_node *vna, *vnb;
  for (vna = vector_head (va); vna; vna = vector_next (vna))
    for (vnb = vector_head (vb); vnb; vnb = vector_next (vnb))
      if (vna->data && vna->data == vnb->data)
        vector_add (vna->data, cap);
  return cap;
}

int
vector_is_empty (struct vector *v)
{
  if (v->size == 0)
    return 1;
  return 0;
}

int
vector_empty_index (struct vector *v)
{
  int index;
  for (index = 0; index < v->size; index++)
    if (v->array[index] == NULL)
      return index;
  return index;
}

struct vector *
vector_catenate (struct vector *va, struct vector *vb)
{
  struct vector_node *vn;
  for (vn = vector_head (vb); vn; vn = vector_next (vn))
    vector_add (vn->data, va);
  return va;
}

struct vector *
vector_merge (struct vector *dst, struct vector *src)
{
  struct vector_node *vn;
  for (vn = vector_head (src); vn; vn = vector_next (vn))
    if (! vector_lookup (vn->data, dst))
      vector_add (vn->data, dst);
  return dst;
}
