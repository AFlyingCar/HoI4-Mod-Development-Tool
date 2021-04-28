cmake_minimum_required(VERSION 3.0)

# Linux installation: https://developer.gnome.org/gtkmm-tutorial/stable/sec-install-unix-and-linux.html.en
# Windows installation: https://wiki.gnome.org/Projects/gtkmm/MSWindows

# TODO: Where does windows put them?
if(WIN32)
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



