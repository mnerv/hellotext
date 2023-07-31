cmake_minimum_required(VERSION 3.21)
project(stb VERSION 0.1.0)

add_library(stb STATIC
    "stb_image_write.h"
    "stb_image.h"
    "stb_image.cpp"
)
target_include_directories(stb PUBLIC "${PROJECT_SOURCE_DIR}")
target_compile_features(stb PRIVATE cxx_std_20)
add_library(stb::stb ALIAS stb)

include(CMakePackageConfigHelpers)
set(targets_export_name stb-targets)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/stb-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/support/cmake/stb-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/stb-config.cmake"
    INSTALL_DESTINATION "lib/cmake/stb"
)

set(INSTALL_TARGETS stb)
export(TARGETS ${INSTALL_TARGETS} NAMESPACE stb::
    FILE ${PROJECT_BINARY_DIR}/${targets_export_name}.cmake)

