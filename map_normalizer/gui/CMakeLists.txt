cmake_minimum_required(VERSION 3.0)

find_package(gtkmm REQUIRED)
find_package(NativeDialogs REQUIRED)

add_library(gui STATIC
    src/MapNormalizerApplication.cpp
    src/GraphicalDebugger.cpp
    src/Driver.cpp
    src/MainWindow.cpp
    src/Window.cpp
    src/MapDrawingArea.cpp
    src/IMapDrawingArea.cpp
    src/ProgressBarDialog.cpp
    src/NewProjectDialog.cpp
    src/ProvincePropertiesPane.cpp
    src/ProvincePreviewDrawingArea.cpp
    src/IProvincePreviewDrawingArea.cpp
    src/InterruptableScrolledWindow.cpp
    src/LogViewerWindow.cpp
)

target_include_directories(gui PUBLIC inc)

target_link_libraries(gui gtkmm native_dialogs)

add_subdirectory(gl)

# Link against local libraries
target_link_libraries(gui common province_utils project logging gui_gl)

