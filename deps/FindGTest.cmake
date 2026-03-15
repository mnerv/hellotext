# ==============================================================================
# Find GTest
# ==============================================================================
# This module fetches the Google Test framework.
#
# Targets provided:
#   GTest::gtest      - The gtest library target
#   GTest::gtest_main - The gtest_main library target
#
# Variables set:
#   GTEST_FOUND        - TRUE if GTest is available
#   GTEST_LIBRARIES    - The gtest library targets (gtest gtest_main)
#   GTEST_INCLUDE_DIRS - Include directories for GTest
#   GTEST_VERSION      - Version of GTest
# ==============================================================================

if(DEFINED _FINDGTEST_INCLUDED)
    return()
endif()
set(_FINDGTEST_INCLUDED TRUE)

# Use the version passed to find_package(), or default to 1.17.0
if (DEFINED gtest_FIND_VERSION AND NOT gtest_FIND_VERSION STREQUAL "")
    set(GTEST_VERSION "${gtest_FIND_VERSION}")
else()
    set(GTEST_VERSION "1.17.0")
endif()

message(STATUS "Fetching GoogleTest ${GTEST_VERSION}")
include(FetchContent)

find_program(GIT_EXECUTABLE git)
if (GIT_EXECUTABLE)
    set(GTEST_FETCH_METHOD "GIT")
else()
    set(GTEST_FETCH_METHOD "ZIP")
endif()

if (GTEST_FETCH_METHOD STREQUAL "GIT")
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v${GTEST_VERSION}
    )
else()
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v${GTEST_VERSION}.zip
    )
endif()

FetchContent_MakeAvailable(googletest)

message(STATUS "{GoogleTest} version: ${GTEST_VERSION}")

# Provide in-tree aliases to the raw library targets
if(TARGET gtest AND NOT TARGET GTest::gtest)
    add_library(GTest::gtest ALIAS gtest)
endif()
if(TARGET gtest_main AND NOT TARGET GTest::gtest_main)
    add_library(GTest::gtest_main ALIAS gtest_main)
endif()

set(GTEST_INCLUDE_DIRS ${googletest_SOURCE_DIR}/googletest/include)
set(GTEST_LIBRARIES gtest gtest_main)
set(GTEST_FOUND TRUE)

# Mark GTest includes as SYSTEM to suppress warnings from its headers
if (GTEST_INCLUDE_DIRS AND TARGET gtest)
    set_target_properties(gtest PROPERTIES
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${GTEST_INCLUDE_DIRS}"
    )
endif()
if (GTEST_INCLUDE_DIRS AND TARGET gtest_main)
    set_target_properties(gtest_main PROPERTIES
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${GTEST_INCLUDE_DIRS}"
    )
endif()

mark_as_advanced(
    GTest_INCLUDE_DIRS
    GTest_LIBRARY
    GTest_MAIN_LIBRARY
)
