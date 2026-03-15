# ==============================================================================
# Find asio
# ==============================================================================
# This module fetches the asio networking library.
#
# Targets provided:
#   asio::asio - The asio library target
#
# Variables set:
#   asio_FOUND       - TRUE if asio is available
#   asio_LIBRARIES   - The asio library target (asio::asio)
#   asio_INCLUDE_DIR - Include directories for asio
#   asio_VERSION     - Version of asio (if available)
# ==============================================================================

if (DEFINED _FINDASIO_INCLUDED)
    return()
endif()
set(_FINDASIO_INCLUDED TRUE)

# Use the version passed to find_package(), or default to 1.32.0
if (DEFINED asio_FIND_VERSION AND NOT asio_FIND_VERSION STREQUAL "")
    set(ASIO_VERSION "${asio_FIND_VERSION}")
else()
    set(ASIO_VERSION "1.30.2")
endif()

message(STATUS "Fetching asio ${ASIO_VERSION}")

include(FetchContent)

find_program(GIT_EXECUTABLE git)
if (GIT_EXECUTABLE)
    set(ASIO_FETCH_METHOD "GIT")
else()
    message(FATAL_ERROR "Fetch method ZIP not supported")
endif()

if (ASIO_FETCH_METHOD STREQUAL "GIT")
    FetchContent_Declare(
        asio
        GIT_REPOSITORY https://github.com/mononerv/asio.git
        GIT_TAG        ${ASIO_VERSION}
    )
endif()

FetchContent_MakeAvailable(asio)

if (NOT TARGET asio::asio)
    if (TARGET asio)
        add_library(asio::asio ALIAS asio)
    else()
        message(FATAL_ERROR "Could not fetch asio; no target asio or asio::asio available")
    endif()
endif()

set(asio_FOUND       TRUE)
set(asio_LIBRARIES   asio::asio)
set(asio_VERSION     "${ASIO_VERSION}")
get_target_property(_asio_inc asio::asio INTERFACE_INCLUDE_DIRECTORIES)
set(asio_INCLUDE_DIR "${_asio_inc}")

# Mark asio includes as SYSTEM to suppress warnings from its headers
if (_asio_inc AND TARGET asio)
    set_target_properties(asio PROPERTIES
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_asio_inc}"
    )
endif()

set(ASIO_LICENSE_FILE "${asio_SOURCE_DIR}/LICENSE_1_0.txt" CACHE FILEPATH "Path to Asio license file")
