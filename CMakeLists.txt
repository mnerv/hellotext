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
elseif (UNIX AND NOT APPLE AND NOT EMSCRIPTEN)  # Linux, BSD, Solaris, Minix
    set(PLATFORM_LINK_LIBRARIES
        "dl"
        "m"
        "GL"
        "X11"
    )
elseif (WIN32)
    set(PLATFORM_LINK_LIBRARIES "OpenGL32.lib")
elseif(EMSCRIPTEN)
    # Emscripten build
    add_compile_definitions(
        "GL_GLEXT_PROTOTYPES"
        "GL3_PROTOTYPES"
    )
    add_compile_options("-pthread" "-fexceptions")
    add_link_options(
        "-sLEGACY_GL_EMULATION=0"
        "-sUSE_WEBGL2=1"
        "-sFULL_ES3=1"
        "-fexceptions"
    )
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

set(HEADERS
    txt/buffer.hpp
    txt/fonts.hpp
    txt/image.hpp
    txt/renderer.hpp
    txt/shader.hpp
    txt/utility.hpp
    txt/window.hpp
)
set(SOURCES
    txt/buffer.cpp
    txt/fonts.cpp
    txt/image.cpp
    txt/renderer.cpp
    txt/shader.cpp
    txt/window.cpp
    hellotext.cpp
)
add_executable(hellotext ${HEADERS} ${SOURCES})
target_include_directories(hellotext PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_features(hellotext PRIVATE cxx_std_20)
target_compile_options(hellotext PRIVATE ${BASE_OPTIONS})
target_link_libraries(hellotext
    PRIVATE
    ${PLATFORM_LINK_LIBRARIES}
    ${BASE_LIBRARIES}
    freetype
    fmt
    glm
    utf8::cpp
    stb::stb
)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${HEADERS} ${SOURCES})
