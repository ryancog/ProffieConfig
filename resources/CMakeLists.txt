# ProffieConfig, All-In-One Proffieboard Management Utility!
#
# resources/CMakeLists.txt

install(FILES
    # icons/blade.png
    # icons/chip.png
    # icons/edit.png
    icons/icon.png
    # icons/import.png
    # icons/led.png
    # icons/new.png
    # icons/reload.png
    # icons/serial_monitor.png
    # icons/sound.png
    # icons/settings.png
    # icons/wiring.png
    # icons/presets.png
    # icons/props.png

    DESTINATION ${INSTALL_RESOURCE_DIR}/icons
)

install(FILES
    props/BC.pconf
    props/caiwyn.pconf
    props/fett263.pconf
    props/sa22c.pconf
    props/shtok.pconf

    DESTINATION ${INSTALL_RESOURCE_DIR}/props
)

if (CMAKE_SYSTEM_NAME STREQUAL Darwin)
    install(PROGRAMS bin/macOS/arduino-cli DESTINATION ${INSTALL_BIN_DIR})
elseif (CMAKE_SYSTEM_NAME STREQUAL Linux)
    install(PROGRAMS bin/Linux/arduino-cli DESTINATION ${INSTALL_BIN_DIR})
elseif (CMAKE_SYSTEM_NAME STREQUAL Windows)
    install(PROGRAMS 
        bin/Win32/arduino-cli.exe 
        bin/Win32/proffie-dfu-setup.exe
        bin/Win32/windowMode.exe

        DESTINATION ${INSTALL_BIN_DIR}
    )
endif()

# install(FILES
#     component-icons/proffieboardv1.png
#     component-icons/proffieboardv2.png
#     component-icons/proffieboardv3.png
#     component-icons/pixel.png
#     component-icons/led.png
#     component-icons/resistor.png
#     component-icons/rfid.png
# 
#     DESTINATION ${INSTALL_RESOURCE_DIR}/components
# )

