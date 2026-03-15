# ==============================================================================
# Find ctre
# ==============================================================================
# This module fetches the CTRE (Compile Time Regular Expressions) library.
#
# Targets provided:
#   ctre::ctre - The ctre library target
#
# Variables set:
#   ctre_FOUND       - TRUE if ctre is available
#   ctre_LIBRARIES   - The ctre library target (ctre::ctre)
#   ctre_INCLUDE_DIR - Include directories for ctre
#   ctre_VERSION     - Version of ctre (if available)
# ==============================================================================

if (DEFINED _FINDCTRE_INCLUDED)
    return()
endif()
set(_FINDCTRE_INCLUDED TRUE)

# Use the version passed to find_package(), or default to 3.10.0
if (DEFINED ctre_FIND_VERSION AND NOT ctre_FIND_VERSION STREQUAL "")
    set(CTRE_VERSION "${ctre_FIND_VERSION}")
else()
    set(CTRE_VERSION "3.10.0")
endif()

message(STATUS "Fetching ctre ${CTRE_VERSION}")

include(FetchContent)

find_program(GIT_EXECUTABLE git)
if (GIT_EXECUTABLE)
    set(CTRE_FETCH_METHOD "GIT")
else()
    message(FATAL_ERROR "Fetch with zip not supported.")
endif()

if (CTRE_FETCH_METHOD STREQUAL "GIT")
    # Turn off BUILD_TESTING globally to prevent CTest from being included in CTRE
    set(BUILD_TESTING OFF CACHE BOOL "Disable testing globally" FORCE)
    # Set the CTRE_BUILD_TESTS option before including the CTRE library
    set(CTRE_BUILD_TESTS OFF CACHE BOOL "Build ctre Tests" FORCE)
    set(CTRE_BUILD_PACKAGE OFF CACHE BOOL "Build ctre Package" FORCE)

    FetchContent_Declare(
        ctre
        GIT_REPOSITORY https://github.com/hanickadot/compile-time-regular-expressions.git
        GIT_TAG        v${CTRE_VERSION}
    )
endif()

FetchContent_MakeAvailable(ctre)

# Turn BUILD_TESTING back on after including CTRE
set(BUILD_TESTING ON CACHE BOOL "Enable testing globally" FORCE)

if (NOT TARGET ctre::ctre)
    if (TARGET ctre)
        add_library(ctre::ctre ALIAS ctre)
    else()
        message(FATAL_ERROR "Could not fetch ctre; no target ctre or ctre::ctre available")
    endif()
endif()

set(ctre_FOUND        TRUE)
set(ctre_LIBRARIES    ctre::ctre)
set(ctre_VERSION      "${CTRE_VERSION}")
get_target_property(_ctre_inc ctre::ctre INTERFACE_INCLUDE_DIRECTORIES)
set(ctre_INCLUDE_DIR  "${_ctre_inc}")

# Mark ctre includes as SYSTEM to suppress warnings from its headers
if (_ctre_inc AND TARGET ctre)
    set_target_properties(ctre PROPERTIES
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_ctre_inc}"
    )
endif()

set(CTRE_LICENSE_FILE "${ctre_SOURCE_DIR}/LICENSE" CACHE FILEPATH "Path to CTRE license file")
