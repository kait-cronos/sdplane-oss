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
void
free_fib_node (struct fib_node *n)
{
  int i;

  if (n != NULL)
    {
      for (i = 0; i < BRANCH_SZ; i++)
        {
          if (n->child[i] != NULL)
            free_fib_node (n->child[i]);
        }
      free (n);
    }
}

int
count_nonzero (const int *arr, int len)
{
  int count = 0;
  for (int i = 0; i < len; i++)
    {
        if (arr[i] != -1)
          count++;
    }
  return count;
}

struct fib_node *
create_fib_node (void)
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

/* traverse FIB tree depth-first in-order */
static int
_traverse (struct fib_node *n, fib_traverse_callback callback, void *arg,
           int depth)
{
  int i;

  if (! n)
    return 0;

  /* process current node if it's a leaf */
  if (n->leaf && n->num_routes != 0 && callback)
    {
      DEBUG_SDPLANE_LOG (ROUTE_ENTRY,
                          "show route: depth=%d keylen=%d", depth,
                          n->keylen);
      if (callback (n, arg) != 0)
        return -1;
    }

  /* process children in order */
  for (i = 0; i < BRANCH_SZ; i++)
    {
      if (_traverse (n->child[i], callback, arg, depth + K) != 0)
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
  uint8_t dst_str[INET6_ADDRSTRLEN + 5]; // support IPv6 prefix/prefixlen string size

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

          inet_ntop (family, &entry->nexthop, nexthop_str, sizeof (nexthop_str));
          snprintf (dst_str, sizeof(dst_str), "%s/%d", prefix_str, n->keylen);
          
          fprintf (shell->terminal, "%-30s  via %-26s  dev %u%s",
                   dst_str, nexthop_str, entry->oif, shell->NL);
        }
    }

  return 0;
}
