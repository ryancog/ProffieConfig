# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/paths/CMakeLists.txt

add_library(paths SHARED 
    paths.cpp
)

target_link_libraries(paths
    wxWidgets
)

include(../Component.cmake)
setup_component_and_static(paths 1.0.0)

