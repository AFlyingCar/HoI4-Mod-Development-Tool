cmake_minimum_required(VERSION 3.0)

include(FetchContent)

FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
    GIT_TAG v3.10.5
)

FetchContent_GetProperties(json)

if(NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)

    install(FILES ${json_SOURCE_DIR}/LICENSE.MIT
            DESTINATION ${INSTALL_DESTINATION}/share/licenses
            RENAME LICENSE.json.txt)
endif()

