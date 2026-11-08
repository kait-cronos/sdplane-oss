#!/bin/bash
# Generate the benchmark, build it, and run it pinned to one core -- the whole
# measurement in one command, so the recorded numbers can be reproduced without
# reconstructing the invocation by hand.
#
#   bash run.sh                        # the pinned defaults in make-bench.sh
#   bash run.sh <old> <new> <umeda>    # compare other revisions
#
# Writes bitindex-bench.txt, whose header records what the machine was doing at
# the time.  That header is the point: on a shared desktop the ns column moves
# with the temperature and with whatever else is running, so a reader has to be
# able to see the conditions rather than take the numbers on faith.
#
# No DPDK, no sdplane, no root.
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$HERE"
# Locate the sdplane-dev checkout WITHOUT assuming where this script sits.
#
# The obvious "$HERE/../../../.." is right only while crazy_conf is checked out
# at build/crazy_conf, and when it is wrong it still resolves to *some*
# directory -- so the mistake is silent.  `git rev-parse --show-toplevel` does
# not help either: crazy_conf is its own repository, so from here it answers
# with crazy_conf rather than with sdplane-dev.
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
  echo "!! cannot find an sdplane-dev checkout at or above $HERE" >&2
  echo "   (looked for sdplane/fib.c + configure.ac)" >&2
  echo "   set SDPLANE_REPO=/path/to/sdplane-dev and re-run" >&2
  exit 1
}

bash ./make-bench.sh "$@"

# -O2 only: this measures a leaf routine whose cost is a pipeline stall, and the
# stall is what -O0 buries under everything else being slow.  (The sibling
# benches quote both levels because they measure code whose instruction count is
# the story; here it is not -- see README.md.)
# -Wno-unused-variable: the trie is copied verbatim out of sdplane/fib.c and has
# an unused local in _create_fib_node().  Editing extracted code would defeat
# the point of extracting it.
gcc -O2 -g -Wall -Wextra -Wno-unused-variable -o bitindex-bench bitindex-bench.c

CPU="${BENCH_CPU:-2}"
PERF_REPS="${PERF_REPS:-2000000}"
command -v taskset > /dev/null && PIN=(taskset -c "$CPU") || PIN=()

# what the banner reports is read out of the generated file, not repeated here,
# so the two cannot disagree about which revisions were measured
REVS=$(grep -oE 'from .*, prefix [a-z_]+' bitindex-bench.c | sed -e 's/^from /  /' -e 's/, prefix / -> /')

{
  echo "# bitindex-bench"
  echo "# date      : $(date -Is)"
  echo "# cpu       : $(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2- | sed 's/^ //')"
  echo "# pinned to : cpu $CPU"
  echo "# governor  : $(cat /sys/devices/system/cpu/cpu$CPU/cpufreq/scaling_governor 2>/dev/null || echo unknown)"
  echo "# freq now  : $(cat /sys/devices/system/cpu/cpu$CPU/cpufreq/scaling_cur_freq 2>/dev/null || echo '?') kHz" \
       "of max $(cat /sys/devices/system/cpu/cpu$CPU/cpufreq/cpuinfo_max_freq 2>/dev/null || echo '?')"
  # Whether the clock was PINNED cannot be read off the two lines above: a
  # governor plus one instantaneous sample is equally consistent with a clock
  # that is about to move.  min == max is what says it cannot.  See
  # ../cpufreq-pin.sh, which sets them and measures which value actually holds.
  if [[ -r /sys/devices/system/cpu/intel_pstate/min_perf_pct ]]; then
    pmin=$(cat /sys/devices/system/cpu/intel_pstate/min_perf_pct)
    pmax=$(cat /sys/devices/system/cpu/intel_pstate/max_perf_pct)
    [[ "$pmin" == "$pmax" ]] \
      && echo "# clock     : perf_pct $pmin == $pmax -- PINNED" \
      || echo "# clock     : perf_pct $pmin..$pmax -- NOT pinned, the ns column will wander"
  fi
  echo "# pkg temp  : $(cat /sys/class/thermal/thermal_zone1/temp 2>/dev/null || echo '?')"
  echo "# load      : $(cut -d' ' -f1-3 /proc/loadavg)"
  echo "# gcc       : $(gcc --version | head -1)"
  echo "# revisions measured:"
  echo "$REVS"
  echo
  "${PIN[@]}" ./bitindex-bench
  echo
  echo "# load after: $(cut -d' ' -f1-3 /proc/loadavg)   pkg temp: $(cat /sys/class/thermal/thermal_zone1/temp 2>/dev/null || echo '?')"
} | tee bitindex-bench.txt

# The disassembly is the evidence for "the recursion is already a loop" and for
# "only the store stayed in it", so it is regenerated alongside the numbers.
#
# Symbols are resolved by PATTERN, not by name: gcc decorates what it clones
# (old__lookup.constprop.0) and drops what it fully inlines -- new__lookup does
# not exist at all, its walk having been folded into new_fib_route_lookup.  A
# hardcoded name silently degrades into "no such symbol" the next time either
# decision changes.
walk_symbol () {
  local pfx="$1"
  nm bitindex-bench | awk -v p="$pfx" '
    $3 ~ ("^" p "_lookup(\\.|$)")          { print $3; found = 1; exit }
    END { if (!found) exit 1 }' && return 0
  nm bitindex-bench | awk -v p="$pfx" '
    $3 ~ ("^" p "fib_route_lookup(\\.|$)") { print $3; found = 1; exit }
    END { if (!found) exit 1 }'
}

{
  echo "# disassembly of the walks in bitindex-bench (-O2, $(gcc --version | head -1))"
  echo "# old_     store per level, and the loads read what was just stored"
  echo "# new_     the store is hoisted out; the loop only loads (this change)"
  echo "# farload_ DIAGNOSTIC: the store stays per level, but the loads read a"
  echo "#          different buffer.  Look at the offsets: the store goes to"
  echo "#          (%rsp), the loads come from 0x20(%rsp).  This runs at new_"
  echo "#          speed, which is what shows the cost is reading back what was"
  echo "#          just written -- not the store"
  echo "# nocopy_  no store at all, bounds handled by a branch per level"
  echo "# umeda_   like new_, hand-written loop -- included because 'is it"
  echo "#          actually different code?' is answered here, not by the 1%"
  echo "#          the cycle counter reports"
  echo "# (a walk with no _lookup symbol was inlined into its entry point)"
  for pfx in old_ new_ farload_ nocopy_ umeda_; do
    sym=$(walk_symbol "$pfx") || { echo; echo "!! $pfx: no walk symbol found"; continue; }
    line=$(nm -S bitindex-bench | awk -v s="$sym" '$4 == s { print $1, $2 }')
    [[ -n "$line" ]] || { echo; echo "!! $sym: no size in the symbol table"; continue; }
    a=${line%% *}; sz=${line##* }
    echo
    echo "=== $sym (size 0x$sz) ==="
    objdump -d --no-show-raw-insn \
            --start-address="0x$a" --stop-address="$((0x$a + 0x$sz))" bitindex-bench \
      | grep -E '^\s+[0-9a-f]+:' | sed 's/^ *//'
  done
} > disasm.txt

# Where the cycles actually go.  The in-process counters cannot answer this:
# gcc inlines the thin wrappers (old_, nocopy_) into main() and leaves the ones
# holding a local array (new_, umeda_) as calls, so a per-symbol perf profile of
# the full run mixes some variants together.  One variant per process fixes it.
if command -v perf > /dev/null; then
  {
    echo "# where the cycles go, one variant per process (perf stat, user only)"
    echo "# reps=$PERF_REPS lookups of the /128 host route, 4096-route trie"
    echo "#"
    echo "# stores, loads   retired memory uops per lookup.  old_ and farload_ must"
    echo "#                 match here: that is what rules out \"farload_ is only"
    echo "#                 faster because the compiler dropped the store\""
    echo "# ldhead_cyc      CYCLES (not events) the oldest load was stalled, any reason"
    echo "# stfwd_blk       loads blocked because an older store to the same address"
    echo "#                 had no data ready yet -- the store-forward FAILURE case"
    echo
    printf '%-12s %9s %9s %6s %8s %8s %9s %8s\n' \
           variant insns cycles IPC stores loads ldhead_cyc stfwd_blk
    for v in old new umeda nocopy nocopyloop farload; do
      perf stat -x, -e instructions:u,cycles:u,mem_uops_retired.all_stores:u,mem_uops_retired.all_loads:u,ld_head.any_at_ret:u,ld_blocks.data_unknown:u \
        "${PIN[@]}" ./bitindex-bench isolate "$v" "$PERF_REPS" 2>&1 >/dev/null \
      | awk -F, -v v="$v" -v r="$PERF_REPS" '
          /instructions/            { i = $1 }
          /cycles/                  { c = $1 }
          /all_stores/              { st = $1 }
          /all_loads/               { ld = $1 }
          /ld_head.any_at_ret/      { h = $1 }
          /ld_blocks.data_unknown/  { b = $1 }
          END { printf "%-12s %9.1f %9.1f %6.2f %8.1f %8.1f %9.1f %8.2f\n",
                       v, i/r, c/r, i/c, st/r, ld/r, h/r, b/r }'
    done
  } | tee perf-events.txt
else
  echo "(perf not installed; skipping perf-events.txt)" >&2
fi

echo
echo "wrote bitindex-bench.txt, disasm.txt and perf-events.txt"
