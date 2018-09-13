#!/bin/bash
DIRECTORY="$1"
ROLE="$2"

cd "$DIRECTORY"                                || exit 1
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/local . || exit 1
make -j16 "$ROLE"                              || exit 1 
