cmake_minimum_required(VERSION 3.0)

find_package(SDL2 REQUIRED)

# All subprojects
add_subdirectory(unique_colors)
add_subdirectory(common)
add_subdirectory(project)
add_subdirectory(province_utils)
add_subdirectory(gui)

# This should be last
add_subdirectory(exe)

add_executable(map_normalizer $<TARGET_OBJECTS:map_normalizer_lib>)

# Link against 3rd party libraries
target_link_libraries(map_normalizer stdc++fs pthread)

# Link against local libraries
target_link_libraries(map_normalizer unique_colors common gui)

