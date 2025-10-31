/*
 * Copyright (C) 2025.  Yasuhiro Ohara <yasu1976@gmail.com>
 */

#ifndef _PTREE_H_
#define _PTREE_H_

struct ptree_node {
  char *key;
  int   keylen;
  struct ptree_node *parent;
  struct ptree_node *child[2];
  void *data;
};

#define PTREE_KEY_SIZE(len) (((len) + 7) / 8)

#if 0
#define PTREE_LEFT(x) (&(x)->child[0])
#define PTREE_RIGHT(x) (&(x)->child[1])
#endif /*0*/

struct ptree {
  struct ptree_node *top;
};

#define XRTMALLOC(p, t, n) (p = (t) malloc ((unsigned int)(n)))
#define XRTFREE(p) free((char *)p)
#define XRTLOG(pri, fmt, ...) printf (fmt, __VA_ARGS__)
#define XRTASSERT(exp, string) assert (exp)

#ifndef MIN
#define MIN(x, y) ((x) > (y) ? (y) : (x))
#endif /*MIN*/

void ptree_node_print (struct ptree_node *x);

int check_bit (char *key, int keylen);
int ptree_match (char *keyi, char *keyj, int keylen);
struct ptree_node *ptree_lookup (char *key, int keylen, struct ptree *t);

struct ptree_node *ptree_search (char *key, int keylen, struct ptree *t);
struct ptree_node *ptree_search_exact (char *key, int keylen, struct ptree *t);

struct ptree_node *ptree_add (char *key, int keylen, void *data, struct ptree *t);
void ptree_remove (struct ptree_node *v);

struct ptree_node *ptree_head (struct ptree *t);
struct ptree_node *ptree_next (struct ptree_node *v);
struct ptree_node *ptree_next_within (int from, int to, struct ptree_node *v);

struct ptree *ptree_create (void);
void ptree_delete (struct ptree *t);

int ptree_count (struct ptree *t);

#endif /*_PTREE_H_*/

