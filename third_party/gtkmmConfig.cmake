cmake_minimum_required(VERSION 3.0)

# Linux installation: https://developer.gnome.org/gtkmm-tutorial/stable/sec-install-unix-and-linux.html.en
# Windows installation: https://wiki.gnome.org/Projects/gtkmm/MSWindows

# TODO: Where does windows put them?
if(WIN32)
    find_program(MSYS2_INSTALLED bash.exe)

    if(MSYS2_INSTALLED)
        # Install GTKMM via MSYS
        # Running MSYS via CMake: https://stackoverflow.com/a/36347852
        set(BOOTSTRAP_COMMAND_DIR "${CMAKE_SOURCE_DIR}")
        set(BOOTSTRAP_COMMAND bash -l -c "cd ${BOOTSTRAP_COMMAND_DIR} && sh ./win32.bootstrap.sh")
        execute_process(COMMAND ${BOOTSTRAP_COMMAND}
                        WORKING_DIRECTORY ${BOOTSTRAP_COMMAND_DIR}
                        RESULT_VARIABLE boostrap_result)
        if(NOT "${boostrap_result}" STREQUAL "0")
            message(FATAL_ERROR "Bootstrap returned ${command_result}")
        endif()
    else()
        message(WARNING "Bash not found. We require bash installed via MSYS2 in order to run the bootstrapping script.")
        message(WARNING "If you have already installed the required packages, then this will not be an error and we can attempt to continue regardless.")
    endif()

    find_package(gtkmm REQUIRED)

    # GTKMM does not have native CMake support, so we must find the files
    #  manually
    # See: https://github.com/microsoft/vcpkg/blob/master/docs/examples/installing-and-using-packages.md#handling-libraries-without-native-cmake-support

    find_path(GTKMM3_INCLUDE_DIRS
        NAMES gtkmm.h
        # PATHS ...
        # PATH_SUFFIXES ...
    )

    # TODO: Lib name?
    find_library(GTKMM3_LIBRARIES gtkmm)
else()
    #    find_path(GTK_INCLUDE_DIRS
    #        NAMES gtkmm.h
    #        PATHS /usr/include
    #        PATH_SUFFIXES gtkmm-3.0
    #    )

    include(FindPkgConfig)
    pkg_search_module(GTKMM3 REQUIRED gtkmm-3.0)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(gtkmm
    FOUND_VAR
        gtkmm_FOUND
    REQUIRED_VARS
        GTKMM3_INCLUDE_DIRS
        GTKMM3_LIBRARIES
)

if(gtkmm_FOUND)
    if(NOT TARGET gtkmm)
        add_library(gtkmm INTERFACE)
        target_include_directories(gtkmm PUBLIC INTERFACE ${GTKMM3_INCLUDE_DIRS})
        target_link_libraries(gtkmm INTERFACE ${GTKMM3_LIBRARIES})
    endif(NOT TARGET gtkmm)
endif(gtkmm_FOUND)



