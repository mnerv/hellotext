cmake_minimum_required(VERSION 3.21)
project(hellotext VERSION 0.0.1)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)  # Group CMake targets inside a folder
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)         # Generate compile_commands.json for language servers

include(FetchContent)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_DOCS OFF)
option(GLFW_INSTALL OFF)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)
list(APPEND FETCH_CONTENTS glfw)
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/mononerv/glad.git
    GIT_TAG        f4759d7c5143c0a23391ab05caaf43052cefdd65
)
list(APPEND FETCH_CONTENTS glad)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        10.1.0
)
list(APPEND FETCH_CONTENTS fmt)
FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG        VER-2-13-0
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

# Group dependencies in Visual Studio and Xcode
if (CMAKE_GENERATOR MATCHES "Visual Studio" OR CMAKE_GENERATOR MATCHES "Xcode")
    set_target_properties(fmt PROPERTIES FOLDER deps)
    set_target_properties(freetype PROPERTIES FOLDER deps)
    set_target_properties(glad PROPERTIES FOLDER deps)
    set_target_properties(stb PROPERTIES FOLDER deps)
    set_target_properties(update_mappings PROPERTIES FOLDER "deps/GLFW3")
    set_target_properties(glfw PROPERTIES FOLDER "deps/GLFW3")
endif()

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
        "X11"
    )
elseif (WIN32)
    set(PLATFORM_LINK_LIBRARIES "OpenGL32.lib")
else()
    message(FATAL_ERROR "Unkown platform!")
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
        "-Wno-deprecated-literal-operator"
        # glad warnings
        "-Wno-language-extension-token"
        # glm warnings
        "-Wno-nested-anon-types"
        "-Wno-gnu-anonymous-struct"
    )
else()
    set(BASE_OPTIONS
        "/W4"
        "/WX"
        "/utf-8"
        "/Zc:__cplusplus"
        "/wd4201"
        #"/fsanitize=address"  # Doesn't work without Visual Studio
    )
endif()

# libtxtr
set(HEADERS
    txt/buffer.hpp
    txt/event.hpp
    txt/fonts.hpp
    txt/image.hpp
    txt/input.hpp
    txt/renderer.hpp
    txt/shader.hpp
    txt/text_engine.hpp
    txt/texture.hpp
    txt/types.hpp
    txt/utility.hpp
    txt/window.hpp
)
set(SOURCES
    txt/buffer.cpp
    txt/fonts.cpp
    txt/image.cpp
    txt/input.cpp
    txt/renderer.cpp
    txt/shader.cpp
    txt/text_engine.cpp
    txt/texture.cpp
    txt/window.cpp
)
add_library(libtxtr STATIC ${HEADERS} ${SOURCES})
target_include_directories(libtxtr
    PRIVATE
    ${PROJECT_SOURCE_DIR}/txt
    PUBLIC
    ${PROJECT_SOURCE_DIR}
)
target_compile_features(libtxtr PRIVATE cxx_std_23)
target_compile_options(libtxtr PRIVATE ${BASE_OPTIONS})
target_link_libraries(libtxtr
    PRIVATE
    ${PLATFORM_LINK_LIBRARIES}
    glfw
    fmt
    utf8::cpp
    stb::stb

    PUBLIC
    freetype
    glad
    glm
)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${HEADERS} ${SOURCES})

set(HEADERS
    demos/app.hpp
    demos/text_bounce.hpp
)
set(SOURCES
    hellotext.cpp
)
add_executable(hellotext ${HEADERS} ${SOURCES})
target_include_directories(hellotext PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_features(hellotext PRIVATE cxx_std_20)
target_compile_options(hellotext PRIVATE ${BASE_OPTIONS})
target_link_libraries(hellotext
    PRIVATE
    ${PLATFORM_LINK_LIBRARIES}
    libtxtr
    fmt
)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${HEADERS} ${SOURCES})
