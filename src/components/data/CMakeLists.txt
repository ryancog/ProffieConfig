# ProffieConfig, All-In-One Proffieboard Management Utility!
#
# components/data/CMakeLists.txt

add_library(data SHARED
    hierarchy/action.cpp
    hierarchy/model.cpp
    hierarchy/node.cpp
    hierarchy/root.cpp

    bool.cpp
    choice.cpp
    generic.cpp
    number.cpp
    option.cpp
    selection.cpp
    selector.cpp
    string.cpp
    vector.cpp
    version.cpp
)

target_link_libraries(data
    utils
)

include(../Component.cmake)
setup_component_and_static(data 1.0.0)

