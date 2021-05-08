cmake_minimum_required(VERSION 3.0)

# Generate ToolsVersion.h
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/ToolsVersion.h"
    COMMAND python3 "${CMAKE_SOURCE_DIR}/tools/genVersionHeader.py" "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}"
    DEPENDS "${CMAKE_SOURCE_DIR}/.config/version.txt"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

# Yeah, I know the CMake documentation _says_ this isn't necessary anymore
#   https://cmake.org/cmake/help/latest/prop_sf/OBJECT_DEPENDS.html
# But for some reason I can't figure out, src/Constants.cpp doesn't get rebuilt
#  when ToolsVersion.h gets regenerated without this, so just set the property 
#  anyway, the documentation is clearly lying to me.
set_source_files_properties(src/Constants.cpp
    PROPERTIES
    OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/ToolsVersion.h"
    )

add_library(common STATIC
    src/BitMap.cpp
    src/Types.cpp
    src/Util.cpp
    src/Logger.cpp
    src/UniqueColorGenerator.cpp
    src/Version.cpp
    src/Constants.cpp

    "${CMAKE_BINARY_DIR}/ToolsVersion.h"
)

target_include_directories(common PUBLIC inc)
target_include_directories(common PRIVATE "${CMAKE_BINARY_DIR}")

target_link_libraries(common PUBLIC unique_colors)

