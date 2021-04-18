cmake_minimum_required(VERSION 3.0)

find_path(third_party_ROOT
    NAMES MyGUIConfig.cmake
)

# Specify the Render System. Possible values:
#   1 - Dummy
#   2 - Export (not used)
#   3 - Ogre
#   4 - OpenGL
#   5 - Direct3D 9
#   6 - Direct3D 11
#   7 - OpenGL 3.x
#   8 - OpenGL ES 2.0 (Emscripten)
set(MYGUI_RENDERSYSTEM 4 CACHE STRING "Use OpenGL RenderSystem for the GUI." FORCE)

# Tell MyGUI that we want it to use folders in VS
set(MYGUI_USE_PROJECT_FOLDERS TRUE)

# Disable some projects that are not needed for us
set(MYGUI_BUILD_DEMOS FALSE)
set(MYGUI_BUILD_DOCS FALSE)
set(MYGUI_BUILD_UNITTESTS FALSE)
set(MYGUI_BUILD_PLUGINS FALSE)

include(FetchContent)
FetchContent_Populate(MyGUI
    GIT_REPOSITORY https://github.com/MyGUI/mygui/
    GIT_TAG "MyGUI3.4.1"
    SOURCE_DIR "${CMAKE_BINARY_DIR}/MyGUI-src"
)

add_subdirectory(${CMAKE_BINARY_DIR}/MyGUI-src ${CMAKE_BINARY_DIR}/MyGUI-build)

# Set up an INTERFACE target for MyGUI to simplify dependencies on it
if(NOT TARGET MyGUI)
    add_library(MyGUI INTERFACE)
    target_include_directories(MyGUI INTERFACE $<TARGET_PROPERTY:MyGUIEngine,INCLUDE_DIRECTORIES> $<TARGET_PROPERTY:MyGUI.OpenGLPlatform,INCLUDE_DIRECTORIES>)
    target_link_libraries(MyGUI INTERFACE MyGUIEngine MyGUI.OpenGLPlatform)
endif(NOT TARGET MyGUI)



