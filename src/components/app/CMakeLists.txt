# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/app/CMakeLists.txt

add_library(app SHARED
    app.cpp
    critical_dialog.cpp
    info.cpp
)

set(headers
    critical_dialog.hpp
    info.hpp
    app.hpp
)

target_link_libraries(app
    wxWidgets

    utils
    log
)

include(../Component.cmake)
setup_component_and_static(app 1.1.3 ${headers})

if (CMAKE_SYSTEM_NAME STREQUAL Windows) 
    target_link_libraries(app dbghelp imagehlp)
    target_link_libraries(app-static dbghelp imagehlp)
endif()

if (CROSS_COMPILE STREQUAL Windows)
    target_link_libraries(app libbacktrace)
    target_link_libraries(app-static libbacktrace)
endif()

