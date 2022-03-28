cmake_minimum_required(VERSION 3.0)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

if(WIN32 AND NOT GLEW_INCLUDE_DIRS)
    message(STATUS "GLEW_INCLUDE_DIRS not found, attempting to first access it out of the GLEW::GLEW target")
    
    get_target_property(GLEW_INCLUDE_DIRS GLEW::GLEW INTERFACE_INCLUDE_DIRECTORIES)
    
    if(NOT GLEW_INCLUDE_DIRS)
        message(STATUS "Failed to find GLEW_INCLUDE_DIRS from GLEW::GLEW target, attempting to construct one based on the provided MSYS_PREFIX environment variable, or failing that our best guess")
        if(DEFINED ENV{MSYS_PREFIX})
            message(STATUS "Using MSYS_PREFIX environment varaible '$ENV{MSYS_PREFIX}'")
            set(GLEW_INCLUDE_DIRS "$ENV{MSYS_PREFIX}\\mingw64\\include")
        elseif(MSYS_PREFIX)
            message(STATUS "Using MSYS_PREFIX varaible '${MSYS_PREFIX}'")
            set(GLEW_INCLUDE_DIRS "${MSYS_PREFIX}\\mingw64\\include")
        else()
            message(STATUS "No MSYS_PREFIX environment variable or cmake variable provided, assuming MSYS_PREFIX='C:\\msys64'")
            set(GLEW_INCLUDE_DIRS "C:\\msys64\\mingw64\\include")
        endif()
            
        message(STATUS "Assuming GLEW_INCLUDE_DIRS=${GLEW_INCLUDE_DIRS}")
	endif()
endif()

# Generate GLShaderSources.h
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/GLShaderSources.h"
    COMMAND ${CMAKE_COMMAND} -E echo python3 "${CMAKE_SOURCE_DIR}/tools/genGLShaderSourcesHeader.py" "${CMAKE_CURRENT_SOURCE_DIR}/glsl" "${CMAKE_BINARY_DIR}"
    COMMAND python3 "${CMAKE_SOURCE_DIR}/tools/genGLShaderSourcesHeader.py" "${CMAKE_CURRENT_SOURCE_DIR}/glsl" "${CMAKE_BINARY_DIR}"
    DEPENDS "${CMAKE_SOURCE_DIR}/tools/genGLShaderSourcesHeader.py" "${CMAKE_CURRENT_SOURCE_DIR}/glsl/"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

# Generate GLXMacro.h
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/GLXMacros.h"
    COMMAND ${CMAKE_COMMAND} -E echo python3 "${CMAKE_SOURCE_DIR}/tools/genGLXMacrosHeader.py" "${GLEW_INCLUDE_DIRS}" "${CMAKE_BINARY_DIR}"
    COMMAND python3 "${CMAKE_SOURCE_DIR}/tools/genGLXMacrosHeader.py" "${GLEW_INCLUDE_DIRS}" "${CMAKE_BINARY_DIR}"
    DEPENDS "${CMAKE_SOURCE_DIR}/tools/genGLXMacrosHeader.py"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

# Yeah, I know the CMake documentation _says_ this isn't necessary anymore
#   https://cmake.org/cmake/help/latest/prop_sf/OBJECT_DEPENDS.html
# But for some reason I can't figure out, src/MapDrawingAreaGL.cpp doesn't get rebuilt
#  when GLShaderSources.h gets regenerated without this, so just set the property
#  anyway, the documentation is clearly lying to me.
set_source_files_properties(src/MapDrawingAreaGL.cpp
    PROPERTIES
    OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/GLShaderSources.h"
    )
set_source_files_properties(src/GLUtils.cpp
    PROPERTIES
    OBJECT_DEPENDS "${CMAKE_BINARY_DIR}/GLXMacros.h"
    )

add_library(gui_gl STATIC
    src/Program.cpp
    src/Shader.cpp
    src/Texture.cpp
    src/GLUtils.cpp

    src/MapRenderingViewBase.cpp
    src/ProvinceRenderingView.cpp
    src/StateRenderingView.cpp

    src/MapDrawingAreaGL.cpp
    src/GLEWInitializationException.cpp
)

target_include_directories(gui_gl PUBLIC inc)
target_include_directories(gui_gl PRIVATE "${CMAKE_BINARY_DIR}"
                                  PRIVATE glm)

target_link_libraries(gui_gl GLEW::GLEW OpenGL::GL)

# Local libraries
target_link_libraries(gui_gl gui)

