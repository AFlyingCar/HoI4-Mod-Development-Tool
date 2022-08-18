cmake_minimum_required(VERSION 3.0)

find_package(json REQUIRED)

# Generate ToolsVersion.h
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/ToolsVersion.h"
    COMMAND python3 "${CMAKE_SOURCE_DIR}/tools/genVersionHeader.py" "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}"
    DEPENDS "${CMAKE_SOURCE_DIR}/.config/version.txt"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

# Generate License.h
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/License.h"
    COMMAND python3 "${CMAKE_SOURCE_DIR}/tools/genLicenseHeader.py" "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}"
    DEPENDS "${CMAKE_SOURCE_DIR}/LICENSE" "${CMAKE_SOURCE_DIR}/tools/genLicenseHeader.py"
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
set_source_files_properties(src/Constants.cpp
    PROPERTIES
    OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/License.h"
    )

add_library(common STATIC
    src/BitMap.cpp
    src/Types.cpp
    src/Util.cpp
    src/UniqueColorGenerator.cpp
    src/Version.cpp
    src/Constants.cpp
    src/MapData.cpp
    src/Preferences.cpp
    src/StatusCategory.cpp
    src/StatusCodes.cpp

    "${CMAKE_BINARY_DIR}/ToolsVersion.h"
)

target_include_directories(common PUBLIC inc)
target_include_directories(common PRIVATE "${CMAKE_BINARY_DIR}")

target_link_libraries(common PRIVATE nlohmann_json::nlohmann_json)

target_link_libraries(common PUBLIC unique_colors logging)

# Generate StatusCodes.csv after common is finished
add_custom_command(TARGET common POST_BUILD
    COMMAND python3 "${CMAKE_SOURCE_DIR}/tools/genStatusCodesList.py" "${CMAKE_CURRENT_SOURCE_DIR}/inc/StatusCodesDef.h" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/inc/StatusCodesDef.h" "${CMAKE_SOURCE_DIR}/tools/genStatusCodesList.py"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

