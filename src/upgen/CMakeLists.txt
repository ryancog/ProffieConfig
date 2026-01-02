# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# upgen/CMakeLists.txt

add_executable(upgen EXCLUDE_FROM_ALL 
    main.cpp

    ../launcher/update/update.cpp
)

target_link_libraries(upgen
    wxWidgets-static

    app-static
    utils-static
    log-static
    ui-static
    pconf-static
)

include (../Common.cmake)
setup_target(upgen)

install(TARGETS upgen DESTINATION ${PROJECT_SOURCE_DIR} OPTIONAL)

