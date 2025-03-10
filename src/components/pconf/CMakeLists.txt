# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/pconf/CMakeLists.txt

set(files
    pconf.cpp
)

add_library(pconf SHARED ${files})
add_library(pconf-static STATIC ${files})

target_link_libraries(pconf log)

target_link_libraries(pconf-static log-static)

include(../Component.cmake)
setup_component_and_static(pconf)

