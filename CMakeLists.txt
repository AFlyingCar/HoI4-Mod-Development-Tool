cmake_minimum_required(VERSION 3.16)

project(HoI4ModDevelopmentTool CXX ASM)

set(MSYS_PREFIX "C:/msys64" CACHE PATH "Prefix for where packages are installed to with MSYS. Only applies to WIN32.")
set(DEBUG_BUILD CACHE BOOL OFF "Specifies if builds should be built with debugging information.")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Set up cmake prefix path to search for .cmake files
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${CMAKE_PREFIX_PATH}
)

set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}/mod_dev_tool")

# Set up install destinations
if(WIN32)
    set(INSTALL_DESTINATION .)
else()
    set(INSTALL_DESTINATION "opt/${CPACK_PACKAGE_NAME}")
endif()

# Common variables containing information about the program
set(HMDT_TOOL_NAME "HoI4ModDevelopmentTool")
set(HMDT_TOOL_AUTHOR "AFlyingCar")
set(HMDT_TOOL_DESCRIPTION "A tool which takes BMP files as input and normalizes them to a format which can be read by Hearts of Iron 4.")
set(HMDT_TOOL_WEBSITE "https://github.com/AFlyingCar/HoI4-Mod-Development-Tool")
set(HMDT_TOOL_CONTACT "https://github.com/AFlyingCar/HoI4-Mod-Development-Tool/issues")

file(STRINGS "${CMAKE_SOURCE_DIR}/.config/version.txt" HMDT_TOOL_VERSION)

message(STATUS "Current Tool Info:")
list(APPEND CMAKE_MESSAGE_INDENT "  ")
message(STATUS "HMDT_TOOL_NAME=${HMDT_TOOL_NAME}")
message(STATUS "HMDT_TOOL_AUTHOR=${HMDT_TOOL_AUTHOR}")
message(STATUS "HMDT_TOOL_DESCRIPTION=${HMDT_TOOL_DESCRIPTION}")
message(STATUS "HMDT_TOOL_WEBSITE=${HMDT_TOOL_WEBSITE}")
message(STATUS "HMDT_TOOL_CONTACT=${HMDT_TOOL_CONTACT}")
list(POP_BACK CMAKE_MESSAGE_INDENT)

# TODO: What other MSVC options should we pass?
if(MSVC)
    add_compile_options(/W4 /WX /WX-:C4996)
    add_definitions(/DHMDT_PROJECT_ROOT="${PROJECT_ROOT}")
    add_definitions(/DHMDT_TOOL_NAME="${HMDT_TOOL_NAME}")
else()
    add_compile_options(-Wall -Werror -Wextra -Wno-sign-compare -Wno-unused-label -Wno-unused-const-variable -Wno-unused-parameter -Wno-unused-function -Wno-error=deprecated -Wno-error=deprecated-declarations)
    add_definitions(-DHMDT_PROJECT_ROOT="${PROJECT_ROOT}")
    add_definitions(-DHMDT_TOOL_NAME="${HMDT_TOOL_NAME}")

    if(DEBUG_BUILD)
        message(STATUS "Including debug symbols in build!")
        add_compile_options(-g)
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wno-unused-command-line-argument )

    if(WIN32)
        add_compile_options(-Xclang -flto-visibility-public-std -m32)
        add_definitions(-D_REENTRANT)
    endif()
endif()

# Set up the resources first, as they will get depended on by subsequent projects
add_subdirectory(resources)
add_definitions(-DHMDT_GLIB_RESOURCES="${RESOURCE_C_SRC_NAME}")

# Note: We are adding the 'gettext_' alias so that we can localize without
#   getting picked up by xgettext
add_definitions(-Dgettext_=gettext)

add_subdirectory(mod_dev_tool)

add_subdirectory(tests)

add_dependencies(mod_dev_tool glib_resources locale_resources)

################################################################################

######################
#                    #
#   CPACK SETTINGS   #
#                    #
######################

# https://dominoc925.blogspot.com/2016/09/create-windows-installer-using-cmake.html
set(CPACK_PACKAGE_NAME ${HMDT_TOOL_NAME})
set(CPACK_PACKAGE_VENDOR ${HMDT_TOOL_AUTHOR})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${HMDT_TOOL_DESCRIPTION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "mod_dev_tool")
set(CPACK_PACKAGE_CONTACT ${HMDT_TOOL_CONTACT})

# Calculate package version information
set(CPACK_PACKAGE_VERSION ${HMDT_TOOL_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR "") # TODO: Grab from file
set(CPACK_PACKAGE_VERSION_MINOR "") # TODO: Grab from file
set(CPACK_PACKAGE_VERSION_PATCH "") # TODO: Grab from file

set(CPACK_COMPONENTS_ALL applications libraries)
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "HoI4 Mod Development Tools")
set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Libraries")

set(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
# set(CPACK_COMPONENT_LIBRARIES_DESCRIPTION "")

set(CPACK_COMPONENT_APPLICATIONS_GROUP "Runtime")
set(CPACK_COMPONENT_LIBRARIES_GROUP "Runtime")

# Make sure we output to the right place
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/packages)

set(CPACK_PACKAGING_INSTALL_PREFIX "/")

if(WIN32)
    set(CPACK_GENERATOR ZIP;NSIS)
else()
    set(CPACK_GENERATOR ZIP;DEB)
endif()

# Generator specific settings
if(WIN32)
    set(CPACK_NSIS_MODIFY_PATH OFF)
else()
    set(CPACK_DEBIAN_PACKAGE_PREDEPENDS "")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtkmm-3.0-1v5")
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
endif()

#######################
#                     #
#    INSTALL STEPS    #
#                     #
#######################

install(TARGETS mod_dev_tool
        DESTINATION ${INSTALL_DESTINATION}/bin
        COMPONENT applications)

include(InstallRequiredSystemLibraries)

# Make sure we include the resource files into the installation as well
# install(DIRECTORY resources/etc
#         DESTINATION ${INSTALL_DESTINATION})
# install(DIRECTORY resources/lib
#         DESTINATION ${INSTALL_DESTINATION})
# install(DIRECTORY resources/share
#         DESTINATION ${INSTALL_DESTINATION})
install(FILES ${RESOURCE_C_SRC}
        DESTINATION ${INSTALL_DESTINATION}/bin)
install(FILES README.md
        DESTINATION ${INSTALL_DESTINATION}/share/doc/)
install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/StatusCodesList.csv
        DESTINATION ${INSTALL_DESTINATION}/share/doc)
install(DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/locale/
        DESTINATION ${INSTALL_DESTINATION}/bin/locale)
# TODO: Install the license as well to /share/licenses/, once we have one

# Code taken from here: https://stackoverflow.com/questions/62884439/how-to-use-cmake-file-get-runtime-dependencies-in-an-install-statement
if(WIN32)
    set(DEPENDENCY_PATHS "${MSYS_PREFIX}/mingw64/bin/")
    install(CODE "set(MSYS_PREFIX \"${MSYS_PREFIX}\")")
    install(CODE "set(DEPENDENCY_PATHS  \"${DEPENDENCY_PATHS}\")")
    install(CODE "set(INSTALL_DESTINATION \"${INSTALL_DESTINATION}\")")

    message(STATUS "Grabbing GTK resources")

    # Make sure we get the icons (part of the resources, but installed via GTK, so
    #  we have to grab them seperately)
    install(DIRECTORY ${MSYS_PREFIX}/mingw64/etc/gtk-3.0
            DESTINATION ${INSTALL_DESTINATION}/etc/)
    install(DIRECTORY ${MSYS_PREFIX}/mingw64/share/icons
            DESTINATION ${INSTALL_DESTINATION}/share/)
    install(DIRECTORY ${MSYS_PREFIX}/mingw64/share/locale
            DESTINATION ${INSTALL_DESTINATION}/share/)
    install(DIRECTORY ${MSYS_PREFIX}/mingw64/share/themes/Default
            DESTINATION ${INSTALL_DESTINATION}/share/themes)

    message(STATUS "Grabbing pixbuf modules")

    # Make sure we grab the pixbuf modules as well
    install(FILES ${MSYS_PREFIX}/mingw64/lib/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-png.dll
            DESTINATION ${INSTALL_DESTINATION}/lib/gdk-pixbuf-2.0/2.10.0/loaders/)
    
    # NOTE: This is for packaging up loaders.cache, generated from gdk-pixbuf-query-loaders
    #   If additional dependencies have been added, make sure to re-run:
    #     gdk-pixbuf-query-loaders.exe [OPTIONS] > resources/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache
    install(DIRECTORY resources/lib/
            DESTINATION ${INSTALL_DESTINATION}/lib/)

# TODO: Add this to the installer executable only
#     find_program(GDK_PIXBUF_QUERY_LOADERS gdk-pixbuf-query-loaders.exe
#                  PATHS "${MSYS_PREFIX}/mingw64/bin/")
# 
#     if(NOT GDK_PIXBUF_QUERY_LOADERS)
#         message(WARNING "Unable to find `${MSYS_PREFIX}/mingw64/bin/gdk-pixbuf-query-loaders.exe`, which means we may not be able to correctly generate an installer!")
#     else()
#         message(STATUS "Found ${GDK_PIXBUF_QUERY_LOADERS}")
#     endif()
# 
#     install(CODE "set(GDK_PIXBUF_QUERY_LOADERS \"${GDK_PIXBUF_QUERY_LOADERS}\")")
# 
#     install(CODE [[
#         execute_process(COMMAND "${GDK_PIXBUF_QUERY_LOADERS}" "loaders/libpixbufloader-png.dll"
#                         COMMAND_ECHO STDOUT
#                         WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}/${INSTALL_DESTINATION}/lib/gdk-pixbuf-2.0/2.10.0"
#                         RESULT_VARIABLE GDK_PIXBUF_QUERY_LOADERS_RESULT
#                         OUTPUT_FILE loaders.cache)
#     ]])
#     install(CODE [[
#         if(GDK_PIXBUF_QUERY_LOADERS_RESULT)
#             message(FATAL_ERROR "${GDK_PIXBUF_QUERY_LOADERS} exited with status: `${GDK_PIXBUF_QUERY_LOADERS_RESULT}")
#         endif()
#     ]])

    install(CODE [[
        # https://github.com/longturn/freeciv21/blob/d53d0181ba837d0d8e10fd76811f0aaeabd6fe5b/CMakeLists.txt
        string(REGEX REPLACE "objdump.exe" "" MINGW_PATH ${CMAKE_OBJDUMP})
        file(GET_RUNTIME_DEPENDENCIES
            EXECUTABLES "$<TARGET_FILE:mod_dev_tool>"
            RESOLVED_DEPENDENCIES_VAR _r_deps
            UNRESOLVED_DEPENDENCIES_VAR _u_deps
            DIRECTORIES ${DEPENDENCY_PATHS}
            PRE_EXCLUDE_REGEXES "api-ms-*"
            POST_EXCLUDE_REGEXES ".*system32/.*\\.dll" ".*SysWOW64/.*\\.dll"
        )
        foreach(_file ${_r_deps})
            message(STATUS "Copying dependency ${_file}")
            file(INSTALL
                DESTINATION "${CMAKE_INSTALL_PREFIX}/${INSTALL_DESTINATION}/bin/"
                TYPE SHARED_LIBRARY
                FOLLOW_SYMLINK_CHAIN
                FILES "${_file}"
            )
        endforeach()
        list(LENGTH _u_deps _u_length)
        if("${_u_length}" GREATER 0)
            message(WARNING "Unresolved dependencies detected! ${_u_deps}")
        endif()
    ]])
endif()

include(CPack)

