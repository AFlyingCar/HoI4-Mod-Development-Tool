cmake_minimum_required(VERSION 3.0)

project(MapNormalizerTools CXX ASM)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Set up cmake prefix path to search for .cmake files
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${CMAKE_PREFIX_PATH}
)

# TODO: What other MSVC options should we pass?
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Werror -Wextra -Wno-unused-command-line-argument -Wno-sign-compare -Wno-unused-label -Wno-unused-const-variable -Wno-unused-parameter -Wno-unused-function)
endif()

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Xclang -flto-visibility-public-std -m32)
    add_definitions(-D_REENTRANT)
endif()

add_subdirectory(map_normalizer)

add_subdirectory(tests)

