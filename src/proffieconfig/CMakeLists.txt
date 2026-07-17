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
    mainmenu/dialogs/preferences.cpp
    mainmenu/dialogs/serialmonitor.cpp
    mainmenu/dialogs/versions.cpp

    editor/editorwindow.cpp
    editor/special/splitvisualizer.cpp
    editor/dialogs/awareness.cpp
    editor/dialogs/bladearray.cpp
    editor/dialogs/buttons.cpp
    editor/dialogs/customopts.cpp
    editor/dialogs/injections.cpp
    editor/dialogs/presetarray.cpp
    editor/dialogs/propbuttons.cpp
    editor/dialogs/propinfo.cpp
    editor/dialogs/stylealiases.cpp
    editor/pages/general.cpp
    editor/pages/props.cpp
    editor/pages/presets.cpp
    editor/pages/blades.cpp

    onboard/onboard.cpp
    onboard/pages/welcome.cpp
    onboard/pages/setup.cpp
    onboard/pages/info.cpp

    tools/arduino.cpp
    tools/serialmonitor.cpp
)

set(headers
    tools/serialmonitor.hpp
    tools/arduino.hpp
    core/state.hpp
    core/licenses.hpp
    onboard/onboard.hpp
    onboard/pages/info.hpp
    onboard/pages/setup.hpp
    onboard/pages/welcome.hpp
    mainmenu/dialogs/versions.hpp
    mainmenu/dialogs/addconfig.hpp
    mainmenu/dialogs/serialmonitor.hpp
    mainmenu/dialogs/manifest.hpp
    mainmenu/dialogs/licenses.hpp
    mainmenu/dialogs/about.hpp
    mainmenu/dialogs/preferences.hpp
    mainmenu/mainmenu.hpp
    editor/editorwindow.hpp
    editor/special/splitvisualizer.hpp
    editor/dialogs/injections.hpp
    editor/dialogs/presetarray.hpp
    editor/dialogs/customopts.hpp
    editor/dialogs/bladearray.hpp
    editor/dialogs/buttons.hpp
    editor/dialogs/awareness.hpp
    editor/dialogs/propinfo.hpp
    editor/dialogs/stylealiases.hpp
    editor/dialogs/propbuttons.hpp
    editor/pages/blades.hpp
    editor/pages/presets.hpp
    editor/pages/general.hpp
    editor/pages/props.hpp
)

set_target_properties(proffieconfig PROPERTIES
    OUTPUT_NAME "ProffieConfig"
    DESCRIPTION "ProffieConfig" # Windows is stupid so this isn't actually a description
    BIN_VERSION 1.9.7
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
setup_target(proffieconfig ${headers})

if (CMAKE_SYSTEM_NAME STREQUAL Windows)
    add_custom_command(TARGET proffieconfig POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:proffieconfig> $<TARGET_FILE_DIR:proffieconfig>
        COMMAND_EXPAND_LISTS
    )
endif()

