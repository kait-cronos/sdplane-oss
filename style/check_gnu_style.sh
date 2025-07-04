#!/bin/bash

#set -x

style=`dirname $0`

echo $style

diffcmd=`which diff`
fixdefun=$style/fix-defun.awk
clangformat=clang-format
clangformat_min_version='18.1.3'

IGNORE_PATHS="module/"

# Function to compare version numbers
version_compare() {
    local version1=$1
    local version2=$2
    
    # Split version into array
    IFS='.' read -ra ver1 <<< "$version1"
    IFS='.' read -ra ver2 <<< "$version2"
    
    # Compare major, minor, patch
    for i in {0..2}; do
        local v1=${ver1[i]:-0}
        local v2=${ver2[i]:-0}
        
        if (( v1 > v2 )); then
            return 0  # version1 > version2
        elif (( v1 < v2 )); then
            return 1  # version1 < version2
        fi
    done
    
    return 0  # versions are equal
}

check_clangformat_version() {
    if ! command -v $clangformat &> /dev/null; then
        echo "clang-format is not installed. Please install it first."
        exit 1
    fi
    
    local version_output=$(clang-format --version)
    local version=$(echo "$version_output" | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)
    
    if [ -z "$version" ]; then
        echo "Could not determine clang-format version from: $version_output"
        exit 1
    fi
    
    if ! version_compare "$version" "$clangformat_min_version"; then
        echo "clang-format version $clangformat_min_version or greater is required, but found: $version"
        echo "Please install a newer version of clang-format."
        echo "Or please use check_gnu_style_docker.sh to run this script in a Docker container with the correct version."
        exit 1
    fi
}

check () {
    local needs_update=0

    if [ $# -eq 0 ]; then
        while IFS= read -r -d '' file
        do
            [[ ! -z "$IGNORE_PATHS" && "$file" =~ $IGNORE_PATHS ]] && \
                continue;
            cat "$file" | $clangformat | awk -f $fixdefun | \
                $diffcmd -q "$file" - > /dev/null
            if [[ $? -eq 1 ]]; then
                echo "$file needs to be fixed by update.";
                needs_update=1
            fi
        done <   <(find . -name '*.[ch]' -print0)
    else
        for file in "$@"; do
            cat "$file" | $clangformat | awk -f $fixdefun | \
                $diffcmd -q "$file" - > /dev/null
            if [[ $? -eq 1 ]]; then
                echo "$file needs to be fixed by update.";
                needs_update=1
            fi
        done
    fi

    if [[ $needs_update -eq 1 ]]; then
        exit 1;
    fi
}

diff () {
    if [ $# -eq 0 ]; then
        while IFS= read -r -d '' file
        do
            [[ ! -z "$IGNORE_PATHS" && "$file" =~ $IGNORE_PATHS ]] && \
                continue;
            cat "$file" | $clangformat | awk -f $fixdefun | \
                $diffcmd -q "$file" - > /dev/null
            if [[ $? -eq 1 ]]; then
                cat "$file" | $clangformat | awk -f $fixdefun \
                    | $diffcmd -u "$file" -
            fi
        done <   <(find . -name '*.[ch]' -print0)
     else
        for file in "$@"
        do
            cat "$file" | $clangformat | awk -f $fixdefun | \
                $diffcmd -q "$file" - > /dev/null
            if [[ $? -eq 1 ]]; then
                cat "$file" | $clangformat | awk -f $fixdefun \
                    | $diffcmd -u "$file" -
            fi
        done
     fi
}

update () {
    if [ $# -eq 0 ]; then
        while IFS= read -r -d '' file
        do
            [[ ! -z "$IGNORE_PATHS" && "$file" =~ $IGNORE_PATHS ]] && \
                continue;
            cat "$file" | $clangformat | awk -f $fixdefun | \
                $diffcmd -q "$file" - > /dev/null
            if [[ $? -eq 1 ]]; then
                cp -f "$file" "$file".bak
                cat "$file".bak | $clangformat | \
                    awk -f $fixdefun > "$file"
                echo "$file has been fixed by update.";
                rm "$file".bak
            fi
        done <   <(find . -name '*.[ch]' -print0)
     else
        for file in "$@"; do
            cat "$file" | $clangformat | awk -f $fixdefun | \
                $diffcmd -q "$file" - > /dev/null
            if [[ $? -eq 1 ]]; then
                cp -f "$file" "$file".bak
                cat "$file".bak | $clangformat | \
                    awk -f $fixdefun > "$file"
                echo "$file has been fixed by update.";
                rm "$file".bak
            fi
        done
     fi
}

help () {
    echo "$0 [check | diff | update] [file ...]"
    exit 1
}

subcommand=$1
shift

check_clangformat_version

case $subcommand in
    check) check "$@" ;;
    diff) diff "$@" ;;
    update) update "$@" ;;
    *) help "$@" ;;
esac

# vim: set ft=sh ts=8 expandtab sw=4 sts=4 :
