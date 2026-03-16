# hellotext

Rendering text in OpenGL using FreeType! The `hellotext.cpp` file is the entry
of the application, it setups the application logic for the user. The project is
split into two parts, the application and the library. The application contains
the code for business logic, handling user data and storing different logic for
user interaction. The library provides the application developer to interact
with the computer on higher level, reading fonts, rendering it to window and
drawing shapes. When using the built in renderer and its text engine it will
batch the draw calls.

## Requirements

  - [CMake](https://cmake.org/)

## Build

Use command below to generate build system. The `cmake` script will
automatically download the dependencies.

```sh
cmake -S . -Bbuild -GNinja -DCMAKE_BUILD_TYPE=Debug
```

## Build Emscripten

Generate the build system using the `emscripten/emsdk` docker image (the docker
command can be omitted if `emsdk` is installed locally).

```sh
docker run -it --rm -v "$(pwd):/src" emscripten/emsdk:5.0.2 emcmake cmake -S . -Bbuild-web -DCMAKE_BUILD_TYPE=Release
```

Build the project:

```sh
docker run -it --rm -v "$(pwd):/src" emscripten/emsdk:5.0.2 cmake --build build-web -j
```

To collect all web artifacts (`index.html`, `.js`, `.wasm`, and `res/`) into a
single `build-web/deploy/` directory, run the `deploy` target:

```sh
docker run -it --rm -v "$(pwd):/src" emscripten/emsdk:5.0.2 cmake --build build-web --target deploy
```

## Text Rendering

The application uses FreeType 2 to read most font file types, `ttf`
(**TrueTypeFont**) and `otf` (**OpenTypeFont**) and OpenGL as its backend to
render it to screen. For window creation **GLFW** library is used as window
abstraction layer for the desktop version. On the emscripten platform the native
**HTML5 DOM API** from emscripten is used to create **WebGL 2.0** context and
event registrations.

### Resources

  - [Learn OpenGL - Text Rendering](https://learnopengl.com/In-Practice/Text-Rendering)
  - [mrandri19/freetype-opengl-experiments](https://github.com/mrandri19/freetype-opengl-experiments)
  - [Family, type family, or font family - Google Fonts](https://fonts.google.com/knowledge/glossary/family_or_type_family_or_font_family)
