#!/bin/bash

set -eux

style=`dirname $0`

cd $style
docker build -t check_gnu_style .

cd ..
docker run --rm -v .:/src check_gnu_style /src/style/check_gnu_style.sh "$@"
