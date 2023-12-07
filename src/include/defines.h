#pragma once
#include <wx/event.h>

#define ABOUT_MESSAGE \
  "Tool for GUI Configuration and flashing of Proffieboard\n" \
  "\n" \
  "Created by Ryryog25\n" \
  "https://github.com/ryryog25/ProffieConfig\n" \
  "\n" \
  "Version " VERSION "\n" \
  "ProffieOS v7.13 | Arduino CLI v0.34.2"

#define FIRSTITEMFLAGS wxSizerFlags(0).Border(wxALL, 5)
#define MENUITEMFLAGS  wxSizerFlags(0).Border(wxLEFT | wxBOTTOM | wxRIGHT, 5)
#define BOXITEMFLAGS wxSizerFlags(0).Border(wxALL, 10).Expand()
#define TEXTITEMFLAGS wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 5)

#define UPDATEWINDOW MainWindow::instance->master->Layout();
#define FULLUPDATEWINDOW MainWindow::instance->master->Layout(); MainWindow::instance->SetSizerAndFit(MainWindow::instance->master);

#define PR_DEFAULT "Default"
#define PR_SA22C "SA22C"
#define PR_FETT263 "Fett263"
#define PR_SHTOK "Shtok"
#define PR_BC "BC"
#define PR_CAIWYN "Caiwyn"

#define TIP(object, msg) object->SetToolTip(new wxToolTip(msg));

#define ARDUINOCORE_PBV1 "proffieboard:stm32l4:Proffieboard-L433CC"
#define ARDUINOCORE_PBV2 "proffieboard:stm32l4:ProffieboardV2-L433CC"
#define ARDUINOCORE_PBV3 "proffieboard:stm32l4:ProffieboardV3-L452RE"

#define SMALLBUTTONSIZE wxSize(30, 20)

#if defined(__WXMSW__)
#define ARDUINO_PATH "resources\\arduino-cli\\arduino-cli.exe"
#define PROFFIEOS_PATH "resources\\ProffieOS"
#define CONFIG_PATH PROFFIEOS_PATH "\\config\\ProffieConfig_autogen.h"
#define DRIVER_INSTALL popen("resources\\proffie-dfu-setup.exe 2>&1", "r")
#define STYLEEDIT_PATH "resources\\StyleEditor\\style_editor.html"
#elif defined(__WXGTK__)
#define ARDUINO_PATH "resources/arduino-cli/arduino-cli"
#define PROFFIEOS_PATH "resources/ProffieOS"
#define CONFIG_PATH PROFFIEOS_PATH "/config/ProffieConfig_autogen.h"
#define DRIVER_INSTALL popen("pkexec cp ~/.arduino15/packages/proffieboard/hardware/stm32l4/3.6/drivers/linux/*rules /etc/udev/rules.d", "r")
#define STYLEEDIT_PATH "resources/StyleEditor/style_editor.html"
#elif defined(__WXOSX__)
#define ARDUINO_PATH "../Resources/arduino-cli/arduino-cli"
#define PROFFIEOS_PATH "../Resources/ProffieOS"
#define CONFIG_PATH PROFFIEOS_PATH "/config/ProffieConfig_autogen.h"
#define DRIVER_INSTALL popen("", "r");
#define STYLEEDIT_PATH "../Resources/StyleEditor/style_editor.html"
#endif
