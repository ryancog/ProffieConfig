# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/config/CMakeLists.txt

add_library(config SHARED 
    config.cpp

    bladeconfig/bladeconfig.cpp
    bladeconfig/simple.cpp
    bladeconfig/ws281x.cpp

    preset/array.cpp
    preset/preset.cpp

    settings/settings.cpp
)
target_link_libraries(config 
    wxWidgets

    paths
    utils
    # A good story for another time
    # led
    # styles
)

include(../Component.cmake)
setup_component(config 1.0.0)

