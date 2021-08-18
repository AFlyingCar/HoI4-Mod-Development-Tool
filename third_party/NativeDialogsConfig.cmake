cmake_minimum_required(VERSION 3.0)

cmake_policy(SET CMP0097 NEW)

include(FetchContent)

find_package(gtkmm REQUIRED)

FetchContent_Declare(native_dialogs
    GIT_REPOSITORY https://github.com/Geequlim/NativeDialogs
    GIT_SUBMODULES ""
)

FetchContent_GetProperties(native_dialogs)

if(NOT native_dialogs_POPULATED)
    FetchContent_Populate(native_dialogs)

    set(ND_SOURCES ${native_dialogs_SOURCE_DIR}/src/NativeDialog.cpp)

    if(WIN32)
        set(ND_SOURCES ${ND_SOURCES}
            ${native_dialogs_SOURCE_DIR}/src/win/ColorPickerDialog.cpp
            ${native_dialogs_SOURCE_DIR}/src/win/FileDialog-Windows.cpp
            ${native_dialogs_SOURCE_DIR}/src/win/MessageDialog.cpp)
    elseif(APPLE)
        set(ND_SOURCES ${ND_SOURCES}
            ${native_dialogs_SOURCE_DIR}/src/osx/ColorPickerDialog-OSX.mm
            ${native_dialogs_SOURCE_DIR}/src/osx/FileDialog-OSX.mm
            ${native_dialogs_SOURCE_DIR}/src/osx/MessageDialog-OSX.mm)
    else()
        set(ND_SOURCES ${ND_SOURCES}
            ${native_dialogs_SOURCE_DIR}/src/gtk/ColorPickerDialog-GTK.cpp
            ${native_dialogs_SOURCE_DIR}/src/gtk/FileDialog-GTK.cpp
            ${native_dialogs_SOURCE_DIR}/src/gtk/MessageDialog-GTK.cpp)
    endif()

    add_library(native_dialogs STATIC ${ND_SOURCES})
    target_include_directories(native_dialogs SYSTEM PUBLIC ${native_dialogs_SOURCE_DIR}/src)
    target_link_libraries(native_dialogs gtkmm)

    target_compile_options(native_dialogs PRIVATE -Wno-error -fpermissive -Wno-missing-field-initializers -Wno-unknown-pragmas)

    install(FILES ${native_dialogs_SOURCE_DIR}/LICENSE.txt
            DESTINATION ${INSTALL_DESTINATION}/share/licenses
            RENAME LICENSE.NativeDialogs.txt)
endif()

