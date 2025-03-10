# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/utils/CMakeLists.txt

set(files
    frame.cpp
    movable.cpp
    plaque.cpp
    controls.cpp
    message.cpp
)

add_library(ui SHARED ${files})
add_library(ui-static STATIC ${files})

target_link_libraries(ui 
    wxWidgets
    app
)

target_link_libraries(ui-static
    wxWidgets-static
    app-static
)

if (CMAKE_SYSTEM_NAME STREQUAL Windows) 
    target_link_libraries(ui dwmapi)
    target_link_libraries(ui-static dwmapi)
endif()

include(../Component.cmake)
setup_component_and_static(ui)

