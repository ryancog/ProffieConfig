# ProffieConfig, All-In-One Proffieboard Management Utility!
#
# components/log/CMakeLists.txt

add_library(log SHARED 
    info.cpp

    logger.cpp
    context.cpp
    branch.cpp
    message.cpp
)
target_link_libraries(log
    wxWidgets

    utils
)

include(../Component.cmake)
setup_component_and_static(log 1.0.3)

