#!/bin/bash
DIRECTORY="$1"
ROLE="$2"

cd "$DIRECTORY" || exit 1

cmake \
    -DUSE_OMP=OFF \
    -DUSE_SSE2=OFF \
    -DCMAKE_VERBOSE_MAKEFILE=FALSE \
    -DCMAKE_INSTALL_PREFIX:PATH=/usr/local \
    . || exit 1

make -j16 "$ROLE" || exit 1 
