# ==============================================================================
# Find freetype
# ==============================================================================
# This module fetches the FreeType font rendering library.
#
# Targets provided:
#   freetype - The FreeType library target
#
# Variables set:
#   freetype_FOUND       - TRUE if FreeType is available
#   freetype_LIBRARIES   - The FreeType library target (freetype)
#   freetype_INCLUDE_DIR - Include directories for FreeType
#   freetype_VERSION     - Version of FreeType (if available)
# ==============================================================================

if (DEFINED _FINDFREETYPE_INCLUDED)
    return()
endif()
set(_FINDFREETYPE_INCLUDED TRUE)

# Use the version passed to find_package(), or default to VER-2-13-3
if (DEFINED freetype_FIND_VERSION AND NOT freetype_FIND_VERSION STREQUAL "")
    set(FREETYPE_VERSION "${freetype_FIND_VERSION}")
else()
    set(FREETYPE_VERSION "VER-2-13-3")
endif()

message(STATUS "Fetching freetype ${FREETYPE_VERSION}")

include(FetchContent)

find_program(GIT_EXECUTABLE git)
if (GIT_EXECUTABLE)
    set(FREETYPE_FETCH_METHOD "GIT")
else()
    set(FREETYPE_FETCH_METHOD "ZIP")
endif()

# Disable optional FreeType dependencies to keep builds lightweight.
set(FT_DISABLE_ZLIB      ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2     ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG       ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ  ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI    ON CACHE BOOL "" FORCE)

if (FREETYPE_FETCH_METHOD STREQUAL "GIT")
    FetchContent_Declare(
        freetype
        GIT_REPOSITORY https://github.com/freetype/freetype.git
        GIT_TAG        ${FREETYPE_VERSION}
    )
else()
    FetchContent_Declare(
        freetype
        URL https://github.com/freetype/freetype/archive/refs/tags/${FREETYPE_VERSION}.zip
    )
endif()

set(CMAKE_WARN_DEPRECATED             OFF CACHE BOOL "" FORCE)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON  CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(freetype)
unset(CMAKE_WARN_DEPRECATED             CACHE)
unset(CMAKE_SUPPRESS_DEVELOPER_WARNINGS CACHE)

if (NOT TARGET freetype)
    if (TARGET freetype::freetype)
        add_library(freetype INTERFACE)
        target_link_libraries(freetype INTERFACE freetype::freetype)
    else()
        message(FATAL_ERROR "Could not fetch freetype; no target freetype or Freetype::Freetype available")
    endif()
endif()

if (TARGET freetype AND NOT TARGET freetype::freetype)
    add_library(freetype::freetype ALIAS freetype)
endif()

set(freetype_FOUND        TRUE)
set(freetype_LIBRARIES    freetype)
set(freetype_VERSION      "${FREETYPE_VERSION}")
get_target_property(_ft_inc freetype INTERFACE_INCLUDE_DIRECTORIES)
if (NOT _ft_inc AND TARGET freetype::freetype)
    get_target_property(_ft_inc freetype::freetype INTERFACE_INCLUDE_DIRECTORIES)
endif()
set(freetype_INCLUDE_DIR  "${_ft_inc}")

# Mark freetype includes as SYSTEM to suppress warnings from its headers
if (_ft_inc AND TARGET freetype)
    set_target_properties(freetype PROPERTIES
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_ft_inc}"
    )
endif()

set(FREETYPE_LICENSE_FILE "${freetype_SOURCE_DIR}/LICENSE.TXT" CACHE FILEPATH "Path to FreeType license file")
