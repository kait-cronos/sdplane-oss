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

#include "route_info.h"

/* create new nexthop entry */
struct nexthop_info *
nexthop_create (int family, const void *nexthop, uint32_t oif)
{
  struct nexthop_info *nh;

  nh = malloc (sizeof (struct nexthop_info));
  if (nh == NULL)
    return NULL;

  memset (nh, 0, sizeof (struct nexthop_info));
  nh->family = family;
  nh->oif = oif;
  nh->next = NULL;

  if (family == AF_INET)
    memcpy (&nh->nexthop.nexthop4, nexthop, sizeof (struct in_addr));
  else // AF_INET6
    memcpy (&nh->nexthop.nexthop6, nexthop, sizeof (struct in6_addr));

  return nh;
}

/* free nexthop entry and all following entries in the list */
void
nexthop_free (struct nexthop_info *nh)
{
  struct nexthop_info *next;

  while (nh)
    {
      next = nh->next;
      free (nh);
      nh = next;
    }
}

/* add nexthop to route_info */
int
route_info_add_nexthop (struct route_info *ri, struct nexthop_info *nh)
{
  if (!ri || !nh)
    return -1;

  /* add to the head of the list */
  nh->next = ri->nexthops;
  ri->nexthops = nh;
  ri->num_nexthop++;

  return 0;
}

/* delete a specific nexthop from route_info */
int
route_info_del_nexthop (struct route_info *ri, struct nexthop_info *nh)
{
  struct nexthop_info *curr, *prev;

  if (!ri || !nh)
    return -1;

  prev = NULL;
  curr = ri->nexthops;

  while (curr)
    {
      int match = 0;

      /* check if this nexthop matches */
      if (curr->oif == nh->oif && curr->family == nh->family)
        {
          if (curr->family == AF_INET)
            match = (memcmp (&curr->nexthop.nexthop4, &nh->nexthop.nexthop4,
                             sizeof (struct in_addr)) == 0);
          else if (curr->family == AF_INET6)
            match = (memcmp (&curr->nexthop.nexthop6, &nh->nexthop.nexthop6,
                             sizeof (struct in6_addr)) == 0);
        }

      if (match)
        {
          /* remove from list */
          if (prev)
            prev->next = curr->next;
          else
            ri->nexthops = curr->next;

          free (curr);
          ri->num_nexthop--;
          return 0;
        }

      prev = curr;
      curr = curr->next;
    }

  return -1;  /* not found */
}

/* create new route_info structure */
struct route_info *
route_info_create (void)
{
  struct route_info *ri;

  ri = malloc (sizeof (struct route_info));
  if (ri == NULL)
    return NULL;

  memset (ri, 0, sizeof (struct route_info));
  ri->num_nexthop = 0;
  ri->nexthops = NULL;

  return ri;
}

/* free route_info structure and all its nexthops */
void
route_info_free (struct route_info *ri)
{
  if (ri)
    {
      nexthop_free (ri->nexthops);
      free (ri);
    }
}

/* copy route_info with deep copy of all nexthops */
struct route_info *
copy_route_info (struct route_info *ri)
{
  struct route_info *new_ri;
  struct nexthop_info *nh, *new_nh;

  if (!ri)
    return NULL;

  /* create new route_info */
  new_ri = route_info_create ();
  if (!new_ri)
    return NULL;

  /* copy all nexthops */
  nh = ri->nexthops;

  while (nh)
    {
      /* create new nexthop */
      if (nh->family == AF_INET)
        new_nh = nexthop_create (AF_INET, &nh->nexthop.nexthop4,
                                 nh->oif);
      else // AF_INET6
        new_nh = nexthop_create (AF_INET6, &nh->nexthop.nexthop6,
                                 nh->oif);

      /* add to destination route_info */
      if (route_info_add_nexthop (new_ri, new_nh) != 0)
        {
          nexthop_free (new_nh);
          route_info_free (new_ri);
          return NULL;
        }

      nh = nh->next;
    }

  return new_ri;
}
