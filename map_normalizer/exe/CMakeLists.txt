cmake_minimum_required(VERSION 3.0)

# This is an "object" library, its purpose is basically to compile the exe
#  as a sub-project, while still not deviating _too much_ from the rest of how
#  all the sub-projects are laid out
add_library(map_normalizer_lib OBJECT
    src/ArgParser.cpp
    src/ProvinceMapBuilder.cpp
    src/ShapeFinder2.cpp
    src/StateDefinitionBuilder.cpp
    src/Terrain.cpp
    src/UniqueColorGenerator.cpp
    src/WorldNormalBuilder.cpp
    src/main.cpp
)

target_include_directories(map_normalizer_lib PRIVATE inc)

# Link against local libraries
target_link_libraries(map_normalizer_lib unique_colors common gui)

