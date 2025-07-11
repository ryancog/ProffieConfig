include(GenerateExportHeader)
include(${CMAKE_CURRENT_LIST_DIR}/../Common.cmake)

function(setup_component TARGET VERSION)
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../) # Include components/

    set_target_properties(${TARGET} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
        POSITION_INDEPENDENT_CODE ON
        BIN_VERSION ${VERSION}
    )

    generate_export_header(${TARGET}
        EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/private/export.h
    )

    setup_target(${TARGET})
endfunction()

function(setup_component_and_static TARGET VERSION)
    setup_component(${TARGET} ${VERSION})

    get_target_property(SOURCES ${TARGET} SOURCES)
    get_target_property(LINK_LIBRARIES ${TARGET} LINK_LIBRARIES)
    add_library(${TARGET}-static STATIC ${SOURCES})
    foreach(LIB IN LISTS LINK_LIBRARIES)
        string(REGEX MATCH "-static$" ALREADY_STATIC ${LIB})
        if (ALREADY_STATIC)
            target_link_libraries(${TARGET}-static ${LIB})
        else()
            target_link_libraries(${TARGET}-static ${LIB}-static)
        endif()
    endforeach()

    set_target_properties(${TARGET}-static PROPERTIES BIN_VERSION ${VERSION}})
    target_include_directories(${TARGET}-static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../) # Include components/
    string(TOUPPER ${TARGET} TARGET_UPPER)
    target_compile_definitions(${TARGET}-static PUBLIC -D${TARGET_UPPER}_STATIC_DEFINE)

    setup_target(${TARGET}-static)
endfunction()

