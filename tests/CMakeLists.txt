cmake_minimum_required(VERSION 3.2)

include(GoogleTest OPTIONAL RESULT_VARIABLE GTest_FOUND)

find_library(gtest_location gtest)
find_library(gtest_main_location gtest_main)

set(TEST_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TEST_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")

if(GTest_FOUND AND gtest_location AND gtest_main_location)
    set(TEST_SOURCES
        ${TEST_SRC_DIR}/ShapeFinderTests.cpp
        ${TEST_SRC_DIR}/UniqueColorTests.cpp
        ${TEST_SRC_DIR}/UtilTests.cpp

        ${TEST_SRC_DIR}/TestOverrides.cpp
        ${TEST_SRC_DIR}/TestUtils.cpp
    )

    add_executable(unit_tests ${TEST_SOURCES})
    target_link_libraries(unit_tests PRIVATE gtest gtest_main common province_utils unique_colors)
    target_link_libraries(unit_tests PUBLIC stdc++fs pthread)
    target_include_directories(unit_tests PRIVATE inc)

    target_compile_options(unit_tests PRIVATE -g)

    add_custom_command(TARGET unit_tests
                       POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${TEST_BIN_DIR} $<TARGET_FILE_DIR:unit_tests>/bin/
    )

    gtest_add_tests(TARGET unit_tests)
else()
    message(WARNING "Could not find one or more libraries required for unit tests. As such, those tests will not be compiled.")
    if(NOT GTest_FOUND)
        message(WARNING "Could not find GoogleTest.")
    endif()

    if(NOT gtest_location)
        message(WARNING "Could not find gtest.")
    endif()

    if(NOT gtest_main_location)
        message(WARNING "Could not find gtest_main.")
    endif()

    # TODO: These instructions are specific to Linux, we should make sure to add
    #       ones that work for Windows as well
    message(STATUS "To compile the unit tests, install the following packages: googletest, libgtest-dev")
    message(STATUS "And then make sure to build and install gtest-dev (the package only provides the source code, not the binaries). Source code is usually placed in /usr/src/gtest")
    message(STATUS "For help with building gtest-dev, see: https://www.eriksmistad.no/getting-started-with-google-test-on-ubuntu/")
endif()

