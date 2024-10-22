cmake_minimum_required(VERSION 3.16)

include(FetchContent)

set(IONICONS_RESOURCE_PREFIX "/com/ionicons" PARENT_SCOPE)
set(IONICONS_RESOURCE_PREFIX "/com/ionicons")

set(IONICONS_RESOURCE_C_SRC_NAME "ionicons_resources.gresource.c" PARENT_SCOPE)
set(IONICONS_RESOURCE_C_SRC_NAME "ionicons_resources.gresource.c")

set(IONICONS_RESOURCE_XML_SRC "${CMAKE_BINARY_DIR}/IoniconsResourceDefinition.gresource.xml")

# Define twice so that it is available both here and in the parent scope
set(IONICONS_RESOURCE_C_SRC "${CMAKE_BINARY_DIR}/${IONICONS_RESOURCE_C_SRC_NAME}" PARENT_SCOPE)
set(IONICONS_RESOURCE_C_SRC "${CMAKE_BINARY_DIR}/${IONICONS_RESOURCE_C_SRC_NAME}")

FetchContent_Declare(ionicons
    GIT_REPOSITORY https://github.com/ionic-team/ionicons
    GIT_TAG "origin/main"
)

FetchContent_GetProperties(ionicons)

if(NOT ionicons_POPULATED)
    FetchContent_Populate(ionicons)
    # add_subdirectory(${ionicons_SOURCE_DIR} ${ionicons_BINARY_DIR} EXCLUDE_FROM_ALL)
    # set(IONICONS_RESOURCE_DIR "${ionicons_SOURCE_DIR}")
    set(IONICONS_RESOURCE_DIR "${CMAKE_BINARY_DIR}")
    set(IONICONS_IMAGES_DIR "${IONICONS_RESOURCE_DIR}/ionicons")

    install(FILES ${ionicons_SOURCE_DIR}/LICENSE
            DESTINATION ${INSTALL_DESTINATION}/share/licenses
            RENAME LICENSE.ionicons.txt)

    # We only use "/com" as the prefix to genResourceDefinitionXML.py because
    #  the "ionicons" part will be included from the symlink
    add_custom_command(OUTPUT "${IONICONS_RESOURCE_XML_SRC}"
        COMMAND ${CMAKE_COMMAND} -E create_symlink "${ionicons_SOURCE_DIR}/src/svg" "${IONICONS_IMAGES_DIR}"
        COMMAND python3 "${CMAKE_SOURCE_DIR}/tools/genResourceDefinitionXML.py" Ionicons "/com" "${IONICONS_RESOURCE_DIR}" "${IONICONS_IMAGES_DIR}"
        DEPENDS "${CMAKE_SOURCE_DIR}/tools/genResourceDefinitionXML.py"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

    add_custom_command(OUTPUT ${IONICONS_RESOURCE_C_SRC}
        COMMAND "${GLIB_COMPILE_RESOURCES}" "--target=${IONICONS_RESOURCE_C_SRC}" "${IONICONS_RESOURCE_XML_SRC}"
        MAIN_DEPENDENCY ${IONICONS_RESOURCE_XML_SRC}
        WORKING_DIRECTORY "${IONICONS_RESOURCE_DIR}"
        DEPENDS
            # ${IONICONS_RESOURCE_DIR}
            ${IONICONS_RESOURCE_XML_SRC}
            "${CMAKE_SOURCE_DIR}/tools/genResourceDefinitionXML.py"
    )

    add_custom_target(glib_ionicons_resources
        COMMAND ${CMAKE_COMMAND} -E copy ${IONICONS_RESOURCE_C_SRC} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        DEPENDS ${IONICONS_RESOURCE_C_SRC}
    )

    set_source_files_properties(${IONICONS_RESOURCE_C_SRC} PROPERTIES GENERATED TRUE)
endif()

