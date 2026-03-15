# ==============================================================================
# Find tinyobjloader
# ==============================================================================
# This module fetches the tinyobjloader library.
#
# Targets provided:
#   tinyobjloader::tinyobjloader - The tinyobjloader library target
#
# Variables set:
#   tinyobjloader_FOUND       - TRUE if tinyobjloader is available
#   tinyobjloader_LIBRARIES   - The tinyobjloader library target
#   tinyobjloader_INCLUDE_DIR - Include directories for tinyobjloader
#   tinyobjloader_VERSION     - Version of tinyobjloader (if available)
# ==============================================================================

if (DEFINED _FINDTINYOBJLOADER_INCLUDED)
    return()
endif()
set(_FINDTINYOBJLOADER_INCLUDED TRUE)

# Use the version passed to find_package(), or default to v2.0.0rc13
if (DEFINED tinyobjloader_FIND_VERSION AND NOT tinyobjloader_FIND_VERSION STREQUAL "")
    set(TINYOBJLOADER_VERSION "${tinyobjloader_FIND_VERSION}")
else()
    set(TINYOBJLOADER_VERSION "v2.0.0rc13")
endif()

message(STATUS "Fetching tinyobjloader ${TINYOBJLOADER_VERSION} via FetchContent...")

include(FetchContent)

find_program(GIT_EXECUTABLE git)
if (GIT_EXECUTABLE)
    set(TINYOBJLOADER_FETCH_METHOD "GIT")
else()
    message(FATAL_ERROR "Fetch with zip not supported.")
endif()

if (TINYOBJLOADER_FETCH_METHOD STREQUAL "GIT")
    FetchContent_Declare(
        tinyobjloader
        GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
        GIT_TAG        ${TINYOBJLOADER_VERSION}
    )
endif()

FetchContent_MakeAvailable(tinyobjloader)

if (NOT TARGET tinyobjloader::tinyobjloader)
    if (TARGET tinyobjloader)
        add_library(tinyobjloader::tinyobjloader ALIAS tinyobjloader)
    else()
        message(FATAL_ERROR "Could not fetch tinyobjloader; no target tinyobjloader or tinyobjloader::tinyobjloader available")
    endif()
endif()

set(tinyobjloader_FOUND      TRUE)
set(tinyobjloader_LIBRARIES  tinyobjloader::tinyobjloader)
set(tinyobjloader_VERSION    "${TINYOBJLOADER_VERSION}")
get_target_property(_tol_inc tinyobjloader::tinyobjloader INTERFACE_INCLUDE_DIRECTORIES)
set(tinyobjloader_INCLUDE_DIR "${_tol_inc}")

# Mark tinyobjloader includes as SYSTEM to suppress warnings from its headers
if (_tol_inc AND TARGET tinyobjloader)
    set_target_properties(tinyobjloader PROPERTIES
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_tol_inc}"
    )
endif()

set(TINYOBJLOADER_LICENSE_FILE "${tinyobjloader_SOURCE_DIR}/LICENSE" CACHE FILEPATH "Path to tinyobjloader license file")
