#!/bin/bash
# Prepend a debian/changelog entry generated from the git history.
# Run at the top of the source tree. The result is not committed.

set -euo pipefail

pkg=$(sed -n 's/^Source: *//p' debian/control)
maint=$(sed -n 's/^Maintainer: *//p' debian/control)

# annotated tag only: e.g., v0.1.3-943-g1234abc
desc=$(git describe --long --match 'v[0-9]*')
upstream=${desc%%-*}
upstream=${upstream#v}
ncommit=$(sed -E 's/^.*-([0-9]+)-g[0-9a-f]+$/\1/' <<< "$desc")

. /etc/os-release
debversion="${upstream}-${ncommit}~ubuntu${VERSION_ID}"

# previous release: lightweight tag like v0.1.3-683.
prev=$(git describe --tags --abbrev=0 --match 'v[0-9]*-[0-9]*' HEAD^ \
         2>/dev/null || true)
log=$(git --no-pager log --no-merges --format='  * %s' ${prev:+$prev..}HEAD)

{
  echo "$pkg ($debversion) $VERSION_CODENAME; urgency=low"
  echo
  echo "${log:-  * rebuild.}"
  echo
  echo " -- $maint  $(git log -1 --format=%cD)"
  echo
  cat debian/changelog
} > debian/changelog.new
mv debian/changelog.new debian/changelog

echo "$pkg $debversion (previous release: ${prev:-none})"
