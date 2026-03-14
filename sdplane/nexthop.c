#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include "debug_sdplane.h"

#include "jhash.h"
#include "hash_table.h"
#include "internal_message.h"

/* global hash table: mapping for kernel_nh_id to sdplane_nh_id. */
struct hash_table nh_ht;

char *
nexthop_format (struct internal_msg_nh_entry *nh, char *buf, size_t buf_size)
{
  char nh_str[INET6_ADDRSTRLEN] = { 0 };
  int len = 0, i;

  if (nh->nhcnt == 0)
    {
      if (nh->kernel_nh_id)
        snprintf (buf, buf_size, "nhid=%d", nh->kernel_nh_id);
      else
        snprintf (buf, buf_size, "(empty)");
      return buf;
    }

  if (nh->nhcnt == 1 && nh->members[0].kernel_nh_id == 0)
    {
      const struct nh_info *info = &nh->members[0].info;
      inet_ntop (info->family, &info->gw, nh_str, sizeof (nh_str));
      snprintf (buf, buf_size, "%s(oif=%d)", nh_str, info->oif);
      return buf;
    }

  len += snprintf (buf + len, buf_size - len, "[");
  for (i = 0; i < nh->nhcnt; i++)
    {
      const char *sep = (i == nh->nhcnt - 1) ? "" : ", ";

      if (nh->members[i].kernel_nh_id != 0)
        {
          len += snprintf (buf + len, buf_size - len, "nhid=%d(w=%u)%s",
                           nh->members[i].kernel_nh_id,
                           nh->members[i].weight, sep);
        }
      else
        {
          const struct nh_info *info = &nh->members[i].info;
          inet_ntop (info->family, &info->gw, nh_str, sizeof (nh_str));
          len += snprintf (buf + len, buf_size - len, "%s(oif=%d,w=%u)%s",
                           nh_str, info->oif, nh->members[i].weight, sep);
        }
    }
  snprintf (buf + len, buf_size - len, "]");
  return buf;
}

void
nexthop_show_group_table (struct shell *shell, struct rib_info *rib_info)
{
  char nh_str[INET6_ADDRSTRLEN];
  int i, j, k;
  struct nexthop *nexthop = &rib_info->nexthop;

  for (i = 0; i < MAX_NEXTHOP_GRP_SIZE; i++)
    {
      struct nh_group *grp = &nexthop->groups[i];
      if (grp->nhcnt <= 0)
        continue;

      /* ID: sdplane internal index */
      fprintf (shell->terminal, "ID: %d%s", i, shell->NL);
      if (grp->kernel_nh_id != 0)
        fprintf (shell->terminal, "     KernelID: %d%s",
                 grp->kernel_nh_id, shell->NL);
      fprintf (shell->terminal, "     RefCnt: %d%s", grp->refcnt, shell->NL);
      fprintf (shell->terminal, "     Nexthop Count: %d%s",
               grp->nhcnt, shell->NL);

      /* Depends: sdplane IDs of groups that has single nexthop this GROUP_CAP depends on */
      int has_deps = 0;
      for (j = 0; j < grp->nhcnt; j++)
        {
          if (grp->members[j].ref_sdplane_nh_id >= 0)
            {
              if (! has_deps)
                {
                  fprintf (shell->terminal, "     Depends:");
                  has_deps = 1;
                }
              fprintf (shell->terminal, " (%d)",
                       grp->members[j].ref_sdplane_nh_id);
            }
        }
      if (has_deps)
        fprintf (shell->terminal, "%s", shell->NL);

      /* Dependents: sdplane IDs of GROUP_CAP groups that reference this */
      int has_dependents = 0;
      for (k = 0; k < MAX_NEXTHOP_GRP_SIZE; k++)
        {
          struct nh_group *other = &nexthop->groups[k];
          if (other->nhcnt <= 0 || k == i)
            continue;
          for (j = 0; j < other->nhcnt; j++)
            {
              if (other->members[j].ref_sdplane_nh_id == i)
                {
                  if (! has_dependents)
                    {
                      fprintf (shell->terminal, "     Dependents:");
                      has_dependents = 1;
                    }
                  fprintf (shell->terminal, " (%d)", k);
                  break;
                }
            }
        }
      if (has_dependents)
        fprintf (shell->terminal, "%s", shell->NL);

      /* Display per-member nexthop info */
      for (j = 0; j < grp->nhcnt; j++)
        {
          int idx = grp->members[j].info_index;
          if (idx < 0)
            continue;
          const struct nh_info *info = &nexthop->info_pool[idx];
          const char *family_str =
            (info->family == AF_INET6) ? "IPv6" : "IPv4";

          if (info->type == NEXTHOP_TYPE_CONNECTED)
            {
              fprintf (shell->terminal,
                       "        is directly connected, %s (%u), %s, weight %u%s",
                       info->oif_name, info->oif, family_str,
                       grp->members[j].weight, shell->NL);
            }
          else
            {
              memset (nh_str, 0, sizeof (nh_str));
              inet_ntop (info->family, &info->gw, nh_str, sizeof (nh_str));
              fprintf (shell->terminal,
                       "        via %s, %s (%u), %s, weight %u%s",
                       nh_str, info->oif_name, info->oif, family_str,
                       grp->members[j].weight, shell->NL);
            }
        }

      fprintf (shell->terminal, "%s", shell->NL);
    }
}

void
nexthop_show_pool (struct shell *shell, struct rib_info *rib_info)
{
  char gw_str[INET6_ADDRSTRLEN];
  int i;
  struct nexthop *nexthop = &rib_info->nexthop;

  fprintf (shell->terminal, "%-6s %-8s %-10s %-20s %-20s %s%s",
           "IDX", "RefCnt", "Family", "Type", "Gateway",
           "Interface", shell->NL);
  fprintf (shell->terminal,
           "---------------------------------------------------------------------------------%s",
           shell->NL);

  for (i = 0; i < MAX_NEXTHOP_INFO_SIZE; i++)
    {
      struct nh_info *info = &nexthop->info_pool[i];
      if (info->refcnt <= 0)
        continue;

      const char *family_str = (info->family == AF_INET6) ? "IPv6" : "IPv4";
      const char *type_str =
        (info->type == NEXTHOP_TYPE_CONNECTED) ? "connected" : "gateway";

      if (info->type == NEXTHOP_TYPE_CONNECTED)
        {
          fprintf (shell->terminal, "%-6d %-8d %-10s %-20s %-20s %s (%u)%s",
                   i, info->refcnt, family_str, type_str, "-",
                   info->oif_name, info->oif, shell->NL);
        }
      else
        {
          memset (gw_str, 0, sizeof (gw_str));
          inet_ntop (info->family, &info->gw, gw_str, sizeof (gw_str));
          fprintf (shell->terminal, "%-6d %-8d %-10s %-20s %-20s %s (%u)%s",
                   i, info->refcnt, family_str, type_str, gw_str,
                   info->oif_name, info->oif, shell->NL);
        }
    }
}

uint32_t
nexthop_hash_kernel_nh_id (const void *key)
{
  uintptr_t p = (uintptr_t)key;
  return jhash ((const int32_t *)&p, (int)(sizeof (p) / 4), JHASH_SEED);
}


int
nexthop_eq_kernel_nh_id (const void *a, const void *b)
{
  return a == b;
}

void
nexthop_init (void)
{
  hash_table_init (&nh_ht, NEXTHOP_HASH_BUCKETS_SIZE,
                   nexthop_hash_kernel_nh_id, nexthop_eq_kernel_nh_id);
}

void
nexthop_cleanup (void)
{
  hash_table_destroy (&nh_ht);
}

static int
_info_find (struct nexthop *nexthop, const struct nh_info *src)
{
  int i;
  for (i = 0; i < MAX_NEXTHOP_INFO_SIZE; i++)
    {
      const struct nh_info *p = &nexthop->info_pool[i];
      if (p->refcnt <= 0)
        continue;
      if (p->family == src->family &&
          memcmp (&p->gw, &src->gw, sizeof (p->gw)) == 0 &&
          p->oif == src->oif)
        return i;
    }
  return -1;
}

static int
_info_alloc (struct nexthop *nexthop, const struct nh_info *src)
{
  /* Try to reuse an existing matching entry */
  int idx = _info_find (nexthop, src);
  if (idx >= 0)
    {
      nexthop->info_pool[idx].refcnt++;
      return idx;
    }

  /* Scan for a free slot starting from the hint */
  int start = nexthop->info_top;
  int i = (start + 1) % MAX_NEXTHOP_INFO_SIZE;
  for (; i != start; i = (i + 1) % MAX_NEXTHOP_INFO_SIZE)
    {
      if (nexthop->info_pool[i].refcnt <= 0)
        {
          nexthop->info_pool[i] = *src;
          nexthop->info_pool[i].refcnt = 1;
          nexthop->info_top = (i + 1) % MAX_NEXTHOP_INFO_SIZE;
          return i;
        }
    }
  return -1; /* pool full */
}

static void
_info_release (struct nexthop *nexthop, int idx)
{
  if (idx < 0 || idx >= MAX_NEXTHOP_INFO_SIZE)
    return;
  struct nh_info *info = &nexthop->info_pool[idx];

  info->refcnt--;
  if (info->refcnt <= 0)
    memset (info, 0, sizeof (*info));
}

static int
_find_next_free_group_slot (struct nexthop *nexthop, int start)
{
  int i = (start + 1) % MAX_NEXTHOP_GRP_SIZE;
  for (; i != start; i = (i + 1) % MAX_NEXTHOP_GRP_SIZE)
    {
      if (nexthop->groups[i].nhcnt == 0)
        return i;
    }
  return -1; /* table full */
}

int
nexthop_add_entry (struct rib_info *rib_info, struct internal_msg_nh_entry *nh)
{
  struct nexthop *nexthop = &rib_info->nexthop;
  int i, j;

  /* If kernel_nh_id is set, check whether it already exists */
  if (nh->kernel_nh_id != 0)
    {
      int existing = nexthop_get_index_by_nhid (rib_info, nh->kernel_nh_id);
      if (existing >= 0)
        {
          DEBUG_NEW (RIB, "reused group: kernel_nh_id=%d sdplane_nh_id=%d",
                     nh->kernel_nh_id, existing);
          return existing;
        }
    }
  else
    {
      /* LEGACY: deduplicate by content */
      for (j = 0; j < MAX_NEXTHOP_GRP_SIZE; j++)
        {
          struct nh_group *g = &nexthop->groups[j];
          if (g->nhcnt == 0 || g->kernel_nh_id != 0)
            continue;
          if (g->nhcnt != nh->nhcnt)
            continue;
          int match = 1;
          for (i = 0; i < nh->nhcnt; i++)
            {
              int aidx = g->members[i].info_index;
              const struct nh_info *a =
                (aidx >= 0) ? &nexthop->info_pool[aidx] : NULL;
              const struct nh_info *b = &nh->members[i].info;
              if (!a || a->family != b->family ||
                  memcmp (&a->gw, &b->gw, sizeof (a->gw)) != 0 ||
                  a->oif != b->oif)
                {
                  match = 0;
                  break;
                }
            }
          if (match)
            {
              DEBUG_NEW (RIB, "reused group: kernel_nh_id=N/A sdplane_nh_id=%d", j);
              return j;
            }
        }
    }

  int slot;
  if (nexthop->groups[nexthop->groups_top].nhcnt == 0)
    slot = nexthop->groups_top;
  else
    slot = _find_next_free_group_slot (nexthop, nexthop->groups_top);

  if (slot < 0)
    {
      DEBUG_NEW (RIB, "nexthop group table full");
      return -1;
    }

  struct nh_group *grp = &nexthop->groups[slot];
  grp->sdplane_nh_id = slot;
  grp->kernel_nh_id = nh->kernel_nh_id;
  grp->refcnt = 0;
  grp->nhcnt = nh->nhcnt;

  /* Populate members */
  for (i = 0; i < nh->nhcnt; i++)
    {
      struct internal_msg_nh_member *member = &nh->members[i];

      if (member->kernel_nh_id != 0)
        {
          /* GROUP_CAP: resolve info from the referenced group */
          int ref_slot = nexthop_get_index_by_nhid (rib_info, member->kernel_nh_id);
          if (ref_slot < 0)
            {
              DEBUG_NEW (RIB,
                         "unresolved kernel_nh_id=%d for member %d; leaving -1",
                         member->kernel_nh_id, i);
              grp->members[i].info_index = -1;
              grp->members[i].ref_sdplane_nh_id = -1;
              grp->members[i].weight = member->weight;
              continue;
            }

          /* Share the first member's info_idx from the referenced group */
          int shared_idx = nexthop->groups[ref_slot].members[0].info_index;
          grp->members[i].info_index = shared_idx;
          grp->members[i].ref_sdplane_nh_id = ref_slot;
          if (shared_idx >= 0)
            nexthop->info_pool[shared_idx].refcnt++;
          nexthop->groups[ref_slot].refcnt++; /* group-to-group reference */
        }
      else if (nh->nhcnt > 1 && nh->kernel_nh_id == 0)
        {
          /* LEGACY ECMP: auto-create a standalone group for each member so
           * every unique nexthop gets its own group entry. */
          struct internal_msg_nh_entry legacy = { .kernel_nh_id = 0,
                                                  .nhcnt = 1,
                                                  .members[0] = *member };
          int ref_slot = nexthop_add_entry (rib_info, &legacy);
          if (ref_slot < 0)
            {
              DEBUG_NEW (RIB,
                         "failed to create standalone group for ECMP member %d",
                         i);
              for (j = 0; j < i; j++)
                {
                  if (grp->members[j].ref_sdplane_nh_id >= 0)
                    nexthop->groups[grp->members[j].ref_sdplane_nh_id]
                        .refcnt--;
                  _info_release (nexthop, grp->members[j].info_index);
                }
              memset (grp, 0, sizeof (*grp));
              return -1;
            }

          int shared_idx = nexthop->groups[ref_slot].members[0].info_index;
          grp->members[i].info_index = shared_idx;
          grp->members[i].ref_sdplane_nh_id = ref_slot;
          if (shared_idx >= 0)
            nexthop->info_pool[shared_idx].refcnt++;
          nexthop->groups[ref_slot].refcnt++; /* group-to-group reference */
        }
      else
        {
          /* Single inline member: find or allocate from info_pool */
          int info_idx = _info_alloc (nexthop, &member->info);
          if (info_idx < 0)
            {
              DEBUG_NEW (RIB, "info_pool full at member %d; rolling back", i);
              for (int j = 0; j < i; j++)
                _info_release (nexthop, grp->members[j].info_index);
              memset (grp, 0, sizeof (*grp));
              return -1;
            }
          grp->members[i].info_index = info_idx;
          grp->members[i].ref_sdplane_nh_id = -1;
        }

      grp->members[i].weight = member->weight;
    }

  /* Update the free-slot hint */
  int next_free = _find_next_free_group_slot (nexthop, slot);
  nexthop->groups_top = (next_free >= 0) ? next_free : slot;

  /* Insert into hash table if kernel_nh_id is set */
  if (nh->kernel_nh_id != 0)
    {
      if (hash_table_insert (&nh_ht,
                             (void *)(intptr_t)nh->kernel_nh_id,
                             (void *)(intptr_t)slot) < 0)
        {
          /* Rollback */
          for (i = 0; i < grp->nhcnt; i++)
            _info_release (nexthop, grp->members[i].info_index);
          memset (grp, 0, sizeof (*grp));
          return -1;
        }
    }

  DEBUG_NEW (RIB, "created group: slot=%d kernel_nh_id=%d nhcnt=%d",
             slot, nh->kernel_nh_id, nh->nhcnt);
  return slot;
}

int
nexthop_del_entry (struct rib_info *rib_info, struct internal_msg_nh_entry *nh)
{
  struct nexthop *nexthop = &rib_info->nexthop;
  int slot = -1, i;

  if (nh->kernel_nh_id != 0)
    {
      /* Lookup by kernel_nh_id via hash table */
      slot = nexthop_get_index_by_nhid (rib_info, nh->kernel_nh_id);
      if (slot < 0)
        {
          DEBUG_NEW (RIB, "nexthop not found: kernel_nh_id=%d",
                     nh->kernel_nh_id);
          return -1;
        }

      /* Remove from hash table */
      hash_table_remove (&nh_ht, (void *)(intptr_t)nh->kernel_nh_id);
    }
  else
    {
      /* LEGACY: linear scan — match by nhcnt and inline info content */
      for (slot = 0; slot < MAX_NEXTHOP_INFO_SIZE; slot++)
        {
          struct nh_group *g = &nexthop->groups[slot];
          if (g->nhcnt == 0 || g->kernel_nh_id != 0)
            continue;
          if (g->nhcnt != nh->nhcnt)
            continue;

          int match = 1;
          for (i = 0; i < nh->nhcnt; i++)
            {
              int aidx = g->members[i].info_index;
              const struct nh_info *a =
                (aidx >= 0) ? &nexthop->info_pool[aidx] : NULL;
              const struct nh_info *b = &nh->members[i].info;
              if (! a || a->family != b->family ||
                  memcmp (&a->gw, &b->gw, sizeof (a->gw)) != 0 ||
                  a->oif != b->oif)
                {
                  match = 0;
                  break;
                }
            }
          if (match)
            break;
        }
      if (slot >= MAX_NEXTHOP_INFO_SIZE)
        {
          DEBUG_NEW (RIB, "LEGACY nexthop not found");
          return -1;
        }
    }

  struct nh_group *grp = &nexthop->groups[slot];

  /* Release all member resources */
  for (i = 0; i < grp->nhcnt; i++)
    {
      _info_release (nexthop, grp->members[i].info_index);
      if (grp->members[i].ref_sdplane_nh_id >= 0)
        nexthop->groups[grp->members[i].ref_sdplane_nh_id].refcnt--;
    }

  DEBUG_NEW (RIB, "deleted group: slot=%d kernel_nh_id=%d",
             slot, grp->kernel_nh_id);

  memset (grp, 0, sizeof (*grp)); /* nhcnt -> 0 == free marker */
  return slot;
}

void
nexthop_increment_refcnt (struct rib_info *rib_info, int sdplane_nh_id)
{
  if (sdplane_nh_id >= 0 && sdplane_nh_id < MAX_NEXTHOP_GRP_SIZE)
    rib_info->nexthop.groups[sdplane_nh_id].refcnt++;
}

void
nexthop_decrement_refcnt (struct rib_info *rib_info, int sdplane_nh_id)
{
  if (sdplane_nh_id >= 0 && sdplane_nh_id < MAX_NEXTHOP_GRP_SIZE)
    rib_info->nexthop.groups[sdplane_nh_id].refcnt--;
}

struct nh_info *
nexthop_get_info_by_index (struct rib_info *rib_info, int sdplane_nh_id)
{
  if (sdplane_nh_id < 0 || sdplane_nh_id >= MAX_NEXTHOP_GRP_SIZE)
    return NULL;

  struct nh_group *grp = &rib_info->nexthop.groups[sdplane_nh_id];
  int idx = grp->members[0].info_index;
  if (idx < 0)
    return NULL;

  return &rib_info->nexthop.info_pool[idx];
}

int
nexthop_get_index_by_nhid (struct rib_info *rib_info, int kernel_nh_id)
{
  if (kernel_nh_id == 0)
    return -1;

  void *v = hash_table_lookup (&nh_ht, (void *)(intptr_t)kernel_nh_id);
  if (! v)
    return -1;

  return (int)(intptr_t)v;
}
