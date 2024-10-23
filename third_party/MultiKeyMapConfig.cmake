cmake_minimum_required(VERSION 3.0)

include(FetchContent)

set(mkm_BuildTests OFF CACHE INTERNAL "Turn off MKM unit tests.")

FetchContent_Declare(mkm
    GIT_REPOSITORY https://github.com/AFlyingCar/MultiKeyMap
)

FetchContent_GetProperties(mkm)

if(NOT mkm_POPULATED)
    FetchContent_Populate(mkm)
    add_subdirectory(${mkm_SOURCE_DIR} ${mkm_BINARY_DIR} EXCLUDE_FROM_ALL)

    install(FILES ${mkm_SOURCE_DIR}/LICENSE
            DESTINATION ${INSTALL_DESTINATION}/share/licenses
            RENAME LICENSE.mkm.txt)
endif()

