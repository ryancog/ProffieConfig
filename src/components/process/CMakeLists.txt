# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# components/process/CMakeLists.txt

add_library(process SHARED 
    process.cpp
)

include(../Component.cmake)
setup_component_and_static(process 1.0.0)

