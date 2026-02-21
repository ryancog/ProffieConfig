# ProffieConfig, All-In-One Proffieboard Management Utility!
#
# components/data/CMakeLists.txt

add_library(data SHARED
)

target_link_libraries(data
)

include(../Component.cmake)
setup_component_and_static(data 1.0.0)

