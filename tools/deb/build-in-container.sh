#!/bin/bash
# Build libsdplane and sdplane .deb packages inside the build container.
# Invoked by build-debian.sh.

set -euo pipefail

. /etc/os-release
out=/out/ubuntu${VERSION_ID}
mkdir -p "$out"

build() {
  local src=$1 ref=$2
  local dir=/build/$(basename "$src")
  git clone -q "$src" "$dir"
  cd "$dir"
  git checkout -q "$ref"
  /src/sdplane-dev/tools/deb/gen-changelog.sh
  dpkg-buildpackage -b -us -uc
}

build /src/libsdplane-dev "${LIBSDPLANE_REF:-HEAD}"
apt-get install -y /build/libsdplane_*.deb
build /src/sdplane-dev "${SDPLANE_REF:-HEAD}"

cp /build/*.deb "$out/"
ls -l "$out"
