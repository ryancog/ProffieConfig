# ProffieConfig, All-In-One Proffieboard Management Utility!
#
# updater/CMakeLists.txt

set(ICON_PATH ${CMAKE_SOURCE_DIR}/resources/icons/icon.icns)
set_source_files_properties(${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

# set(TRANSLATION_ES ${CMAKE_CURRENT_LIST_DIR}/i18n/es/proffieconfig.mo)
# set_source_files_properties(${TRANSLATION_ES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/es.lproj)

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
    BIN_VERSION 1.0.6
    TRANSLATIONS es
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
    paths-static
)

include(../Common.cmake)
setup_target(launcher)

