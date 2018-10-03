#!/bin/bash
DIRECTORY="$1"
ROLE="$2"

cd "$DIRECTORY" || exit 1

cmake \
    -DUSE_OMP=OFF \
    -DUSE_SSE2=ON \
    -DUSE_OPENCL_DETECT=OFF \
    -DUSE_OPENCL_TRANSFORM=OFF \
    -DUSE_OPENCL_DEBUG=OFF \
    -DCMAKE_VERBOSE_MAKEFILE=FALSE \
    -DCMAKE_INSTALL_PREFIX:PATH=/usr/local \
    -DCMAKE_BUILD_TYPE=Release \
    . || exit 1

make -j16 "$ROLE" || exit 1 
