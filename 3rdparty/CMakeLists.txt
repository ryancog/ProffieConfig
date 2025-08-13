# ProffieConfig, All-In-One Proffieboard Management Utility!
#
# 3rdparty/CMakeLists.txt

# add_library(tomcrypt SHARED IMPORTED GLOBAL)
# target_include_directories(tomcrypt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tomcrypt/src/headers)
add_library(tomcrypt-static STATIC IMPORTED GLOBAL)
target_include_directories(tomcrypt-static INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/tomcrypt/src/headers)

add_library(wxWidgetsBase       SHARED IMPORTED GLOBAL)
add_library(wxWidgetsBaseNet    SHARED IMPORTED GLOBAL)
add_library(wxWidgetsBaseXML    SHARED IMPORTED GLOBAL)
add_library(wxWidgetsCore       SHARED IMPORTED GLOBAL)
add_library(wxWidgetsXRC        SHARED IMPORTED GLOBAL)
add_library(wxWidgetsHTML       SHARED IMPORTED GLOBAL)
add_library(wxWidgetsQA         SHARED IMPORTED GLOBAL)

add_library(wxWidgets INTERFACE)
target_link_libraries(wxWidgets INTERFACE
    wxWidgetsBase
    wxWidgetsBaseNet
    wxWidgetsBaseXML
    wxWidgetsCore
    wxWidgetsXRC
    wxWidgetsHTML
    wxWidgetsQA
)

add_library(wxWidgetsBase-static       STATIC IMPORTED GLOBAL)
add_library(wxWidgetsBaseNet-static    STATIC IMPORTED GLOBAL)
add_library(wxWidgetsBaseXML-static    STATIC IMPORTED GLOBAL)
add_library(wxWidgetsCore-static       STATIC IMPORTED GLOBAL)
add_library(wxWidgetsXRC-static        STATIC IMPORTED GLOBAL)
add_library(wxWidgetsHTML-static       STATIC IMPORTED GLOBAL)
add_library(wxWidgetsQA-static         STATIC IMPORTED GLOBAL)
add_library(wxWidgetsPNG-static        STATIC IMPORTED GLOBAL)
add_library(wxWidgetsZLib-static       STATIC IMPORTED GLOBAL)

add_library(wxWidgets-static INTERFACE)
target_link_libraries(wxWidgets-static INTERFACE
    wxWidgetsBase-static
    wxWidgetsBaseNet-static
    wxWidgetsBaseXML-static
    wxWidgetsCore-static
    wxWidgetsXRC-static
    wxWidgetsHTML-static
    wxWidgetsQA-static
    wxWidgetsPNG-static
    wxWidgetsZLib-static
)

# add_library(swresample   SHARED IMPORTED GLOBAL)
# add_library(avutil       SHARED IMPORTED GLOBAL)
# add_library(avformat     SHARED IMPORTED GLOBAL)
# add_library(avcodec      SHARED IMPORTED GLOBAL)
#
# add_library(ffmpeg INTERFACE)
# target_link_libraries(ffmpeg INTERFACE
#     swresample
#     avutil
#     avformat
#     avcodec
# )

if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(TC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tomcrypt/build-linux/)
    # set_target_properties(tomcrypt PROPERTIES
    #     IMPORTED_LOCATION ${TC_LIB_DIR}/libtomcrypt.so)
    set_target_properties(tomcrypt-static PROPERTIES
        IMPORTED_LOCATION ${TC_LIB_DIR}/libtomcrypt.a)

    # set(FFMPEG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg/install-linux)
    # set(FFMPEG_LIB_DIR ${FFMPEG_DIR}/lib)
    # target_include_directories(ffmpeg INTERFACE ${FFMPEG_DIR}/include)
    # set_target_properties(swresample PROPERTIES
    #     IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libswresample.so.5)
    # set_target_properties(avutil PROPERTIES
    #     IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavutil.so.59)
    # set_target_properties(avformat PROPERTIES
    #     IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavformat.so.61)
    # set_target_properties(avcodec PROPERTIES
    #     IMPORTED_LOCATION ${FFMPEG_LIB_DIR}/libavcodec.so.61)

    set(WX_SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wxWidgets/install-linux-shared)
    set(WX_LIB_DIR ${WX_SHARED_DIR}/lib64)
    set_target_properties(wxWidgetsBase PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu-3.3.so.0)
	set_target_properties(wxWidgetsBaseNet PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu_net-3.3.so.0)
	set_target_properties(wxWidgetsBaseXML PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu_xml-3.3.so.0)
    set_target_properties(wxWidgetsCore PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_gtk3u_core-3.3.so.0)
	set_target_properties(wxWidgetsXRC PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_gtk3u_xrc-3.3.so.0)
	set_target_properties(wxWidgetsHTML PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_gtk3u_html-3.3.so.0)
	set_target_properties(wxWidgetsQA PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_gtk3u_qa-3.3.so.0)


    set(WX_STATIC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wxWidgets/install-linux-static)
    set(WX_LIB_DIR ${WX_STATIC_DIR}/lib64)
    set_target_properties(wxWidgetsBase-static PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu-3.3.a)
	set_target_properties(wxWidgetsBaseNet-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu_net-3.3.a)
	set_target_properties(wxWidgetsBaseXML-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu_xml-3.3.a)
    set_target_properties(wxWidgetsCore-static PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_gtk3u_core-3.3.a)
	set_target_properties(wxWidgetsXRC-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_gtk3u_xrc-3.3.a)
	set_target_properties(wxWidgetsHTML-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_gtk3u_html-3.3.a)
	set_target_properties(wxWidgetsQA-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_gtk3u_qa-3.3.a)

elseif (CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(TC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tomcrypt/build-macOS/)
    # set_target_properties(tomcrypt-shared PROPERTIES
    #    IMPORTED_LOCATION ${TC_LIB_DIR}/libtomcrypt.dylib)
    set_target_properties(tomcrypt-static PROPERTIES
        IMPORTED_LOCATION ${TC_LIB_DIR}/libtomcrypt.a)

    set(WX_SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wxWidgets/install-macOS-shared)
    set(WX_LIB_DIR ${WX_SHARED_DIR}/lib)
    set_target_properties(wxWidgetsBase PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu.dylib)
	set_target_properties(wxWidgetsBaseNet PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu_net.dylib)
	set_target_properties(wxWidgetsBaseXML PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu_xml.dylib)
    set_target_properties(wxWidgetsCore PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_osx_cocoau_core.dylib)
	set_target_properties(wxWidgetsXRC PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_osx_cocoau_xrc.dylib)
	set_target_properties(wxWidgetsHTML PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_osx_cocoau_html.dylib)
	set_target_properties(wxWidgetsQA PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_osx_cocoau_qa.dylib)

    set(WX_STATIC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wxWidgets/install-macOS-static)
    set(WX_LIB_DIR ${WX_STATIC_DIR}/lib)
    set_target_properties(wxWidgetsBase-static PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu-3.3.a)
	set_target_properties(wxWidgetsBaseNet-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu_net-3.3.a)
	set_target_properties(wxWidgetsBaseXML-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu_xml-3.3.a)
    set_target_properties(wxWidgetsCore-static PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_osx_cocoau_core-3.3.a)
	set_target_properties(wxWidgetsXRC-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_osx_cocoau_xrc-3.3.a)
	set_target_properties(wxWidgetsHTML-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_osx_cocoau_html-3.3.a)
	set_target_properties(wxWidgetsQA-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_osx_cocoau_qa-3.3.a)
    set_target_properties(wxWidgetsPNG-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwxpng-3.3.a)
    set_target_properties(wxWidgetsZLib-static PROPERTIES
		IMPORTED_LOCATION ${WX_LIB_DIR}/libwxzlib-3.3.a)

elseif (CMAKE_SYSTEM_NAME STREQUAL Windows)
    set(TC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tomcrypt/build-win32/)
    # set_target_properties(tomcrypt PROPERTIES
    #     IMPORTED_IMPLIB ${TC_LIB_DIR}/tomcrypt.dll)
    set_target_properties(tomcrypt-static PROPERTIES
        IMPORTED_LOCATION ${TC_LIB_DIR}/libtomcrypt.a
        IMPORTED_IMPLIB ${TC_LIB_DIR}/libtomcrypt.a)

    set(WX_SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wxWidgets/install-win32-shared)
    set(WX_LIB_DIR ${WX_SHARED_DIR}/bin)
    set_target_properties(wxWidgetsBase PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/wxbase330u_gcc_win32.dll
        IMPORTED_IMPLIB ${WX_LIB_DIR}/wxbase330u_gcc_win32.dll)
    set_target_properties(wxWidgetsBaseNet PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/wxbase330u_net_gcc_win32.dll
        IMPORTED_IMPLIB ${WX_LIB_DIR}/wxbase330u_net_gcc_win32.dll)
    set_target_properties(wxWidgetsBaseXML PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/wxbase330u_xml_gcc_win32.dll
        IMPORTED_IMPLIB ${WX_LIB_DIR}/wxbase330u_xml_gcc_win32.dll)
    set_target_properties(wxWidgetsCore PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/wxmsw330u_core_gcc_win32.dll
        IMPORTED_IMPLIB ${WX_LIB_DIR}/wxmsw330u_core_gcc_win32.dll)
    set_target_properties(wxWidgetsXRC PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/wxmsw330u_xrc_gcc_win32.dll
        IMPORTED_IMPLIB ${WX_LIB_DIR}/wxmsw330u_xrc_gcc_win32.dll)
    set_target_properties(wxWidgetsHTML PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/wxmsw330u_html_gcc_win32.dll
        IMPORTED_IMPLIB ${WX_LIB_DIR}/wxmsw330u_html_gcc_win32.dll)
    set_target_properties(wxWidgetsQA PROPERTIES
        IMPORTED_LOCATION ${WX_LIB_DIR}/wxmsw330u_qa_gcc_win32.dll
        IMPORTED_IMPLIB ${WX_LIB_DIR}/wxmsw330u_qa_gcc_win32.dll)

    set(WX_STATIC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wxWidgets/install-win32-static)
    if (CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
        set(WX_LIB_DIR ${WX_STATIC_DIR}/lib64)
        set_target_properties(wxWidgetsBase-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu-3.3-x86_64-w64-mingw32.static.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwx_baseu-3.3-x86_64-w64-mingw32.static.a)
        set_target_properties(wxWidgetsBaseNet-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu_net-3.3-x86_64-w64-mingw32.static.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwx_baseu_net-3.3-x86_64-w64-mingw32.static.a)
        set_target_properties(wxWidgetsBaseXML-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu_xml-3.3-x86_64-w64-mingw32.static.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwx_baseu_xml-3.3-x86_64-w64-mingw32.static.a)
        set_target_properties(wxWidgetsCore-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_mswu_core-3.3-x86_64-w64-mingw32.static.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwx_mswu_core-3.3-x86_64-w64-mingw32.static.a)
        set_target_properties(wxWidgetsXRC-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_mswu_xrc-3.3-x86_64-w64-mingw32.static.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwx_mswu_xrc-3.3-x86_64-w64-mingw32.static.a)
        set_target_properties(wxWidgetsHTML-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_mswu_html-3.3-x86_64-w64-mingw32.static.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwx_mswu_html-3.3-x86_64-w64-mingw32.static.a)
        set_target_properties(wxWidgetsQA-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_mswu_qa-3.3-x86_64-w64-mingw32.static.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwx_mswu_qa-3.3-x86_64-w64-mingw32.static.a)
        set_target_properties(wxWidgetsPNG-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwxpng-3.3-x86_64-w64-mingw32.static.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwxpng-3.3-x86_64-w64-mingw32.static.a)
        set_target_properties(wxWidgetsZLib-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwxzlib-3.3-x86_64-w64-mingw32.static.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwxzlib-3.3-x86_64-w64-mingw32.static.a)
    elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL Darwin)
        set(WX_LIB_DIR ${WX_STATIC_DIR}/lib)
        set_target_properties(wxWidgetsBase-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu-3.3-x86_64-w64-mingw32.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwx_baseu-3.3-x86_64-w64-mingw32.a)
        set_target_properties(wxWidgetsBaseNet-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu_net-3.3-x86_64-w64-mingw32.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwx_baseu_net-3.3-x86_64-w64-mingw32.a)
        set_target_properties(wxWidgetsBaseXML-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_baseu_xml-3.3-x86_64-w64-mingw32.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwx_baseu_xml-3.3-x86_64-w64-mingw32.a)
        set_target_properties(wxWidgetsCore-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_mswu_core-3.3-x86_64-w64-mingw32.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwx_mswu_core-3.3-x86_64-w64-mingw32.a)
        set_target_properties(wxWidgetsXRC-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_mswu_xrc-3.3-x86_64-w64-mingw32.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwx_mswu_xrc-3.3-x86_64-w64-mingw32.a)
        set_target_properties(wxWidgetsHTML-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_mswu_html-3.3-x86_64-w64-mingw32.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwx_mswu_html-3.3-x86_64-w64-mingw32.a)
        set_target_properties(wxWidgetsQA-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwx_mswu_qa-3.3-x86_64-w64-mingw32.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwx_mswu_qa-3.3-x86_64-w64-mingw32.a)
        set_target_properties(wxWidgetsPNG-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwxpng-3.3-x86_64-w64-mingw32.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwxpng-3.3-x86_64-w64-mingw32.a)
        set_target_properties(wxWidgetsZLib-static PROPERTIES
            IMPORTED_LOCATION ${WX_LIB_DIR}/libwxzlib-3.3-x86_64-w64-mingw32.a
            IMPORTED_IMPLIB ${WX_LIB_DIR}/libwxzlib-3.3-x86_64-w64-mingw32.a)
    endif()
endif()

# Use wx-config to get the proper compile and link flags
set(WX_SHARED_CONFIG "${WX_SHARED_DIR}/bin/wx-config")
set(WX_STATIC_CONFIG "${WX_STATIC_DIR}/bin/wx-config")

execute_process(
    COMMAND ${WX_SHARED_CONFIG} --cxxflags
    OUTPUT_VARIABLE WX_CXXFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Extract and correctly place cxxflags
string(REPLACE " " ";" WX_CXXFLAG_LIST "${WX_CXXFLAGS}")
foreach(FLAG ${WX_CXXFLAG_LIST})
    string(FIND "${FLAG}" "-I" pos)
    if (pos EQUAL -1)
        target_compile_options(wxWidgets INTERFACE ${FLAG})
    else()
        string(SUBSTRING ${FLAG} 2 -1 FLAG)
        target_include_directories(wxWidgets INTERFACE ${FLAG})
    endif()
endforeach()

execute_process(
    COMMAND ${WX_STATIC_CONFIG} --cxxflags
    OUTPUT_VARIABLE WX_CXXFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Extract and correctly place cxxflags
string(REPLACE " " ";" WX_CXXFLAG_LIST "${WX_CXXFLAGS}")
foreach(FLAG ${WX_CXXFLAG_LIST})
    string(FIND "${FLAG}" "-I" pos)
    if (pos EQUAL -1)
        target_compile_options(wxWidgets-static INTERFACE ${FLAG})
    else()
        string(SUBSTRING ${FLAG} 2 -1 FLAG)
        target_include_directories(wxWidgets-static INTERFACE ${FLAG})
    endif()
endforeach()

execute_process(
    COMMAND ${WX_SHARED_CONFIG} --libs
    OUTPUT_VARIABLE WX_LDFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Remove all -lwx links... CMake will handle those
string(REPLACE " " ";" WX_LDFLAG_LIST "${WX_LDFLAGS}")
foreach(FLAG ${WX_LDFLAG_LIST})
    string(FIND "${FLAG}" "-framework" pos)
    if (NOT pos EQUAL -1)
        set(FOUND_FRAMEWORK true)
        continue()
    endif()
    string(FIND "${FLAG}" "-lwx" pos)
    if (pos EQUAL -1)
        if (FOUND_FRAMEWORK)
            set(FLAG "-framework ${FLAG}")
            set(FOUND_FRAMEWORK false)
        endif()
        target_link_libraries(wxWidgets INTERFACE "${FLAG}")
    endif()
endforeach()

execute_process(
    COMMAND ${WX_STATIC_CONFIG} --libs
    OUTPUT_VARIABLE WX_LDFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Remove all -lwx links... CMake will handle those
string(REPLACE " " ";" WX_LDFLAG_LIST "${WX_LDFLAGS}")
foreach(FLAG ${WX_LDFLAG_LIST})
    string(FIND "${FLAG}" "-framework" pos)
    if (NOT pos EQUAL -1)
        set(FOUND_FRAMEWORK true)
        continue()
    endif()
    string(FIND "${FLAG}" "-lwx" pos)
    if (pos EQUAL -1)
        if (FOUND_FRAMEWORK)
            set(FLAG "-framework ${FLAG}")
            set(FOUND_FRAMEWORK false)
        endif()
        target_link_libraries(wxWidgets-static INTERFACE "${FLAG}")
    endif()
endforeach()

set(3RDPARTY_INSTALL_FILES
    # $<TARGET_FILE:tomcrypt>
    # $<TARGET_FILE:swresample>
    # $<TARGET_FILE:avutil>
    # $<TARGET_FILE:avformat>
    # $<TARGET_FILE:avcodec>
    $<TARGET_FILE:wxWidgetsBase>
    $<TARGET_FILE:wxWidgetsBaseNet>
    $<TARGET_FILE:wxWidgetsBaseXML>
    $<TARGET_FILE:wxWidgetsCore>
    $<TARGET_FILE:wxWidgetsXRC>
    $<TARGET_FILE:wxWidgetsHTML>
    $<TARGET_FILE:wxWidgetsQA>
)

install(FILES ${3RDPARTY_INSTALL_FILES} DESTINATION ${INSTALL_LIB_DIR})

if (CMAKE_BUILD_TYPE STREQUAL Release)
    if (CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
        set(STRIP_COMMAND "strip --strip-all")
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
