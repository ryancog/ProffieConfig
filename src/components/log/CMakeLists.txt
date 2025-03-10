# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/log/CMakeLists.txt

set(files
    logger.cpp
    context.cpp
    branch.cpp
    message.cpp
)

add_library(log SHARED ${files})
add_library(log-static STATIC ${files})

target_link_libraries(log
    utils
)

target_link_libraries(log-static 
    utils-static
)

include(../Component.cmake)
setup_component_and_static(log)

