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

#include "radix.h"
#include "fib.h"

/* key: byte array, b: bit index */
#define BIT_CHECK(key, b)                                                     \
  (((uint8_t *) (key))[(b) >> 3] & (0x80 >> ((b) & 0x7)))

struct rib_tree *
rib_new (struct rib_tree *t)
{
  if (! t)
    {
      t = malloc (sizeof (struct rib_tree));
      if (! t)
        return NULL;
    }
  t->root = NULL;
  t->family = 0;
  t->table_id = 0;
  return t;
}

/* free RIB node */
static void
_free_rib_node (struct rib_node *n)
{
  if (n)
    {
      _free_rib_node (n->left);
      _free_rib_node (n->right);
      free (n);
    }
}

void
rib_free (struct rib_tree *t)
{
  if (t)
    {
      _free_rib_node (t->root);
      free (t);
    }
}

static struct rib_node *
_create_rib_node (void)
{
  struct rib_node *new;
  int i;

  new = malloc (sizeof (struct rib_node));
  if (! new)
    return NULL;

  memset (new, 0, sizeof (struct rib_node));

  /* initialize route_idx to -1 to distinguish from valid index 0 */
  for (i = 0; i < MAX_ECMP_ENTRY; i++)
    new->route_idx[i] = -1;

  return new;
}

static struct rib_node *
_add (struct rib_node *n, const uint8_t *key, int keylen, int idx, int depth,
      int *success)
{
  if (! n)
    {
      n = _create_rib_node ();
      if (! n)
        {
          *success = -1; // failed, not enough memory
          return n;
        }
    }

  if (depth == keylen)
    {
      if (n->valid)
        {
          if (n->num_routes >= MAX_ECMP_ENTRY)
            {
              *success = -1; // failed, full ECMP entry
              return n;
            }
          else
            {
              /* add to available slots */
              for (int i = 0; i < MAX_ECMP_ENTRY; i++)
                {
                  if (n->route_idx[i] == -1)
                    {
                      n->route_idx[i] = idx;
                      n->num_routes++;
                      *success = 0; // successed
                      return n;
                    }
                }
            }
          *success = -1; // failed, should not be reached
          return n;
        }
      memset (n->key, 0, 16);
      memcpy (n->key, key, KEY_SIZE (keylen));
      n->keylen = keylen;
      n->valid = 1;
      n->route_idx[0] = idx;
      n->num_routes++;
      *success = 0;
      return n;
    }
  else
    {
      if (BIT_CHECK (key, depth))
        n->right = _add (n->right, key, keylen, idx, depth + 1, success);
      else
        n->left = _add (n->left, key, keylen, idx, depth + 1, success);
      return n;
    }
}

int
rib_route_add (struct rib_tree *t, const uint8_t *key, int keylen, int idx)
{
  int success = 0;
  t->root = _add (t->root, key, keylen, idx, 0, &success);
  return success;
}

static struct rib_node *
_shrink (struct rib_node *n)
{
  if (! n)
    return NULL;

  n->left = _shrink (n->left);
  n->right = _shrink (n->right);

  if (! n->left && ! n->right && ! n->valid)
    {
      free (n);
      n = NULL;
      return NULL;
    }
  else
    return n;
}

static struct rib_node *
_delete (struct rib_node *n, const uint8_t *key, int keylen, int depth,
         int idx, int *success)
{
  if (! n)
    return NULL;

  if (depth == keylen)
    {
      if (n->valid)
        {
          /* delete route entry */
          int found = 0;
          for (int i = 0; i < n->num_routes; i++)
            {
              if (n->route_idx[i] == idx)
                {
                  /* shift remaining entries to fill the gap */
                  for (int j = i; j < n->num_routes - 1; j++)
                    {
                      n->route_idx[j] = n->route_idx[j + 1];
                    }
                  /* clear the last slot */
                  n->route_idx[n->num_routes - 1] = -1;
                  n->num_routes--;
                  *success = 0;
                  found = 1;
                  break;
                }
            }

          if (! found)
            {
              *success = -1;
              return n;
            }

          if (n->num_routes == 0)
            {
              memset (n->key, 0, sizeof (n->key));
              n->keylen = 0;
              n->valid = 0;
              return _shrink (n);
            }
          return n;
        }
      else
        {
          *success = -1;
          return NULL;
        }
    }
  else
    {
      if (BIT_CHECK (key, depth))
        n->right = _delete (n->right, key, keylen, depth + 1, idx, success);
      else
        n->left = _delete (n->left, key, keylen, depth + 1, idx, success);
      return n;
    }
}

int
rib_route_delete (struct rib_tree *t, const uint8_t *key, int keylen, int idx)
{
  int success = 0;
  t->root = _delete (t->root, key, keylen, 0, idx, &success);
  return success;
}

static struct rib_node *
_lookup (struct rib_node *n, struct rib_node *cand, const uint8_t *key,
         int depth)
{
  if (! n)
    return cand;

  if (n->valid)
    cand = n;

  DEBUG_SDPLANE_LOG (ROUTE_ENTRY, "depth=%d", depth);
  if (BIT_CHECK (key, depth))
    return _lookup (n->right, cand, key, depth + 1);
  else
    return _lookup (n->left, cand, key, depth + 1);
}

struct rib_node *
rib_route_lookup (struct rib_tree *t, const uint8_t *key)
{
  return _lookup (t->root, NULL, key, 0);
}

/* traverse RIB tree */
static int
_traverse (struct rib_node *n, rib_traverse_callback callback, void *arg)
{
  if (! n)
    return 0;

  /* process current node if valid */
  if (n->valid && n->num_routes != 0 && callback)
    {
      if (callback (n, arg) != 0)
        return -1;
    }

  if (_traverse (n->left, callback, arg) != 0)
    return -1;

  if (_traverse (n->right, callback, arg) != 0)
    return -1;

  return 0;
}

int
rib_traverse (struct rib_tree *t, rib_traverse_callback callback, void *arg)
{
  if (! t || ! t->root || ! callback)
    return 0;
  return _traverse (t->root, callback, arg);
}

/* callback for rebuilding FIB from RIB */
static int
_add_to_fib (struct rib_node *n, void *arg)
{
  struct fib_tree *fib_tree = (struct fib_tree *) arg;

  uint8_t key_str[INET6_ADDRSTRLEN];
  inet_ntop (fib_tree->family, &n->key, key_str, sizeof (key_str));
  DEBUG_SDPLANE_LOG (ROUTE_ENTRY, "adding route to fib: keylen=%d key=%s",
                     n->keylen, key_str);

  if (fib_tree->family == AF_INET)
    return fib_route_add4 (fib_tree, n->key, n->keylen, n->route_idx);
  else
    return fib_route_add6 (fib_tree, n->key, n->keylen, n->route_idx);
}

/* rebuild FIB from RIB */
int
rebuild_fib_from_rib (struct rib_tree *rib_tree, struct fib_tree *fib_tree)
{
  /* copy family and table_id from RIB to FIB */
  fib_tree->family = rib_tree->family;
  fib_tree->table_id = rib_tree->table_id;
  return rib_traverse (rib_tree, _add_to_fib, fib_tree);
}

/* callback for show ip route */
int
rib_show_route (struct rib_node *n, void *arg)
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
  inet_ntop (family, &n->key, prefix_str, sizeof (prefix_str));

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
