#!/bin/bash
# Run sdplane by hand in netns a, for debugging a config.
# Usage: [SDPLANE=path] ./run_sdplane.sh [config]
#   (default: ../../sdplane/sdplane, configs/rib-lcore-netlink-lcore.conf)
set -euo pipefail

dir=$(dirname -- "$(readlink -f -- "$0")")
conf=${1:-$dir/configs/rib-lcore-netlink-lcore.conf}
sudo "$dir/setup_netns.py" up
exec sudo ip netns exec a "${SDPLANE:-$dir/../../sdplane/sdplane}" -f "$conf"
