# ==============================================================================
# IDE Integration
# ==============================================================================
# This module configures IDE-specific settings for better project organization.
# Currently supports Visual Studio and Xcode by grouping dependency targets
# into folders for cleaner navigation in the IDE's solution/project explorer.
#
# Folder structure created:
#   deps/           - Third-party libraries (fmt, glad, imgui, sqlite3, etc.)
#   deps/GLFW/      - GLFW library and related targets
#   deps/gtest/     - Google Test targets
#   deps/tinyobj/   - tinyobjloader and its uninstall target
# ==============================================================================

# ------------------------------------------------------------------------------
# Visual Studio and Xcode Folder Organization
# ------------------------------------------------------------------------------
# Group dependency targets into logical folders for easier navigation.
# This only affects IDE project generation and has no effect on the build.
# Helper function to set folder property if target exists
function(set_target_folder target folder)
    if (TARGET ${target})
        set_target_properties(${target} PROPERTIES FOLDER ${folder})
    endif()
endfunction()

if (CMAKE_GENERATOR MATCHES "Visual Studio" OR CMAKE_GENERATOR MATCHES "Xcode")
    # General deps
    # Networking / async
    set_target_folder(asio deps)

    # Text / parsing
    set_target_folder(ctre    deps)
    set_target_folder(fmt     deps)
    set_target_folder(utf8cpp deps)

    # Math / geometry
    set_target_folder(earcut deps)
    set_target_folder(glm    deps)

    # Graphics / windowing
    set_target_folder(glad          deps)
    set_target_folder(imgui         deps)
    set_target_folder(imgui_backend deps)
    set_target_folder(implot        deps)
    set_target_folder(sokol         deps)
    set_target_folder(stb           deps)

    # Audio
    set_target_folder(miniaudio deps)

    # Map / geo
    set_target_folder(protozero deps)
    set_target_folder(mvt       deps)

    # Data / storage
    set_target_folder(sqlite3    deps)
    set_target_folder(zlib       deps)
    set_target_folder(zlibstatic deps)

    # Logging
    set_target_folder(spdlog deps)

    # 3D assets
    set_target_folder(tinyobjloader deps/tinyobj)

    # GLFW
    set_target_folder(glfw            deps/GLFW)
    set_target_folder(update_mappings deps/GLFW)

    # GTest
    set_target_folder(gtest      deps/gtest)
    set_target_folder(gtest_main deps/gtest)
    set_target_folder(gmock      deps/gtest)
    set_target_folder(gmock_main deps/gtest)

    # Hide uninstall target
    if (TARGET uninstall)
        set_target_properties(uninstall PROPERTIES FOLDER deps/tinyobj)
        set_target_properties(uninstall PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()
endif()
