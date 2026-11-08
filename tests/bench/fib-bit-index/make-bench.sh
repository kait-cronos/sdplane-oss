#!/bin/bash
# Assemble bitindex-bench.c: ONE self-contained file a reviewer can paste out of
# the PR and run.  The trie itself is lifted verbatim out of sdplane/fib.c at
# the two revisions, so the benchmark cannot drift from what is being reviewed.
#
#   bash make-bench.sh [old-rev] [new-rev]     # default: origin/main and HEAD
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Locate the sdplane checkout WITHOUT assuming where this script sits.
#
# The obvious "$HERE/../../../.." is right only while crazy_conf is checked out
# at build/crazy_conf, and when it is wrong it still resolves to *some*
# directory -- so the mistake is silent.  `git rev-parse --show-toplevel` does
# not help either: crazy_conf is its own repository, so from here it answers
# with crazy_conf rather than with sdplane.
#
# So walk up looking for the tree itself, and let SDPLANE_REPO override.
find_sdplane_repo () {
  local d="${SDPLANE_REPO:-$1}"
  while [[ -n "$d" && "$d" != / ]]; do
    if [[ -f "$d/sdplane/fib.c" && -f "$d/configure.ac" ]]; then
      echo "$d"
      return 0
    fi
    d="$(dirname "$d")"
  done
  return 1
}

REPO="$(find_sdplane_repo "$HERE")" || {
  echo "!! cannot find an sdplane checkout at or above $HERE" >&2
  echo "   (looked for sdplane/fib.c + configure.ac)" >&2
  echo "   set SDPLANE_REPO=/path/to/sdplane and re-run" >&2
  exit 1
}
# Defaults are commit hashes, not branch names.  Branch names move, and they
# vanish: feat/t-umeda/inline-fib_route_lookup below was deleted from the remote
# while this was being written, which is exactly the failure a recorded
# measurement must not inherit.
OLD_REV="${1:-f12ce59d7f}"   # origin/main at the time of measurement
NEW_REV="${2:-348d5f6}"      # the change itself
NOCOPY_REV="${NOCOPY_REV:-16a898b853}"   # no copy at all, a branch per level
# 134d2fb, "perf: inline fib_route_lookup and copy the lookup key once".  The
# branch it sat on (origin/feat/t-umeda/inline-fib_route_lookup) was DELETED
# from the remote without being merged; it survives here only as the local ref
# below and as the .patch beside this script.
# PATCH:<file> rather than a commit hash.  The hash pins a name, but this
# commit has no home to be fetched from: the branch it sat on was deleted from
# the remote, so 134d2fb2f2 resolves only in a clone that happens to still hold
# the object.  The patch is committed next to this script, so deriving the
# variant from it is what makes the comparison reproducible elsewhere.
# A plain rev is still accepted (e.g. the local ref ref/t-umeda-inline-fib-lookup).
UMEDA_REV="${3:-PATCH:134d2fb-t-umeda-inline-fib_route_lookup.patch}"
OUT="$HERE/bitindex-bench.c"

rev_label () {
  case "$1" in
    WORKTREE) echo "the working tree" ;;
    PATCH:*)  echo "${1#PATCH:} applied to $(git -C "$REPO" rev-parse --short "$OLD_REV")" ;;
    *)        echo "$(git -C "$REPO" rev-parse --short "$1") ($1)" ;;
  esac
}

# Rebuild a revision of fib.c by applying a patch to OLD_REV's copy.  Verified
# applicable: 134d2fb's parent has the same sdplane/fib.c as origin/main.
fib_c_from_patch () {
  local pf="$HERE/$1" tmp
  [[ -f "$pf" ]] || { echo "!! no such patch: $pf" >&2; exit 1; }
  tmp="$(mktemp -d)"
  git -C "$REPO" show "$OLD_REV:sdplane/fib.c" > "$tmp/fib.c"
  patch -s -p2 -d "$tmp" < "$pf" \
    || { echo "!! $1 does not apply to $OLD_REV:sdplane/fib.c" >&2; rm -rf "$tmp"; exit 1; }
  cat "$tmp/fib.c"
  rm -rf "$tmp"
}

# read one revision of sdplane/fib.c
fib_c () {
  case "$1" in
    WORKTREE) cat "$REPO/sdplane/fib.c" ;;
    PATCH:*)  fib_c_from_patch "${1#PATCH:}" ;;
    *)        git -C "$REPO" show "$1:sdplane/fib.c" ;;
  esac
}

# pull one function out verbatim.  Absent is not an error: the revisions differ
# in which helpers exist (bit_index_key_init only shows up after the change).
extract_fn () {
  fib_c "$1" | awk -v want="$2" '
    {
      if (emit) { print; if ($0 == "}") { emit = 0; print "" }; next }
      if ($0 ~ "^" want " \\(") { emit = 1; if (prev != "") print prev; print; next }
      if ($0 != "") prev = $0
    }'
}

extract_define () {
  fib_c "$1" | grep -E "^#define $2\\b" || true
}

# rename every symbol of the group at once, so the walks of two revisions can
# live in one translation unit without colliding
prefix_names () {
  local pfx="$1"; shift
  local expr=""
  for n in "$@"; do expr="$expr;s/\\b$n\\b/$pfx$n/g"; done
  sed "${expr#;}"
}

WALK_SYMS="BIT_INDEX_KEY_SIZE bit_index_key_init BIT_INDEX _lookup fib_route_lookup"

# The whole walk of one revision, verbatim, under one prefix.  What is measured
# is therefore the code as committed -- not a hand-written approximation of it.
emit_walk () {
  local rev="$1" prefix="$2"
  {
    extract_define "$rev" BIT_INDEX_KEY_SIZE
    echo
    for fn in bit_index_key_init BIT_INDEX _lookup fib_route_lookup; do
      extract_fn "$rev" "$fn"
    done
  } | prefix_names "$prefix" $WALK_SYMS
}

{
  cat <<'PRELUDE'
/* Does removing the per-level memcpy from BIT_INDEX pay, and is it the same
 * function afterwards?
 *
 *   gcc -O2 -o bitindex-bench bitindex-bench.c && ./bitindex-bench
 *
 * Self-contained: no DPDK, no sdplane, no root.  Hardware counters come from
 * perf_event_open(2) with exclude_kernel, which needs only the default
 * perf_event_paranoid=2; without them the program still prints ns.
 *
 * The trie and both BIT_INDEX variants below are copied VERBATIM out of
 * sdplane/fib.c at the two revisions named in the banner, with an old_/new_
 * prefix so the two can run in the same process.  Nothing else was edited.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>

/* --- from sdplane/rib.h and fib.h, verbatim ---------------------------- */
#define K                     2
#define BRANCH_SZ             (1 << K)
#define KEY_SIZE(len)         (((len) + 7) / 8)

struct fib_node
{
  int leaf;
  union
  {
    struct in_addr ipv4_addr;
    struct in6_addr ipv6_addr;
  } key_ip_addr;
  int keylen;
  int sdplane_nh_id;
  uint32_t table_id;
  struct fib_node *child[BRANCH_SZ];
};

struct fib_tree
{
  int family;
  int table_id;
  struct fib_node *root;
};

/* The real DEBUG_SDPLANE_LOG ends in a test against a global.  Kept as one so
 * the debug-off branch stays in the measurement instead of being folded away. */
uint64_t bench_debug_config = 0;
static void bench_sink (const char *fmt, ...) { (void) fmt; }
#define DEBUG_SDPLANE_LOG(cat, fmt, ...)                                      \
  do { if (bench_debug_config) bench_sink (fmt); } while (0)

PRELUDE

  echo "/* ============ from $(rev_label "$OLD_REV"), prefix old_ ============ */"
  echo
  emit_walk "$OLD_REV" old_
  echo "/* ====== from $(rev_label "$NOCOPY_REV"), prefix nocopy_ ======"
  echo "   No copy at all: the two bounds cases are handled by a branch on every"
  echo "   level instead.  Measured and not adopted -- see the table. */"
  echo
  emit_walk "$NOCOPY_REV" nocopy_
  echo "/* ============ from $(rev_label "$NEW_REV"), prefix new_ ============ */"
  echo
  emit_walk "$NEW_REV" new_

  # umeda's version has no _lookup at all -- it is folded into fib_route_lookup
  # as a while loop, with the copy hoisted to run once per lookup instead of
  # once per level.  So only the one function comes across.
  echo "/* ====== from $(rev_label "$UMEDA_REV"), prefix umeda_ ======"
  echo "   loop instead of recursion, copy hoisted out of the walk.  Never merged;"
  echo "   the branch was deleted from the remote. */"
  echo
  emit_walk "$UMEDA_REV" umeda_

  cat <<'BOTH'
/* ====== diagnostic: keep the per-level store, move the loads away ========= */
/* old_ stores 16 bytes and then reads 2 of them back, every level.  Two things
   are bundled there: the store itself, and reading what was just stored.  This
   keeps the store per level but takes the index from a buffer nothing writes
   to, so the two can be told apart.
     ~ old_ speed  -> the store itself is what costs
     ~ new_ speed  -> the store is cheap; the coupling to the load is what costs
   The empty asm with "+m" stops the compiler from deleting the store as dead or
   hoisting it out of the loop, without the memory clobber that would also force
   the reads to reload.  Not a candidate implementation. */

struct farload_buf { uint8_t b[new_BIT_INDEX_KEY_SIZE]; };

static struct fib_node *
farload_fib_route_lookup (struct fib_tree *t, const uint8_t *key)
{
  struct fib_node *n = t->root;
  struct fib_node *cand = NULL;
  int cur_plen = 0;
  struct farload_buf scratch, readbuf;

  new_bit_index_key_init (readbuf.b, key);
  while (n)
    {
      if (n->leaf)
        cand = n;
      new_bit_index_key_init (scratch.b, key);
      __asm__ volatile ("" : "+m" (scratch));
      n = n->child[new_BIT_INDEX (readbuf.b, cur_plen, K)];
      cur_plen += K;
    }
  return cand;
}

/* ====== loop + the discarded copy-free index ============================ */
/* Written here, not taken from any revision.  It exists to answer one question:
   does turning the recursion into a loop buy anything on top of the index
   change?  If this matches nocopy_ exactly, gcc was already doing it. */

static struct fib_node *
nocopyloop_fib_route_lookup (struct fib_tree *t, const uint8_t *key)
{
  struct fib_node *n = t->root;
  struct fib_node *cand = NULL;
  int cur_plen = 0;

  while (n)
    {
      if (n->leaf)
        cand = n;
      n = n->child[nocopy_BIT_INDEX (key, cur_plen, K)];
      cur_plen += K;
    }
  return cand;
}

BOTH
  echo "/* ============ tree construction (identical on both revisions) ========= */"
  echo
  extract_fn "$OLD_REV" _count_nonzero
  extract_fn "$OLD_REV" _create_fib_node
  extract_fn "$OLD_REV" _add | sed "s/\\bBIT_INDEX\\b/old_BIT_INDEX/"
  extract_fn "$OLD_REV" fib_route_add
  extract_fn "$OLD_REV" _free_fib_node
  extract_fn "$OLD_REV" fib_free
  extract_fn "$OLD_REV" fib_new

  cat <<'MAIN'
/* --- hardware counters -------------------------------------------------- */
enum { C_CYCLES, C_INSNS, C_BRANCHES, C_BRMISS, C_N };
static int cfd[C_N] = { -1, -1, -1, -1 };

static int
pmu_open (void)
{
  static const uint64_t cfg[C_N] = {
    PERF_COUNT_HW_CPU_CYCLES, PERF_COUNT_HW_INSTRUCTIONS,
    PERF_COUNT_HW_BRANCH_INSTRUCTIONS, PERF_COUNT_HW_BRANCH_MISSES
  };
  int leader = -1;
  for (int i = 0; i < C_N; i++)
    {
      struct perf_event_attr a;
      memset (&a, 0, sizeof (a));
      a.type = PERF_TYPE_HARDWARE;
      a.size = sizeof (a);
      a.config = cfg[i];
      a.disabled = (i == 0);
      a.exclude_kernel = 1;     /* required at perf_event_paranoid >= 1 */
      a.exclude_hv = 1;
      cfd[i] = (int) syscall (__NR_perf_event_open, &a, 0, -1,
                              i == 0 ? -1 : leader, 0);
      if (cfd[i] < 0)
        return -1;
      if (i == 0)
        leader = cfd[0];
    }
  return 0;
}

static void pmu_start (void)
{
  ioctl (cfd[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
  ioctl (cfd[0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
}

static void pmu_stop (double out[C_N])
{
  ioctl (cfd[0], PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
  for (int i = 0; i < C_N; i++)
    {
      uint64_t v = 0;
      out[i] = (read (cfd[i], &v, sizeof (v)) == sizeof (v)) ? (double) v : -1.0;
    }
}

/* --- the measurement ---------------------------------------------------- */
#define ITER    100000
#define BEST_OF 7

static double now_ns (void)
{
  struct timespec ts;
  clock_gettime (CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1e9 + ts.tv_nsec;
}

static volatile uintptr_t sink;
static int have_pmu;

#define MEASURE(label, tree, key, FN)                                         \
  do                                                                          \
    {                                                                         \
      double best_ns = 1e18, best[C_N] = { 0, 0, 0, 0 };                      \
      for (int rep = 0; rep < BEST_OF; rep++)                                 \
        {                                                                     \
          if (have_pmu) pmu_start ();                                         \
          double t0 = now_ns ();                                              \
          for (int i = 0; i < ITER; i++)                                      \
            sink += (uintptr_t) FN ((tree), (key));                           \
          double t1 = now_ns ();                                              \
          double c[C_N] = { -1, -1, -1, -1 };                                 \
          if (have_pmu) pmu_stop (c);                                         \
          if ((t1 - t0) / ITER < best_ns)                                     \
            {                                                                 \
              best_ns = (t1 - t0) / ITER;                                     \
              for (int k = 0; k < C_N; k++) best[k] = c[k] / ITER;            \
            }                                                                 \
        }                                                                     \
      printf ("  %-42s %8.1f %9.1f %9.1f %9.1f %9.2f\n", label, best_ns,      \
              best[C_INSNS], best[C_CYCLES], best[C_BRANCHES], best[C_BRMISS]);\
    }                                                                         \
  while (0)

static struct fib_tree *
build (int n_routes, const struct in6_addr *host)
{
  struct fib_tree *t = fib_new (NULL);
  t->family = AF_INET6;
  fib_route_add (t, (const uint8_t *) host, 128, 1, 254);
  for (int i = 0; i < n_routes; i++)
    {
      uint8_t key[16] = { 0 };
      key[0] = 0x20; key[1] = 0x01;
      key[2] = (uint8_t) (i >> 16); key[3] = (uint8_t) (i >> 8);
      key[4] = (uint8_t) i; key[5] = (uint8_t) (i * 31);
      fib_route_add (t, key, 64, 2 + i, 254);
    }
  return t;
}

/* One variant, one process, nothing else -- so `perf stat` can attribute an
 * event to it.  The in-process counters below cannot: gcc inlines the thin
 * wrappers (old_, nocopy_) into main() while leaving the ones that hold a local
 * array (new_, umeda_) as calls, so a per-symbol profile of the full run mixes
 * some variants into main and separates others.  That asymmetry is invisible in
 * the numbers the MEASURE macro reports, because those bracket the loop itself.
 *
 *   ./bitindex-bench isolate <variant> [reps]
 *   variant: old | new | umeda | nocopy | nocopyloop
 */
static int
isolate (const char *which, long reps)
{
  struct in6_addr host;
  struct fib_tree *t;
  uintptr_t acc = 0;

  inet_pton (AF_INET6, "2001:db8:ace:d004::1", &host);
  t = fib_new (NULL);
  t->family = AF_INET6;
  fib_route_add (t, (const uint8_t *) &host, 128, 1, 254);
  for (int i = 0; i < 4096; i++)
    {
      uint8_t key[16] = { 0 };
      key[0] = 0x20; key[1] = 0x01;
      key[2] = (uint8_t) (i >> 16); key[3] = (uint8_t) (i >> 8);
      key[4] = (uint8_t) i; key[5] = (uint8_t) (i * 31);
      fib_route_add (t, key, 64, 2 + i, 254);
    }

  const uint8_t *k = (const uint8_t *) &host;
  /* The call is loop-invariant -- same tree, same key -- so gcc hoists the whole
     lookup out and leaves an empty loop.  Measured before this barrier was
     added: 10.4 instructions per "lookup".  Making the tree pointer opaque each
     iteration is enough to stop it. */
#define ISOLATE_RUN(name, fn)                                                 \
  if (! strcmp (which, name))                                                 \
    {                                                                         \
      for (long i = 0; i < reps; i++)                                         \
        {                                                                     \
          __asm__ volatile ("" : "+r" (t));                                  \
          acc += (uintptr_t) fn (t, k);                                       \
        }                                                                     \
    }                                                                         \
  else
  ISOLATE_RUN ("old", old_fib_route_lookup)
  ISOLATE_RUN ("new", new_fib_route_lookup)
  ISOLATE_RUN ("umeda", umeda_fib_route_lookup)
  ISOLATE_RUN ("nocopy", nocopy_fib_route_lookup)
  ISOLATE_RUN ("nocopyloop", nocopyloop_fib_route_lookup)
  ISOLATE_RUN ("farload", farload_fib_route_lookup)
    { fprintf (stderr, "unknown variant: %s\n", which); return 2; }
#undef ISOLATE_RUN

  fprintf (stderr, "%s: %ld lookups (acc %lu)\n", which, reps,
           (unsigned long) acc);
  return 0;
}

int
main (int argc, char **argv)
{
  struct in6_addr host, covered;

  if (argc >= 3 && ! strcmp (argv[1], "isolate"))
    return isolate (argv[2], argc > 3 ? atol (argv[3]) : 20000000L);

  inet_pton (AF_INET6, "2001:db8:ace:d004::1", &host);     /* a /128 route */
  inet_pton (AF_INET6, "2001:db8:7::10", &covered);        /* inside a /64 */

  /* 1. are they the same function?  every s the walk can reach, over random
        keys, plus the uniform keys at both ends of the range. */
  unsigned long checked = 0, bad = 0;
  srandom (1);
  for (int trial = 0; trial < 200000; trial++)
    {
      uint8_t key[16], padded[new_BIT_INDEX_KEY_SIZE];
      for (int i = 0; i < 16; i++) key[i] = (uint8_t) (random () & 0xff);
      new_bit_index_key_init (padded, key);
      for (int s = 0; s <= 128; s += K)
        { checked += 2;
          /* old_ reads the raw key and pads on the spot; new_ is handed the
             padded copy.  Same answer is the whole contract. */
          if (old_BIT_INDEX (key, s, K) != new_BIT_INDEX (padded, s, K)) bad++;
          if (old_BIT_INDEX (key, s, K) != nocopy_BIT_INDEX (key, s, K)) bad++; }
    }
  for (int v = 0; v <= 255; v++)
    {
      uint8_t key[16], padded[new_BIT_INDEX_KEY_SIZE];
      memset (key, v, 16);
      new_bit_index_key_init (padded, key);
      for (int s = 0; s <= 128; s += K)
        { checked += 2;
          if (old_BIT_INDEX (key, s, K) != new_BIT_INDEX (padded, s, K)) bad++;
          if (old_BIT_INDEX (key, s, K) != nocopy_BIT_INDEX (key, s, K)) bad++; }
    }
  printf ("BIT_INDEX equivalence: %lu comparisons, %lu mismatches -> %s\n\n",
          checked, bad, bad ? "FAIL" : "IDENTICAL");
  if (bad)
    return 1;

  have_pmu = (pmu_open () == 0);
  if (! have_pmu)
    fprintf (stderr, "(perf_event_open: %s; ns only)\n\n", strerror (errno));

  static const int sizes[] = { 1, 16, 256, 4096 };
  for (unsigned si = 0; si < sizeof (sizes) / sizeof (sizes[0]); si++)
    {
      struct fib_tree *t = build (sizes[si], &host);

      /* the same tree must answer the same, walked either way */
      const uint8_t *keys[2] = { (const uint8_t *) &host, (const uint8_t *) &covered };
      for (int k = 0; k < 2; k++)
        {
          struct fib_node *r = old_fib_route_lookup (t, keys[k]);
          if (new_fib_route_lookup (t, keys[k]) != r
              || umeda_fib_route_lookup (t, keys[k]) != r
              || nocopy_fib_route_lookup (t, keys[k]) != r
              || nocopyloop_fib_route_lookup (t, keys[k]) != r)
            { printf ("!! lookups disagree at %d routes, key %d\n", sizes[si], k);
              return 1; }
        }

      printf ("%d routes + one /128\n", sizes[si]);
      printf ("  %-42s %8s %9s %9s %9s %9s\n", "", "ns", "insns", "cycles",
              "branches", "br-miss");
      MEASURE ("/128  copy per level        (old_)", t, (const uint8_t *) &host, old_fib_route_lookup);
      MEASURE ("/128  copy per lookup       (new_)", t, (const uint8_t *) &host, new_fib_route_lookup);
      MEASURE ("/128  copy per lookup, loop (umeda_)", t, (const uint8_t *) &host, umeda_fib_route_lookup);
      MEASURE ("/128  no copy, branch/level (nocopy_)", t, (const uint8_t *) &host, nocopy_fib_route_lookup);
      MEASURE ("/128  no copy, loop         (nocopyloop_)", t, (const uint8_t *) &host, nocopyloop_fib_route_lookup);
      MEASURE ("/128  store/level, far loads (farload_)", t, (const uint8_t *) &host, farload_fib_route_lookup);
      MEASURE ("/64   copy per level        (old_)", t, (const uint8_t *) &covered, old_fib_route_lookup);
      MEASURE ("/64   copy per lookup       (new_)", t, (const uint8_t *) &covered, new_fib_route_lookup);
      MEASURE ("/64   copy per lookup, loop (umeda_)", t, (const uint8_t *) &covered, umeda_fib_route_lookup);
      MEASURE ("/64   no copy, branch/level (nocopy_)", t, (const uint8_t *) &covered, nocopy_fib_route_lookup);
      MEASURE ("/64   no copy, loop         (nocopyloop_)", t, (const uint8_t *) &covered, nocopyloop_fib_route_lookup);
      printf ("\n");
      fib_free (t);
    }
  return 0;
}
MAIN
} > "$OUT"

echo "wrote $OUT ($(grep -c '' "$OUT") lines)"
