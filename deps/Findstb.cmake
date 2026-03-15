# ==============================================================================
# Find stb
# ==============================================================================
# This module fetches the stb single-file public domain libraries.
#
# Targets provided:
#   stb::stb - The stb library target
#
# Variables set:
#   stb_FOUND       - TRUE if stb is available
#   stb_LIBRARIES   - The stb library target (stb::stb)
#   stb_INCLUDE_DIR - Include directories for stb
#   stb_VERSION     - Version of stb (commit-based)
#
# Usage notes:
#   stb headers are header-only but require an implementation macro to be
#   defined in exactly ONE translation unit (.cpp file) before including the
#   header, e.g.:
#
#     #define STB_IMAGE_IMPLEMENTATION
#     #include <stb_image.h>
#
#   Each stb header has its own implementation macro. Do NOT define the macro
#   in header files or in more than one translation unit or you will get
#   duplicate symbol linker errors.
# ==============================================================================

if (DEFINED _FINDSTB_INCLUDED)
    return()
endif()
set(_FINDSTB_INCLUDED TRUE)

# Use the version passed to find_package(), or default to 2.30
if (DEFINED stb_FIND_VERSION AND NOT stb_FIND_VERSION STREQUAL "")
    set(STB_VERSION "${stb_FIND_VERSION}")
else()
    set(STB_VERSION "2fb8c5a3deb2110c89669f8d6f36e5833b556b44")
endif()

message(STATUS "Fetching stb ${STB_VERSION}")

include(FetchContent)

find_program(GIT_EXECUTABLE git)
if (GIT_EXECUTABLE)
    set(STB_FETCH_METHOD "GIT")
else()
    message(FATAL_ERROR "Fetch with zip not supported.")
endif()

if (STB_FETCH_METHOD STREQUAL "GIT")
    FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG        ${STB_VERSION}
    )
endif()

FetchContent_MakeAvailable(stb)

if (NOT TARGET stb)
    add_library(stb INTERFACE)
    target_include_directories(stb SYSTEM INTERFACE "${stb_SOURCE_DIR}")
endif()

if (NOT TARGET stb::stb)
    add_library(stb::stb ALIAS stb)
endif()

set(stb_FOUND       TRUE)
set(stb_LIBRARIES   stb::stb)
set(stb_VERSION     "${STB_VERSION}")
set(stb_INCLUDE_DIR "${stb_SOURCE_DIR}")

set(STB_LICENSE_FILE "${stb_SOURCE_DIR}/LICENSE" CACHE FILEPATH "Path to stb license file")
