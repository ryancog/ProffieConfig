# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/utils/CMakeLists.txt

set(files
    trace.cpp
    paths.cpp
    image.cpp
    undo.cpp
    theme.cpp
    crypto.cpp
)

add_library(utils SHARED ${files})
add_library(utils-static STATIC ${files})

target_link_libraries(utils
    wxWidgets
    tomcrypt-static
)
target_link_libraries(utils-static
    wxWidgets-static
    tomcrypt-static
)

include(../Component.cmake)
setup_component_and_static(utils)

