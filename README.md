# hellotext

Rendering text in OpenGL using FreeType! The hellotext.cpp file includes all the necessary code to load a font and render it using OpenGL. It utilizes FreeType to load the desired font and uses instanced rendering to efficiently render individual characters.

## Requirements

  - [cmake](https://cmake.org/)
  - [vcpkg](https://vcpkg.io/)

## Build

Use command below to generate build system.

```
cmake -S . -Bbuild "-DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
```
