cmake_minimum_required(VERSION 3.16)

find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

set(RESOURCE_PREFIX "/com/aflyingcar/${PROJECT_NAME}")

# Define twice so that it is available both here and in the parent scope
set(RESOURCE_C_SRC_NAME "resources.gresource.c" PARENT_SCOPE)
set(RESOURCE_C_SRC_NAME "resources.gresource.c")

set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(RESOURCE_XML_SRC "${CMAKE_BINARY_DIR}/ResourceDefinition.gresource.xml")
set(RESOURCE_XML_CFG_SRC "${RESOURCE_DIR}/ResourceDefinition.gresource.xml.in")

# Define twice so that it is available both here and in the parent scope
set(RESOURCE_C_SRC "${CMAKE_BINARY_DIR}/${RESOURCE_C_SRC_NAME}" PARENT_SCOPE)
set(RESOURCE_C_SRC "${CMAKE_BINARY_DIR}/${RESOURCE_C_SRC_NAME}")

configure_file(${RESOURCE_XML_CFG_SRC} ${RESOURCE_XML_SRC} @ONLY)

add_custom_command(OUTPUT ${RESOURCE_C_SRC}
    COMMAND ${GLIB_COMPILE_RESOURCES}
    ARGS
        --target=${RESOURCE_C_SRC}
        ${RESOURCE_XML_SRC}
    MAIN_DEPENDENCY ${RESOURCE_XML_SRC}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS
        ${RESOURCE_DIR}/textures/selection.bmp
)

add_custom_target(glib_resources
    COMMAND ${CMAKE_COMMAND} -E copy ${RESOURCE_C_SRC} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    DEPENDS ${RESOURCE_C_SRC}
)

set_source_files_properties(${RESOURCE_C_SRC} PROPERTIES GENERATED TRUE)

