#include "arduino.h"
#include "mainwindow.h"

#include <string>
#include <vector>
#include <cstring>

#if defined(__WXGTK__)
#define ARDUINO_CLI(command) popen("resources/arduino-cli/arduino-cli " command, "r")
#define DRIVER_INSTALL popen("pkexec cp ~/.arduino15/packages/proffieboard/hardware/stm32l4/3.6/drivers/linux/*rules /etc/udev/rules.d", "r")
#elif defined(__WXMSW__)
#define ARDUINO_CLI(command) popen("../resources/win32/arduino-cli/arduino-cli.exe " command, "r")
#define DRIVER_INSTALL popen("", "r")
#elif defined(__WXOSX__)
#define ARDUINO_CLI(command) "../resources/macOS/arduino-cli/arduino-cli " #command
#endif

void Arduino::init() {
  MainWindow::instance->progDialog = new Progress(MainWindow::instance);

  new ThreadRunner([&]() -> void {
    FILE* install;
    char buffer[128];

#ifndef __WXOSX__
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
    wxMessageBox("Please restart to apply changes.", "Dependency Installation Success", wxOK | wxICON_INFORMATION);
  });
}

std::vector<std::string> Arduino::getBoards() {
  std::vector<std::string> boards{"Select Device..."};
  char buffer[128];

  FILE *arduinoCli = ARDUINO_CLI("board list");

  if (!arduinoCli) {
    return boards;
  }


  while (fgets(buffer, 128, arduinoCli) != nullptr) {
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

std::string Arduino::compile() {
  std::string output;
  char buffer[128];
  FILE* arduinoCli = ARDUINO_CLI("compile -b proffieboard:stm32l4:ProffieboardV2-L433CC resources/ProffieOS --log");

  while(fgets(buffer, 128, arduinoCli) != NULL) {
    MainWindow::instance->progDialog->emitEvent(-1, ""); // Pulse
    // Do something
  }

  return "OK";
}

std::string Arduino::upload() {
  std::string output;
  char buffer[128];
  FILE* arduinoCli = ARDUINO_CLI("upload resources/ProffieOS -b proffieboard:stm32l4:ProffieboardV2-L433CC --board-options usb=cdc_msc -p /dev/ttyACM0 --log");

  while(fgets(buffer, 128, arduinoCli) != NULL) {
    MainWindow::instance->progDialog->emitEvent(-1, ""); // Pulse
    // Do Something
  }
  return "OK";
}
