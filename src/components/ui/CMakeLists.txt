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

set(headers
    info.hpp
    build.hpp
    values.hpp
    builders/selector.hpp
    builders/vecstack.hpp
    builders/choice.hpp
    layout/spacer.hpp
    layout/group.hpp
    layout/detail/panel.hpp
    layout/priv/groupbox.hpp
    layout/notebook.hpp
    layout/stack.hpp
    layout/panel.hpp
    layout/scrolled.hpp
    layout/collapsible.hpp
    layout/split.hpp
    symbols.hpp
    indication/progress.hpp
    controls/combobox.hpp
    controls/styledtext.hpp
    controls/list.hpp
    controls/segmented.hpp
    controls/choice.hpp
    controls/filepicker.hpp
    controls/detail/btnstyle.hpp
    controls/radios.hpp
    controls/checklist.hpp
    controls/stepper.hpp
    controls/checkbox.hpp
    controls/button.hpp
    controls/text.hpp
    controls/toggle_button.hpp
    dialog.hpp
    detail/scaffold.hpp
    detail/window.hpp
    detail/helpers.hpp
    detail/general.hpp
    detail/descriptor.hpp
    detail/datawin.hpp
    priv/tlw.hpp
    priv/tracker.hpp
    font.hpp
    static/label.hpp
    static/divider.hpp
    static/hyperlink.hpp
    static/image.hpp
    dialogs/progress.hpp
    dialogs/message_types.hpp
    dialogs/message.hpp
    utils.hpp
    types.hpp
    frame.hpp
    helpers/dialog_buttons.hpp
    helpers/if.hpp
    helpers/foreach.hpp
    helpers/busy.hpp
    helpers/labeled.hpp
    bitmap.hpp
    dynamic_list.hpp
)

target_link_libraries(ui 
    wxWidgets
    app
    data
)

include(../Component.cmake)
setup_component_and_static(ui 1.1.4 ${headers})

if (CMAKE_SYSTEM_NAME STREQUAL Windows OR CROSS_COMPILE STREQUAL Windows)
    target_link_libraries(ui dwmapi)
    target_link_libraries(ui-static dwmapi)
endif()

