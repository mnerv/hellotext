#!/usr/bin/env sh

preload_files="--preload-file ./shaders/webgl --preload-file ./res/fonts/Cozette"

# setup
if ! [ -d build-web ]; then
    emcmake cmake -S . -Bbuild-web -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS="${preload_files}"
fi

# copy resources
cp ./index.html ./build-web
cp -rf ./shaders ./build-web
cp -rf ./res ./build-web

# build
cmake --build build-web -j
