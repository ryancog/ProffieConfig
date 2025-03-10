include(GenerateExportHeader)
include(${CMAKE_CURRENT_LIST_DIR}/../Common.cmake)

function(setup_component TARGET)
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../) # Include components/

    set_target_properties(${TARGET} PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
        POSITION_INDEPENDENT_CODE ON
    )

    generate_export_header(${TARGET}
        EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/private/export.h
    )

    setup_target(${TARGET})
endfunction()

function(setup_component_and_static TARGET)
    setup_component(${TARGET})

    target_include_directories(${TARGET}-static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../) # Include components/
    string(TOUPPER ${TARGET} TARGET_UPPER)
    target_compile_definitions(${TARGET}-static PUBLIC -D${TARGET_UPPER}_STATIC_DEFINE)

    setup_target(${TARGET}-static)
endfunction()

