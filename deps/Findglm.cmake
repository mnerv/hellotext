# ==============================================================================
# Find glm
# ==============================================================================
# This module fetches the GLM (OpenGL Mathematics) library.
#
# Targets provided:
#   glm::glm - The glm library target
#
# Variables set:
#   glm_FOUND       - TRUE if glm is available
#   glm_LIBRARIES   - The glm library target (glm::glm)
#   glm_INCLUDE_DIR - Include directories for glm
#   glm_VERSION     - Version of glm (if available)
# ==============================================================================

if (DEFINED _FINDGLM_INCLUDED)
    return()
endif()
set(_FINDGLM_INCLUDED TRUE)

# Use the version passed to find_package(), or default to 1.0.1
if (DEFINED glm_FIND_VERSION AND NOT glm_FIND_VERSION STREQUAL "")
    set(GLM_VERSION "${glm_FIND_VERSION}")
else()
    set(GLM_VERSION "1.0.1")
endif()

message(STATUS "Fetching glm ${GLM_VERSION}")

include(FetchContent)

find_program(GIT_EXECUTABLE git)
if (GIT_EXECUTABLE)
    set(GLM_FETCH_METHOD "GIT")
else()
    message(FATAL_ERROR "Fetch with zip not supported.")
endif()

set(_glm_warn_deprecated "${CMAKE_WARN_DEPRECATED}")
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "Suppress deprecated warnings while fetching glm" FORCE)

if (GLM_FETCH_METHOD STREQUAL "GIT")
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        ${GLM_VERSION}
    )
endif()

FetchContent_MakeAvailable(glm)

set(CMAKE_WARN_DEPRECATED "${_glm_warn_deprecated}" CACHE BOOL "Restore deprecation warnings" FORCE)

if (NOT TARGET glm::glm)
    if (TARGET glm)
        add_library(glm::glm ALIAS glm)
    else()
        message(FATAL_ERROR "Could not fetch glm; no target glm or glm::glm available")
    endif()
endif()

set(glm_FOUND        TRUE)
set(glm_LIBRARIES    glm::glm)
set(glm_VERSION      "${GLM_VERSION}")
get_target_property(_glm_inc glm::glm INTERFACE_INCLUDE_DIRECTORIES)
set(glm_INCLUDE_DIR  "${_glm_inc}")

# Mark glm includes as SYSTEM to suppress warnings from its headers
if (_glm_inc AND TARGET glm)
    set_target_properties(glm PROPERTIES
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_glm_inc}"
    )
endif()

set(GLM_LICENSE_FILE "${glm_SOURCE_DIR}/copying.txt" CACHE FILEPATH "Path to GLM license file")
