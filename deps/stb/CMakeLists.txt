cmake_minimum_required(VERSION 3.21)
project(stb)
set(TARGET stb)
add_library(${TARGET} STATIC
    "stb_image_write.h"
    "stb_image.h"
    "stb_image.cpp"
)
target_include_directories(${TARGET} PUBLIC
    "${PROJECT_SOURCE_DIR}"
)
target_compile_features(${TARGET} PRIVATE cxx_std_20)
