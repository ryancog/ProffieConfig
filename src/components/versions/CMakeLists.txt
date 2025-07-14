# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/versions/CMakeLists.txt

add_library(versions SHARED 
    versions.cpp
)

target_link_libraries(versions
    wxWidgets

    pconf
)

include(../Component.cmake)
setup_component_and_static(versions 1.0.0)

