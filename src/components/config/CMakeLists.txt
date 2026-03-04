# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/config/CMakeLists.txt

add_library(config SHARED 
    config.cpp
    info.cpp

    priv/parse/parse.cpp
    priv/generate/generate.cpp

    misc/injection.cpp

    blades/bladeconfig.cpp
    blades/simple.cpp
    blades/ws281x.cpp

    buttons/button.cpp

    presets/array.cpp
    presets/preset.cpp

    settings/settings.cpp
    settings/bladeawareness.cpp
    settings/define.cpp
)
target_link_libraries(config 
    wxWidgets

    data
    utils
    ui
    # A good story for another time
    # led
    # styles
)

include(../Component.cmake)
setup_component_and_static(config 1.0.11)

