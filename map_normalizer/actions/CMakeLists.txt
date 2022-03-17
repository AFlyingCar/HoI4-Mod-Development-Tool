cmake_minimum_required(VERSION 3.0)

add_library(actions STATIC
    src/ActionManager.cpp
)

target_include_directories(actions PUBLIC inc)

# Link against local libraries
# target_link_libraries(actions PUBLIC common province_utils)

