cmake_minimum_required(VERSION 3.0)

add_library(unique_colors STATIC
    src/ColorArray_x86.asm
)

target_include_directories(unique_colors PUBLIC inc)
target_include_directories(unique_colors PRIVATE .)

add_custom_command(OUTPUT lakes.bin
    COMMAND python color_generator.py lakes.bin
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIRECTORY}
    )

add_custom_command(OUTPUT lands.bin
    COMMAND python color_generator.py lands.bin
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIRECTORY}
    )

add_custom_command(OUTPUT seas.bin
    COMMAND python color_generator.py seas.bin
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIRECTORY}
    )

add_custom_command(OUTPUT unknowns.bin
    COMMAND python color_generator.py unknowns.bin
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIRECTORY}
    )



