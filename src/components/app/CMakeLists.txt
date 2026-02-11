# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/app/CMakeLists.txt

add_library(app SHARED
    app.cpp
    critical_dialog.cpp
)

target_link_libraries(app
    wxWidgets

    utils
    log
)

include(../Component.cmake)
setup_component_and_static(app 1.0.0)


if (CMAKE_SYSTEM_NAME STREQUAL Windows) 
    target_link_libraries(app libbacktrace dbghelp imagehlp)
    target_link_libraries(app-static libbacktrace dbghelp imagehlp)
endif()

