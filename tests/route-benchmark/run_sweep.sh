#!/bin/bash
# Full-speed route loss sweep from 100 to 1,000,000 routes, 3 trials each.
# Takes about 50 minutes and writes more than 1 GB of logs.
# Usage: ./run_sweep.sh [measure_route_loss.py options...]
#   e.g. ./run_sweep.sh --conf configs/rib-lthread-netlink-lcore.conf
#        ./run_sweep.sh --counts 100 1000 --trials 1    # quick check
set -euo pipefail

dir=$(dirname -- "$(readlink -f -- "$0")")
uv=$(command -v uv) || { echo "uv not found: https://docs.astral.sh/uv/" >&2; exit 1; }
exec sudo "$uv" run "$dir/measure_route_loss.py" \
    --counts 100 300 1000 2000 3000 5000 10000 30000 100000 300000 1000000 \
    --trials 3 --settle-timeout 1800 "$@"
