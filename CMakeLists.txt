cmake_minimum_required(VERSION 3.21)
project(hellotext VERSION 0.2.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)  # Group CMake targets inside a folder
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)         # Generate compile_commands.json for language servers
# Add the deps directory to the module path
# Requires for find_package injection
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/deps")

# Setup flags for platform and compiler
include(deps/Platform.cmake)
include(deps/Flags.cmake)

# Dependencies
if(NOT IS_EMSCRIPTEN)
    find_package(glfw3 REQUIRED)
    find_package(Vulkan REQUIRED)
    find_package(glad REQUIRED)
endif()
find_package(asio REQUIRED)
find_package(ctre REQUIRED)
find_package(fmt REQUIRED)
find_package(stb REQUIRED)
find_package(utf8cpp REQUIRED)
find_package(freetype REQUIRED)
find_package(glm REQUIRED)

# Target setup
set(TARGET_NAME "app")
set(TARGET_HEADERS
    txt/buffer.hpp
    txt/event.hpp
    txt/fonts.hpp
    txt/image.hpp
    txt/input.hpp
    txt/renderer.hpp
    txt/shader.hpp
    txt/text_engine.hpp
    txt/texture.hpp
    txt/utility.hpp
    txt/window.hpp
)
set(TARGET_SOURCES
    txt/buffer.cpp
    txt/fonts.cpp
    txt/image.cpp
    txt/input.cpp
    txt/renderer.cpp
    txt/shader.cpp
    txt/text_engine.cpp
    txt/texture.cpp
    txt/window.cpp
    hellotext.cpp
)
add_executable(${TARGET_NAME} ${TARGET_HEADERS} ${TARGET_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_features(${TARGET_NAME} PRIVATE c_std_23 cxx_std_23)
target_compile_options(${TARGET_NAME} PRIVATE ${BASE_OPTIONS})
target_compile_definitions(${TARGET_NAME} PRIVATE
    ${BASE_DEFINITIONS}
)
target_link_libraries(${TARGET_NAME}
    PRIVATE
    ${BASE_LIBRARIES}
    asio::asio
    ctre::ctre
    fmt::fmt
    glm::glm
    stb::stb
    utf8cpp::utf8cpp
    freetype::freetype
    # Not needed on Emscripten
    $<$<NOT:$<BOOL:${IS_EMSCRIPTEN}>>:glfw::glfw>
    $<$<NOT:$<BOOL:${IS_EMSCRIPTEN}>>:glad::glad>
)
source_group(TREE "${CMAKE_SOURCE_DIR}" FILES ${TARGET_HEADERS} ${TARGET_SOURCES})

# Copy resources to build directory
if(IS_EMSCRIPTEN)
    configure_file(
        "${CMAKE_SOURCE_DIR}/index.html"
        "${CMAKE_BINARY_DIR}/index.html"
        @ONLY
    )
    # Deploy target: collects all web artifacts into a deploy/ directory
    set(DEPLOY_DIR "${CMAKE_BINARY_DIR}/deploy")
    add_custom_target(deploy
        DEPENDS ${TARGET_NAME}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DEPLOY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DEPLOY_DIR}/res"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/index.html" "${DEPLOY_DIR}/index.html"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${TARGET_NAME}.js"   "${DEPLOY_DIR}/${TARGET_NAME}.js"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${TARGET_NAME}.wasm" "${DEPLOY_DIR}/${TARGET_NAME}.wasm"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/res" "${DEPLOY_DIR}/res"
        COMMENT "Deploying web artifacts to ${DEPLOY_DIR}"
    )
endif()

# Setup IDE integration
include(deps/IDE.cmake)
