# ProffieConfig, The All-In-One Proffieboard Management Utility!

# src/CMakeLists.txt

set(SHARED_COMPONENTS
    utils
    log
    ui
    pconf
)

set(STATIC_COMPONENTS
    app
)
set(ALL_COMPONENTS ${SHARED_COMPONENTS} ${STATIC_COMPONENTS})

foreach(component ${ALL_COMPONENTS})
    add_subdirectory(components/${component})
endforeach()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/components)

add_subdirectory(proffieconfig)
add_subdirectory(launcher)
add_subdirectory(upgen)

install(TARGETS proffieconfig RUNTIME DESTINATION ${INSTALL_BIN_DIR})
install(TARGETS launcher RUNTIME DESTINATION ${INSTALL_DIR} BUNDLE DESTINATION ${INSTALL_DIR})

install(TARGETS ${SHARED_COMPONENTS} 
    LIBRARY DESTINATION ${INSTALL_COMPONENTS_DIR}
    RUNTIME DESTINATION ${INSTALL_COMPONENTS_DIR}
)

