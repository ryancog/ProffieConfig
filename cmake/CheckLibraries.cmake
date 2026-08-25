
file(READ "${CMAKE_BINARY_DIR}/install_manifest.txt" INSTALLED_FILES)
string(REGEX MATCHALL "[^\n]+" INSTALLED_FILES ${INSTALLED_FILES})

# Make sure I'm not pulling in Homebrew or MacPorts dependencies...
foreach (FILE ${INSTALLED_FILES})
    execute_process(
        COMMAND otool -L ${FILE}
        OUTPUT_VARIABLE OTOOL_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Check both executables and libraries, skip non-objects
    string(FIND "${OTOOL_OUTPUT}" "is not an object file" INDEX)
    if (INDEX LESS 0)
        continue()
    endif()

    string(REGEX MATCHALL "/opt/local[^\n]+" BAD_DEPS ${OTOOL_OUTPUT})
    if (BAD_DEPS)
        string(JOIN "\n " BAD_DEPS ${BAD_DEPS})
        message(FATAL_ERROR
            " Bad Dependencies in ${LIBRARY}\n"
            " ${BAD_DEPS}"
        )
    endif()
endforeach()

