# ProffieConfig, The All-In-One Proffieboard Management Utility!

# src/CMakeLists.txt

set(SHARED_COMPONENTS
    app
    config
    data
    log
    pconf
    process
    ui
    utils
    versions
    versions_manager
)

set(STATIC_COMPONENTS
)

set(ALL_COMPONENTS ${SHARED_COMPONENTS} ${STATIC_COMPONENTS})

foreach(component ${ALL_COMPONENTS})
    add_subdirectory(components/${component})
endforeach()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/components)

add_subdirectory(proffieconfig)
add_subdirectory(launcher)
add_subdirectory(upgen)
add_subdirectory(test)

install(TARGETS proffieconfig RUNTIME DESTINATION ${INSTALL_BIN_DIR})
install(TARGETS launcher RUNTIME DESTINATION ${INSTALL_DIR} BUNDLE DESTINATION ${INSTALL_DIR})

foreach(component ${SHARED_COMPONENTS})
    install(FILES $<TARGET_FILE:${component}>
        DESTINATION ${INSTALL_COMPONENTS_DIR}
    )
endforeach()

