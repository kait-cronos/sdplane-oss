#!/bin/bash

set -euo pipefail

# ---------------------------------------------------------------------------
# constants
# ---------------------------------------------------------------------------

SDPLANE_DEV_REPO="git@github.com:kait-cronos/sdplane-dev.git"
LIBSDPLANE_DEV_REPO="git@github.com:kait-cronos/libsdplane-dev.git"
SDPLANE_OSS_REPO="git@github.com:kait-cronos/sdplane-oss.git"

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
    echo "usage: $0 <workdir> (step1|step2|step3|step4)" >&2
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

    git merge --allow-unrelated-histories libsdplane/main \
        -m "integrate lib/ from libsdplane-dev"
}

merge_sdplane() {
    cd "$WORKDIR"
    cd sdplane-dev

    cd "$WORKDIR"
    cd sdplane-oss
    git remote add sdplane-dev "$WORKDIR/sdplane-dev"
    git fetch sdplane-dev

    git merge --allow-unrelated-histories sdplane-dev/main \
        -m "integrate all from sdplane-dev"
}

apply_oss_build_files() {
    cd "$WORKDIR"
    cd sdplane-oss

    # no need...
    #cd "$SDPLANE_OSS_DIR"
    #cp -r tools/update-oss/oss-build/. .
    #git add configure.ac Makefile.am lib/Makefile.am sdplane/Makefile.am
    #git commit -m "apply oss build system"
}

remove_internal_files() {
    cd "$WORKDIR"
    cd sdplane-oss
    git rm -rf --ignore-unmatch "${FILES_TO_REMOVE[@]}"
    git diff --cached --quiet || git commit -m "remove internal-only files for oss release"
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
    apply_oss_build_files
    remove_internal_files
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
    apply_oss_build_files
    remove_internal_files
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
      echo unknown arg: $2.
      ;;
    esac
fi
}

main "$@"
