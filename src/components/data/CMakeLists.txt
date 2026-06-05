# ProffieConfig, All-In-One Proffieboard Management Utility!
#
# components/data/CMakeLists.txt

add_library(data SHARED
    receiver.cpp

    logic/logic.cpp
    logic/adapter.cpp
    logic/operators.cpp
    logic/constant.cpp

    base/model.cpp
    base/models/bool.cpp
    base/models/choice.cpp
    base/models/exclusive.cpp
    base/models/number.cpp
    base/models/selection.cpp
    base/models/selector.cpp
    base/models/string.cpp
    base/models/vector.cpp
    base/models/version.cpp

    hierarchic/action.cpp
    hierarchic/model.cpp
    hierarchic/root.cpp
    hierarchic/models/bool.cpp
    hierarchic/models/choice.cpp
    hierarchic/models/exclusive.cpp
    hierarchic/models/number.cpp
    hierarchic/models/selection.cpp
    hierarchic/models/selector.cpp
    hierarchic/models/string.cpp
    hierarchic/models/vector.cpp
    hierarchic/models/version.cpp

    primitive/model.cpp
    primitive/models/bool.cpp
    primitive/models/choice.cpp
    primitive/models/exclusive.cpp
    primitive/models/number.cpp
    primitive/models/selection.cpp
    primitive/models/selector.cpp
    primitive/models/string.cpp
    primitive/models/vector.cpp
    primitive/models/version.cpp
)

target_link_libraries(data
    utils
)

include(../Component.cmake)
setup_component_and_static(data 1.0.1)

