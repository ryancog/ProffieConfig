# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/versions_manager/CMakeLists.txt

add_library(versions_manager SHARED 
    manager.cpp

    info.cpp
)

target_link_libraries(versions_manager
    wxWidgets

    versions
)

include(../Component.cmake)
setup_component_and_static(versions_manager 1.0.0)


