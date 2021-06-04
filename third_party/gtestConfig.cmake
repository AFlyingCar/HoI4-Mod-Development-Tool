cmake_minimum_required(VERSION 3.0)

find_path(ThirdParty_ROOT
    NAMES gtestConfig.cmake
)

if(NOT TARGET gtest)
    configure_file(${ThirdParty_ROOT}/gtest.in ${CMAKE_BINARY_DIR}/gtest-download/CMakeLists.txt)

    execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/gtest-download"
        RESULT_VARIABLE exec_res_1)

    execute_process(COMMAND "${CMAKE_COMMAND}" --build .
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/gtest-download"
        RESULT_VARIABLE exec_res_2)

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build. This defines
    #  the gtest and gtest_main targets.
    add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src" "${CMAKE_BINARY_DIR}/googletest-build")

    # organize the gtest projects in the folder view
    set_target_properties(gtest PROPERTIES FOLDER "third_party/gtest")
    set_target_properties(gtest_main PROPERTIES FOLDER "third_party/gtest_main")
endif()

