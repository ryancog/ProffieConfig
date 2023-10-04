#include "arduino.h"
#include "mainwindow.h"
#include "configuration.h"

#include <string>
#include <vector>
#include <cstring>

#if defined(__WXGTK__)
#define ARDUINO_CLI(command) popen("resources/arduino-cli/arduino-cli " command, "r")
#define DRIVER_INSTALL popen("pkexec cp ~/.arduino15/packages/proffieboard/hardware/stm32l4/3.6/drivers/linux/*rules /etc/udev/rules.d", "r")
#elif defined(__WXMSW__)
#define ARDUINO_CLI(command) popen("start ../resources/win32/arduino-cli/arduino-cli.exe " command, "r")
#define DRIVER_INSTALL popen("", "r")
#elif defined(__WXOSX__)
#define ARDUINO_CLI(command) popen("../resources/macOS/arduino-cli/arduino-cli " command, "r");
#define DRIVER_INSTALL popen("", "r");
#endif
#define ARDUINO_CLOSE(fd) pclose(fd)

void Arduino::init() {
  MainWindow::instance->progDialog = new Progress(MainWindow::instance);

  new ThreadRunner([&]() -> void {
    FILE* install;
    char buffer[128];

#ifdef __WXGTK__
    wxMessageBox("You will be prompted for your password.", "Driver Install", wxOK);

    MainWindow::instance->progDialog->emitEvent(5, "Installing drivers...");
    install = DRIVER_INSTALL;
    while (fgets(buffer, 128, install) != nullptr) { MainWindow::instance->progDialog->emitEvent(-1, ""); }
    if (pclose(install)) {
      MainWindow::instance->progDialog->emitEvent(100, "Error");
      return;
    }
#endif

    MainWindow::instance->progDialog->emitEvent(40, "Downloading dependencies...");
    install = ARDUINO_CLI("core install proffieboard:stm32l4@3.6 --additional-urls https://profezzorn.github.io/arduino-proffieboard/package_proffieboard_index.json");
    while (fgets(buffer, 128, install) != nullptr) { MainWindow::instance->progDialog->emitEvent(-1, ""); }
    if (pclose(install)) {
      MainWindow::instance->progDialog->emitEvent(100, "Error");
      return;
    }
    MainWindow::instance->progDialog->emitEvent(100, "Done.");
    wxMessageBox("Please restart your computer to apply changes.", "Dependency Installation Success", wxOK | wxICON_INFORMATION);
  });
}

void Arduino::refreshBoards(MainWindow* win) {
  win->progDialog = new Progress(win);

  win->thread = new ThreadRunner([&]() {
    win->progDialog->SetTitle("Device Update");
    Progress::emitEvent(0, "Initializing...");
    wxString lastSel = MainWindow::devSelect->GetStringSelection();
    MainWindow::devSelect->Clear();
    Progress::emitEvent(20, "Fetching Devices...");
    for (const std::string& item : Arduino::getBoards()) {
      MainWindow::devSelect->Append(item);
    }
    MainWindow::devSelect->SetValue(lastSel);
    Progress::emitEvent(100, "Done.");
  });
}
std::vector<std::string> Arduino::getBoards() {
  std::vector<std::string> boards{"Select Device..."};
  char buffer[1024];

  FILE *arduinoCli = ARDUINO_CLI("board list");

  if (!arduinoCli) {
    return boards;
  }


  while (fgets(buffer, 1024, arduinoCli) != nullptr) {
    if (std::strstr(buffer, "No boards found.") != nullptr) {
      break;
    }

    if ((std::strstr(buffer, "serial") != NULL && std::strstr(buffer, "proffieboard") != NULL)) {
      *std::strpbrk(buffer, " ") = '\0'; // End string at break to get dev path
      boards.push_back(buffer);
    } else if (std::strstr(buffer, "dfu") != NULL) {
      *std::strpbrk(buffer, " ") = '\0';
      boards.push_back("BOOTLOADER|" + std::string(buffer));
    }
  }

  return boards;
}

void Arduino::applyToBoard(MainWindow* win) {
  win->progDialog = new Progress(win);

  win->thread = new ThreadRunner([&]() {
    std::string returnVal;
    win->progDialog->SetTitle("Applying Changes | DO NOT DISCONNECT BOARD");
    Progress::emitEvent(0, "Initializing...");

    Progress::emitEvent(10, "Checking board presence...");
    if (Arduino::getBoards()[MainWindow::devSelect->GetSelection()] != MainWindow::devSelect->GetStringSelection()) {
      Progress::emitEvent(100, "Error!");
      wxMessageBox("Please refresh boards and try again!", "Board Selection Error", wxOK | wxICON_ERROR);
      return;
    }

    Progress::emitEvent(20, "Generating configuration file...");
    Configuration::updateBladesConfig();
    Configuration::outputConfig();
    Arduino::updateIno();

    Progress::emitEvent(40, "Compiling ProffieOS...");
    returnVal = Arduino::compile();
    if (returnVal != "OK") {
      Progress::emitEvent(100, "Error");
      wxMessageBox("There was an error while compiling, "
                   "if it doesn't make sense to you, please report it, "
                   "I probably broke something:\n\n"
                       + returnVal, "Compile Error");
      return;
    }

    Progress::emitEvent(65, "Uploading to ProffieBoard...");
    returnVal = Arduino::upload();
    if (returnVal != "OK") {
      Progress::emitEvent(100, "Error");
      wxMessageBox("There was an error while uploading, please report it:\n" + returnVal);
      return;
    }

    Progress::emitEvent(100, "Done.");
    wxMessageBox("Changes Successfully Applied to ProffieBoard!", "Apply Changes to Board", wxOK | wxICON_INFORMATION);
  });
}
std::string Arduino::compile() {
  std::string output;
  char buffer[1024];
  FILE* arduinoCli = ARDUINO_CLI("compile -b proffieboard:stm32l4:ProffieboardV2-L433CC resources/ProffieOS --log");

  while(fgets(buffer, 1024, arduinoCli) != NULL) {
    MainWindow::instance->progDialog->emitEvent(-1, ""); // Pulse
    if (std::strstr(buffer, "error")) {
      return buffer;
    }
  }
  if (ARDUINO_CLOSE(arduinoCli) != 0) {
    return "Unkown Compile Error";
  }

  return "OK";
}
std::string Arduino::upload() {
  char buffer[1024];
  FILE* arduinoCli = ARDUINO_CLI("upload resources/ProffieOS -b proffieboard:stm32l4:ProffieboardV2-L433CC --board-options usb=cdc_msc -p /dev/ttyACM0 --log");

  while(fgets(buffer, 1024, arduinoCli) != NULL) {
    MainWindow::instance->progDialog->emitEvent(-1, ""); // Pulse
    if (std::strstr(buffer, "error")) {
      return buffer;
    }
  }
  if (ARDUINO_CLOSE(arduinoCli) != 0) {
    return "Unknown Upload Error";
  }


  return "OK";
}
void Arduino::updateIno() {
  std::ifstream input("resources/ProffieOS/ProffieOS.ino");
  std::string fileData;
  std::vector<std::string> outputData;
  while(!input.eof()) {
    std::getline(input, fileData);
    outputData.push_back(fileData == "// #define CONFIG_FILE \"config/YOUR_CONFIG_FILE_NAME_HERE.h\"" ? "#define CONFIG_FILE \"config/ProffieConfig_autogen.h\"" : fileData);
  }
  input.close();
  std::ofstream output("resources/ProffieOS/ProffieOS.ino");
  for (const std::string& line : outputData) {
    output << line << std::endl;
  }
  output.close();
}
