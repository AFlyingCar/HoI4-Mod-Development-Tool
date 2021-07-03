cmake_minimum_required(VERSION 3.0)

add_library(logging STATIC
    src/Logger.cpp
    src/Message.cpp
    src/Source.cpp
    src/Format.cpp
    src/ConsoleOutputFunctions.cpp
)

target_include_directories(logging PUBLIC inc)

