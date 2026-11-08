#!/bin/bash
# Build the sdplane .deb package of sdplane-oss inside the build container.
# libsdplane is bundled in lib/, so no separate libsdplane package is needed.
# Invoked by build-debian.sh.

set -euo pipefail

. /etc/os-release
out=/out/ubuntu${VERSION_ID}
mkdir -p "$out"

src=/src/sdplane-oss
dir=/build/sdplane-oss
git clone -q "$src" "$dir"
cd "$dir"
git checkout -q "${SDPLANE_REF:-HEAD}"
tools/deb/gen-changelog.sh
# the build rewrites these tracked files (INSTALL: autoreconf -fi); hide the
# changes so that git describe --dirty does not report -modified.
git update-index --assume-unchanged debian/changelog INSTALL
dpkg-buildpackage -b -us -uc

cp /build/*.deb "$out/"
ls -l "$out"
