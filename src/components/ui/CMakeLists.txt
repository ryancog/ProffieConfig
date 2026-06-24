# ProffieConfig, All-In-One Proffieboard Management Utility!
#
# components/ui/CMakeLists.txt

add_library(ui SHARED
    frame.cpp
    dialog.cpp
    build.cpp
    info.cpp
    font.cpp
    values.cpp
    bitmap.cpp
    utils.cpp
    symbols.cpp

    priv/tlw.cpp
    priv/tracker.cpp

    detail/descriptor.cpp
    detail/general.cpp
    detail/helpers.cpp
    detail/scaffold.cpp
    detail/window.cpp
    detail/datawin.cpp

    dialogs/message.cpp
    dialogs/progress.cpp

    static/divider.cpp
    static/image.cpp
    static/label.cpp
    static/hyperlink.cpp

    indication/progress.cpp

    helpers/busy.cpp
    helpers/dialog_buttons.cpp
    helpers/labeled.cpp

    builders/selector.cpp
    builders/choice.cpp
    builders/vecstack.cpp

    controls/detail/btnstyle.cpp
    controls/button.cpp
    controls/toggle_button.cpp
    controls/checkbox.cpp
    controls/checklist.cpp
    controls/choice.cpp
    controls/combobox.cpp
    controls/radios.cpp
    controls/segmented.cpp
    controls/stepper.cpp
    controls/text.cpp
    controls/filepicker.cpp
    controls/list.cpp

    layout/collapsible.cpp
    layout/group.cpp
    layout/panel.cpp
    layout/notebook.cpp
    layout/scrolled.cpp
    layout/spacer.cpp
    layout/split.cpp
    layout/stack.cpp
    layout/priv/groupbox.cpp
    layout/detail/panel.cpp
)

target_link_libraries(ui 
    wxWidgets
    app
    data
)

include(../Component.cmake)
setup_component_and_static(ui 1.1.3)

if (CMAKE_SYSTEM_NAME STREQUAL Windows OR CROSS_COMPILE STREQUAL Windows)
    target_link_libraries(ui dwmapi)
    target_link_libraries(ui-static dwmapi)
endif()

