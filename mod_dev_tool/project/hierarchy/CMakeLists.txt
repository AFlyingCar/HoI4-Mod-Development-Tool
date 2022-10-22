cmake_minimum_required(VERSION 3.0)

add_library(project_hierarchy STATIC
    src/INode.cpp
    src/GroupNode.cpp
    src/LinkNode.cpp
    src/StateNode.cpp
    src/ProvinceNode.cpp
)

target_include_directories(project_hierarchy PUBLIC inc)

# Local libraries
target_link_libraries(project_hierarchy common logging)

