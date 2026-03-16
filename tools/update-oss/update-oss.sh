#!/bin/bash

set -euo pipefail

# ---------------------------------------------------------------------------
# constants
# ---------------------------------------------------------------------------

SDPLANE_DEV_REPO="git@github.com:kait-cronos/sdplane-dev.git"
LIBSDPLANE_DEV_REPO="git@github.com:kait-cronos/libsdplane-dev.git"
SDPLANE_OSS_REPO="git@github.com:kait-cronos/sdplane-oss.git"

BRANCH="update/$(date +%Y-%m-%d)"

FILES_TO_REMOVE=(
    ai-workspace
    CLAUDE.md
    NEWS
    TODO
    doc
    tools/update-oss
)

# ---------------------------------------------------------------------------
# logging
# ---------------------------------------------------------------------------

info()  { echo "[INFO]  $*"; }
error() { echo "[ERROR] $*" >&2; exit 1; }

# ---------------------------------------------------------------------------
# check val
# ---------------------------------------------------------------------------

if [ $# -lt 1 ]; then
    echo "usage: $0 <workdir>" >&2
    exit 1
fi

WORKDIR="$(realpath "$1")"
SDPLANE_OSS_DIR="$WORKDIR/update-sdplane-oss"
LIBSDPLANE_TMP_DIR="$WORKDIR/libsdplane-tmp"

# ---------------------------------------------------------------------------
# functions
# ---------------------------------------------------------------------------

pre_check() {
    command -v git-filter-repo >/dev/null 2>&1 \
        || error "git-filter-repo not found"

    [ ! -e "$SDPLANE_OSS_DIR" ] \
        || error "$SDPLANE_OSS_DIR already exist"
    [ ! -e "$LIBSDPLANE_TMP_DIR" ] \
        || error "$LIBSDPLANE_TMP_DIR already exist"

    mkdir -p "$WORKDIR"
}

setup_sdplane_oss() {
    cd "$WORKDIR"
    git clone "$SDPLANE_DEV_REPO" update-sdplane-oss

    cd "$SDPLANE_OSS_DIR"
    git remote remove origin
    git remote add origin "$SDPLANE_OSS_REPO"

    git checkout -b "$BRANCH"
}

merge_libsdplane() {
    cd "$WORKDIR"
    git clone "$LIBSDPLANE_DEV_REPO" libsdplane-tmp

    cd "$LIBSDPLANE_TMP_DIR"
    git filter-repo --subdirectory-filter lib
    git filter-repo --to-subdirectory-filter lib

    cd "$SDPLANE_OSS_DIR"
    git remote add libsdplane "$LIBSDPLANE_TMP_DIR"
    git fetch libsdplane

    git merge --allow-unrelated-histories libsdplane/main \
        -m "integrate lib/ from libsdplane-dev"

    git remote remove libsdplane
    rm -rf "$LIBSDPLANE_TMP_DIR"
}

apply_oss_build_files() {
    cd "$SDPLANE_OSS_DIR"
    cp -r tools/update-oss/oss-build/. .
    git add configure.ac Makefile.am lib/Makefile.am sdplane/Makefile.am
    git commit -m "apply oss build system"
}

remove_internal_files() {
    cd "$SDPLANE_OSS_DIR"
    git rm -rf --ignore-unmatch "${FILES_TO_REMOVE[@]}"
    git diff --cached --quiet || git commit -m "remove internal-only files for oss release"
}

# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------

main() {
    echo "============================================"
    echo " sdplane-oss integration script"
    echo "============================================"

    pre_check
    setup_sdplane_oss
    merge_libsdplane
    apply_oss_build_files
    remove_internal_files
}

main "$@"
