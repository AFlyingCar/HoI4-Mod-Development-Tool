cmake_minimum_required(VERSION 3.0)

project(MapNormalizerTools CXX ASM)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# TODO: What other MSVC options should we pass?
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Werror -Wextra -Wno-unused-command-line-argument -Wno-sign-compare -Wno-unused-label -Wno-unused-const-variable -Wno-unused-parameter -Wno-unused-function)
endif()

if(WIN32 AND NOT MSVC)
    add_compile_options(-Xclang -flto-visibility-public-std -m32)
    add_definitions(-D_REENTRANT)
endif()

add_subdirectory(map_normalizer)

