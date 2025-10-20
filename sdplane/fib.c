#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "radix.h"

// key: byte array, b: bit index
#define BIT_CHECK(key, b) (((uint8_t *)(key))[(b) >> 3] & (0x80 >> ((b)&0x7)))

// key: address, s: start bit, n: number of bits
static inline uint32_t
BIT_INDEX32 (const uint8_t *key, int s, int n)
{
  uint32_t key32 = ((uint32_t)key[0] << 24) | ((uint32_t)key[1] << 16)
                   | ((uint32_t)key[2] << 8) | ((uint32_t)key[3]);
  return (((uint64_t)(key32) << 32 >> (64 - ((s) + (n)))) & ((1 << (n)) - 1));
}

struct rib_tree *
rib_new (struct rib_tree *t)
{
  if (!t)
    {
      t = malloc (sizeof (struct rib_tree));
      if (!t)
        return NULL;
    }
  t->root = NULL;
  return t;
}

static void
_free_rib_node (struct rib_node *n)
{
  int i;

  if (n != NULL)
    {
      for (i = 0; i < BRANCH_SZ; i++)
        _free_rib_node (n->child[i]);
      free (n);
    }
}

void
rib_free (struct rib_tree *t)
{
  if (t != NULL)
    {
      _free_rib_node (t->root);
      free (t);
    }
}

static inline struct rib_node *
_create_rib_node (void)
{
  struct rib_node *new;

  new = malloc (sizeof (struct rib_node));
  if (new != NULL)
    memset (new, 0, sizeof (struct rib_node));
  return new;
}

static struct rib_node *
_add (struct rib_node *n, const uint8_t *key, int plen, void *data, int depth)
{
  uint32_t index, i;
  uint32_t bits_in_depth, base, first, count;
  int exists = (n != NULL);

  if (!exists)
    n = _create_rib_node ();

  /* case1: 階層がプレフィックスに到達した場合 */
  if (plen <= depth)
    {
      /* 葉ノードではない（=子ノードが存在する）かつ、
       * 既にノードが存在する（=内部ノード）場合に
       * 全ての子に新しいプレフィックスを伝播 */
      if (!n->leaf && exists)
        {
          for (i = 0; i < BRANCH_SZ; i++)
            n->child[i] = _add (n->child[i], key, plen, data, depth + K);
          return n;
        }
      /* 葉ノードの場合 */
      else if (n->leaf)
        {
          /* より長いプレフィックスの場合に更新 */
          if (plen > n->plen)
            {
              n->plen = plen;
              n->data = data;
#ifdef DEBUG
              printf ("update leaf plen=%d, data=%p, depth=%d\n", plen, data,
                      depth);
#endif
            }
          return n;
        }
      /* 新規葉ノードとして登録 */
      else
        {
          n->leaf = 1;
          n->plen = plen;
          n->data = data;
#ifdef DEBUG
          printf ("set leaf plen=%d, data=%p, depth=%d\n", plen, data, depth);
#endif
          return n;
        }
    }

  /* case2: プレフィックスが次の階層の途中で終わる場合 */
  if (plen < depth + K)
    {
      /*
       * - Example: K=2 (4-ary)
       *   - 96.0.0.0/3 (0b011/3)
       * ------------------------------------------
       *    v depth=0
       * root      v depth=2
       *    |---- 01      v depth=4
       *           |---- 00
       *           |---- 01
       *           |---- 10 <- new node: 96.0.0.0/3
       *           |---- 11 <- new node: 96.0.0.0/3
       * ------------------------------------------
       * - plen=3. depth=2 (3 < 2 + 2)
       *   - bits_in_depth: 3 - 2 = 1 (0b01|1*)
       *                                    ^
       *   - base = 0b01|10
       *               ^x1 = 1
       *   - first = 1 << (2 - 1) = 0b010 = 2
       *   - count = 1 << (2 - 1) = 0b010 = 2
       *   - range: child[2] to child[3]
       */
      /* 新しいプレフィックスが登録される子ノードの範囲を計算 */
      bits_in_depth = plen - depth; // この階層で決定されるビット数（1〜K-1）
      base = BIT_INDEX32 (key, depth, bits_in_depth);
      first = base << (K - bits_in_depth); // 範囲の開始インデックス
      count = 1 << (K - bits_in_depth);    // 範囲のサイズ

      /* 全ての子ノードに対して */
      for (i = 0; i < BRANCH_SZ; i++)
        {
          if (i >= first && i < first + count)
            /* この範囲には新しいノードを登録 */
            n->child[i] = _add (n->child[i], key, plen, data, depth + K);
          else if (n->leaf)
            /* 範囲外には親ノードのデータをコピー */
            n->child[i] = _add (n->child[i], key, n->plen, n->data, depth + K);
        }
      /* 現在のノードはもはや葉ノードではない */
      n->leaf = 0;
      n->plen = 0;
      n->data = NULL;
      return n;
    }

  /* case3: さらに深い階層へ再帰 */
  index = BIT_INDEX32 (key, depth, K);
  n->child[index] = _add (n->child[index], key, plen, data, depth + K);
  return n;
}

int
rib_route_add (struct rib_tree *t, const uint8_t *key, int plen, void *data)
{
#ifdef DEBUG
  static int count = 0;
  count++;
  printf ("----- Add %d: key=%u.%u.%u.%u/%d, data=%p -----\n", count, key[0],
          key[1], key[2], key[3], plen, data);
#endif
  t->root = _add (t->root, key, plen, data, 0);
  return (t->root == NULL) ? -1 : 0; /* error(-1) if root is NULL */
}

static int
_shrink (struct rib_node *n)
{
  return 0;
}

static struct rib_node *
_delete (struct rib_node *n, uint8_t *key, int plen, int depth)
{
  uint32_t i;
  uint32_t bits_in_depth, base, first, count;
  uint32_t index;

  if (n == NULL)
    return NULL;

  /* case1: 階層がプレフィックスに到達した場合 */
  if (plen <= depth)
    {
      if (n->leaf)
        {
          /* 葉ノードを削除 */
          free (n);
          return NULL; /* 親が child[index] を NULL に更新できるように */
        }
      else
        return n; /* 葉ノードでない場合は削除しない */
    }

  /* case2: プレフィックスが次の階層の途中で終わる場合 */
  if (plen < depth + K)
    {
      bits_in_depth = plen - depth; // この階層で決定されるビット数（1〜K-1）
      base = BIT_INDEX32 (key, depth, bits_in_depth);
      first = base << (K - bits_in_depth); // 範囲の開始インデックス
      count = 1 << (K - bits_in_depth);    // 範囲のサイズ

      /* 範囲内のすべての子ノードを削除 */
      for (i = first; i < first + count; i++)
        n->child[i] = _delete (n->child[i], key, plen, depth + K);

      return n; /* 更新された親ノードを返す */
    }

  /* case3: さらに深い階層へ再帰 */
  index = BIT_INDEX32 (key, depth, K);
  n->child[index] = _delete (n->child[index], key, plen, depth + K);
  return n; /* 更新された親ノードを返す */
}

int
rib_route_delete (struct rib_tree *t, uint8_t *key, int plen)
{
  t->root = _delete (t->root, key, plen, 0);
  return 0; /* 削除成功 */
}

static struct rib_node *
_lookup (struct rib_node *n, struct rib_node *cand, const uint8_t *key,
         int depth)
{
  uint32_t index;

  if (n == NULL)
    return cand;

  if (n->leaf)
    cand = n;

  index = BIT_INDEX32 (key, depth, K);
#ifdef DEBUG
  printf ("depth=%d, index=%d\n", depth, index);
#endif

  return _lookup (n->child[index], cand, key, depth + K);
}

struct rib_node *
rib_route_lookup (struct rib_tree *t, const uint8_t *key)
{
  return _lookup (t->root, NULL, key, 0);
}