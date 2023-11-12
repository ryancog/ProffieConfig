#pragma once
#include <wx/event.h>

#define VERSION "1.3.1"
#define ABOUT_MESSAGE \
  "Tool for GUI Configuration and flashing of Proffieboard\n" \
  "\n" \
  "Created by Ryryog25\n" \
  "https://github.com/ryryog25/ProffieConfig\n" \
  "\n" \
  "Version " VERSION "\n" \
  "ProffieOS v7.9 | Arduino Plugin v3.6.0 | Arduino CLI v0.34.2"

#define FIRSTITEMFLAGS wxSizerFlags(0).Border(wxALL, 5)
#define MENUITEMFLAGS  wxSizerFlags(0).Border(wxLEFT | wxBOTTOM | wxRIGHT, 5)
#define BOXITEMFLAGS wxSizerFlags(0).Border(wxALL, 10).Expand()

#define UPDATEWINDOW master->Layout(); SetSizerAndFit(master)

#define PR_DEFAULT "Default"
#define PR_SA22C "SA22C"
#define PR_FETT263 "Fett263"
#define PR_SHTOK "Shtok"
#define PR_BC "BC"
#define PR_CAIWYN "Caiwyn"

#define ARDUINOCORE_PBV1 "proffieboard:stm32l4:Proffieboard-L433CC"
#define ARDUINOCORE_PBV2 "proffieboard:stm32l4:ProffieboardV2-L433CC"
#define ARDUINOCORE_PBV3 "proffieboard:stm32l4:ProffieboardV3-L452RE"

#if defined(__WXMSW__)
#define ARDUINO_PATH "resources\\arduino-cli\\arduino-cli.exe"
#define PROFFIEOS_PATH "resources\\ProffieOS"
#define CONFIG_PATH PROFFIEOS_PATH "\\config\\ProffieConfig_autogen.h"
#define DRIVER_INSTALL popen("resources\\proffie-dfu-setup.exe 2>&1", "r")
#elif defined(__WXGTK__)
#define ARDUINO_PATH "resources/arduino-cli/arduino-cli"
#define PROFFIEOS_PATH "resources/ProffieOS"
#define CONFIG_PATH PROFFIEOS_PATH "/config/ProffieConfig_autogen.h"
#define DRIVER_INSTALL popen("pkexec cp ~/.arduino15/packages/proffieboard/hardware/stm32l4/3.6/drivers/linux/*rules /etc/udev/rules.d", "r")
#elif defined(__WXOSX__)
#define ARDUINO_PATH "ProffieConfig.app/Contents/Resources/arduino-cli/arduino-cli"
#define PROFFIEOS_PATH "ProffieConfig.app/Contents/Resources/ProffieOS"
#define CONFIG_PATH PROFFIEOS_PATH "/config/ProffieConfig_autogen.h"
#define DRIVER_INSTALL popen("", "r");
#endif
