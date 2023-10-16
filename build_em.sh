#!/usr/bin/env sh

preload_files=""

# setup
if ! [ -d build-web ]; then
    emcmake cmake -S . -Bbuild-web -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS="${preload_files}"
fi

# copy resources
cp index.html build-web

# build
cmake --build build-web -j
