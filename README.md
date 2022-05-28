# hellotext

Rendering text in OpenGL using FreeType!

## Requirements

  - [FreeType](https://freetype.org/)
  - [spdlog](https://github.com/gabime/spdlog)
  - [GLFW](https://www.glfw.org/)
  - [Glad](https://glad.dav1d.de/) or [mononerv/glad](https://github.com/mononerv/glad)
  - [glm](https://glm.g-truc.net/0.9.9/index.html)
  - [stb](https://github.com/nothings/stb) - `stb_image.h` and `stb_image_write.h`

## Build

Required environment variable definitions

```
FREETYPE_SDK
SPDLOG_SDK
GLFW_SDK
GLAD_SDK
GLM_SDK
STB_SDK
```

These variables should point to the library directory in the requirements.

For `FreeType` library make sure to compile with this flag.

```
cmake -S . -Bbuild -D FT_DISABLE_ZLIB=TRUE -D FT_DISABLE_BZIP2=TRUE -D FT_DISABLE_PNG=TRUE -D FT_DISABLE_HARFBUZZ=TRUE -D FT_DISABLE_BROTLI=TRUE -D CMAKE_BUILD_TYPE=Release
```

