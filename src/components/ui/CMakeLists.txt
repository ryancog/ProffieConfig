# ProffieConfig, All-In-One Proffieboard Management Utility!
#
# components/ui/CMakeLists.txt

add_library(ui SHARED
    frame.cpp
    build.cpp
    info.cpp
    text.cpp

    priv/helpers.cpp

    detail/descriptor.cpp
    detail/scaffold.cpp

    misc/message.cpp

    static/divider.cpp
    static/image.cpp
    static/label.cpp
    static/hyperlink.cpp

    indication/progress.cpp

    controls/button.cpp
    controls/checkbox.cpp
    controls/radios.cpp
    controls/stepper.cpp

    layout/group.cpp
    layout/selector.cpp
    layout/spacer.cpp
    layout/stack.cpp
    layout/panel.cpp
    layout/priv/groupbox.cpp
)

target_link_libraries(ui 
    wxWidgets
    app
    data
)

include(../Component.cmake)
setup_component_and_static(ui 1.0.12)

if (CMAKE_SYSTEM_NAME STREQUAL Windows) 
    target_link_libraries(ui dwmapi)
    target_link_libraries(ui-static dwmapi)
endif()
