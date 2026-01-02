# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# test/CMakeLists.txt

add_executable(test EXCLUDE_FROM_ALL 
    main.cpp
)

Include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.1
)
FetchContent_MakeAvailable(Catch2)

target_link_libraries(test
    Catch2::Catch2WithMain

    app
    config
    log
    pconf
    process
    ui
    utils
    versions
    versions_manager
)

include (../Common.cmake)
setup_target(test)

install(TARGETS test DESTINATION ${INSTALL_DIR}/test/ OPTIONAL)

