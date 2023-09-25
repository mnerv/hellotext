#!/usr/bin/env sh

# setup
if ! [ -d build-web ]; then
    emcmake cmake -S . -Bbuild-web -DCMAKE_BUILD_TYPE=Release
fi

# copy resources
cp index.html build-web

# build
cmake --build build-web -j
