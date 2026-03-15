# ==============================================================================
# Find utf8cpp
# ==============================================================================
# This module fetches the UTF-8 with C++ library.
#
# Targets provided:
#   utf8cpp::utf8cpp - The utf8cpp library target
#
# Variables set:
#   utf8cpp_FOUND       - TRUE if utf8cpp is available
#   utf8cpp_LIBRARIES   - The utf8cpp library target (utf8cpp::utf8cpp)
#   utf8cpp_INCLUDE_DIR - Include directories for utf8cpp
#   utf8cpp_VERSION     - Version of utf8cpp
# ==============================================================================

if (DEFINED _FINDUTF8CPP_INCLUDED)
    return()
endif()
set(_FINDUTF8CPP_INCLUDED TRUE)

# Use the version passed to find_package(), or default to 4.0.6
if (DEFINED utf8cpp_FIND_VERSION AND NOT utf8cpp_FIND_VERSION STREQUAL "")
    set(UTF8CPP_VERSION "${utf8cpp_FIND_VERSION}")
else()
    set(UTF8CPP_VERSION "4.0.6")
endif()

message(STATUS "Fetching utf8cpp ${UTF8CPP_VERSION}")

include(FetchContent)

find_program(GIT_EXECUTABLE git)
if (GIT_EXECUTABLE)
    set(UTF8CPP_FETCH_METHOD "GIT")
else()
    set(UTF8CPP_FETCH_METHOD "ZIP")
endif()

if (UTF8CPP_FETCH_METHOD STREQUAL "GIT")
    FetchContent_Declare(
        utf8cpp
        GIT_REPOSITORY https://github.com/nemtrif/utfcpp.git
        GIT_TAG        v${UTF8CPP_VERSION}
    )
else()
    FetchContent_Declare(
        utf8cpp
        URL https://github.com/nemtrif/utfcpp/archive/refs/tags/v${UTF8CPP_VERSION}.zip
    )
endif()

FetchContent_MakeAvailable(utf8cpp)

if (NOT TARGET utf8cpp::utf8cpp)
    if (TARGET utf8cpp)
        add_library(utf8cpp::utf8cpp ALIAS utf8cpp)
    else()
        message(FATAL_ERROR "Could not fetch utf8cpp; no target utf8cpp or utf8cpp::utf8cpp available")
    endif()
endif()

set(utf8cpp_FOUND        TRUE)
set(utf8cpp_LIBRARIES    utf8cpp::utf8cpp)
set(utf8cpp_VERSION      "${UTF8CPP_VERSION}")
get_target_property(_utf8cpp_inc utf8cpp::utf8cpp INTERFACE_INCLUDE_DIRECTORIES)
set(utf8cpp_INCLUDE_DIR  "${_utf8cpp_inc}")

# Mark utf8cpp includes as SYSTEM to suppress warnings from its headers
if (_utf8cpp_inc AND TARGET utf8cpp)
    set_target_properties(utf8cpp PROPERTIES
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_utf8cpp_inc}"
    )
endif()

set(UTF8CPP_LICENSE_FILE "${utf8cpp_SOURCE_DIR}/LICENSE" CACHE FILEPATH "Path to utf8cpp license file")
