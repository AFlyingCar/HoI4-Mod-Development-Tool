cmake_minimum_required(VERSION 3.16)

find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext REQUIRED)
find_program(GETTEXT_MSGMERGE_EXECUTABLE msgmerge REQUIRED)
find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt REQUIRED)

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

set(LOCALIZATION_KEYWORD gettext)
set(LOCALIZATION_KEYWORD_DELAY HMDT_LOCALIZE)
set(LOCALIZATION_RESOURCE_DIR "${CMAKE_BINARY_DIR}/locale/")

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

    string(REPLACE "." "," WIN32_RC_VERSION_NUMBER ${HMDT_TOOL_VERSION})

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

# Following code adapted from https://p-o.co.uk/tech-articles/using-cmake-to-update-gettext-pot-po-and-mo-files/
if(GETTEXT_XGETTEXT_EXECUTABLE)
    # We want to extract translation prefixes from every source file
    file(GLOB_RECURSE CPP_FILES RELATIVE ${CMAKE_SOURCE_DIR} ${PROJECT_ROOT}/*.cpp ${PROJECT_ROOT}/*.inl ${PROJECT_ROOT}/*.h)

    message(DEBUG "CPP_FILES=${CPP_FILES}")

    add_custom_target(
        pot-update
        COMMENT "pot-update: Done."
        # DEPENDS ${RESOURCE_DIR}/locale/${HMDT_TOOL_NAME}.pot
    )
    # Skip writing the location comment so that the .pot file doesn't get
    #   updated every time a source file changes (even for changes that don't
    #   actually affect the user-facing strings).
    add_custom_command(
        TARGET pot-update
        PRE_BUILD
        COMMAND
            ${GETTEXT_XGETTEXT_EXECUTABLE}
            --from-code=utf-8
            --c++
            --force-po
            --output=${RESOURCE_DIR}/locale/${HMDT_TOOL_NAME}.pot
            --keyword=${LOCALIZATION_KEYWORD}
            --keyword=${LOCALIZATION_KEYWORD_DELAY}
            --width=80
            --no-location
            ${CPP_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "pot-update: Pot file generated: ${RESOURCE_DIR}/locale/${HMDT_TOOL_NAME}.pot"
    )
endif(GETTEXT_XGETTEXT_EXECUTABLE)

if(GETTEXT_MSGMERGE_EXECUTABLE)
    add_custom_target(
        pot-merge
        COMMENT "pot-merge: Done."
        DEPENDS ${RESOURCE_DIR}/locale/${HMDT_TOOL_NAME}.pot
    )

    file(GLOB PO_FILES ${RESOURCE_DIR}/locale/*/${HMDT_TOOL_NAME}.po)
    message(TRACE " PO_FILES: ${PO_FILES}")

    foreach(PO_FILE IN ITEMS ${PO_FILES})
        message(STATUS " Adding msgmerge for: ${PO_FILE}")
        add_custom_command(
            TARGET pot-merge
            PRE_BUILD
            COMMAND
                ${GETTEXT_MSGMERGE_EXECUTABLE} ${PO_FILE}
                ${RESOURCE_DIR}/locale/${HMDT_TOOL_NAME}.pot
                --output-file=${PO_FILE}
            COMMENT "pot-merge: ${PO_FILE}"
        )
    endforeach()
endif(GETTEXT_MSGMERGE_EXECUTABLE)

if(GETTEXT_MSGFMT_EXECUTABLE)
    file(GLOB PO_LANGS LIST_DIRECTORIES true ${RESOURCE_DIR}/locale/*)
    message(TRACE " PO_LANGS: ${PO_LANGS}")

    add_custom_target(
        po-compile
        COMMENT "po-compile: Done."
    )

    foreach(PO_LANG IN ITEMS ${PO_LANGS})
        if(IS_DIRECTORY ${PO_LANG})
            file(RELATIVE_PATH lang ${RESOURCE_DIR}/locale ${PO_LANG})

            message(STATUS " Adding msgfmt for: ${PO_LANG}")
            add_custom_command(
                TARGET po-compile
                PRE_BUILD
                COMMAND echo ${LOCALIZATION_RESOURCE_DIR}/${lang}/LC_MESSAGES/${HMDT_TOOL_NAME}.mo
                COMMAND ${CMAKE_COMMAND} -E make_directory ${LOCALIZATION_RESOURCE_DIR}/${lang}/LC_MESSAGES/
                COMMAND
                    ${GETTEXT_MSGFMT_EXECUTABLE}
                    --output-file=${LOCALIZATION_RESOURCE_DIR}/${lang}/LC_MESSAGES/${HMDT_TOOL_NAME}.mo
                    ${HMDT_TOOL_NAME}.po
                WORKING_DIRECTORY ${PO_LANG}
                COMMENT "po-compile: ${PO_LANG}"
            )
        endif()
    endforeach()
endif(GETTEXT_MSGFMT_EXECUTABLE)

add_custom_target(locale_resources
    COMMAND echo "Copying ${LOCALIZATION_RESOURCE_DIR}/* to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/locale/"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/locale/
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${LOCALIZATION_RESOURCE_DIR}/ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/locale/
    DEPENDS po-compile pot-update
)

