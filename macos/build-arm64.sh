#!/bin/bash

export CFLAGS="-arch arm64 -mmacosx-version-min=11.0"
export CXXFLAGS="-arch arm64 -mmacosx-version-min=11.0"
export LDFLAGS="-arch arm64 -mmacosx-version-min=11.0"
autoreconf -i
./configure --host=arm64

make clean
make

cp -f app/opensh app/opensh.arm64

