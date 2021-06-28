cmake_minimum_required(VERSION 3.0)

# Linux installation: https://developer.gnome.org/gtkmm-tutorial/stable/sec-install-unix-and-linux.html.en
# Windows installation: https://wiki.gnome.org/Projects/gtkmm/MSWindows

if(WIN32)
    # Attempt to use pkgconfig to find gtkmm-3
    include(FindPkgConfig)
    pkg_search_module(GTKMM3 REQUIRED gtkmm-3.0)

    # GTKMM does not have native CMake support, so we must find the files
    #  manually
    # See: https://github.com/microsoft/vcpkg/blob/master/docs/examples/installing-and-using-packages.md#handling-libraries-without-native-cmake-support

    # However, if the above doesn't work, attempt to find the paths manually
    if(NOT GTKMM3_INCLUDE_DIRS)
        message(WARNING "Unable to find GTKMM3 include dirs with pkgconfig, going to try to find them manually.")

        # If the user has defined where to find GTK, then go with that, otherwise
        #  assume it was installed with msys2
        if(DEFINED ENV{GTK_INC_ROOT})
            set(GTK_INC_ROOT "$ENV{GTK_INC_ROOT}")
        else()
            if(DEFINED ENV{MSYS2_ROOT})
                set(GTK_INC_ROOT "$ENV{MSYS2_ROOT}\\mingw64\\include")
            else()
                set(GTK_INC_ROOT "C:\\msys64\\mingw64\\include")
            endif()
        endif()

        find_path(GTKMM3_INCLUDE_DIRS
            NAMES gtkmm.h
            PATHS "${GTK_INC_ROOT}"
            PATH_SUFFIXES "gtkmm-3.0"
        )
    endif()

    if(NOT GTKMM3_LIBRARIES)
        message(WARNING "Unable to find GTKMM3 libraries with pkgconfig, going to try to find them manually.")

        # If the user has defined where to find GTK, then go with that, otherwise
        #  assume it was installed with msys2
        if(DEFINED ENV{GTK_LIB_ROOT})
            set(GTK_LIB_ROOT "$ENV{GTK_LIB_ROOT}")
        else()
            if(DEFINED ENV{MSYS2_ROOT})
                set(GTK_LIB_ROOT "$ENV{MSYS2_ROOT}\\mingw64\\lib")
            else()
                set(GTK_LIB_ROOT "C:\\msys64\\mingw64\\lib")
            endif()
        endif()

        find_library(GTKMM3_LIBRARIES
            NAMES libgtkmm-3.0-1 gtkmm-3.0-1 gtkmm-3.0 libgtkmm-3.0
            PATHS "${GTK_LIB_ROOT}"
            )
    endif()
else()
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
        target_include_directories(gtkmm SYSTEM PUBLIC INTERFACE ${GTKMM3_INCLUDE_DIRS})
        target_link_libraries(gtkmm INTERFACE ${GTKMM3_LIBRARIES})

        # Make sure we get the license as well
        file(DOWNLOAD https://www.gnu.org/licenses/lgpl-3.0.txt
                      ${CMAKE_BINARY_DIR}/LICENSE-gtkmm-3.0.txt)
        install(FILES ${CMAKE_BINARY_DIR}/LICENSE-gtkmm-3.0.txt
                DESTINATION ${INSTALL_DESTINATION}/share/licenses)
    endif(NOT TARGET gtkmm)
endif(gtkmm_FOUND)

