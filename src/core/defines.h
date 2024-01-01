// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include <wx/event.h>

#define PROFFIEOS_VERSION "7.13"
#define ARDUINO_CLI_VERSION "0.34.2"

#define COPYRIGHT_NOTICE \
  "ProffieConfig Copyright (C) 2024 Ryan Ogurek\n" \
  "\n" \
  "This program is free software: you can redistribute it and/or modify\n" \
  "it under the terms of the GNU General Public License as published by\n" \
  "the Free Software Foundation, either version 3 of the License, or\n" \
  "(at your option) any later version.\n" \
  "\n" \
  "This program is distributed in the hope that it will be useful,\n" \
  "but WITHOUT ANY WARRANTY; without even the implied warranty of\n" \
  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n" \
  "GNU General Public License for more details.\n" \
  "\n" \
  "You should have received a copy of the GNU General Public License\n" \
  "along with this program. If not, see https://www.gnu.org/licenses/."

#define FIRSTITEMFLAGS wxSizerFlags(0).Border(wxALL, 5)
#define MENUITEMFLAGS  wxSizerFlags(0).Border(wxLEFT | wxBOTTOM | wxRIGHT, 5)
#define BOXITEMFLAGS wxSizerFlags(0).Border(wxALL, 10).Expand()
#define TEXTITEMFLAGS wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 5)

#define UPDATEWINDOW(window) window->sizer->Layout();
#define FULLUPDATEWINDOW(window) window->sizer->Layout(); window->SetSizerAndFit(window->sizer); window->Refresh();

#define PR_DEFAULT "Default"
#define PR_SA22C "SA22C"
#define PR_FETT263 "Fett263"
#define PR_SHTOK "Shtok"
#define PR_BC "BC"
#define PR_CAIWYN "Caiwyn"

#include <wx/tooltip.h>
#define TIP(object, msg) object->SetToolTip(new wxToolTip(msg));

#define ARDUINOCORE_PBV1 "proffieboard:stm32l4:Proffieboard-L433CC"
#define ARDUINOCORE_PBV2 "proffieboard:stm32l4:ProffieboardV2-L433CC"
#define ARDUINOCORE_PBV3 "proffieboard:stm32l4:ProffieboardV3-L452RE"

#define ARDUINO_PBPLUGIN_VERSION "3.6" // Make sure the compile output parsing doesn't break if we update this!

#define SMALLBUTTONSIZE wxSize(30, 20)

#if defined(__WXMSW__)
#define RESOURCES_PATH "resources\\"
#define ARDUINO_PATH RESOURCES_PATH "arduino-cli\\arduino-cli.exe"
#define PROFFIEOS_INO PROFFIEOS_PATH "\\ProffieOS.ino"
#define CONFIG_DIR PROFFIEOS_PATH "\\config\\"
#define PROPCONFIG_DIR RESOURCES_PATH "props\\"
#define DRIVER_INSTALL popen("title ProffieConfig Worker && resources\\windowmode -title \"ProffieConfig Worker\" -mode force_minimized && resources\\proffie-dfu-setup.exe 2>&1", "r")
#define STYLEEDIT_PATH RESOURCES_PATH "StyleEditor\\style_editor.html"
#elif defined(__WXGTK__)
#define RESOURCES_PATH "resources/"
#define ARDUINO_PATH RESOURCES_PATH "arduino-cli/arduino-cli"
#define PROFFIEOS_INO PROFFIEOS_PATH "/ProffieOS.ino"
#define CONFIG_DIR PROFFIEOS_PATH "/config/"
#define PROPCONFIG_DIR RESOURCES_PATH "props/"
#define STYLEEDIT_PATH RESOURCES_PATH "StyleEditor/style_editor.html"
#define DRIVER_INSTALL popen("pkexec cp ~/.arduino15/packages/proffieboard/hardware/stm32l4/3.6/drivers/linux/*rules /etc/udev/rules.d", "r")
#elif defined(__WXOSX__)
#define RESOURCES_PATH "../Resources/"
#define ARDUINO_PATH RESOURCES_PATH "arduino-cli/arduino-cli"
#define PROFFIEOS_INO PROFFIEOS_PATH "/ProffieOS.ino"
#define CONFIG_DIR PROFFIEOS_PATH "/config/"
#define PROPCONFIG_DIR RESOURCES_PATH "props/"
#define DRIVER_INSTALL popen("", "r");
#define STYLEEDIT_PATH RESOURCES_PATH "StyleEditor/style_editor.html"
#endif

#define STATEFILE_PATH RESOURCES_PATH "state.pconf"
#define PROFFIEOS_PATH RESOURCES_PATH "ProffieOS"
