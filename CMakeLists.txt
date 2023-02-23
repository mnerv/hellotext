cmake_minimum_required(VERSION 3.21)
project(hellotext VERSION 0.0.1)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)  # Group CMake targets inside a folder
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)         # Generate compile_commands.json for language servers

add_subdirectory(deps/stb)
find_package(glfw3 CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

if (APPLE)
    set(PLATFORM_LINK_LIBRARIES
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework OpenGL"
    )
elseif (LINUX)
    set(PLATFORM_LINK_LIBRARIES
        "-dl"
        "-m"
        "-GL"
        "-X11"
    )
elseif (WIN32)
    set(PLATFORM_LINK_LIBRARIES "OpenGL32.lib")
endif()
if (NOT DEFINED PLATFORM_LINK_LIBRARIES)
    message(FATAL_ERROR "PLATFORM_LINK_LIRBARIES not defined!")
endif()

set(TARGET ${PROJECT_NAME})
add_executable(${TARGET}
    hellotext.cpp
)
target_include_directories(${TARGET} PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_features(${TARGET} PRIVATE cxx_std_20)
target_link_libraries(${TARGET}
    PRIVATE
    ${PLATFORM_LINK_LIBRARIES}
    stb
    freetype
    glad::glad
    glfw
    glm::glm
    spdlog::spdlog
)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${TARGET_SOURCE_FILES})
