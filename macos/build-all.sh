#!/bin/bash

prefix=`dirname $0`

bash $prefix/build-universal.sh
bash $prefix/build-pkg.sh
# bash $prefix/build-pkg-notarization.sh

