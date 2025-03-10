# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/app/CMakeLists.txt

set(files
    app.cpp
)

# This gets statically linked regardless, but there's still 
# distinction in its dependencies
add_library(app STATIC ${files})
add_library(app-static STATIC ${files})

target_link_libraries(app 
    wxWidgets

    utils
    log
)
target_link_libraries(app-static
    wxWidgets-static

    utils-static
    log-static
)

include(../Component.cmake)
setup_component_and_static(app)

