# ProffieConfig, All-In-One Proffieboard Management Utility!
#
# 3rdparty/CMakeLists.txt

# add_library(tomcrypt SHARED IMPORTED GLOBAL)
# target_include_directories(tomcrypt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tomcrypt/src/headers)
add_library(tomcrypt-static STATIC IMPORTED GLOBAL)
target_include_directories(tomcrypt-static INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tomcrypt/src/headers)

add_library(wxWidgets INTERFACE)
add_library(wxWidgets-static INTERFACE)

add_library(wxBase  SHARED IMPORTED GLOBAL)
add_library(wxNet   SHARED IMPORTED GLOBAL)
add_library(wxCore  SHARED IMPORTED GLOBAL)

add_library(wxBase-static           STATIC IMPORTED GLOBAL)
add_library(wxNet-static            STATIC IMPORTED GLOBAL)
add_library(wxCore-static           STATIC IMPORTED GLOBAL)
add_library(wxWidgets-PNG-static    STATIC IMPORTED GLOBAL)
add_library(wxWidgets-ZLib-static   STATIC IMPORTED GLOBAL)

target_link_libraries(wxWidgets INTERFACE
    wxBase
    wxNet
    wxCore
)

# Use `--start-group` and `--end-group` to handle the linking between
# wxWidgets libraries
set(wxWidgets-static-libraries
    wxBase-static
    wxNet-static
    wxCore-static
    wxWidgets-PNG-static
    wxWidgets-ZLib-static
)

target_link_libraries(wxWidgets-static INTERFACE
    "$<LINK_GROUP:RESCAN,${wxWidgets-static-libraries}>"
)

if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(TC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tomcrypt/build-linux/)
    # set_target_properties(tomcrypt PROPERTIES
    #     IMPORTED_LOCATION ${TC_LIB_DIR}/libtomcrypt.so)
    set_target_properties(tomcrypt-static PROPERTIES
        IMPORTED_LOCATION ${TC_LIB_DIR}/libtomcrypt.a)

    set(WX_SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wxWidgets/install-linux-shared)
    if (IS_DIRECTORY ${WX_SHARED_DIR}/lib64)
        set(WX_LIB_DIR ${WX_SHARED_DIR}/lib64)
    elseif(IS_DIRECTORY ${WX_SHARED_DIR}/lib)
        set(WX_LIB_DIR ${WX_SHARED_DIR}/lib)
    else()
        message(FATAL_ERROR "Missing lib dir")
    endif()
    set_target_properties(wxBase PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu.so)
	set_target_properties(wxNet PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu_net.so)
    set_target_properties(wxCore PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_gtk3u_core.so)

    set(WX_STATIC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wxWidgets/install-linux-static)
    if (IS_DIRECTORY ${WX_STATIC_DIR}/lib64)
        set(WX_LIB_DIR ${WX_STATIC_DIR}/lib64)
    elseif(IS_DIRECTORY ${WX_STATIC_DIR}/lib)
        set(WX_LIB_DIR ${WX_STATIC_DIR}/lib)
    else()
        message(FATAL_ERROR "Missing lib dir")
    endif()
    set_target_properties(wxBase-static PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu-3.3.a)
	set_target_properties(wxNet-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu_net-3.3.a)
    set_target_properties(wxCore-static PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_gtk3u_core-3.3.a)
    set_target_properties(wxWidgets-PNG-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwxpng-3.3.a)
    set_target_properties(wxWidgets-ZLib-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwxzlib-3.3.a)

    # TODO: I don't like find_package and I don't know how it works
    # this is internet copypasta
    find_package(CURL REQUIRED)
    set(OPENSSL_USE_STATIC_LIBS TRUE)
    set(CURL_STATICLIB TRUE)

    target_link_libraries(wxNet-static INTERFACE CURL::libcurl ${CURL_LIBRARIES} ${CURL_STATIC_LIBRARIES})

elseif (CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(TC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tomcrypt/build-macOS/)
    # set_target_properties(tomcrypt-shared PROPERTIES
    #    IMPORTED_LOCATION ${TC_LIB_DIR}/libtomcrypt.dylib)
    set_target_properties(tomcrypt-static PROPERTIES
        IMPORTED_LOCATION ${TC_LIB_DIR}/libtomcrypt.a)

    set(WX_SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wxWidgets/install-macOS-shared)
    set(WX_LIB_DIR ${WX_SHARED_DIR}/lib)
    set_target_properties(wxBase PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu.dylib)
	set_target_properties(wxNet PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu_net.dylib)
    set_target_properties(wxCore PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_osx_cocoau_core.dylib)

    set(WX_STATIC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wxWidgets/install-macOS-static)
    set(WX_LIB_DIR ${WX_STATIC_DIR}/lib)
    set_target_properties(wxBase-static PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu-3.3.a)
	set_target_properties(wxNet-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu_net-3.3.a)
    set_target_properties(wxCore-static PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_osx_cocoau_core-3.3.a)
    set_target_properties(wxWidgets-PNG-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwxpng-3.3.a)
    set_target_properties(wxWidgets-ZLib-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwxzlib-3.3.a)

elseif (CMAKE_SYSTEM_NAME STREQUAL Windows)
    add_library(libbacktrace STATIC IMPORTED GLOBAL)
    set_target_properties(libbacktrace PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libbacktrace/install-win32/lib/libbacktrace.a
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/libbacktrace/install-win32/include)

    set(TC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tomcrypt/build-win32/)
    # set_target_properties(tomcrypt PROPERTIES
    #     IMPORTED_IMPLIB ${TC_LIB_DIR}/tomcrypt.dll)
    set_target_properties(tomcrypt-static PROPERTIES
        IMPORTED_LOCATION ${TC_LIB_DIR}/libtomcrypt.a
        IMPORTED_IMPLIB ${TC_LIB_DIR}/libtomcrypt.a)

    set(WX_SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wxWidgets/install-win32-shared)
    set(WX_LIB_DIR ${WX_SHARED_DIR}/bin)
    set_target_properties(wxBase PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/wxbase.dll
        IMPORTED_IMPLIB ${WX_LIB_DIR}/wxbase.dll)
    set_target_properties(wxNet PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/wxbase_net.dll
        IMPORTED_IMPLIB ${WX_LIB_DIR}/wxbase_net.dll)
    set_target_properties(wxCore PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/wxmsw_core.dll
        IMPORTED_IMPLIB ${WX_LIB_DIR}/wxmsw_core.dll)

    set(WX_STATIC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wxWidgets/install-win32-static)
    if (IS_DIRECTORY ${WX_STATIC_DIR}/lib64)
        set(WX_LIB_DIR ${WX_STATIC_DIR}/lib64)
    elseif(IS_DIRECTORY ${WX_STATIC_DIR}/lib)
        set(WX_LIB_DIR ${WX_STATIC_DIR}/lib)
    else()
        message(FATAL_ERROR "Missing lib dir")
    endif()

    set_target_properties(wxBase-static PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu-3.3-x86_64-w64-mingw32.a
        IMPORTED_IMPLIB ${WX_LIB_DIR}/libwx_baseu-3.3-x86_64-w64-mingw32.a)
    set_target_properties(wxNet-static PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu_net-3.3-x86_64-w64-mingw32.a
        IMPORTED_IMPLIB ${WX_LIB_DIR}/libwx_baseu_net-3.3-x86_64-w64-mingw32.a)
    set_target_properties(wxCore-static PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_mswu_core-3.3-x86_64-w64-mingw32.a
        IMPORTED_IMPLIB ${WX_LIB_DIR}/libwx_mswu_core-3.3-x86_64-w64-mingw32.a)
    set_target_properties(wxWidgets-PNG-static PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwxpng-3.3-x86_64-w64-mingw32.a
        IMPORTED_IMPLIB ${WX_LIB_DIR}/libwxpng-3.3-x86_64-w64-mingw32.a)
    set_target_properties(wxWidgets-ZLib-static PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwxzlib-3.3-x86_64-w64-mingw32.a
        IMPORTED_IMPLIB ${WX_LIB_DIR}/libwxzlib-3.3-x86_64-w64-mingw32.a)
endif()

# Use wx-config to get the proper compile and link flags
set(WX_SHARED_CONFIG "${WX_SHARED_DIR}/bin/wx-config")
set(WX_STATIC_CONFIG "${WX_STATIC_DIR}/bin/wx-config")

function(process_wx_cxxflags CONFIG TARGET)
    execute_process(
        COMMAND ${CONFIG} --cxxflags
        OUTPUT_VARIABLE WX_CXXFLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Extract and correctly place cxxflags
    string(REPLACE " " ";" WX_CXXFLAG_LIST "${WX_CXXFLAGS}")
    foreach(FLAG ${WX_CXXFLAG_LIST})
        string(FIND "${FLAG}" "-I" pos)
        if (pos EQUAL -1)
            target_compile_options(${TARGET} INTERFACE ${FLAG})
        else()
            string(SUBSTRING ${FLAG} 2 -1 FLAG)
            target_include_directories(${TARGET} INTERFACE ${FLAG})
        endif()
    endforeach()
endfunction()

function(process_wx_ldflags CONFIG TARGET)
    execute_process(
        COMMAND ${CONFIG} --libs
        OUTPUT_VARIABLE WX_LDFLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    string(REPLACE " " ";" WX_LDFLAG_LIST "${WX_LDFLAGS}")
    foreach(FLAG ${WX_LDFLAG_LIST})
        string(FIND "${FLAG}" "-framework" pos)
        if (NOT pos EQUAL -1)
            set(FOUND_FRAMEWORK true)
            continue()
        endif()

        # Ignore all -lwx links... CMake will handle those
        string(FIND "${FLAG}" "-lwx" pos1)
        if (NOT pos1 EQUAL -1)
            continue()
        endif()

        # Ignore all other libwx links... CMake will handle those
        string(FIND "${FLAG}" "libwx" pos1)
        if (NOT pos1 EQUAL -1)
            continue()
        endif()

        if (FOUND_FRAMEWORK)
            set(FLAG "-framework ${FLAG}")
            set(FOUND_FRAMEWORK false)
        endif()

        target_link_libraries(${TARGET} INTERFACE "${FLAG}")
    endforeach()
endfunction()

process_wx_cxxflags(${WX_SHARED_CONFIG} wxWidgets)
process_wx_cxxflags(${WX_STATIC_CONFIG} wxWidgets-static)

process_wx_ldflags(${WX_SHARED_CONFIG} wxWidgets)
process_wx_ldflags(${WX_STATIC_CONFIG} wxWidgets-static)

set(3RDPARTY_INSTALL_FILES
    $<TARGET_FILE:wxBase>
    $<TARGET_FILE:wxNet>
    $<TARGET_FILE:wxCore>
)

install(FILES ${3RDPARTY_INSTALL_FILES} DESTINATION ${INSTALL_LIB_DIR})

if (CMAKE_BUILD_TYPE STREQUAL Release)
    if (CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
        set(STRIP_COMMAND "strip -p --strip-all")
    elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Darwin)
        set(STRIP_COMMAND "strip -S -N -x -X")
    endif()
    install(CODE
        "
        foreach(file ${3RDPARTY_INSTALL_FILES})
            cmake_path(GET file FILENAME filename)
            execute_process(COMMAND ${STRIP_COMMAND} ${INSTALL_LIB_DIR}/\${filename})
        endforeach()
        "
    )
endif()
