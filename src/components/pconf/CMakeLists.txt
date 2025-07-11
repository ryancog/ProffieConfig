# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/pconf/CMakeLists.txt

add_library(pconf SHARED
    pconf.cpp
)

target_link_libraries(pconf log)

include(../Component.cmake)
setup_component_and_static(pconf 1.0.5)

