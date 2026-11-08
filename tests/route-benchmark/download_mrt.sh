#!/bin/bash
# Download the MRT dump the benchmark takes its routes from (about 400 MB).
# Usage: ./download_mrt.sh [url] (default: the latest RIPE RIS rrc00 dump)
set -euo pipefail

dir=$(dirname -- "$(readlink -f -- "$0")")
url=${1:-https://data.ris.ripe.net/rrc00/latest-bview.gz}
out=$dir/data/bview.gz

if [[ -e $out ]]; then
    echo "$out already exists; delete it to download again" >&2
    exit 0
fi

mkdir -p "$dir/data"
curl -fL --retry 3 -o "$out.tmp" "$url"
mv "$out.tmp" "$out"
echo "saved $out"
