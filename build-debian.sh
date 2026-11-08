#!/bin/bash
# Build release .deb packages in Ubuntu containers.
# usage: ./build-debian.sh [ubuntu-version ...]   (default: 24.04)
#   env: LIBSDPLANE_DIR (default: ../libsdplane-dev)
#        SDPLANE_REF, LIBSDPLANE_REF (default: HEAD)
# Only committed changes are built. Output: dist/ubuntu<version>/*.deb

set -euo pipefail
cd "$(dirname "$0")"

libsdplane=$(cd "${LIBSDPLANE_DIR:-../libsdplane-dev}" && pwd)

for v in "${@:-24.04}"; do
  docker build --platform linux/amd64 --build-arg UBUNTU_VERSION="$v" \
    -t sdplane-deb-build:"$v" tools/deb
  docker run --rm --platform linux/amd64 \
    -e SDPLANE_REF -e LIBSDPLANE_REF \
    -v "$PWD":/src/sdplane-dev:ro \
    -v "$libsdplane":/src/libsdplane-dev:ro \
    -v "$PWD/dist":/out \
    sdplane-deb-build:"$v" /src/sdplane-dev/tools/deb/build-in-container.sh
done
