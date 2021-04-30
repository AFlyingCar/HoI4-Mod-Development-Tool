cmake_minimum_required(VERSION 3.0)

find_package(json REQUIRED)

add_library(project STATIC
    src/Project.cpp
    src/MapProject.cpp
)

target_include_directories(project PUBLIC inc)

target_link_libraries(project PRIVATE nlohmann_json::nlohmann_json)

# Link against local libraries
target_link_libraries(project PUBLIC common province_utils)

