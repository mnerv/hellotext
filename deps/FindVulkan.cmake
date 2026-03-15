# ==============================================================================
# Find Vulkan
# ==============================================================================
# This module finds a locally installed Vulkan SDK.
#
# Targets provided:
#   vulkan::vulkan - The Vulkan library target
#
# Variables set:
#   Vulkan_FOUND       - TRUE if Vulkan is available
#   Vulkan_LIBRARIES   - The Vulkan library target (Vulkan::Vulkan)
#   Vulkan_INCLUDE_DIR - Include directory for Vulkan headers
#   Vulkan_VERSION     - Version of the found Vulkan SDK
# ==============================================================================

if (DEFINED _FINDVULKAN_INCLUDED)
    return()
endif()
set(_FINDVULKAN_INCLUDED TRUE)

# detect_vulkan_sdk(<out_inc> <out_lib> <out_ver>)
#
# Detects the Vulkan SDK root and sets the include path, lib path, and version.
# Search order:
#   1. VULKAN_SDK environment variable  (all platforms)
#   2. C:/VulkanSDK/<latest>            (Windows fallback)
#   3. /usr                             (Linux system packages)
#
# Parameters:
#   - OUT_INC  Name of the variable to set with the SDK include path
#   - OUT_LIB  Name of the variable to set with the SDK lib path
#   - OUT_VER  Name of the variable to set with the SDK version string
function(detect_vulkan_sdk OUT_INC OUT_LIB OUT_VER)
    # 1. VULKAN_SDK env var (set by the LunarG installer on all platforms)
    if (DEFINED ENV{VULKAN_SDK})
        set(_ROOT "$ENV{VULKAN_SDK}")
        # The env var already points inside the versioned dir on all platforms
        # e.g. C:/VulkanSDK/1.4.335.0  or  ~/VulkanSDK/1.4.xxx/x86_64
        set(${OUT_VER} "env"              PARENT_SCOPE)
        set(${OUT_INC} "${_ROOT}/Include" PARENT_SCOPE)  # Windows
        set(${OUT_LIB} "${_ROOT}/Lib"     PARENT_SCOPE)  # Windows
        # On Linux/macOS the SDK uses lowercase paths
        if (NOT WIN32)
            set(${OUT_INC} "${_ROOT}/include" PARENT_SCOPE)
            set(${OUT_LIB} "${_ROOT}/lib"     PARENT_SCOPE)
        endif()
        return()
    endif()

    # 2. Windows: scan C:/VulkanSDK for latest version directory
    if (WIN32)
        set(_ROOT "C:/VulkanSDK")
        file(GLOB _VERSIONS RELATIVE "${_ROOT}" "${_ROOT}/*")
        list(SORT _VERSIONS ORDER DESCENDING)
        list(GET _VERSIONS 0 _VERSION)
        set(${OUT_VER} "${_VERSION}"                  PARENT_SCOPE)
        set(${OUT_INC} "${_ROOT}/${_VERSION}/Include" PARENT_SCOPE)
        set(${OUT_LIB} "${_ROOT}/${_VERSION}/Lib"     PARENT_SCOPE)
        return()
    endif()

    # 3. Linux/macOS: fall back to system paths
    set(${OUT_VER} "system"       PARENT_SCOPE)
    set(${OUT_INC} "/usr/include" PARENT_SCOPE)
    set(${OUT_LIB} "/usr/lib"     PARENT_SCOPE)
endfunction()

detect_vulkan_sdk(_Vulkan_INC _Vulkan_LIB Vulkan_VERSION)
message(STATUS "Vulkan SDK: ${Vulkan_VERSION}")

# Library name differs by platform
if (WIN32)
    set(_Vulkan_LIB_NAME "vulkan-1")
else()
    set(_Vulkan_LIB_NAME "vulkan")
endif()

find_library(_Vulkan_LIBRARY ${_Vulkan_LIB_NAME} HINTS "${_Vulkan_LIB}")

if (NOT _Vulkan_LIBRARY)
    if (Vulkan_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find ${_Vulkan_LIB_NAME} in ${_Vulkan_LIB}")
    else()
        message(STATUS "  [ ] Vulkan (${_Vulkan_LIB_NAME})")
        return()
    endif()
endif()

message(STATUS "  [x] Vulkan (${_Vulkan_LIBRARY})")

if (NOT TARGET vulkan::vulkan)
    add_library(vulkan::vulkan UNKNOWN IMPORTED)
    set_target_properties(vulkan::vulkan PROPERTIES
        IMPORTED_LOCATION                    "${_Vulkan_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES        "${_Vulkan_INC}"
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_Vulkan_INC}"
    )
endif()

set(Vulkan_FOUND       TRUE)
set(Vulkan_LIBRARIES   Vulkan::Vulkan)
set(Vulkan_INCLUDE_DIR "${_Vulkan_INC}")

unset(_Vulkan_LIBRARY CACHE)
