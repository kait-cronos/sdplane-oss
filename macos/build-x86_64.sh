#!/bin/bash

prefix=`dirname $0`

export CFLAGS="-arch x86_64 -mmacosx-version-min=10.5"
export CXXFLAGS="-arch x86_64 -mmacosx-version-min=10.5"
export LDFLAGS="-arch x86_64 -mmacosx-version-min=10.5"
autoreconf -i
./configure --host=x86_64

make clean
make

cp -f app/opensh app/opensh.x86_64

