cmake_minimum_required(VERSION 3.0)

add_library(actions STATIC
    src/ActionManager.cpp
)

target_include_directories(actions PUBLIC inc)

