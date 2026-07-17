# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/pconf/CMakeLists.txt

add_library(pconf SHARED
    info.cpp
    read.cpp
    utils.cpp
    write.cpp
    types.cpp
)

set(headers
    info.hpp
    read.hpp
    write.hpp
    utils.hpp
    types.hpp
)

target_link_libraries(pconf log)

include(../Component.cmake)
setup_component_and_static(pconf 1.1.0 ${headers})

