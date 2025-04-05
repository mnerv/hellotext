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

## Development

Setup with build-system with `ninja`.

```sh
cmake -S . -Bbuild -GNinja -DCMAKE_BUILD_TYPE=Debug
```

Compile using `ninja`.


```sh
ninja -C build
```

### Link compile_commands.json

**Unix**

```sh
ln -sfn ./build/compile_commands.json .
```

**Tips working with git**

Clone repo as bare and then create a `worktree` with it.

```sh
git clone --bare git@github.com:mnerv/hellotext.git
```

`cd` into the `hellotext.git` directory and then use the command below to create
a work tree.

```sh
git worktree add {path} {branch}
```

`{path}`: The path you want to put the working tree.

`{branch}`: The branch you want to work with.


Example:

```sh
git worktree add trunk trunk
```

To remove `worktree` you can use the following command

```sh
git worktree remove {path}
```

Or you can also just remove the directory

```sh
rm -rf {path}
```

And then you'll need to use the command below to remove stale `worktrees`.

```sh
git worktree prune
```

### Emscripten

```sh
docker run --rm -v "$(pwd):/src" emscripten/emsdk ./build_em.sh --production
```

## Text Rendering

The application uses FreeType 2 to read most font file types, `ttf`
(**TrueTypeFont**) and `otf` (**OpenTypeFont**) and OpenGL as its backend to
render it to screen. For window creation **GLFW** library is used as window
abstraction layer for the desktop version.

### Resources

  - [Learn OpenGL - Text Rendering](https://learnopengl.com/In-Practice/Text-Rendering)
  - [mrandri19/freetype-opengl-experiments](https://github.com/mrandri19/freetype-opengl-experiments)
  - [Family, type family, or font family - Google Fonts](https://fonts.google.com/knowledge/glossary/family_or_type_family_or_font_family)

