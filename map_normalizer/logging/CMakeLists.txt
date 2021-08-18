cmake_minimum_required(VERSION 3.0)

add_library(logging SHARED
    src/Logger.cpp
    src/Message.cpp
    src/Source.cpp
    src/Format.cpp
    src/ConsoleOutputFunctions.cpp
)

target_include_directories(logging PUBLIC inc)

if(NOT WIN32)
    target_link_libraries(logging PRIVATE dl)
endif()

