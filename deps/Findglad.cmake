# ==============================================================================
# Find glad
# ==============================================================================
# This module fetches the glad OpenGL loader library.
#
# Targets provided:
#   glad::glad - The glad library target
#
# Variables set:
#   glad_FOUND       - TRUE if glad is available
#   glad_LIBRARIES   - The glad library target (glad::glad)
#   glad_INCLUDE_DIR - Include directories for glad
#   glad_VERSION     - Version of glad (commit hash or tag)
# ==============================================================================

if (DEFINED _FINDGLAD_INCLUDED)
    return()
endif()
set(_FINDGLAD_INCLUDED TRUE)

# Use the version passed to find_package(), or default to commit hash
if (DEFINED glad_FIND_VERSION AND NOT glad_FIND_VERSION STREQUAL "")
    set(GLAD_VERSION "${glad_FIND_VERSION}")
else()
    set(GLAD_VERSION "f4759d7c5143c0a23391ab05caaf43052cefdd65")
endif()

message(STATUS "Fetching glad ${GLAD_VERSION}")

include(FetchContent)

find_program(GIT_EXECUTABLE git)
if (GIT_EXECUTABLE)
    set(GLAD_FETCH_METHOD "GIT")
else()
    message(FATAL_ERROR "Fetch with zip not supported.")
endif()

if (GLAD_FETCH_METHOD STREQUAL "GIT")
    FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/mononerv/glad.git
        GIT_TAG        ${GLAD_VERSION}
    )
endif()

FetchContent_MakeAvailable(glad)

if (NOT TARGET glad::glad)
    if (TARGET glad)
        add_library(glad::glad ALIAS glad)
    else()
        message(FATAL_ERROR "Could not fetch glad; no target glad or glad::glad available")
    endif()
endif()

set(glad_FOUND        TRUE)
set(glad_LIBRARIES    glad::glad)
set(glad_VERSION      "${GLAD_VERSION}")
get_target_property(_glad_inc glad::glad INTERFACE_INCLUDE_DIRECTORIES)
set(glad_INCLUDE_DIR  "${_glad_inc}")

# Mark glad includes as SYSTEM to suppress warnings from its headers
if (_glad_inc AND TARGET glad)
    set_target_properties(glad PROPERTIES
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_glad_inc}"
    )
endif()
