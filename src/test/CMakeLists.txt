# ProffieConfig, All-In-One Proffieboard Management Utility!
# 
# test/CMakeLists.txt

add_executable(test EXCLUDE_FROM_ALL 
    main.cpp

    tests/crypto.cpp
    tests/config.cpp
    tests/style.cpp
)

Include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.1
)
FetchContent_MakeAvailable(Catch2)

target_compile_options(Catch2 PUBLIC
    -fexceptions
)

target_link_libraries(test
    Catch2::Catch2

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

target_compile_definitions(test PRIVATE
    CONFIG_DIR_STR="${PROJECT_SOURCE_DIR}/testing/configs"
)

include (../Common.cmake)
setup_target(test)

install(TARGETS test DESTINATION ${PROJECT_SOURCE_DIR} OPTIONAL)

