# ==============================================================================
# Find GLFW
# ==============================================================================
# This module fetches the GLFW library for window/input handling.
#
# Targets provided:
#   glfw::glfw - The GLFW library target
#   glfw       - The GLFW library target (non-namespaced)
#
# Variables set:
#   GLFW_FOUND       - TRUE if GLFW is available
#   GLFW_LIBRARIES   - The GLFW library target (glfw::glfw)
#   GLFW_INCLUDE_DIR - Include directories for GLFW
#   GLFW_VERSION     - Version of GLFW (if available)
# ==============================================================================

if (DEFINED _FINDGLFW_INCLUDED)
    return()
endif()
set(_FINDGLFW_INCLUDED TRUE)

# Use the version passed to find_package(), or default to 3.4
if (DEFINED GLFW_FIND_VERSION AND NOT GLFW_FIND_VERSION STREQUAL "")
    set(GLFW_VERSION "${GLFW_FIND_VERSION}")
else()
    set(GLFW_VERSION "3.4")
endif()

message(STATUS "Fetching GLFW ${GLFW_VERSION}")

include(FetchContent)

find_program(GIT_EXECUTABLE git)
if (GIT_EXECUTABLE)
    set(GLFW_FETCH_METHOD "GIT")
else()
    message(FATAL_ERROR "Fetch with zip not supported.")
endif()

if (GLFW_FETCH_METHOD STREQUAL "GIT")
    # Disable GLFW extras to keep builds lightweight.
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
    # Environment issue where a cross-compilation script
    # set(GLFW_BUILD_WAYLAND      OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        ${GLFW_VERSION}
    )
endif()

set(_glfw_suppress_prev ${CMAKE_SUPPRESS_DEVELOPER_WARNINGS})
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS TRUE CACHE INTERNAL "")
FetchContent_MakeAvailable(glfw)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ${_glfw_suppress_prev} CACHE INTERNAL "")

if (NOT TARGET glfw::glfw)
    if (TARGET glfw)
        add_library(glfw::glfw ALIAS glfw)
    else()
        message(FATAL_ERROR "Could not fetch GLFW; no target glfw or glfw::glfw available")
    endif()
endif()

set(GLFW_FOUND        TRUE)
set(GLFW_LIBRARIES    glfw::glfw)
get_target_property(_glfw_inc glfw::glfw INTERFACE_INCLUDE_DIRECTORIES)
set(GLFW_INCLUDE_DIR  "${_glfw_inc}")

# Mark GLFW includes as SYSTEM to suppress warnings from its headers
if (_glfw_inc AND TARGET glfw)
    set_target_properties(glfw PROPERTIES
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_glfw_inc}"
    )
endif()

set(GLFW_LICENSE_FILE "${glfw_SOURCE_DIR}/LICENSE.md" CACHE FILEPATH "Path to GLFW license file")
