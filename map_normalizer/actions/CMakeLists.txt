cmake_minimum_required(VERSION 3.0)

add_library(actions STATIC
    src/ActionManager.cpp
    src/CreateRemoveContinentAction.cpp
)

target_include_directories(actions PUBLIC inc)

target_link_libraries(actions common project)

