# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# proffieconfig/CMakeLists.txt

add_executable(proffieconfig WIN32
    main.cpp

    editor/dialogs/bladearraydlg.cpp 
    editor/dialogs/customoptionsdlg.cpp
    editor/editorwindow.cpp
    editor/pages/propspage.cpp
    core/appstate.cpp
    core/utilities/misc.cpp
    core/utilities/progress.cpp
    core/config/configuration.cpp
    core/config/settings.cpp
    core/config/propfile.cpp
    editor/pages/generalpage.cpp
    editor/pages/presetspage.cpp
    editor/pages/bladespage.cpp
    mainmenu/dialogs/addconfig.cpp
    mainmenu/dialogs/props.cpp
    mainmenu/mainmenu.cpp
    onboard/onboard.cpp
    onboard/pages/dependencypage.cpp
    onboard/pages/overviewpage.cpp
    onboard/pages/welcomepage.cpp
    tools/arduino.cpp
    tools/serialmonitor.cpp
)

set_target_properties(proffieconfig PROPERTIES
    OUTPUT_NAME "ProffieConfig"
    DESCRIPTION "ProffieConfig" # Windows is stupid so this isn't actually a description
)

target_link_libraries(proffieconfig 
    wxWidgets

    app
    log
    ui
    utils
    pconf
    paths
)

include (../Common.cmake)
setup_target(proffieconfig)

