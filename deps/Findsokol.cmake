# ==============================================================================
# Findsokol.cmake
# ==============================================================================
# This module fetches the sokol single-header C libraries.
# https://github.com/floooh/sokol
#
# Targets provided:
#   sokol::sokol - Interface target (headers at repo root)
#
# Variables set:
#   sokol_FOUND       - TRUE if sokol is available
#   sokol_LIBRARIES   - The library target (sokol::sokol)
#   sokol_INCLUDE_DIR - Include directory (repo root)
# ==============================================================================

if (DEFINED _FINDSOKOL_INCLUDED)
    return()
endif()
set(_FINDSOKOL_INCLUDED TRUE)

if (DEFINED sokol_FIND_VERSION AND NOT sokol_FIND_VERSION STREQUAL "")
    set(SOKOL_TAG "${sokol_FIND_VERSION}")
else()
    set(SOKOL_TAG "223caffb4213a7605c54354dba65c7827d8c0bb9")
endif()

message(STATUS "Fetching sokol @ ${SOKOL_TAG}")

include(FetchContent)

find_program(GIT_EXECUTABLE git)
if (NOT GIT_EXECUTABLE)
    message(FATAL_ERROR "Fetch with zip not supported.")
endif()

FetchContent_Declare(
    sokol
    GIT_REPOSITORY https://github.com/floooh/sokol.git
    GIT_TAG        ${SOKOL_TAG}
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  _unused  # no CMakeLists.txt here; skips add_subdirectory
)

# SOURCE_SUBDIR points to a non-existent directory so FetchContent_MakeAvailable
# skips add_subdirectory; sokol is single-header files, we only need the root dir.
FetchContent_MakeAvailable(sokol)

add_library(sokol INTERFACE)
target_include_directories(sokol SYSTEM INTERFACE "${sokol_SOURCE_DIR}")
add_library(sokol::sokol ALIAS sokol)

set(sokol_FOUND       TRUE)
set(sokol_LIBRARIES   sokol::sokol)
set(sokol_INCLUDE_DIR "${sokol_SOURCE_DIR}")

set(SOKOL_LICENSE_FILE "${sokol_SOURCE_DIR}/LICENSE" CACHE FILEPATH "Path to sokol license file")
