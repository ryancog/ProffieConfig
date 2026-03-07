# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# proffieconfig/CMakeLists.txt

add_executable(proffieconfig WIN32
    main.cpp

    core/state.cpp

    onboard/onboard.cpp
    onboard/pages/welcome.cpp
    onboard/pages/setup.cpp
    onboard/pages/info.cpp
)

set_target_properties(proffieconfig PROPERTIES
    OUTPUT_NAME "ProffieConfig"
    DESCRIPTION "ProffieConfig" # Windows is stupid so this isn't actually a description
    BIN_VERSION 1.8.8
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

