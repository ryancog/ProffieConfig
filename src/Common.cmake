# ProffieConfig, The All-In-One Proffieboard Management Utility!

# src/Common.cmake

function(setup_target TARGET)
    if (CMAKE_SYSTEM_NAME STREQUAL Linux)
        set(RPATH "$ORIGIN/../lib:$ORIGIN/../components")
    elseif (CMAKE_SYSTEM_NAME STREQUAL Darwin)
        if (CMAKE_BUILD_TYPE STREQUAL Debug)
            set(RPATH "@executable_path/../lib\;@executable_path/../components")
        elseif (CMAKE_BUILD_TYPE STREQUAL Release)
            set(RPATH "@executable_path/../../../../lib\;@executable_path/../../../../components")
        endif()

        get_target_property(EXEC_VERSION ${TARGET} VERSION)
        set_target_properties(${TARGET} PROPERTIES 
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION}"
        )
    elseif (CMAKE_SYSTEM_NAME STREQUAL Windows)
        set(RPATH "ThisMeansNothing")
        get_target_property(DESCRIPTION ${TARGET} DESCRIPTION)
        if (${DESCRIPTION} STREQUAL "DESCRIPTION-NOTFOUND")
            set(DESCRIPTION "$<TARGET_FILE_BASE_NAME:${TARGET}> library for ProffieConfig")
        endif()
        get_target_property(TRANSLATIONS ${TARGET} TRANSLATION_LIST)
        foreach(TRANSLATION TRANSLATION_LIST)
            string(APPEND TRANSLATIONS "${TARGET}_${TRANSLATION} MOFILE \"${CMAKE_CURRENT_LIST_DIR}/i18n/${TRANSLATION}/${TARGET}.mo\"")
        endforeach()
        configure_file(${CMAKE_SOURCE_DIR}/resources/templates/resource.rc.in ${CMAKE_CURRENT_BINARY_DIR}/resource.rc.gen)
        file(GENERATE
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.rc
            INPUT ${CMAKE_CURRENT_BINARY_DIR}/resource.rc.gen
        )
        target_include_directories(${TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/wxWidgets/include)
        target_sources(${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.rc)
    endif()
    set_target_properties(${TARGET} PROPERTIES
        INSTALL_RPATH ${RPATH}
        BUILD_WITH_INSTALL_RPATH true
    )

    get_target_property(VERSION ${TARGET} EXEC_VERSION)
    target_compile_definitions(${TARGET} PRIVATE EXEC_VERSION=${VERSION})
endfunction()
