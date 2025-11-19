#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <sdplane/debug.h>
#include <sdplane/debug_cmd.h>
#include <sdplane/debug_log.h>

#include <sdplane/debug_category.h>
#include <sdplane/debug_zcmdsh.h>
#include "debug_sdplane.h"

#include "sdplane.h"

#include "fib.h"

/* key: address, s: start bit, n: number of bits */
static inline uint16_t
BIT_INDEX (const uint8_t *key, int s, int n)
{
  uint8_t key_safe[17]; /* sentinel */
  memcpy (key_safe, key, 16);
  key_safe[16] = 0;

  /*
   *  extract from two bytes comprising byte to
   *  which s belongs and adjacent byte
   */
  return (((key_safe[s >> 3] << 8) | (key_safe[(s >> 3) + 1])) >>
          (16 - ((s & 0x7) + n))) & ((1 << n) - 1);
}

struct fib_tree *
fib_new (struct fib_tree *t)
{
  if (! t)
    {
      t = malloc (sizeof (struct fib_tree));
      if (! t)
        return NULL;
    }
  t->root = NULL;
  t->family = 0;
  t->table_id = 0;
  return t;
}

/* free FIB node */
static void
_free_fib_node (struct fib_node *n)
{
  int i;

  if (n != NULL)
    {
      for (i = 0; i < BRANCH_SZ; i++)
        {
          if (n->child[i] != NULL)
            _free_fib_node (n->child[i]);
        }
      free (n);
    }
}

void
fib_free (struct fib_tree *t)
{
  if (t)
    {
      _free_fib_node (t->root);
      free (t);
    }
}

static inline int
_count_nonzero (const int *arr, int len)
{
  int count = 0;
  for (int i = 0; i < len; i++)
    {
      if (arr[i] != -1)
        count++;
    }
  return count;
}

static struct fib_node *
_create_fib_node (void)
{
  struct fib_node *new;
  int i;

  new = malloc (sizeof (struct fib_node));
  if (! new)
    return NULL;

  memset (new, 0, sizeof (struct fib_node));

  /* initialize route_idx to -1 to distinguish from valid index 0 */
  for (i = 0; i < MAX_ECMP_ENTRY; i++)
    new->route_idx[i] = -1;

  return new;
}

static struct fib_node *
_add (struct fib_node *n, const uint8_t *key, int keylen, int *route_idx,
      int cur_plen, int *success)
{
  uint32_t index, i;
  uint32_t bits_in_depth, first, count;
  uint16_t base;
  int exists = (n != NULL);

  if (! exists)
    {
      n = _create_fib_node ();
      if (! n)
        {
          *success = -1; // failed, not enough memory
          return n;
        }
    }

  /* case1: 階層がプレフィックスに到達した場合 */
  if (keylen <= cur_plen)
    {
      /* 葉ノードではない（=子ノードが存在する）かつ、
       * 既にノードが存在する（=内部ノード）場合に
       * 全ての子に新しいプレフィックスを伝播 */
      if (! n->leaf && exists)
        {
          for (i = 0; i < BRANCH_SZ; i++)
            n->child[i] =
                _add (n->child[i], key, keylen, route_idx, cur_plen + K, success);
          return n;
        }
      /* 葉ノードの場合 */
      else if (n->leaf)
        {
          /* より長いプレフィックスの場合に更新 */
          if (keylen > n->keylen)
            {
              memset (n->key, 0, 16);
              memcpy (n->key, key, KEY_SIZE (keylen));
              n->keylen = keylen;
              memcpy (n->route_idx, route_idx, sizeof (n->route_idx));
              n->num_routes = _count_nonzero (route_idx, MAX_ECMP_ENTRY);
              DEBUG_SDPLANE_LOG (ROUTE_ENTRY, "update leaf keylen=%d cur_plen=%d",
                                 keylen, cur_plen);
            }
          *success = 0;
          return n;
        }
      /* 新規葉ノードとして登録 */
      else
        {
          memset (n->key, 0, 16);
          memcpy (n->key, key, KEY_SIZE (keylen));
          n->leaf = 1;
          n->keylen = keylen;
          memcpy (n->route_idx, route_idx, sizeof (n->route_idx));
          n->num_routes = _count_nonzero (route_idx, MAX_ECMP_ENTRY);
          DEBUG_SDPLANE_LOG (ROUTE_ENTRY, "set leaf keylen=%d cur_plen=%d",
                             keylen, cur_plen);
          *success = 0;
          return n;
        }
    }

  /* case2: プレフィックスが次の階層の途中で終わる場合, もしくは葉ノードの場合
   */
  if (keylen < cur_plen + K || n->leaf)
    {
      /*
       * 例: K=2 (4-ary)
       * - 96.0.0.0/3 (0b011/3) の経路を追加する場合
       * ------------------------------------------
       *    v cur_plen=0
       * root      v cur_plen=2
       *    |---- 01      v cur_plen=4
       *           |---- 00
       *           |---- 01
       *           |---- 10 <- new node: 96.0.0.0/3
       *           |---- 11 <- new node: 96.0.0.0/3
       * ------------------------------------------
       * - keylen=3, cur_plen=2 のとき case2 に該当
       *   - bits_in_depth: 3 - 2 = 1
       *     - 0b011*
       *           ^ cur_plenから1ビット目まで確定
       *   - base: 0b011* = 1
       *               ^
       *   - first = base << (2 - bits_in_depth)
       *           = 0b01 << 1 = 0b10
       *   - count = 1 << (2 - bits_in_depth)
       *           = 0b01 << 1 = 0b10 = 2
       *   - range: child[first] から child[first + count - 1]
       *            -> child[2] から child[3] に新しいノードを登録
       */
      /* 新しいプレフィックスが登録される子ノードの範囲を計算 */
      bits_in_depth = keylen - cur_plen; // この階層で決定されるビット数（1〜K-1）
      if (bits_in_depth > K - 1)
        bits_in_depth = K;
      base = BIT_INDEX (key, cur_plen, bits_in_depth);
      first = base << (K - bits_in_depth); // 範囲の開始インデックス
      count = 1 << (K - bits_in_depth);    // 範囲のサイズ
      DEBUG_SDPLANE_LOG (ROUTE_ENTRY,
                         "bits_in_depth=%d, base=%d, first=%d, count=%d",
                         bits_in_depth, base, first, count);
      /* 全ての子ノードに対して */
      for (i = 0; i < BRANCH_SZ; i++)
        {
          if (n->leaf)
            {
              /* 範囲外には親ノードのデータをコピー */
              DEBUG_SDPLANE_LOG (
                  ROUTE_ENTRY,
                  "copying parent to child[%d] (out of range, parent keylen=%d)",
                  i, n->keylen);
              n->child[i] = _add (n->child[i], n->key, n->keylen,
                                  n->route_idx, cur_plen + K, success);
            }
          if (i >= first && i < first + count)
            {
              /* この範囲には新しいノードを登録 */
              DEBUG_SDPLANE_LOG (ROUTE_ENTRY,
                                  "adding to child[%d] (in range)", i);
              n->child[i] = _add (n->child[i], key, keylen, route_idx,
                                  cur_plen + K, success);
            }
        }
      /* 現在のノードはもはや葉ノードではない */
      n->leaf = 0;
      n->keylen = 0;
      for (i = 0; i < MAX_ECMP_ENTRY; i++)
        n->route_idx[i] = -1;
      *success = 0;
      return n;
    }

  /* case3: さらに深い階層へ再帰 */
  index = BIT_INDEX (key, cur_plen, K);
  n->child[index] =
      _add (n->child[index], key, keylen, route_idx, cur_plen + K, success);
  return n;
}

int
fib_route_add (struct fib_tree *t, const uint8_t *key, int keylen,
               int *route_idx)
{
  int success = 0;
  t->root = _add (t->root, key, keylen, route_idx, 0, &success);
  return success; /* error(-1) if root is NULL */
}

/* it is unnecessary as it will be recreated from the RIB */
#if 0
static int
_shrink (struct rib_node *n) {}

static struct rib_node *
_delete (struct rib_node *n, uint8_t *key, int keylen, int cur_plen) {}

int
rib_route_delete4 (struct rib_tree *t, uint8_t *key, int keylen) {}
#endif

static struct fib_node *
_lookup (struct fib_node *n, struct fib_node *cand, const uint8_t *key,
         int cur_plen)
{
  uint16_t index;

  if (! n)
    return cand;

  if (n->leaf)
    cand = n;

  index = BIT_INDEX (key, cur_plen, K);
  DEBUG_SDPLANE_LOG (ROUTE_ENTRY, "cur_plen=%d, index=%d", cur_plen, index);

  return _lookup (n->child[index], cand, key, cur_plen + K);
}

struct fib_node *
fib_route_lookup (struct fib_tree *t, const uint8_t *key)
{
  return _lookup (t->root, NULL, key, 0);
}

/* traverse FIB tree depth-first in-order */
static int
_traverse (struct fib_node *n, fib_traverse_callback callback, void *arg,
           int cur_plen)
{
  int i;

  if (! n)
    return 0;

  /* process current node if it's a leaf */
  if (n->leaf && n->num_routes != 0 && callback)
    {
      DEBUG_SDPLANE_LOG (ROUTE_ENTRY, "show route: cur_plen=%d keylen=%d", cur_plen,
                         n->keylen);
      if (callback (n, arg) != 0)
        return -1;
    }

  /* process children in order */
  for (i = 0; i < BRANCH_SZ; i++)
    {
      if (_traverse (n->child[i], callback, arg, cur_plen + K) != 0)
        return -1;
    }

  return 0;
}

int
fib_traverse (struct fib_tree *t, fib_traverse_callback callback, void *arg)
{
  if (! t || ! t->root || ! callback)
    return 0;
  return _traverse (t->root, callback, arg, 0);
}

/* callback for show ip route */
int
fib_show_route (struct fib_node *n, void *arg)
{
  struct show_route_arg *show_arg = (struct show_route_arg *) arg;
  struct shell *shell = show_arg->shell;
  struct rib_info *rib_info = show_arg->rib_info;
  int family = show_arg->family;
  uint8_t prefix_str[INET6_ADDRSTRLEN];
  uint8_t nexthop_str[INET6_ADDRSTRLEN];
  uint8_t dst_str[INET6_ADDRSTRLEN + 5]; // support IPv6 string size

  int i;

  /* format prefix */
  inet_ntop (family, n->key, prefix_str, sizeof (prefix_str));

  /* show each route */
  for (i = 0; i < n->num_routes; i++)
    {
      int idx = n->route_idx[i];
      if (idx >= 0 && idx < ROUTE_TABLE_SIZE)
        {
          struct route_entry *entry = &rib_info->route_table[idx];

          inet_ntop (family, &entry->nexthop, nexthop_str,
                     sizeof (nexthop_str));
          snprintf (dst_str, sizeof (dst_str), "%s/%d", prefix_str, n->keylen);

          fprintf (shell->terminal, "%-30s  via %-26s  dev %u%s", dst_str,
                   nexthop_str, entry->oif, shell->NL);
        }
    }

  return 0;
}
