cmake_minimum_required(VERSION 3.0)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

# Generate GLShaderSources.h
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/GLShaderSources.h"
    COMMAND python3 "${CMAKE_SOURCE_DIR}/tools/genGLShaderSourcesHeader.py" "${CMAKE_CURRENT_SOURCE_DIR}/glsl" "${CMAKE_BINARY_DIR}"
    DEPENDS "${CMAKE_SOURCE_DIR}/tools/genGLShaderSourcesHeader.py" "${CMAKE_CURRENT_SOURCE_DIR}/glsl/"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

# Yeah, I know the CMake documentation _says_ this isn't necessary anymore
#   https://cmake.org/cmake/help/latest/prop_sf/OBJECT_DEPENDS.html
# But for some reason I can't figure out, src/Constants.cpp doesn't get rebuilt
#  when ToolsVersion.h gets regenerated without this, so just set the property 
#  anyway, the documentation is clearly lying to me.
set_source_files_properties(src/MapDrawingAreaGL.cpp
    PROPERTIES
    OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/GLShaderSources.h"
    )


add_library(gui_gl STATIC
    src/Program.cpp
    src/Shader.cpp
    src/Texture.cpp

    src/MapDrawingAreaGL.cpp
    src/GLEWInitializationException.cpp
)

target_include_directories(gui_gl PUBLIC inc)
target_include_directories(gui_gl PRIVATE "${CMAKE_BINARY_DIR}"
                                  PRIVATE glm)

target_link_libraries(gui_gl GLEW::GLEW OpenGL::GL)

# Local libraries
target_link_libraries(gui_gl gui)

