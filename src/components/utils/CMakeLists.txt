# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/utils/CMakeLists.txt

add_library(utils SHARED 
    trace.cpp
    image.cpp
    undo.cpp
    theme.cpp
    crypto.cpp
    version.cpp
)

target_link_libraries(utils
    wxWidgets
    tomcrypt-static

    paths
)

include(../Component.cmake)
setup_component_and_static(utils 1.0.7)

