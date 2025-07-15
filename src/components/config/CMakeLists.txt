# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/config/CMakeLists.txt

add_library(config SHARED 
    config.cpp

    wiring/blades.cpp
    wiring/components.cpp
    wiring/wiring.cpp

    preset/preset.cpp
)
target_link_libraries(config 
    wxWidgets

    # A good story for another time
    # led
    # styles
)

include(../Component.cmake)
setup_component(config)

