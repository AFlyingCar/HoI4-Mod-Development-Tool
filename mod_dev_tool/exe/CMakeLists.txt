cmake_minimum_required(VERSION 3.0)

# This is an "object" library, its purpose is basically to compile the exe
#  as a sub-project, while still not deviating _too much_ from the rest of how
#  all the sub-projects are laid out
add_library(mod_dev_tool_lib OBJECT
    src/ArgParser.cpp
    src/StateDefinitionBuilder.cpp
    src/Interfaces.cpp
    src/main.cpp
)

target_include_directories(mod_dev_tool_lib PUBLIC inc)

# Link against local libraries
target_link_libraries(mod_dev_tool_lib common gui province_utils logging)

