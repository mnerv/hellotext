#!/usr/bin/env sh

preload_files="--preload-file ./shaders/webgl --preload-file ./res/fonts"

# flags
is_rebuild=false

# Parse arguments
while [ $# -gt 0 ]; do  # Check if total number of args is greater than 0
    key="$1"            # Get the first argument
    case $key in
        --rebuild)
            # Set the rebuild flag to true
            is_rebuild=true
            ;;
    esac
    # Shift the arguments to the left
    shift 
done

# Setup
if ! [ -d build-web ] || [ "is_rebuild" = true ]; then
    emcmake cmake -S . -Bbuild-web -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS="${preload_files}"
fi

# Copy resources
cp ./index.html ./build-web
cp -rf ./shaders ./build-web
cp -rf ./res ./build-web

# Clean
if [ "$is_rebuild" = true ]; then
    cmake --build build-web --target clean
fi

# Build
cmake --build build-web -j
