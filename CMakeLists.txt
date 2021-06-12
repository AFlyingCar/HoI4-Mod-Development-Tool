cmake_minimum_required(VERSION 3.16)

project(MapNormalizerTools CXX ASM)

set(MSYS_PREFIX "C:/msys64" CACHE PATH "Prefix for where packages are installed to with MSYS. Only applies to WIN32.")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Set up cmake prefix path to search for .cmake files
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${CMAKE_PREFIX_PATH}
)

# TODO: What other MSVC options should we pass?
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Werror -Wextra -Wno-unused-command-line-argument -Wno-sign-compare -Wno-unused-label -Wno-unused-const-variable -Wno-unused-parameter -Wno-unused-function)
endif()

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Xclang -flto-visibility-public-std -m32)
    add_definitions(-D_REENTRANT)
endif()

add_subdirectory(map_normalizer)

add_subdirectory(tests)

################################################################################

######################
#                    #
#   CPACK SETTINGS   #
#                    #
######################

# https://dominoc925.blogspot.com/2016/09/create-windows-installer-using-cmake.html
set(CPACK_PACKAGE_NAME "MapNormalizerTool")
set(CPACK_PACKAGE_VENDOR "AFlyingCar")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A tool which takes BMP files as input and normalizes them to a format which can be read by Hearts of Iron 4.")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "map_normalizer")
set(CPACK_PACKAGE_CONTACT "https://github.com/AFlyingCar/HoI4-Map-Normalizer-Tool/issues")

# Calculate package version information
file(STRINGS "${CMAKE_SOURCE_DIR}/.config/version.txt" CPACK_PACKAGE_VERSION)
set(CPACK_PACKAGE_VERSION_MAJOR "") # TODO: Grab from file
set(CPACK_PACKAGE_VERSION_MINOR "") # TODO: Grab from file
set(CPACK_PACKAGE_VERSION_PATCH "") # TODO: Grab from file

set(CPACK_COMPONENTS_ALL applications libraries)
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Map Normalizer Tools")
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

if(WIN32)
    set(INSTALL_DESTINATION bin)
else()
    set(INSTALL_DESTINATION "opt/${CPACK_PACKAGE_NAME}")
endif()

install(TARGETS map_normalizer
        DESTINATION ${INSTALL_DESTINATION}
        COMPONENT applications)

include(InstallRequiredSystemLibraries)

# Code taken from here: https://stackoverflow.com/questions/62884439/how-to-use-cmake-file-get-runtime-dependencies-in-an-install-statement
if(WIN32)
    set(DEPENDENCY_PATHS "${MSYS_PREFIX}/mingw64/bin/")
    install(CODE "set(DEPENDENCY_PATHS  \"${DEPENDENCY_PATHS}\")")
    install(CODE "set(INSTALL_DESTINATION \"${INSTALL_DESTINATION}\")")

    install(CODE [[
        # https://github.com/longturn/freeciv21/blob/d53d0181ba837d0d8e10fd76811f0aaeabd6fe5b/CMakeLists.txt
        string(REGEX REPLACE "objdump.exe" "" MINGW_PATH ${CMAKE_OBJDUMP})
        file(GET_RUNTIME_DEPENDENCIES
            EXECUTABLES "$<TARGET_FILE:map_normalizer>"
            RESOLVED_DEPENDENCIES_VAR _r_deps
            UNRESOLVED_DEPENDENCIES_VAR _u_deps
            DIRECTORIES ${DEPENDENCY_PATHS}
            PRE_EXCLUDE_REGEXES "api-ms-*"
            POST_EXCLUDE_REGEXES ".*system32/.*\\.dll" ".*SysWOW64/.*\\.dll"
        )
        foreach(_file ${_r_deps})
            message(STATUS "Copying dependency ${_file}")
            file(INSTALL
                DESTINATION "${CMAKE_INSTALL_PREFIX}/${INSTALL_DESTINATION}"
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

    message(STATUS "Install will create a SYMLINK from ${CMAKE_INSTALL_PREFIX}/${INSTALL_DESTINATION}/$<TARGET_FILE:map_normalizer> to ${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE:map_normalizer>")

    install(CODE "execute_process( \
        COMMAND ${CMAKE_COMMAND} -E create_symlink \
        ${CMAKE_INSTALL_PREFIX}/${INSTALL_DESTINATION}/$<TARGET_FILE:map_normalizer> \
        ${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE:map_normalizer>
        )"
    )
endif()

include(CPack)

