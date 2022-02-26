cmake_minimum_required(VERSION 3.0)

find_package(SDL2)

# Go ahead and only compile this if SDL2 is present on the system
#  We can do this since this is an optional tool for assistance with debugging
if(SDL2_FOUND)
    add_executable(gl_tester
        src/main.cpp
    )

    target_include_directories(gl_tester PUBLIC inc)
    target_link_libraries(gl_tester gui_gl)

    target_link_libraries(gl_tester ${SDL2_LIBRARIES} pthread)
    target_include_directories(gl_tester PUBLIC ${SDL2_INCLUDE_DIRS})

    target_compile_options(gl_tester PRIVATE -Wall -Werror -Wextra -Wno-sign-compare -Wno-unused-label -Wno-unused-const-variable -Wno-unused-parameter -Wno-unused-function -g)

else()
    message(WARNING "Package SDL2 not found. We will skip compiling gl_tester.")
endif()

