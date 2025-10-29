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

// key: address, s: start bit, n: number of bits
static inline uint32_t
BIT_INDEX (const uint8_t *key, int s, int n)
{
  uint32_t key32 = ((uint32_t)key[0] << 24) | ((uint32_t)key[1] << 16)
                   | ((uint32_t)key[2] << 8) | ((uint32_t)key[3]);
  return (((uint64_t)(key32) << 32 >> (64 - ((s) + (n)))) & ((1 << (n)) - 1));
}

static struct fib_node *
_add (struct fib_node *n, const uint8_t *key, int keylen, int *route_idx, int depth, int *success)
{
  uint32_t index, i;
  uint32_t bits_in_depth, base, first, count;
  int exists = (n != NULL);

  if (! exists)
    {
      n = create_fib_node ();
      if (! n)
        {
          *success = -1; // failed, not enough memory
          return n;
        }
    }

  /* case1: 階層がプレフィックスに到達した場合 */
  if (keylen <= depth)
    {
      /* 葉ノードではない（=子ノードが存在する）かつ、
       * 既にノードが存在する（=内部ノード）場合に
       * 全ての子に新しいプレフィックスを伝播 */
      if (! n->leaf && exists)
        {
          for (i = 0; i < BRANCH_SZ; i++)
            n->child[i] = _add (n->child[i], key, keylen, route_idx, depth + K, success);
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
              memcpy(n->route_idx, route_idx, sizeof (n->route_idx));
              n->num_routes = count_nonzero (route_idx, MAX_ECMP_ENTRY);
              DEBUG_SDPLANE_LOG (ROUTE_ENTRY, "update leaf keylen=%d depth=%d",
                                 keylen, depth);
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
          memcpy(n->route_idx, route_idx, sizeof (n->route_idx));
          n->num_routes = count_nonzero (route_idx, MAX_ECMP_ENTRY);
          DEBUG_SDPLANE_LOG (ROUTE_ENTRY, "set leaf keylen=%d depth=%d",
                            keylen, depth);
          *success = 0;
          return n;
        }
    }

  /* case2: プレフィックスが次の階層の途中で終わる場合, もしくは葉ノードの場合 */
  if (keylen < depth + K /* || n->leaf */)
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
       * - keylen=3. depth=2 (3 < 2 + 2)
       *   - bits_in_depth: 3 - 2 = 1 (0b01|1*)
       *                                    ^
       *   - base = 0b01|10
       *               ^x1 = 1
       *   - first = 1 << (2 - 1) = 0b010 = 2
       *   - count = 1 << (2 - 1) = 0b010 = 2
       *   - range: child[2] to child[3]
       */
      /* 新しいプレフィックスが登録される子ノードの範囲を計算 */
      bits_in_depth = keylen - depth; // この階層で決定されるビット数（1〜K-1）
      base = BIT_INDEX (key, depth, bits_in_depth);
      first = base << (K - bits_in_depth); // 範囲の開始インデックス
      count = 1 << (K - bits_in_depth);    // 範囲のサイズ
      DEBUG_SDPLANE_LOG (ROUTE_ENTRY, "bits_in_depth=%d, base=%d, first=%d, count=%d",
                         bits_in_depth, base, first, count);

      /* 全ての子ノードに対して */
      for (i = 0; i < BRANCH_SZ; i++)
        {
          if (i >= first && i < first + count)
            {
              /* この範囲には新しいノードを登録 */
              DEBUG_SDPLANE_LOG (ROUTE_ENTRY, "adding to child[%d] (in range)", i);
              n->child[i] = _add (n->child[i], key, keylen, route_idx, depth + K, success);
            }
          else if (n->leaf)
            {
              /* 範囲外には親ノードのデータをコピー */
              DEBUG_SDPLANE_LOG (ROUTE_ENTRY, "copying parent to child[%d] (out of range, parent keylen=%d)", i, n->keylen);
              n->child[i] = _add (n->child[i], n->key, n->keylen, n->route_idx, depth + K, success);
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

  /* 葉ノードの場合, かつkeylenが子ノードよりも大きい場合 (パッチ1) */
  if (n->leaf)
    {
      /* まず全子ノードに親のデータを展開 */
      for (i = 0; i < BRANCH_SZ; i++)
        n->child[i] = _add (n->child[i], n->key, n->keylen, n->route_idx, depth + K, success);
      /* indexに該当する子ノードのみcase3に遷移（新しいルートで更新） */
      index = BIT_INDEX (key, depth, K);
      n->child[index] = _add (n->child[index], key, keylen, route_idx, depth + K, success);
      /* 現在のノードはもはや葉ノードではない */
      n->leaf = 0;
      n->keylen = 0;
      for (i = 0; i < MAX_ECMP_ENTRY; i++)
        n->route_idx[i] = -1;
      *success = 0;
      return n;
    }

  /* case3: さらに深い階層へ再帰 */
  index = BIT_INDEX (key, depth, K);
  n->child[index] = _add (n->child[index], key, keylen, route_idx, depth + K, success);
  return n;
}

int
fib_route_add4 (struct fib_tree *t, const uint8_t *key, int keylen, int *route_idx)
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
_delete (struct rib_node *n, uint8_t *key, int keylen, int depth) {}

int
rib_route_delete4 (struct rib_tree *t, uint8_t *key, int keylen) {}
#endif

static struct fib_node *
_lookup (struct fib_node *n, struct fib_node *cand, const uint8_t *key,
         int depth)
{
  uint32_t index;

  if (! n)
    return cand;

  if (n->leaf)
    cand = n;

  index = BIT_INDEX (key, depth, K);
  DEBUG_SDPLANE_LOG (ROUTE_ENTRY, "depth=%d, index=%d", depth, index);

  return _lookup (n->child[index], cand, key, depth + K);
}

struct fib_node *
fib_route_lookup4 (struct fib_tree *t, const uint8_t *key)
{
  return _lookup (t->root, NULL, key, 0);
}

/* structure to track last processed node for duplicate detection */
struct fib_last_seen
{
  uint8_t *last_key;
  int last_keylen;
};
