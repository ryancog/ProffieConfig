# ProffieConfig, The All-In-One Proffieboard Management Utility!

# src/Common.cmake

function(setup_target TARGET)
    get_target_property(BIN_VERSION ${TARGET} BIN_VERSION)
    if (BIN_VERSION STREQUAL "BIN_VERSION-NOTFOUND")
        message(FATAL_ERROR "No version set for ${TARGET}")
    endif()

    string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+).*" _ ${BIN_VERSION})
    set_target_properties(${TARGET} PROPERTIES
        BIN_VERSION_MAJOR ${CMAKE_MATCH_1}
        BIN_VERSION_MINOR ${CMAKE_MATCH_2}
        BIN_VERSION_BUGFIX ${CMAKE_MATCH_3}
    )

    if (CMAKE_SYSTEM_NAME STREQUAL Linux)
        if (${TARGET} STREQUAL test)
            set(RPATH "${INSTALL_ITEM_DIR}/lib:${INSTALL_ITEM_DIR}/components")
        else()
            set(RPATH "$ORIGIN/../lib:$ORIGIN/../components")
        endif()
    elseif (CMAKE_SYSTEM_NAME STREQUAL Darwin)
        if (${TARGET} STREQUAL test)
            set(RPATH "${INSTALL_ITEM_DIR}/lib\;${INSTALL_ITEM_DIR}/components")
        elseif (CMAKE_BUILD_TYPE STREQUAL Debug)
            set(RPATH "@executable_path/../lib\;@executable_path/../components")
        elseif (CMAKE_BUILD_TYPE STREQUAL Release)
            set(RPATH "@executable_path/../../../../lib\;@executable_path/../../../../components")
        endif()

        get_target_property(IS_BUNDLE ${TARGET} MACOSX_BUNDLE)
        if (IS_BUNDLE)
            string(TIMESTAMP CURRENT_YEAR "%Y")
            get_target_property(EXECUTABLE_NAME ${TARGET} OUTPUT_NAME)
            get_target_property(BUNDLE_IDENTIFIER ${TARGET} MACOSX_BUNDLE_GUI_IDENTIFIER)
            configure_file(${CMAKE_SOURCE_DIR}/resources/templates/Info.plist.in ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
            set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
        endif()
    elseif (CMAKE_SYSTEM_NAME STREQUAL Windows)
        set(RPATH "ThisMeansNothing")

        get_target_property(DESCRIPTION ${TARGET} DESCRIPTION)
        if (${DESCRIPTION} STREQUAL "DESCRIPTION-NOTFOUND")
            set_target_properties(${TARGET} PROPERTIES
                DESCRIPTION "$<TARGET_FILE_BASE_NAME:${TARGET}> library for ProffieConfig"
            )
        endif()

        get_target_property(TRANSLATIONS ${TARGET} TRANSLATION_LIST)
        set(TRANSLATIONS)
        foreach(TRANSLATION ${TRANSLATION_LIST})
            string(APPEND TRANSLATIONS "${TARGET}_${TRANSLATION} MOFILE \"${CMAKE_CURRENT_LIST_DIR}/i18n/${TRANSLATION}/${TARGET}.mo\"")
        endforeach()
        set_target_properties(${TARGET} PROPERTIES
            TRANSLATIONS "${TRANSLATIONS}"
        )

        configure_file(${CMAKE_SOURCE_DIR}/resources/templates/resource.rc.in ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.rc.gen)
        file(GENERATE
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.rc
            INPUT ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.rc.gen
        )
        target_include_directories(${TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/wxWidgets/include)
        target_sources(${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.rc)
    endif()
    set_target_properties(${TARGET} PROPERTIES
        INSTALL_RPATH ${RPATH}
        BUILD_WITH_INSTALL_RPATH true
    )

    target_compile_definitions(${TARGET} PRIVATE BIN_VERSION=${BIN_VERSION} VERSION=${PROJECT_VERSION})
endfunction()
