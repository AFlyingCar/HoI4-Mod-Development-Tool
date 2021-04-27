cmake_minimum_required(VERSION 3.0)

find_package(SDL2 REQUIRED)
find_package(gtkmm REQUIRED)

add_library(gui STATIC
    src/MapNormalizerApplication.cpp
    src/GraphicalDebugger.cpp
    src/Driver.cpp
    src/MainWindow.cpp
    src/Window.cpp
    src/MapDrawingArea.cpp
    src/ProgressBarDialog.cpp
)

target_include_directories(gui PUBLIC inc)
target_include_directories(gui PRIVATE ${SDL2_INCLUDE_DIRS})

target_link_libraries(gui ${SDL2_LIBRARIES})

target_link_libraries(gui gtkmm)

# Link against local libraries
target_link_libraries(gui common province_utils)

