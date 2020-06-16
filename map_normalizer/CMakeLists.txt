cmake_minimum_required(VERSION 3.0)

find_package(SDL2 REQUIRED)

add_executable(map_normalizer
    src/ArgParser.cpp
    src/BitMap.cpp
    src/GraphicalDebugger.cpp
    src/Logger.cpp
    src/ProvinceMapBuilder.cpp
    src/ShapeFinder2.cpp
    src/StateDefinitionBuilder.cpp
    src/Terrain.cpp
    src/Types.cpp
    src/UniqueColorGenerator.cpp
    src/Util.cpp
    src/WorldNormalBuilder.cpp
    src/main.cpp
)

target_include_directories(map_normalizer PRIVATE inc ${SDL2_INCLUDE_DIRS})
target_link_libraries(map_normalizer stdc++fs ${SDL2_LIBRARIES} unique_colors pthread)

