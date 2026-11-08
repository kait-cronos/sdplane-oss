#!/bin/bash
# Build release .deb packages of sdplane-oss in Ubuntu containers.
# libsdplane is bundled in lib/ and linked statically, so only one
# package (sdplane) is built.
#
# usage: ./build-debian.sh [ubuntu-version ...]   (default: 24.04)
#   env: SDPLANE_REF (default: HEAD)
# Only committed changes are built. Output: dist/ubuntu<version>/*.deb

set -euo pipefail
cd "$(dirname "$0")"

for v in "${@:-24.04}"; do
  docker build --platform linux/amd64 --build-arg UBUNTU_VERSION="$v" \
    -t sdplane-deb-build:"$v" tools/deb
  docker run --rm --platform linux/amd64 \
    -e SDPLANE_REF \
    -v "$PWD":/src/sdplane-oss:ro \
    -v "$PWD/dist":/out \
    sdplane-deb-build:"$v" /src/sdplane-oss/tools/deb/build-in-container.sh
done
