#!/bin/bash
# Pin the CPU clock so that the ns/pps columns of a benchmark mean something.
#
#   bash cpufreq-pin.sh show          # what the knobs say right now (no root)
#   sudo bash cpufreq-pin.sh sweep    # find the highest percentage that HOLDS
#   sudo bash cpufreq-pin.sh pin 40   # performance governor, clock fixed at 40%
#   bash cpufreq-pin.sh watch 30      # sample the clock under whatever is running
#   sudo bash cpufreq-pin.sh restore  # put back what pin saved
#
# Why a percentage and not "maximum": this box is an N100, whose base clock is
# 800 MHz and whose 3.4 GHz is entirely turbo.  Asking for the top of the range
# does not hold it there -- the package hits its power and thermal limits and
# the clock walks back down on its own, which is the variation we are trying to
# remove.  Asking for something the package can actually SUSTAIN holds still.
# Lower absolute numbers, comparable ones.
#
# Which percentage that is cannot be reasoned out from the datasheet -- it
# depends on this chassis and this room -- so `sweep` measures it: it pins each
# candidate in turn, loads every core, and reports whether the clock stayed
# flat.  Measured on gmkhost (N100, 2026-08-31), all four cores busy:
#
#   70% (2380 MHz asked)  2100-2400 MHz, 96 C   the pin does not hold
#   60% (2040 MHz asked)  2100 MHz flat,  95 C   holds, no thermal headroom
#   50% (1700 MHz asked)  1700 MHz flat,  85 C   holds
#   40% (1360 MHz asked)  1400 MHz flat,  79 C   holds
#
# The percentage is of the TURBO maximum (3400 MHz), rounded to a 100 MHz
# P-state -- 50% lands on 1700 MHz exactly.
#
# Only the ns/pps columns benefit.  Instruction counts are deterministic and
# cycle counts are per-cycle by definition, so neither moves with the clock.
set -euo pipefail

PSTATE=/sys/devices/system/cpu/intel_pstate
STATE_FILE="${CPUFREQ_PIN_STATE:-/var/tmp/cpufreq-pin.state}"

governors () { echo /sys/devices/system/cpu/cpu[0-9]*/cpufreq/scaling_governor; }

show () {
  echo "governor : $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor)"
  if [[ -d $PSTATE ]]; then
    echo "driver   : intel_pstate ($(cat $PSTATE/status))"
    echo "perf_pct : min $(cat $PSTATE/min_perf_pct) / max $(cat $PSTATE/max_perf_pct)" \
         " no_turbo $(cat $PSTATE/no_turbo)"
  else
    echo "driver   : not intel_pstate -- pin/restore do not apply"
  fi
  local mx
  mx=$(cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq)
  echo "range    : $(cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq) .. $mx kHz" \
       " (base $(cat /sys/devices/system/cpu/cpu0/cpufreq/base_frequency 2>/dev/null || echo '?'))"
  printf 'now      :'
  for c in /sys/devices/system/cpu/cpu[0-9]*/cpufreq/scaling_cur_freq; do
    printf ' %s' "$(cat "$c")"
  done
  echo " kHz"
  echo "pkg temp : $(cat /sys/class/thermal/thermal_zone1/temp 2>/dev/null || echo '?')"
  [[ -f $STATE_FILE ]] && echo "saved    : $(cat "$STATE_FILE")" || true
}

need_root () {
  [[ $EUID -eq 0 ]] || { echo "!! $1 needs root: sudo bash $0 $*" >&2; exit 1; }
  [[ -d $PSTATE ]]  || { echo "!! this box is not on intel_pstate" >&2; exit 1; }
}

save_once () {
  # Only once: pinning twice must not record the pinned values as the thing to
  # restore.  sweep pins many times, so this matters more than it looks.
  [[ -f $STATE_FILE ]] && return 0
  printf 'governor=%s min_perf_pct=%s max_perf_pct=%s no_turbo=%s\n' \
    "$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor)" \
    "$(cat $PSTATE/min_perf_pct)" "$(cat $PSTATE/max_perf_pct)" \
    "$(cat $PSTATE/no_turbo)" > "$STATE_FILE"
}

apply_pct () {
  local pct="$1" g got_min got_max
  for g in $(governors); do echo performance > "$g"; done

  # Order matters in BOTH directions, and getting it wrong is silent.  The
  # driver keeps min <= max by CLAMPING whichever write would cross the other
  # -- it does not report an error.  Writing max first works while climbing and
  # fails while descending (the new ceiling gets stuck at the old, higher
  # floor); writing min first fails the other way round.  So drop the floor out
  # of the way, set the ceiling, then bring the floor back up to meet it, which
  # is correct regardless of where we came from.  20 is below the driver's own
  # floor (min_pstate/turbo_pstate = 700/3400), so it clamps rather than moving
  # the clock anywhere it was not already allowed to be.
  echo 20 > $PSTATE/min_perf_pct
  echo "$pct" > $PSTATE/max_perf_pct
  echo "$pct" > $PSTATE/min_perf_pct

  # Read back.  The first version of this script did not, and reported a whole
  # sweep whose percentages were one step below what it had actually measured.
  got_min=$(cat $PSTATE/min_perf_pct); got_max=$(cat $PSTATE/max_perf_pct)
  if [[ "$got_min" != "$pct" || "$got_max" != "$pct" ]]; then
    echo "!! asked ${pct}%, driver reports min ${got_min}% / max ${got_max}%" >&2
    return 1
  fi
}

valid_pct () {
  [[ "$1" =~ ^[0-9]+$ ]] && (( $1 >= 20 && $1 <= 100 )) \
    || { echo "!! percentage must be 20..100, got '$1'" >&2; exit 1; }
}

pin () {
  local pct="${1:-40}"
  need_root pin
  valid_pct "$pct"
  save_once
  apply_pct "$pct"
  echo "pinned at ${pct}%"
  show
}

restore () {
  need_root restore
  local gov=powersave min=20 max=100 turbo=0
  if [[ -f $STATE_FILE ]]; then
    # shellcheck disable=SC2046  # the file is four key=value words we wrote
    eval $(cat "$STATE_FILE")
    gov="$governor" min="$min_perf_pct" max="$max_perf_pct" turbo="$no_turbo"
  else
    echo "(no saved state; falling back to the stock powersave 20..100)" >&2
  fi
  echo "$max" > $PSTATE/max_perf_pct
  echo "$min" > $PSTATE/min_perf_pct
  echo "$turbo" > $PSTATE/no_turbo
  for g in $(governors); do echo "$gov" > "$g"; done
  rm -f "$STATE_FILE"
  show
}

LOAD_PIDS=()

load_start () {
  local i
  LOAD_PIDS=()
  for ((i = 0; i < $(nproc); i++)); do
    ( while :; do :; done ) &
    LOAD_PIDS+=($!)
  done
}

load_stop () {
  (( ${#LOAD_PIDS[@]} )) || return 0
  kill "${LOAD_PIDS[@]}" 2>/dev/null || true
  wait "${LOAD_PIDS[@]}" 2>/dev/null || true
  LOAD_PIDS=()
}

# Leaving spinners behind would quietly poison every measurement taken after
# this script, which is the failure mode hardest to notice.
trap load_stop EXIT INT TERM

# Which percentage actually holds is a property of this chassis, not of the
# part number, so measure it.  The verdict is read off the SPREAD rather than
# off the distance to a requested frequency: the driver rounds the request to a
# P-state and does not tell us to which one, whereas a clock that is being held
# is flat by definition and one that is being throttled is not.
sweep () {
  need_root sweep
  local pcts="${*:-70 60 50 40 30}" pct t vals temps tmax
  local settle="${SWEEP_SETTLE:-4}" sample="${SWEEP_SAMPLE:-10}"
  save_once
  for pct in $pcts; do valid_pct "$pct"; done

  local hwmax asked
  hwmax=$(cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq)

  echo "per point: pin, load all $(nproc) cores, discard ${settle}s, sample ${sample}s"
  echo "asked  = pct x ${hwmax} kHz, before the driver rounds it to a P-state"
  echo "HOLD   = the clock stayed within 2% of itself for the whole sample"
  echo
  printf '%5s %8s %9s %9s %9s %8s %8s  %s\n' \
         pct asked min avg max spread tempmax verdict
  for pct in $pcts; do
    apply_pct "$pct"
    asked=$(( pct * hwmax / 100 / 1000 ))
    load_start
    sleep "$settle"
    vals=() temps=()
    for ((t = 0; t < sample; t++)); do
      vals+=($(cat /sys/devices/system/cpu/cpu[0-9]*/cpufreq/scaling_cur_freq))
      temps+=("$(cat /sys/class/thermal/thermal_zone1/temp 2>/dev/null || echo 0)")
      sleep 1
    done
    load_stop
    tmax=$(printf '%s\n' "${temps[@]}" | sort -n | tail -1)
    printf '%s\n' "${vals[@]}" | awk -v pct="$pct" -v asked="$asked" -v tmax="$tmax" '
      { s += $1; n++; if (!min || $1 < min) min = $1; if ($1 > max) max = $1 }
      END { avg = s / n; spread = (max - min) / avg * 100
            printf "%4s%% %7dM %8.0fM %8.0fM %8.0fM %7.1f%% %7.1fC  %s\n",
                   pct, asked, min/1000, avg/1000, max/1000, spread, tmax/1000,
                   (spread <= 2 ? "HOLD" : "drifts") }'
  done
  echo
  echo "left pinned at the last point above; pin the highest HOLD with:"
  echo "  sudo bash $0 pin <pct>"
}

# Sample rather than trust: min_perf_pct is a REQUEST.  The package can still
# fall below it on a power or thermal limit, and that is exactly the case a
# reader needs to see, because it means the pin did not hold and the ns column
# is still moving underneath the measurement.
watch_freq () {
  local secs="${1:-30}" i
  echo "sampling every 1s for ${secs}s -- run the load in another shell"
  printf '%5s  %-40s %s\n' t 'per-cpu kHz' 'temp'
  for ((i = 0; i < secs; i++)); do
    printf '%5d  %-40s %s\n' "$i" \
      "$(cat /sys/devices/system/cpu/cpu[0-9]*/cpufreq/scaling_cur_freq | tr '\n' ' ')" \
      "$(cat /sys/class/thermal/thermal_zone1/temp 2>/dev/null || echo '?')"
    sleep 1
  done
}

case "${1:-show}" in
  show)    show ;;
  pin)     pin "${2:-40}" ;;
  sweep)   shift; sweep "$@" ;;
  restore) restore ;;
  watch)   watch_freq "${2:-30}" ;;
  *) echo "usage: $0 {show|sweep [pct...]|pin [pct]|watch [secs]|restore}" >&2; exit 1 ;;
esac
