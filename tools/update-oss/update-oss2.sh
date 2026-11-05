#!/bin/bash

set -euo pipefail

# ---------------------------------------------------------------------------
# constants
# ---------------------------------------------------------------------------

SDPLANE_DEV_REPO="git@github.com:kait-cronos/sdplane-dev.git"
LIBSDPLANE_DEV_REPO="git@github.com:kait-cronos/libsdplane-dev.git"
# override with env to work on another repo (e.g. kait-cronos/sdplane-oss-candidate)
SDPLANE_OSS_REPO="${SDPLANE_OSS_REPO:-git@github.com:kait-cronos/sdplane-oss.git}"

BRANCH="$(date +%Y-%m-%d)"

FILES_TO_REMOVE=(
    ai-workspace
    CLAUDE.md
    TODO
    doc
    tools/update-oss
    tools/add-ai-comments.sh
    tools/ai-commit-helper.sh
    tools/ai-stats.sh
    tools/check-ai-branch.sh
    README.ja.md
    LICENSE.MIT
)

# build files that differ between sdplane-dev and sdplane-oss (lib/ is bundled
# in sdplane-oss). merges of these need manual care; see check_oss_build_files.
OSS_BUILD_FILES=(
    configure.ac
    Makefile.am
    lib/Makefile.am
    sdplane/Makefile.am
)

# ---------------------------------------------------------------------------
# logging
# ---------------------------------------------------------------------------

info()  { echo "[INFO]  $*"; }
error() { echo "[ERROR] $*" >&2; exit 1; }

# ---------------------------------------------------------------------------
# check val
# ---------------------------------------------------------------------------

if [ $# -lt 1 ] || [ $# -gt 2 ]; then
    echo "usage: $0 <workdir> [step1|step2|step3|step4]" >&2
    echo "  step1: clone sdplane-dev, sdplane-oss and create branch $BRANCH" >&2
    echo "  step2: merge lib/ from libsdplane-dev" >&2
    echo "  step3: merge sdplane-dev" >&2
    echo "  step4: check oss build files, remove internal-only files" >&2
    echo "  (no step: run all; stops at a merge conflict)" >&2
    exit 1
fi

WORKDIR="$(realpath "$1")"

# ---------------------------------------------------------------------------
# functions
# ---------------------------------------------------------------------------

pre_check() {
    command -v git-filter-repo >/dev/null 2>&1 \
        || error "git-filter-repo not found"

    mkdir -p "$WORKDIR"
}

setup_sdplane_oss() {
    cd "$WORKDIR"
    git clone "$SDPLANE_DEV_REPO"
    git clone "$SDPLANE_OSS_REPO"

    cd "$WORKDIR"
    cd sdplane-oss
    git checkout -b "$BRANCH"
}

merge_libsdplane() {
    cd "$WORKDIR"
    git clone "$LIBSDPLANE_DEV_REPO"

    cd libsdplane-dev
    git filter-repo --subdirectory-filter lib
    git filter-repo --to-subdirectory-filter lib

    cd "$WORKDIR"
    cd sdplane-oss
    git remote add libsdplane "$WORKDIR/libsdplane-dev"
    git fetch libsdplane

    # no --allow-unrelated-histories: sdplane-oss already shares history with
    # the filter-repo'ed libsdplane-dev. if this fails as "unrelated histories",
    # the filter-repo result has changed (e.g. filter-repo version); stop and check.
    git merge libsdplane/main \
        -m "integrate lib/ from libsdplane-dev" \
        || error "merge stopped. resolve conflicts, 'git commit', then run: $0 $WORKDIR step3"
}

merge_sdplane() {
    cd "$WORKDIR"
    cd sdplane-oss
    git remote add sdplane-dev "$WORKDIR/sdplane-dev"
    git fetch sdplane-dev

    # no --allow-unrelated-histories: sdplane-oss already shares history with
    # sdplane-dev.
    # NOTE: when resolving conflicts in OSS_BUILD_FILES, keep the oss side
    # (lib/ bundled: ../lib/libsdplane.la, -I$(LIBSDPLANE_DIR), SUBDIRS = lib sdplane)
    # and take only the new sources/options from sdplane-dev.
    git merge sdplane-dev/main \
        -m "integrate all from sdplane-dev" \
        || error "merge stopped. resolve conflicts (keep oss side of ${OSS_BUILD_FILES[*]}), 'git commit', then run: $0 $WORKDIR step4"
}

check_oss_build_files() {
    cd "$WORKDIR"
    cd sdplane-oss

    local ng=0
    must_have() {  # <file> <fixed-string>
        grep -qF -- "$2" "$1" || { echo "[NG]    $1: missing '$2'" >&2; ng=1; }
    }
    must_not_have() {  # <file> <fixed-string>
        ! grep -qF -- "$2" "$1" || { echo "[NG]    $1: should not have '$2'" >&2; ng=1; }
    }

    if grep -nE '^(<<<<<<<|=======|>>>>>>>)( |$)' "${OSS_BUILD_FILES[@]}"; then
        echo "[NG]    conflict markers left" >&2
        ng=1
    fi

    must_have     Makefile.am         'SUBDIRS = lib sdplane'
    must_have     configure.ac        'lib/Makefile'
    must_have     configure.ac        'LT_INIT'
    must_have     configure.ac        'HAVE_SDPLANE_LIBSDPLANE_VERSION_H'
    must_have     sdplane/Makefile.am '../lib/libsdplane.la'
    must_have     sdplane/Makefile.am '-I$(LIBSDPLANE_DIR)'
    must_not_have sdplane/Makefile.am '-L$(libsdplanedir)'

    [ $ng -eq 0 ] || error "oss build files look wrong (dev side taken in merge?). fix them, commit, and rerun step4."

    # for review: differences from the reference oss build files
    local ref="$WORKDIR/sdplane-dev/tools/update-oss/oss-build"
    if [ -d "$ref" ]; then
        local f
        for f in "${OSS_BUILD_FILES[@]}"; do
            if ! diff -q "$ref/$f" "$f" >/dev/null; then
                info "$f differs from oss-build/$f (review; update oss-build/ in sdplane-dev if the change is right):"
                diff -u "$ref/$f" "$f" || true
            fi
        done
    fi
}

check_license_headers() {
    cd "$WORKDIR"
    cd sdplane-oss

    # source files newly added since the last oss release need the GPL header
    local f
    git diff --name-only --diff-filter=A origin/main HEAD -- \
        'sdplane/*.[ch]' 'lib/*.[ch]' 'module/*.[ch]' |
    while read -r f; do
        head -5 "$f" | grep -q 'SPDX-License-Identifier' \
            || echo "[WARN]  no SPDX-License-Identifier: $f" >&2
    done
}

remove_internal_files() {
    cd "$WORKDIR"
    cd sdplane-oss
    git rm -rf --ignore-unmatch "${FILES_TO_REMOVE[@]}"
    git diff --cached --quiet || git commit -m "remove internal-only files for oss release"

    # temporary remotes used for the merges
    git remote remove libsdplane 2>/dev/null || true
    git remote remove sdplane-dev 2>/dev/null || true
}

# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------

step1() {
    echo "============================================"
    echo " sdplane-oss integration script: step 1"
    echo "============================================"

    pre_check
    setup_sdplane_oss
}

step2() {
    echo "============================================"
    echo " sdplane-oss integration script: step 2"
    echo "============================================"
    merge_libsdplane
}

step3() {
    echo "============================================"
    echo " sdplane-oss integration script: step 3"
    echo "============================================"
    merge_sdplane
}

step4() {
    echo "============================================"
    echo " sdplane-oss integration script: step 4"
    echo "============================================"
    check_oss_build_files
    remove_internal_files
    check_license_headers
}

main() {
if [ $# -lt 2 ]; then
    echo "============================================"
    echo " sdplane-oss integration script"
    echo "============================================"

    pre_check
    setup_sdplane_oss
    merge_libsdplane
    merge_sdplane
    check_oss_build_files
    remove_internal_files
    check_license_headers
else
    case $2 in
    step1)
      echo step1.
      step1 "$@"
      ;;
    step2)
      echo step2.
      step2 "$@"
      ;;
    step3)
      echo step3.
      step3 "$@"
      ;;
    step4)
      echo step4.
      step4 "$@"
      ;;
    *)
      error "unknown arg: $2"
      ;;
    esac
fi
}

main "$@"
