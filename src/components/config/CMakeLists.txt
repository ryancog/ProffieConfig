# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/config/CMakeLists.txt

add_library(config SHARED 
    config.cpp
    info.cpp

    priv/data.cpp
    priv/io.cpp
    priv/parse/preset/array.cpp
    priv/parse/preset/blades.cpp
    priv/parse/buttons.cpp
    priv/parse/presets.cpp
    priv/parse/prop.cpp
    priv/parse/styles.cpp
    priv/parse/top.cpp
    priv/parse/utils.cpp
    priv/generate/check/check.cpp
    priv/generate/buttons.cpp
    priv/generate/presets.cpp
    priv/generate/prop.cpp
    priv/generate/styles.cpp
    priv/generate/top.cpp
    priv/utils/style.cpp

    misc/injection.cpp

    blades/bladeconfig.cpp
    blades/simple.cpp
    blades/ws281x.cpp
    blades/servo.cpp

    buttons/button.cpp

    presets/array.cpp
    presets/preset.cpp
    presets/style.cpp

    settings/settings.cpp
    settings/bladeawareness.cpp
    settings/define.cpp

    styles/style.cpp
)

set(headers
    misc/injection.hpp
    settings/settings.hpp
    settings/bladeawareness.hpp
    settings/define.hpp
    info.hpp
    buttons/button.hpp
    config.hpp
    blades/ws281x.hpp
    blades/simple.hpp
    blades/servo.hpp
    blades/bladeconfig.hpp
    strings.hpp
    styles/style.hpp
    priv/keys.hpp
    priv/io.hpp
    priv/parse/preset/array.hpp
    priv/parse/preset/blades.hpp
    priv/parse/styles.hpp
    priv/parse/prop.hpp
    priv/parse/buttons.hpp
    priv/parse/top.hpp
    priv/parse/presets.hpp
    priv/parse/utils.hpp
    priv/utils/style.hpp
    priv/generate/styles.hpp
    priv/generate/prop.hpp
    priv/generate/buttons.hpp
    priv/generate/top.hpp
    priv/generate/presets.hpp
    priv/generate/check/check.hpp
    priv/data.hpp
    presets/preset.hpp
    presets/array.hpp
    presets/style.hpp
)

target_link_libraries(config 
    wxWidgets

    data
    utils
    ui
    versions
    # A good story for another time
    # led
    # styles
)

include(../Component.cmake)
setup_component_and_static(config 1.2.1 ${headers})

