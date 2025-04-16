#!/usr/bin/env sh

set -e

preload_files="--preload-file $(pwd)/shaders/webgl@./shaders/webgl --preload-file $(pwd)/res/fonts@./res/fonts"

if ! [ -d "./build-web" ]; then
    emcmake cmake -S . -Bbuild-web -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS="${preload_files}"
fi

echo "compiling..."

cmake --build build-web -j

echo "transfering files to dist"

mkdir -p build-web/dist
cp ./web/index.html build-web/dist
cp build-web/hellotext.* build-web/dist

