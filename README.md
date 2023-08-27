# hellotext

Rendering text in OpenGL using FreeType! The hellotext.cpp file includes all the necessary code to load a font and render it using OpenGL. It utilizes FreeType to load the desired font and uses instanced rendering to efficiently render individual characters.

![Show application on Windows 11 with text saying "Hello, World! Hej Charlie!"](./res/preview.png)

## Requirements

  - [cmake](https://cmake.org/)
  - [vcpkg](https://vcpkg.io/)

## Build

Use command below to generate build system.

```sh
cmake -S . -Bbuild
```

## Build Emscripten

```sh
docker run -it --rm -v "$(pwd):/src" emscripten/emsdk emcmake cmake -S . -Bbuild-web -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS="-s USE_GLFW=3 -s USE_WEBGL2=1 -s FULL_ES3=1"
```

```sh
docker run -it --rm -v "$(pwd):/src" emscripten/emsdk cmake --build build-web -j
```

## Text Rendering

The application uses FreeType 2 to read most font file types, `ttf` (**TrueTypeFont**) and `otf` (**OpenTypeFont**) and OpenGL as its backend to render it to screen. For window creation GLFW library is used as window abstraction layer.

### Resources

  - [Learn OpenGL - Text Rendering](https://learnopengl.com/In-Practice/Text-Rendering)
  - [mrandri19/freetype-opengl-experiments](https://github.com/mrandri19/freetype-opengl-experiments)
