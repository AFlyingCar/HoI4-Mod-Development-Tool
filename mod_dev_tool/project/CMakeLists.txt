cmake_minimum_required(VERSION 3.0)

find_package(json REQUIRED)
find_package(fifo_map REQUIRED)

add_library(project STATIC
    src/IProject.cpp
    src/HoI4Project.cpp
    src/MapProject.cpp
    src/ProvinceProject.cpp
    src/StateProject.cpp
    src/ContinentProject.cpp
    src/HeightMapProject.cpp
    src/HistoryProject.cpp
    src/RiversProject.cpp
)

target_include_directories(project PUBLIC inc)

target_link_libraries(project PRIVATE nlohmann_json::nlohmann_json)

target_link_libraries(project PUBLIC fifo_map)

add_subdirectory(hierarchy)

# Link against local libraries
target_link_libraries(project PUBLIC common province_utils project_hierarchy)

