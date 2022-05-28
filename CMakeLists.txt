cmake_minimum_required(VERSION 3.12)
project(hellotext VERSION 0.0.0)

# Generate the compile_commands.json for ccls
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# output location
if (NOT DEFINED WORKSPACE_LOCATION)
    set(WORKSPACE_LOCATION ${PROJECT_SOURCE_DIR})
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${WORKSPACE_LOCATION}/bin/${PROJECT_NAME}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${WORKSPACE_LOCATION}/bin/${PROJECT_NAME}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${WORKSPACE_LOCATION}/bin/${PROJECT_NAME}")

# compilation flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

# freetype library from source
add_library(freetype STATIC IMPORTED)
find_library(FREETYPE_LIB
    NAMES libfreetype.a freetype.lib
    PATHS "$ENV{FREETYPE_PATH}/build" "${PROJECT_SOURCE_DIR}/bin"
    NO_DEFAULT_PATH
)
set_target_properties(freetype PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "$ENV{FREETYPE_PATH}/include"
    INTERFACE_LINK_DIRECTORIES "$ENV{FREETYPE_PATH}/build"
    IMPORTED_LOCATION "${FREETYPE_LIB}"
)
# spdlog library interface from source
add_library(spdlog INTERFACE)
target_include_directories(spdlog INTERFACE "$ENV{SPDLOG_PATH}/include")
# glm library interface from source
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE "$ENV{GLM_PATH}")
# GLFW library from source
add_library(glfw STATIC IMPORTED)
find_library(GLFW_LIB
    NAMES libglfw3.a glfw3.lib
    PATHS "$ENV{GLFW_PATH}/build/src"
    NO_DEFAULT_PATH
)
set_target_properties(glfw PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "$ENV{GLFW_PATH}/include"
    INTERFACE_LINK_DIRECTORIES "$ENV{GLFW_PATH}/build"
    IMPORTED_LOCATION "${GLFW_LIB}"
)
# Glad library from source
add_library(glad STATIC IMPORTED)
find_library(GLAD_LIB
    NAMES libglad.a glad.lib
    PATHS "$ENV{GLAD_PATH}/lib"
    NO_DEFAULT_PATH
)
set_target_properties(glad PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "$ENV{GLAD_PATH}/include"
    INTERFACE_LINK_DIRECTORIES "$ENV{GLAD_PATH}/lib"
    IMPORTED_LOCATION "${GLAD_LIB}"
)

# warnings and errors
if (NOT MSVC)
    string(JOIN " " CMAKE_CXX_FLAGS
        ${CMAKE_CXX_FLAGS}
        "-Wall"
        "-Wextra"
        "-Wconversion"
        "-Wpedantic"
        "-Werror"

        # Suppress warnings for GLM library
        "-Wno-deprecated-volatile"
    )
else()
    string(JOIN " " CMAKE_CXX_FLAGS
        ${CMAKE_CXX_FLAGS}
        "/W4"
        "/WX"
    )
endif()

add_executable(${PROJECT_NAME}
    hellotext.cpp
)
include_directories(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(${PROJECT_NAME}
    freetype
    spdlog
    glfw
    glad
    glm
)
if (APPLE)
    target_link_libraries(${PROJECT_NAME}
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework OpenGL"
    )
elseif (LINUX)
    target_link_libraries(${PROJECT_NAME}
        "-dl"
        "-m"
        "-GL"
        "-X11"
    )
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME}
        "OpenGL32.lib"
    )
endif()
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${TARGET_SOURCE_FILES})

