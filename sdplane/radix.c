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

  new = malloc (sizeof (struct rib_node));
  if (! new)
    return NULL;

  memset (new, 0, sizeof (struct rib_node));

  return new;
}

static struct rib_node *
_add (struct rib_node *n, const uint8_t *key, int keylen, struct route_entry *data,
      int depth, int *success)
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
          /**
           * TODO1: Replace nexthop info when nlmsg_flags indicates a replace operation
           * e.g., ip route replace ...
           */
          /**
           * Currently only the destination prefix is used as the key.
           * Routes that share the same prefix but differ in outgoing interface
           * (e.g., link-local routes such as fe80::/64) are treated as identical.
           * TODO2: support composite keys (e.g., prefix + oif).
           */
          *success = -1; // failed, already exists
          return n;
        }
      /* add route entry */
      memcpy (&n->entry, data, sizeof (struct route_entry));
      n->valid = 1;
      *success = 0;
      return n;
    }
  else
    {
      if (BIT_CHECK (key, depth))
        n->right = _add (n->right, key, keylen, data, depth + 1, success);
      else
        n->left = _add (n->left, key, keylen, data, depth + 1, success);
      return n;
    }
}

int
rib_route_add (struct rib_tree *t, const uint8_t *key, int keylen, struct route_entry *data)
{
  int success = 0;
  t->root = _add (t->root, key, keylen, data, 0, &success);
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
         int *success)
{
  if (! n)
    return NULL;

  if (depth == keylen)
    {
      if (n->valid)
        {
          /* delete route entry */
          memset (&n->entry, 0, sizeof (struct route_entry));
          n->valid = 0;
          *success = 0;
          return _shrink (n);
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
        n->right = _delete (n->right, key, keylen, depth + 1, success);
      else
        n->left = _delete (n->left, key, keylen, depth + 1, success);
      return n;
    }
}

int
rib_route_delete (struct rib_tree *t, const uint8_t *key, int keylen)
{
  int success = 0;
  t->root = _delete (t->root, key, keylen, 0, &success);
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
  if (n->valid && callback)
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

unsigned int route_count = 0;

int
rib_traverse (struct rib_tree *t, rib_traverse_callback callback, void *arg)
{
  route_count = 0;
  if (! t || ! t->root || ! callback)
    return 0;
  return _traverse (t->root, callback, arg);
}

/* callback for rebuilding FIB from RIB */
static int
_add_to_fib (struct rib_node *n, void *arg)
{
  struct fib_tree *fib_tree = (struct fib_tree *) arg;

  char key_str[INET6_ADDRSTRLEN];
  inet_ntop (fib_tree->family, &n->entry.dst.dst_ip_addr, key_str, sizeof (key_str));
  DEBUG_SDPLANE_LOG (ROUTE_ENTRY, "adding route to fib: keylen=%d key=%s nh_id=%d",
                     n->entry.dst.plen, key_str, n->entry.nh.nh_id);

  return fib_route_add (fib_tree, (uint8_t *)&n->entry.dst.dst_ip_addr, n->entry.dst.plen, &n->entry.nh);
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
  struct rib_info *rib_info = show_arg->rib_info;
  struct shell *shell = show_arg->shell;
  int i, family = show_arg->family;
  struct nh_common *nh = &n->entry.nh;
  int ret;

  char prefix_str[INET6_ADDRSTRLEN];
  char dst_str[INET6_ADDRSTRLEN + 5];
  char nexthop_str[INET6_ADDRSTRLEN];

  inet_ntop(family, &n->entry.dst.dst_ip_addr, prefix_str, sizeof(prefix_str));
  snprintf(dst_str, sizeof(dst_str), "%s/%d", prefix_str, n->entry.dst.plen);

  switch (nh->nh_type)
    {
      case NH_TYPE_LEGACY:
        {
          struct nh_legacy *nh_legacy = &rib_info->nexthop.legacy.object[nh->nh_id];
          switch (nh_legacy->type)
            {
              case NH_OBJ_TYPE_OBJECT:
                {
                  struct nh_info *nh_info = &nh_legacy->nh_info;

                  inet_ntop (nh_info->family, &nh_info->nh_ip_addr,
                             nexthop_str, sizeof (nexthop_str));

                  ret = fprintf (shell->terminal,
                          "[%d] %-30s  nhid %-3d > %-19s  dev %u%s",
                          route_count, dst_str,
                          nh->nh_id,
                          nexthop_str,
                          nh_info->oif,
                          shell->NL);
                  if (ret < 0)
                    WARNING ("route[%d]: %s fprintf() ret: %d %s",
                             route_count, dst_str, ret, strerror (errno));
                  route_count++;
                  break;
                }

              case NH_OBJ_TYPE_GROUP:
                {
                  struct nh_info_group *nh_grp =
                    &rib_info->nexthop.legacy.object[nh->nh_id].nh_grp;

                  if (rib_info->nexthop.legacy.object[nh->nh_id].ref_count == 0)
                    break;

                  for (i = 0; i < nh_grp->num; i++)
                    {
                      inet_ntop (nh_grp->nh_info_list[i].family,
                                 &nh_grp->nh_info_list[i].nh_ip_addr,
                                 nexthop_str, sizeof (nexthop_str));
                      ret = 0;
                      if (i == 0)
                        ret = fprintf (shell->terminal,
                                "[%d] %-30s  nhid %-3d > %-19s  dev %u%s",
                                route_count, dst_str,
                                nh->nh_id,
                                nexthop_str,
                                nh_grp->nh_info_list[i].oif,
                                shell->NL);
                      else
                        ret = fprintf(shell->terminal,
                                "[%d] %-30s             %-19s  dev %u%s",
                                route_count, "",
                                nexthop_str,
                                nh_grp->nh_info_list[i].oif,
                                shell->NL);
                      if (ret < 0)
                        WARNING ("route[%d]: %s fprintf() ret: %d %s",
                           route_count, dst_str, ret, strerror (errno));
                    }
                  route_count++;
                  break;
                }

              default:
                snprintf (nexthop_str, sizeof (nexthop_str),
                          "unknown legacy nexthop type");
                break;
            }
          break;
        }

      case NH_TYPE_OBJECT_CAP:
        snprintf (nexthop_str, sizeof (nexthop_str),
                  "NH_TYPE_OBJECT_CAP not implemented yet");
        break;

      default:
        snprintf (nexthop_str, sizeof (nexthop_str),
                  "unknown nexthop type");
        break;
    }

  return 0;
}
