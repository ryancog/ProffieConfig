# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/utils/CMakeLists.txt

add_library(utils SHARED 
    info.cpp

    color.cpp
    files.cpp
    hash.cpp
    paths.cpp
    rand.cpp
    string.cpp
    version.cpp
)

target_link_libraries(utils
    wxWidgets
    tomcrypt-static
)

include(../Component.cmake)
setup_component_and_static(utils 1.0.13)

