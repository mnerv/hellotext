# ==============================================================================
# Find miniaudio
# ==============================================================================
# This module fetches the miniaudio audio playback and capture library.
#
# Targets provided:
#   miniaudio::miniaudio - The miniaudio library target
#
# Variables set:
#   miniaudio_FOUND       - TRUE if miniaudio is available
#   miniaudio_LIBRARIES   - The miniaudio library target (miniaudio::miniaudio)
#   miniaudio_INCLUDE_DIR - Include directories for miniaudio
#   miniaudio_VERSION     - Version of miniaudio
# ==============================================================================

if (DEFINED _FINDMINIAUDIO_INCLUDED)
    return()
endif()
set(_FINDMINIAUDIO_INCLUDED TRUE)

# Use the version passed to find_package(), or default to 0.11.21
if (DEFINED miniaudio_FIND_VERSION AND NOT miniaudio_FIND_VERSION STREQUAL "")
    set(MINIAUDIO_VERSION "${miniaudio_FIND_VERSION}")
else()
    set(MINIAUDIO_VERSION "0.11.21")
endif()

message(STATUS "Fetching miniaudio ${MINIAUDIO_VERSION}")

include(FetchContent)

find_program(GIT_EXECUTABLE git)
if (GIT_EXECUTABLE)
    set(MINIAUDIO_FETCH_METHOD "GIT")
else()
    message(FATAL_ERROR "Fetch with zip not supported.")
endif()

if (MINIAUDIO_FETCH_METHOD STREQUAL "GIT")
    FetchContent_Declare(
        miniaudio
        GIT_REPOSITORY https://github.com/mackron/miniaudio.git
        GIT_TAG        ${MINIAUDIO_VERSION}
    )
endif()

FetchContent_MakeAvailable(miniaudio)

if (NOT TARGET miniaudio)
    add_library(miniaudio INTERFACE)
    target_include_directories(miniaudio SYSTEM INTERFACE "${miniaudio_SOURCE_DIR}")
endif()

if (NOT TARGET miniaudio::miniaudio)
    add_library(miniaudio::miniaudio ALIAS miniaudio)
endif()

set(miniaudio_FOUND       TRUE)
set(miniaudio_LIBRARIES   miniaudio::miniaudio)
set(miniaudio_VERSION     "${MINIAUDIO_VERSION}")
set(miniaudio_INCLUDE_DIR "${miniaudio_SOURCE_DIR}")

set(MINIAUDIO_LICENSE_FILE "${miniaudio_SOURCE_DIR}/LICENSE" CACHE FILEPATH "Path to miniaudio license file")
