# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/utils/CMakeLists.txt

add_library(ui SHARED 
    frame.cpp
    movable.cpp
    plaque.cpp
    message.cpp
    panel.cpp

    controls/button.cpp
    controls/checklist.cpp
    controls/choice.cpp
    controls/combobox.cpp
    controls/filepicker.cpp
    controls/numeric.cpp
    controls/text.cpp
    controls/toggle.cpp
)

target_link_libraries(ui 
    wxWidgets
    app
)

include(../Component.cmake)
setup_component_and_static(ui 1.0.5)

if (CMAKE_SYSTEM_NAME STREQUAL Windows) 
    target_link_libraries(ui dwmapi)
    target_link_libraries(ui-static dwmapi)
endif()
