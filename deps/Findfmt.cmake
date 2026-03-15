# ==============================================================================
# Find fmt
# ==============================================================================
# This module fetches the fmt formatting library.
#
# Targets provided:
#   fmt::fmt - The fmt library target
#
# Variables set:
#   fmt_FOUND       - TRUE if fmt is available
#   fmt_LIBRARIES   - The fmt library target (fmt::fmt)
#   fmt_INCLUDE_DIR - Include directories for fmt
#   fmt_VERSION     - Version of fmt (if available)
# ==============================================================================

if (DEFINED _FINDFMT_INCLUDED)
    return()
endif()
set(_FINDFMT_INCLUDED TRUE)

# Use the version passed to find_package(), or default to 12.1.0
if (DEFINED fmt_FIND_VERSION AND NOT fmt_FIND_VERSION STREQUAL "")
    set(FMT_VERSION "${fmt_FIND_VERSION}")
else()
    set(FMT_VERSION "12.1.0")
endif()

message(STATUS "Fetching fmt ${FMT_VERSION}")

include(FetchContent)

find_program(GIT_EXECUTABLE git)
if (GIT_EXECUTABLE)
    set(FMT_FETCH_METHOD "GIT")
else()
    set(FMT_FETCH_METHOD "ZIP")
endif()

if (FMT_FETCH_METHOD STREQUAL "GIT")
    FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG        ${FMT_VERSION}
    )
else()
    FetchContent_Declare(
        fmt
        URL https://github.com/fmtlib/fmt/releases/download/${FMT_VERSION}/fmt-${FMT_VERSION}.zip
    )
endif()

FetchContent_MakeAvailable(fmt)

if (NOT TARGET fmt::fmt)
    if (TARGET fmt)
        add_library(fmt::fmt ALIAS fmt)
    else()
        message(FATAL_ERROR "Could not fetch fmt; no target fmt or fmt::fmt available")
    endif()
endif()

set(fmt_FOUND        TRUE)
set(fmt_LIBRARIES    fmt::fmt)
set(fmt_VERSION      "${FMT_VERSION}")
get_target_property(_fmt_inc fmt::fmt INTERFACE_INCLUDE_DIRECTORIES)
set(fmt_INCLUDE_DIR  "${_fmt_inc}")

# Mark fmt includes as SYSTEM to suppress warnings from its headers
if (_fmt_inc AND TARGET fmt)
    set_target_properties(fmt PROPERTIES
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_fmt_inc}"
    )
endif()

# Suppress warnings from fmt's own compiled sources (third-party code)
if (TARGET fmt)
    target_compile_options(fmt PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wno-unused-result>
    )
endif()

set(FMT_LICENSE_FILE "${fmt_SOURCE_DIR}/LICENSE" CACHE FILEPATH "Path to fmt license file")
