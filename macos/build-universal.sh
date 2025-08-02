#!/bin/bash

prefix=`dirname $0`

rm -f app/opensh.x86_64
rm -f app/opensh.arm64
rm -f app/opensh.universal

bash $prefix/build-x86_64.sh
bash $prefix/build-arm64.sh

cmd="lipo -create -output app/opensh.universal app/opensh.arm64 app/opensh.x86_64"
echo $cmd
$cmd

