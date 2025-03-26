# ProffieConfig, All-In-One Proffieboard Management Utility!
#
# updater/CMakeLists.txt

set(ICON_PATH ${CMAKE_SOURCE_DIR}/resources/icons/icon.icns)
set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION Resources )

add_executable(launcher WIN32 MACOSX_BUNDLE
    main.cpp

    routines.cpp

    update/update.cpp
    update/pulldata.cpp
    update/changelog.cpp
    update/install.cpp

    ${ICON_PATH}
)


set_target_properties(launcher PROPERTIES
    OUTPUT_NAME "ProffieConfig"
    DESCRIPTION "ProffieConfig Launcher" # Windows is stupid so this isn't actually a description
    EXEC_VERSION 1.0.5
    MACOSX_BUNDLE_BUNDLE_VERSION ${CMAKE_PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
    MACOSX_BUNDLE_COPYRIGHT "Copyright (C) 2025 Ryan Ogurek"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.kafrenetrading.proffieconfiglauncher"
    MACOSX_BUNDLE_ICON_FILE "icon.icns"
)

target_link_libraries(launcher
    wxWidgets-static

    app-static
    log-static
    utils-static
    ui-static
    pconf-static
)

include(../Common.cmake)
setup_target(launcher)

