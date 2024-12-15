cmake_minimum_required(VERSION 3.0)

include(FetchContent)

set(logger_BuildTests OFF CACHE INTERNAL "Turn off Logger unit tests.")

FetchContent_Declare(logger
    GIT_REPOSITORY https://codeberg.org/AFlyingCar/Logger
)

FetchContent_GetProperties(logger)

if(NOT logger_POPULATED)
    FetchContent_Populate(logger)
    add_subdirectory(${logger_SOURCE_DIR} ${logger_BINARY_DIR} EXCLUDE_FROM_ALL)

    install(FILES ${logger_SOURCE_DIR}/LICENSE
            DESTINATION ${INSTALL_DESTINATION}/share/licenses
            RENAME LICENSE.logger.txt)
endif()

