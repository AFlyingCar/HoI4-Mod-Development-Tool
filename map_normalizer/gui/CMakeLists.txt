cmake_minimum_required(VERSION 3.0)

find_package(SDL2 REQUIRED)
find_package(MyGUI REQUIRED)

add_library(gui STATIC
    src/GraphicalDebugger.cpp
    src/Driver.cpp
    src/ImageLoader.cpp
)

target_include_directories(gui PUBLIC inc)
target_include_directories(gui PRIVATE ${SDL2_INCLUDE_DIRS})

target_link_libraries(gui ${SDL2_LIBRARIES} MyGUI)

# Link against local libraries
target_link_libraries(gui common)

