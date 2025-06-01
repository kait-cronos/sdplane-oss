#!/bin/bash

#set -x

style=`dirname $0`

echo $style

diffcmd=`which diff`
fixdefun=$style/fix-defun.awk
clangformat=clang-format
clangformat_version='18.1.3'

IGNORE_PATHS=""

check_clangformat_version() {
    if ! command -v $clangformat &> /dev/null; then
        echo "clang-format is not installed. Please install it first."
        exit 1
    fi
    local version=$(clang-format --version)
    if ! grep -q "$clangformat_version" <<< "$version"; then
        echo "clang-format version $clangformat_version is required, but found: $version"
        echo "Please install clang-format of the specified version."
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
