cmake_minimum_required(VERSION 3.0)

# All subprojects
add_subdirectory(logging)
add_subdirectory(unique_colors)
add_subdirectory(common)
add_subdirectory(project)
add_subdirectory(province_utils)
add_subdirectory(gui)
add_subdirectory(actions)

# This should be last
add_subdirectory(exe)

add_executable(map_normalizer $<TARGET_OBJECTS:map_normalizer_lib>)

# Link against 3rd party libraries
target_link_libraries(map_normalizer stdc++fs pthread)

# Link against local libraries
target_link_libraries(map_normalizer unique_colors common gui)

# Link .res file into exe for windows
if(WIN32)
    message(STATUS "WIN32 detected, will attempt to link ${WIN32_RESOURCES} into executable")
    set_target_properties(map_normalizer PROPERTIES
        LINK_FLAGS ${WIN32_RESOURCES}
        LINK_DEPENDS ${WIN32_RESOURCES}
    )
    add_dependencies(map_normalizer win32_resources)
endif()
