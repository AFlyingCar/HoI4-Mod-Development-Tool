cmake_minimum_required(VERSION 3.0)

include(FetchContent)

FetchContent_Declare(fifo_map
    GIT_REPOSITORY https://github.com/nlohmann/fifo_map
)

FetchContent_GetProperties(fifo_map)

if(NOT fifo_map_POPULATED)
    FetchContent_Populate(fifo_map)
    add_subdirectory(${fifo_map_SOURCE_DIR} ${fifo_map_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

