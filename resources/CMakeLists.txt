# ProffieConfig, All-In-One Proffieboard Management Utility!
#
# resources/CMakeLists.txt

install(FILES icons/icon-zoom.png RENAME icon.png DESTINATION ${INSTALL_RESOURCE_DIR}/icons)
install(FILES
    icons/edit.png
    icons/import.png
    icons/new.png
    icons/trash.png

    icons/settings.png
    icons/props.png
    icons/presets.png
    icons/blade.png

    # icons/reload.png
    # icons/led.png
    # icons/sound.png
    # icons/wiring.png

    icons/chip.png
    icons/serial_monitor.png
    icons/management.png

    DESTINATION ${INSTALL_RESOURCE_DIR}/icons
)

if (CMAKE_SYSTEM_NAME STREQUAL Darwin)
    install(PROGRAMS bin/macOS/arduino-cli DESTINATION ${INSTALL_BIN_DIR})
elseif (CMAKE_SYSTEM_NAME STREQUAL Linux)
    install(PROGRAMS bin/Linux/arduino-cli DESTINATION ${INSTALL_BIN_DIR})
elseif (CMAKE_SYSTEM_NAME STREQUAL Windows OR CROSS_COMPILE STREQUAL Windows)
    install(PROGRAMS 
        bin/Win32/arduino-cli.exe 
        bin/Win32/proffie-dfu-setup.exe

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

