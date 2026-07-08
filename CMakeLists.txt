# ProffieConfig, The All-In-One Proffieboard Management Utility!

cmake_minimum_required(VERSION 3.31)

if (CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum OS X deployment version" FORCE)

    if (NOT LOCAL_BUILD)
        set(CMAKE_OSX_ARCHITECTURES arm64;x86_64 CACHE STRING "Build universal binary" FORCE)
    endif()
endif()

project(ProffieConfig)

if (NOT (
    CMAKE_HOST_SYSTEM_NAME STREQUAL Linux OR
    CMAKE_HOST_SYSTEM_NAME STREQUAL Darwin OR
    CMAKE_HOST_SYSTEM_NAME STREQUAL Windows
))
    # Rip Windows
    message(FATAL_ERROR "Building on this platform is not supported.")
endif()

if (CROSS_COMPILE STREQUAL Windows)
    message(NOTICE "Configuring as cross-compile for Windows.")
else()
    message(NOTICE "Configuring as native build for ${CMAKE_HOST_SYSTEM_NAME}...")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_LINK_DEPENDS_NO_SHARED TRUE)

# MinGW requires manual installation of plugin for LTO.
# See: https://stackoverflow.com/a/32461766
# This generally causes issues though...
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_REPRODUCIBLE_BUILD ON)

cmake_path(GET CMAKE_CURRENT_BINARY_DIR FILENAME BUILD_NAME)
set(INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/deploy/${BUILD_NAME})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs-c- /D_HAS_EXCEPTIONS=0")
    # This is a hack because I can't figure out how to otherwise solve the
    # "No Target Architecture" error in `windows.h`
    add_compile_definitions(_AMD64_)
    # I guess because wx has it?
    add_link_options(/NODEFAULTLIB:LIBCMT)
    # add_link_options(/NODEFAULTLIB:MSVCRT)

    add_compile_options(/FS)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
elseif (CMAKE_SYSTEM_NAME STREQUAL Windows)
    add_compile_definitions(UNICODE _UNICODE)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Release)
    if (CMAKE_SYSTEM_NAME STREQUAL Darwin)
        set(REL_COMP_FLAGS "")
        set(REL_LINK_FLAGS "")
        set(PLATFORM_SHORTNAME macOS)
    elseif (CMAKE_SYSTEM_NAME STREQUAL Linux)
        set(REL_COMP_FLAGS "-s -ffunction-sections -fdata-sections")
        set(REL_LINK_FLAGS "-Wl,--gc-sections -Wl,--strip-all -Wl,--as-needed")
        set(PLATFORM_SHORTNAME linux)
    elseif (CMAKE_SYSTEM_NAME STREQUAL Windows)
        set(PLATFORM_SHORTNAME win32)
    elseif (CROSS_COMPILE STREQUAL Windows)
        set(PLATFORM_SHORTNAME win32)
        set(REL_COMP_FLAGS "-s -ffunction-sections -fdata-sections")
        set(REL_LINK_FLAGS "-Wl,--gc-sections -Wl,--strip-all -Wl,--as-needed")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${REL_LINK_FLAGS} -Wl,--build-id=none -Wl,--no-insert-timestamp")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${REL_LINK_FLAGS} -Wl,--build-id=none -Wl,--no-insert-timestamp")
    endif()
    if (CMAKE_SYSTEM_NAME STREQUAL Linux OR CROSS_COMPILE STREQUAL Windows)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${REL_COMP_FLAGS} -fno-record-gcc-switches")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${REL_COMP_FLAGS} -fno-record-gcc-switches")
    endif()
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(LOCAL_BUILD TRUE)

    if (NOT CMAKE_SYSTEM_NAME STREQUAL Windows)
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (LOCAL_BUILD)
    set(INSTALL_ITEM_DIR ${INSTALL_DIR}/items)

    set(APP_DEPLOY_PATH ${INSTALL_ITEM_DIR})

    if (CMAKE_SYSTEM_NAME STREQUAL Windows)
        execute_process(
            COMMAND cmd /c for %I in ("${APP_DEPLOY_PATH}") do @echo %~sI
            OUTPUT_VARIABLE APP_DEPLOY_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        string(REPLACE "\\" "/" APP_DEPLOY_PATH "${APP_DEPLOY_PATH}")
    endif()

    add_compile_definitions(APP_DEPLOY_PATH="${APP_DEPLOY_PATH}")
else()
    set(INSTALL_ITEM_DIR ${CMAKE_CURRENT_LIST_DIR}/staging/${PLATFORM_SHORTNAME})
endif()

if (LOCAL_BUILD AND (CMAKE_SYSTEM_NAME STREQUAL Windows OR CROSS_COMPILE STREQUAL Windows))
    set(INSTALL_BIN_DIR ${INSTALL_ITEM_DIR}/bin)
    set(INSTALL_COMPONENTS_DIR ${INSTALL_BIN_DIR})
    set(INSTALL_LIB_DIR ${INSTALL_BIN_DIR})
    set(INSTALL_RESOURCE_DIR ${INSTALL_ITEM_DIR}/resources)
else()
    set(INSTALL_BIN_DIR ${INSTALL_ITEM_DIR}/bin)
    set(INSTALL_COMPONENTS_DIR ${INSTALL_ITEM_DIR}/components)
    set(INSTALL_LIB_DIR ${INSTALL_ITEM_DIR}/lib)
    set(INSTALL_RESOURCE_DIR ${INSTALL_ITEM_DIR}/resources)
endif()

if (
        CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
        CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"
)
    install(FILES ${CMAKE_BINARY_DIR}/compile_commands.json DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})

    add_compile_definitions(LIFETIMEBOUND=[[clang::lifetimebound]])
else()
    add_compile_definitions(LIFETIMEBOUND=)
endif()

add_subdirectory(3rdparty)
add_subdirectory(resources)
add_subdirectory(src)
# add_subdirectory(tests EXCLUDE_FROM_ALL)

