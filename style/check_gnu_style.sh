#!/bin/bash

#set -x

style=`dirname $0`

echo $style

diffcmd=`which diff`
fixdefun=$style/fix-defun.awk
clangformat=clang-format

IGNORE_PATHS=""

check () {
    if [ $# -eq 0 ]; then
        while IFS= read -r -d '' file
        do
            [[ ! -z "$IGNORE_PATHS" && "$file" =~ $IGNORE_PATHS ]] && \
                continue;
            cat "$file" | $clangformat | awk -f $fixdefun | \
                $diffcmd -q "$file" - > /dev/null
            if [[ $? -eq 1 ]]; then
                echo "$file needs to be fixed by update.";
            fi
        done <   <(find . -name '*.[ch]' -print0)
    else
        for file in "$@"; do
            cat "$file" | $clangformat | awk -f $fixdefun | \
                $diffcmd -q "$file" - > /dev/null
            if [[ $? -eq 1 ]]; then
                echo "$file needs to be fixed by update.";
            fi
        done
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

case $subcommand in
    check) check "$@" ;;
    diff) diff "$@" ;;
    update) update "$@" ;;
    *) help "$@" ;;
esac

# vim: set ft=sh ts=8 expandtab sw=4 sts=4 :
