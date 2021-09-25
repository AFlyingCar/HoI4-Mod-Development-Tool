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
        ${RESOURCE_DIR}/textures/logo.png
        ${RESOURCE_XML_CFG_SRC}
)

add_custom_target(glib_resources
    COMMAND ${CMAKE_COMMAND} -E copy ${RESOURCE_C_SRC} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    DEPENDS ${RESOURCE_C_SRC}
)

set_source_files_properties(${RESOURCE_C_SRC} PROPERTIES GENERATED TRUE)

# Compile win32 .rc file for Win32 metadata
if(WIN32)
    find_program(WINDRES NAMES windres REQUIRED)

    set(WIN32_RESOURCES_RC_IN "${RESOURCE_DIR}/win32_resources.rc.in")
    set(WIN32_RESOURCES_RC "${CMAKE_BINARY_DIR}/win32_resources.rc")

    # Define twice so that it is available both here and in the parent scope
    set(WIN32_RESOURCES "${CMAKE_BINARY_DIR}/win32_resources.res" PARENT_SCOPE)
    set(WIN32_RESOURCES "${CMAKE_BINARY_DIR}/win32_resources.res")

    message("WIN32 detected, will attempt to compile ${WIN32_RESOURCES} from ${WIN32_RESOURCES_RC}, configured from ${WIN32_RESOURCES_RC_IN}")

    string(REPLACE "." "," WIN32_RC_VERSION_NUMBER ${MN_TOOL_VERSION})

    configure_file(${WIN32_RESOURCES_RC_IN} ${WIN32_RESOURCES_RC} @ONLY)

    add_custom_command(OUTPUT ${WIN32_RESOURCES}
        COMMAND ${WINDRES}
        ARGS
            ${WIN32_RESOURCES_RC}
            -O coff
            -o ${WIN32_RESOURCES}
        MAIN_DEPENDENCY ${WIN32_RESOURCES_RC_IN}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS
            ${WIN32_RESOURCES_RC}
    )
    add_custom_target(win32_resources DEPENDS ${WIN32_RESOURCES})

    set_source_files_properties(${WIN32_RESOURCES_RC} PROPERTIES GENERATED TRUE)
    set_source_files_properties(${WIN32_RESOURCES} PROPERTIES GENERATED TRUE)
endif()
