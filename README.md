# hellotext

Rendering text in OpenGL using FreeType! The hellotext.cpp file includes all the necessary code to load a font and render it using OpenGL. It utilizes FreeType to load the desired font and uses instanced rendering to efficiently render individual characters.

![Show application with big text saying "Hello, World!"](./res/preview.png)

## Requirements

  - [cmake](https://cmake.org/)

## Build

Use command below to generate build system.

```sh
cmake -S . -Bbuild
```

## Build Emscripten

Generate build system using `emscripten/emsdk` docker image.

```sh
docker run -it --rm -v "$(pwd):/src" emscripten/emsdk ./build_em.sh
```

## Text Rendering

The application uses FreeType 2 to read most font file types, `ttf` (**TrueTypeFont**) and `otf` (**OpenTypeFont**) and OpenGL as its backend to render it to screen. For window creation GLFW library is used as window abstraction layer.

### Resources

  - [Learn OpenGL - Text Rendering](https://learnopengl.com/In-Practice/Text-Rendering)
  - [mrandri19/freetype-opengl-experiments](https://github.com/mrandri19/freetype-opengl-experiments)
  - [Family, type family, or font family - Google Fonts](https://fonts.google.com/knowledge/glossary/family_or_type_family_or_font_family)
