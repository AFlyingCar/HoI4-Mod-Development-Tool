cmake_minimum_required(VERSION 3.0)

add_library(province_utils STATIC
    src/ShapeFinder2.cpp
    src/ProvinceMapBuilder.cpp
    src/Terrain.cpp
)

target_include_directories(province_utils PUBLIC inc)

target_link_libraries(province_utils common)

target_compile_options(province_utils PUBLIC -Wno-deprecated-declarations)

