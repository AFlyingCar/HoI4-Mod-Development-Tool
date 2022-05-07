cmake_minimum_required(VERSION 3.0)

#
# Custom commands to output the binary files for each set of colors
#

add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/lakes.bin"
    COMMAND python3 "${CMAKE_CURRENT_SOURCE_DIR}/color_generator.py" lakes
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/color_generator.py"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/lands.bin"
    COMMAND python3 "${CMAKE_CURRENT_SOURCE_DIR}/color_generator.py" lands
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/color_generator.py"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/seas.bin"
    COMMAND python3 "${CMAKE_CURRENT_SOURCE_DIR}/color_generator.py" seas
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/color_generator.py"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/unknowns.bin"
    COMMAND python3 "${CMAKE_CURRENT_SOURCE_DIR}/color_generator.py" unknowns
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/color_generator.py"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

add_custom_target(generated_colors
    DEPENDS "${CMAKE_BINARY_DIR}/lakes.bin" "${CMAKE_BINARY_DIR}/lands.bin" "${CMAKE_BINARY_DIR}/seas.bin" "${CMAKE_BINARY_DIR}/unknowns.bin"
    )

# See mod_dev_tool/common/CMakeLists.txt for why this is necessary
# Also, because CMake is weird, we have to do this once for each dependended on
#  binary file
set_source_files_properties(src/ColorArray_x86.asm
    PROPERTIES
    OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/lakes.bin"
    )
set_source_files_properties(src/ColorArray_x86.asm
    PROPERTIES
    OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/lands.bin"
    )
set_source_files_properties(src/ColorArray_x86.asm
    PROPERTIES
    OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/seas.bin"
    )
set_source_files_properties(src/ColorArray_x86.asm
    PROPERTIES
    OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/unknowns.bin"
    )


add_library(unique_colors STATIC
    src/ColorArray_x86.s
)

target_include_directories(unique_colors PUBLIC inc)
target_include_directories(unique_colors PRIVATE ${CMAKE_BINARY_DIR})

add_dependencies(unique_colors generated_colors)

