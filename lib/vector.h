/*
 * Copyright (C) 2007  Yasuhiro Ohara
 * 
 * This file is part of SimRouting.
 * 
 * SimRouting is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * SimRouting is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _VECTOR_H_
#define _VECTOR_H_

typedef int (*vector_cmp_t) (const void *, const void *);

struct vector_node
{
  struct vector *vector;
  int index;
  void *data;
};

#define vector_data(x) ((x)->data)

struct vector
{
  void **array;
  unsigned int limit;
  unsigned int size;
};

void vector_sort (vector_cmp_t cmp, struct vector *v);
int vector_lookup_index_bsearch (void *data, vector_cmp_t cmp, struct vector *v);
int vector_lookup_index (void *data, struct vector *v);
void *vector_lookup_bsearch (void *data, vector_cmp_t cmp, struct vector *v);
void *vector_lookup (void *data, struct vector *v);

void vector_add (void *data, struct vector *v);
void vector_add_allow_dup (void *data, struct vector *v);
void vector_add_sort (void *data, vector_cmp_t cmp, struct vector *v);
void vector_remove (void *data, struct vector *v);
void vector_remove_index (int index, struct vector *v);
void vector_clear (struct vector *v);

void vector_set (struct vector *v, int index, void *data);
void *vector_get (struct vector *v, int index);

struct vector_node *vector_head (struct vector *vector);
struct vector_node *vector_next (struct vector_node *node);
void vector_break (struct vector_node *node);

struct vector *vector_create ();
void vector_delete (struct vector *v);

void vector_assert (struct vector *v);
void vector_debug (struct vector *v);

struct vector *vector_copy (struct vector *v);
int vector_is_same (struct vector *va, struct vector *vb);
struct vector *vector_cap (struct vector *va, struct vector *vb);

int vector_is_empty (struct vector *v);
int vector_empty_index (struct vector *v);

struct vector *vector_catenate (struct vector *va, struct vector *vb);
struct vector *vector_merge (struct vector *dst, struct vector *src);

#endif /*_VECTOR_H_*/

