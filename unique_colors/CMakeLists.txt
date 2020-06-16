cmake_minimum_required(VERSION 3.0)

add_library(unique_colors STATIC
    src/ColorArray_x86.asm
)

target_include_directories(unique_colors PUBLIC inc)
target_include_directories(unique_colors PRIVATE .)

