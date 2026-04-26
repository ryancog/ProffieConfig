# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# proffieconfig/CMakeLists.txt

add_executable(proffieconfig WIN32
    main.cpp

    core/state.cpp

    mainmenu/mainmenu.cpp
    mainmenu/dialogs/about.cpp
    mainmenu/dialogs/addconfig.cpp
    mainmenu/dialogs/licenses.cpp
    mainmenu/dialogs/manifest.cpp

    editor/editorwindow.cpp
    editor/special/splitvisualizer.cpp
    editor/dialogs/awareness.cpp
    editor/dialogs/bladearray.cpp
    editor/dialogs/buttons.cpp
    editor/dialogs/presetarray.cpp
    editor/pages/general.cpp
    editor/pages/props.cpp
    editor/pages/presets.cpp
    editor/pages/blades.cpp

    onboard/onboard.cpp
    onboard/pages/welcome.cpp
    onboard/pages/setup.cpp
    onboard/pages/info.cpp

    tools/arduino.cpp
)

set_target_properties(proffieconfig PROPERTIES
    OUTPUT_NAME "ProffieConfig"
    DESCRIPTION "ProffieConfig" # Windows is stupid so this isn't actually a description
    BIN_VERSION 1.9.0
)

target_link_libraries(proffieconfig 
    wxWidgets

    app
    log
    ui
    utils
    pconf
    config
    process
)

include (../Common.cmake)
setup_target(proffieconfig)

