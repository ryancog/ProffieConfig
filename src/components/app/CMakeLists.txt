# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/app/CMakeLists.txt

# This gets statically linked regardless, but there's still 
# distinction in its dependencies
add_library(app STATIC 
    app.cpp
)

target_link_libraries(app 
    wxWidgets

    utils
    log
    paths
)

include(../Component.cmake)
setup_component_and_static(app 0.0.0)

