# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/utils/CMakeLists.txt

add_library(utils SHARED 
    info.cpp

    color.cpp
    data.cpp
    files.cpp
    hash.cpp
    paths.cpp
    rand.cpp
    string.cpp
    version.cpp
)

set(headers
    paths.hpp
    hash.hpp
    info.hpp
    files.hpp
    objc.hpp
    rand.hpp
    version.hpp
    string.hpp
    data.hpp
    color.hpp
    demangle.hpp
    types.hpp
    defer.hpp
)

target_link_libraries(utils
    wxWidgets
    tomcrypt-static
)

include(../Component.cmake)
setup_component_and_static(utils 1.0.16 ${headers})

