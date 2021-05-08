cmake_minimum_required(VERSION 3.0)

add_library(logging STATIC
    src/Logger.cpp
)

target_include_directories(logging PUBLIC inc)

target_link_libraries(logging common)

