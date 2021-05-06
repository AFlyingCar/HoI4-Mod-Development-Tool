cmake_minimum_required(VERSION 3.0)

add_library(common STATIC
    src/BitMap.cpp
    src/Types.cpp
    src/Util.cpp
    src/Logger.cpp
    src/UniqueColorGenerator.cpp
    src/Version.cpp
)

target_include_directories(common PUBLIC inc)

target_link_libraries(common PUBLIC unique_colors)

target_link_libraries(common PRIVATE tool_version)

