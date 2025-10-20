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

#include "radix.h"
#include "fib.h"

// key: byte array, b: bit index
#define BIT_CHECK(key, b) (((uint8_t *)(key))[(b) >> 3] & (0x80 >> ((b)&0x7)))

struct rib_node *
create_rib_node (void)
{
  struct rib_node *new;

  new = malloc (sizeof (struct rib_node));
  if (new != NULL)
    memset (new, 0, sizeof (struct rib_node));
  return new;
}

static struct rib_node *
_add (struct rib_node *n, const uint8_t *key, int plen, struct route_info *ri,
          int depth)
{
  if (n == NULL)
    n = create_rib_node ();

  if (depth == plen)
    {
      if (n->valid)
        {
          if (n->route)
            {
              /* add nexthop list */
              struct nexthop_info *new_nh = ri->nexthops;
              ri->nexthops = NULL;
              route_info_add_nexthop (n->route, new_nh);
              route_info_free (ri);
            }
          return n;
        }
      n->plen = plen;
      n->valid = 1;
      n->route = ri;
      return n;
    }
  else
    {
      if (BIT_CHECK (key, depth))
        n->right = _add (n->right, key, plen, ri, depth + 1);
      else
        n->left = _add (n->left, key, plen, ri, depth + 1);
      return n;
    }
}

int
rib_route_add (struct rib_tree *t, const uint8_t *key, int plen, struct route_info *ri)
{
  t->root = _add (t->root, key, plen, ri, 0);
  return (t->root == NULL) ? -1 : 0;
}

static struct rib_node *
_shrink(struct rib_node *n)
{
  if (n == NULL)
    return NULL;

  n->left = _shrink(n->left);
  n->right = _shrink(n->right);

  if (n->left || n->right)
    return n;
  else
    {
      if (n->left == NULL && n->right == NULL && !n->valid)
        {
          free(n);
          n = NULL;
          return NULL;
        }
      else
        return n;
    }
}

static struct rib_node *
_delete (struct rib_node *n, const uint8_t *key, int plen, int depth,
         struct nexthop_info *nh)
{
  if (n == NULL)
    return NULL;

  if (depth == plen)
    {
      if (n->valid)
        {
          n->plen = 0;
          n->valid = 0;
          /* delete nexthop */
          route_info_del_nexthop (n->route, nh);
          if (n->route->num_nexthop == 0)
            {
              route_info_free (n->route);
              n->route = NULL;
            }
          return _shrink (n);
        }
      else
        return NULL;
    }
  else
    {
      if (BIT_CHECK (key, depth))
        n->right = _delete (n->right, key, plen, depth + 1, nh);
      else
        n->left = _delete (n->left, key, plen, depth + 1, nh);
      return n;
    }
}

int
rib_route_delete (struct rib_tree *t, const uint8_t *key, int plen,
                  struct nexthop_info *nh)
{
  t->root = _delete (t->root, key, plen, 0, nh);
  return 0;
}

/* recursively add RIB to FIB via DMF */
static int
_traverse_rib_to_fib (struct rib_node *rib, struct fib_tree *fib_tree,
                      uint8_t *key, int depth)
{
  if (rib == NULL)
    return 0;

  if (rib->valid && rib->route != NULL)
    {
      if (fib_route_add (fib_tree, key, rib->plen, rib->route) != 0)
        return -1;
      DEBUG_SDPLANE_LOG (ROUTE_ENTRY, "traverse rib to fib: adding route plen=%d depth=%d key=%d.%d.%d.%d",
            rib->plen, depth, key[0], key[1], key[2], key[3]);
    }

  /* left */
  if (_traverse_rib_to_fib (rib->left, fib_tree, key, depth + 1) != 0)
    return -1;

  /* right */
  if (rib->right)
    {
      key[depth >> 3] |= (0x80 >> (depth & 0x7));
      if (_traverse_rib_to_fib (rib->right, fib_tree, key, depth + 1) != 0)
        return -1;
      key[depth >> 3] &= ~(0x80 >> (depth & 0x7));
    }

  return 0;
}

/* rebuild FIB from RIB */
int
rebuild_fib_from_rib (struct rib_tree *rib_tree, struct fib_tree *fib_tree)
{
  uint8_t key_buf[4] = { 0 };
  return _traverse_rib_to_fib (rib_tree->root, fib_tree, key_buf, 0);
}
