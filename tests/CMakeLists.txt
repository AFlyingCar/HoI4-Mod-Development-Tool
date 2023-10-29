cmake_minimum_required(VERSION 3.2)

find_package(gtest REQUIRED)

set(TEST_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TEST_BIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(TEST_SOURCES
    ${TEST_SRC_DIR}/ShapeFinderTests.cpp
    ${TEST_SRC_DIR}/UniqueColorTests.cpp
    ${TEST_SRC_DIR}/UtilTests.cpp
    ${TEST_SRC_DIR}/VersionTests.cpp
    ${TEST_SRC_DIR}/ProjectTests.cpp
    ${TEST_SRC_DIR}/LogTests.cpp
    ${TEST_SRC_DIR}/GuiTests.cpp
    ${TEST_SRC_DIR}/ActionTests.cpp
    ${TEST_SRC_DIR}/PreferencesTests.cpp
    ${TEST_SRC_DIR}/BitMapTests.cpp

    ${TEST_SRC_DIR}/TestOverrides.cpp
    ${TEST_SRC_DIR}/TestUtils.cpp
)

add_executable(unit_tests ${TEST_SOURCES})
target_link_libraries(unit_tests PRIVATE gtest gtest_main gmock common province_utils unique_colors project logging gui actions)
target_link_libraries(unit_tests PUBLIC stdc++fs pthread)
target_include_directories(unit_tests PRIVATE inc)

target_compile_options(unit_tests PRIVATE -g)

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(unit_tests PRIVATE -mno-ms-bitfields -Wno-class-memaccess)
endif()

# Add all clang-specific compiler options here
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(unit_tests PRIVATE -fno-limit-debug-info)
endif()

add_custom_command(TARGET unit_tests
                   POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${TEST_BIN_DIR} $<TARGET_FILE_DIR:unit_tests>/bin/
)

add_test(NAME unit_tests COMMAND unit_tests)

