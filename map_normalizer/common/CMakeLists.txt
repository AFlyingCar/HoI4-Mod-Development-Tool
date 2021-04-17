cmake_minimum_required(VERSION 3.0)

add_library(common STATIC
    src/BitMap.cpp
    src/Types.cpp
    src/Util.cpp
    src/Logger.cpp
)

target_include_directories(common PUBLIC inc)

