cmake_minimum_required(VERSION 3.21)
project(hellotext VERSION 0.0.1)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)  # Group CMake targets inside a folder
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)         # Generate compile_commands.json for language servers

include(FetchContent)
if (NOT EMSCRIPTEN)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.3.8
    )
    list(APPEND FETCH_CONTENTS glfw)
    FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/mononerv/glad.git
        GIT_TAG        3e150fcc529c089476ebbb7b4a2fe924828be916
    )
    list(APPEND FETCH_CONTENTS glad)
endif()
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        10.1.0
)
list(APPEND FETCH_CONTENTS fmt)
FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://git.nrv.nu/mnerv/freetype.git
    GIT_TAG        2.13.2
)
list(APPEND FETCH_CONTENTS freetype)
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        0.9.9.8
)
list(APPEND FETCH_CONTENTS glm)
FetchContent_Declare(
  utf8cpp
  GIT_REPOSITORY https://github.com/nemtrif/utfcpp.git
  GIT_TAG        v3.2.4
)
list(APPEND FETCH_CONTENTS utf8cpp)
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/mononerv/stb.git
  GIT_TAG        698c6fb9889c71494b49c9187d249af5fc87b211
)
list(APPEND FETCH_CONTENTS stb)
FetchContent_MakeAvailable(${FETCH_CONTENTS})

if (APPLE)
    set(PLATFORM_LINK_LIBRARIES
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework OpenGL"
    )
elseif (UNIX AND NOT APPLE)  # Linux, BSD, Solaris, Minix
    set(PLATFORM_LINK_LIBRARIES
        "dl"
        "m"
        "GL"
        "X11"
    )
elseif (WIN32)
    set(PLATFORM_LINK_LIBRARIES "OpenGL32.lib")
endif()
if (NOT DEFINED PLATFORM_LINK_LIBRARIES)
    message(FATAL_ERROR "PLATFORM_LINK_LIRBARIES not defined!")
endif()

# Compiler specific options
if (NOT MSVC)
    set(BASE_OPTIONS
        "-Wall"
        "-Wextra"
        "-Wconversion"
        "-Wpedantic"
        "-Wshadow"
        "-Werror"
        # fmt warnings
        "-Wno-unknown-attributes"
        # glad warnings
        "-Wno-language-extension-token"
        # glm warnings
        "-Wno-nested-anon-types"
        "-Wno-gnu-anonymous-struct"
    )
    if (EMSCRIPTEN)
        list(APPEND BASE_OPTIONS
            # fmt warnings
            "-Wno-deprecated-literal-operator"
        )
    endif()
else()
    set(BASE_OPTIONS
        "/W4"
        "/WX"
        "/utf-8"
        "/Zc:__cplusplus"
        #"/fsanitize=address"  # Doesn't work without Visual Studio
    )
endif()
if (NOT EMSCRIPTEN)
    set(BASE_LIBRARIES
        glfw
        glad::glad
    )
endif()

set(SOURCE_FILES hellotext.cpp)
add_executable(hellotext ${SOURCE_FILES})
target_include_directories(hellotext PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_features(hellotext PRIVATE cxx_std_20)
target_compile_options(hellotext PRIVATE ${BASE_OPTIONS})
target_link_libraries(hellotext
    PRIVATE
    ${PLATFORM_LINK_LIBRARIES}
    ${BASE_LIBRARIES}
    fmt
    freetype
    glm
    utf8::cpp
    stb::stb
)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${SOURCE_FILES})
