cmake_minimum_required(VERSION 3.21)
project(hellotext VERSION 0.0.1)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)  # Group CMake targets inside a folder
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)         # Generate compile_commands.json for language servers

include(FetchContent)
if (NOT EMSCRIPTEN)
    option(GLFW_BUILD_EXAMPLES OFF)
    option(GLFW_BUILD_TESTS OFF)
    option(GLFW_BUILD_DOCS OFF)
    option(GLFW_INSTALL OFF)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
    )
    list(APPEND FETCH_CONTENTS glfw)
    FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/mononerv/glad.git
        GIT_TAG        f4759d7c5143c0a23391ab05caaf43052cefdd65
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
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/mononerv/asio.git
    GIT_TAG        bcb8a933b27021d77f2d183991d815ea6c8cdc97
)
list(APPEND FETCH_CONTENTS asio)
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
    message("Platform: macOS")
    set(PLATFORM_LINK_LIBRARIES
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework OpenGL"
    )
elseif (UNIX AND NOT APPLE AND NOT EMSCRIPTEN)  # Linux, BSD, Solaris, Minix
    message("Platform: Linux")
    set(PLATFORM_LINK_LIBRARIES
        "dl"
        "m"
        "GL"
        "X11"
    )
elseif (WIN32)
    message("Platform: Windows")
    set(PLATFORM_LINK_LIBRARIES "OpenGL32.lib")
    set(PLATFORM_DEFINTIONS
        ${PLATFORM_DEFINTIONS}
        "_WIN32_WINNT=0x0A00"
    )
elseif(EMSCRIPTEN)
    message("Platform: Emscripten")
    add_compile_definitions(
        "GL_GLEXT_PROTOTYPES=1"
        "GL3_PROTOTYPES=1"
        "EGL_EGLEXT_PROTOTYPES=1"
    )
    add_compile_options("-pthread" "-fexceptions")
    add_link_options(
        "-sLEGACY_GL_EMULATION=0"
        "-sUSE_WEBGL2=1"
        "-sFULL_ES3=1"
        "-fexceptions"
        "-sALLOW_MEMORY_GROWTH"
    )
else()
    message(FATAL_ERROR "Platform: Unkown!")
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
        # clang on windows
        "-Wno-microsoft-include"
        # asio
        "-Wno-sign-conversion"
        "-Wno-implicit-int-conversion"
    )
    if (EMSCRIPTEN)
        set(BASE_OPTIONS
            ${BASE_OPTIONS}
            # asio
            "-Wno-shadow"
            "-Wno-shorten-64-to-32"
            "-Wno-unused-private-field"
            "-Wno-deprecated-declarations"
        )
    endif()
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
    txt/graphics/buffer.hpp
    txt/graphics/image.hpp
    txt/graphics/shader.hpp
    txt/graphics/texture.hpp
    txt/text/engine.hpp
    txt/text/fonts.hpp
    txt/input/keyboard.hpp
    txt/input/mouse.hpp
    txt/input/touch.hpp
    txt/event.hpp
    txt/input.hpp
    txt/renderer.hpp
    txt/types.hpp
    txt/utils.hpp
    txt/window.hpp
)
set(SOURCES
    txt/graphics/buffer.cpp
    txt/graphics/image.cpp
    txt/graphics/renderer.cpp
    txt/graphics/shader.cpp
    txt/graphics/texture.cpp
    txt/input/input.cpp
    txt/text/engine.cpp
    txt/text/fonts.cpp
    txt/window/window.cpp
    $<$<NOT:$<BOOL:${EMSCRIPTEN}>>:txt/window/glfw_window.cpp>
    $<$<BOOL:${EMSCRIPTEN}>:txt/window/web_window.cpp>
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
target_compile_definitions(libtxtr PRIVATE ${PLATFORM_DEFINTIONS})
target_link_libraries(libtxtr
    PRIVATE
    ${PLATFORM_LINK_LIBRARIES}
    utf8::cpp
    stb::stb
    fmt
    $<$<NOT:$<BOOL:${EMSCRIPTEN}>>:glfw>

    PUBLIC
    asio::asio
    freetype
    $<$<NOT:$<BOOL:${EMSCRIPTEN}>>:glad::glad>
    glm
)
# source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${HEADERS} ${SOURCES})

# hellotext app
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
target_compile_definitions(hellotext PRIVATE ${PLATFORM_DEFINTIONS})
target_link_libraries(hellotext
    PRIVATE
    ${PLATFORM_LINK_LIBRARIES}
    libtxtr
    fmt
)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${HEADERS} ${SOURCES})
