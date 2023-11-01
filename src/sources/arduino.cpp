#include "arduino.h"
#include "mainwindow.h"
#include "configuration.h"
#include "defines.h"

#include <string>
#include <vector>
#include <cstring>

void Arduino::init() {
  MainWindow::instance->progDialog = new Progress(MainWindow::instance);

  new ThreadRunner([&]() -> void {
    FILE* install;
    char buffer[128];

#ifndef __WXOSX__
#ifdef __WXGTK__
    wxMessageBox("You will be prompted for your password.", "Driver Install", wxOK);
#endif

    MainWindow::instance->progDialog->emitEvent(5, "Installing drivers...");
    install = DRIVER_INSTALL;
    while (fgets(buffer, 128, install) != nullptr) { MainWindow::instance->progDialog->emitEvent(-1, ""); }
    if (pclose(install)) {
      MainWindow::instance->progDialog->emitEvent(100, "Error");
      return;
    }
#endif

    MainWindow::instance->progDialog->emitEvent(40, "Downloading dependencies...");
    install = Arduino::CLI("core install proffieboard:stm32l4@3.6 --additional-urls https://profezzorn.github.io/arduino-proffieboard/package_proffieboard_index.json");
    while (fgets(buffer, 128, install) != nullptr) { MainWindow::instance->progDialog->emitEvent(-1, ""); }
    if (pclose(install)) {
      MainWindow::instance->progDialog->emitEvent(100, "Error");
      return;
    }
    MainWindow::instance->progDialog->emitEvent(100, "Done.");

#ifdef __WXGTK__
    wxMessageBox("Please restart your computer to apply changes.", "Dependency Installation Success", wxOK | wxICON_INFORMATION);
#endif
  });
}

void Arduino::refreshBoards() {
  MainWindow::instance->progDialog = new Progress(MainWindow::instance);
  MainWindow::instance->progDialog->SetTitle("Device Update");

  MainWindow::instance->thread = new ThreadRunner([&]() {
    Progress::emitEvent(0, "Initializing...");
    wxString lastSel = MainWindow::instance->devSelect->GetStringSelection();
    MainWindow::instance->devSelect->Clear();
    Progress::emitEvent(20, "Fetching Devices...");
    for (const std::string& item : Arduino::getBoards()) {
      MainWindow::instance->devSelect->Append(item);
    }
    MainWindow::instance->devSelect->ChangeValue(lastSel);
    Progress::emitEvent(100, "Done.");
    Progress::emitEvent(100, "Done."); // This has to be called twice to update on macOS?
  });
}
std::vector<std::string> Arduino::getBoards() {
  std::vector<std::string> boards{"Select Device..."};
  char buffer[1024];

  FILE *arduinoCli = Arduino::CLI("board list");

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

void Arduino::applyToBoard() {
  MainWindow::instance->progDialog = new Progress(MainWindow::instance);
  MainWindow::instance->progDialog->SetTitle("Applying Changes");

  MainWindow::instance->thread = new ThreadRunner([&]() {
    std::string returnVal;
    Progress::emitEvent(0, "Initializing...");

    Progress::emitEvent(10, "Checking board presence...");
    if (Arduino::getBoards()[MainWindow::instance->devSelect->GetSelection()] != MainWindow::instance->devSelect->GetStringSelection()) {
      Progress::emitEvent(100, "Error!");
      wxMessageBox("Please refresh boards and try again!", "Board Selection Error", wxOK | wxICON_ERROR);
      return;
    }

    Progress::emitEvent(20, "Generating configuration file...");
    Configuration::instance->updateBladesConfig();
    Configuration::instance->outputConfig();
    Arduino::updateIno();

    Progress::emitEvent(40, "Compiling ProffieOS...");
    returnVal = Arduino::compile();
    if (returnVal != "OK") {
      Progress::emitEvent(100, "Error");
      wxMessageBox("There was an error while compiling:\n\n"
                       + returnVal, "Compile Error");
      return;
    }

    Progress::emitEvent(65, "Uploading to ProffieBoard...");
    returnVal = Arduino::upload();
    if (returnVal != "OK") {
      Progress::emitEvent(100, "Error");
      wxMessageBox("There was an error while uploading:\n\n" + returnVal);
      return;
    }

    Progress::emitEvent(100, "Done.");
    Progress::emitEvent(100, "Done.");

    wxMessageBox("Changes Successfully Applied to ProffieBoard!", "Apply Changes to Board", wxOK | wxICON_INFORMATION);
  });
}
void Arduino::verifyConfig() {
  MainWindow::instance->progDialog = new Progress(MainWindow::instance);
  MainWindow::instance->progDialog->SetTitle("Verify Config");

  MainWindow::instance->thread = new ThreadRunner([&]() {
    std::string returnVal;
    Progress::emitEvent(20, "Generating configuration file...");
    Configuration::instance->updateBladesConfig();
    Configuration::instance->outputConfig();
    Arduino::updateIno();

    Progress::emitEvent(40, "Compiling ProffieOS...");
    returnVal = Arduino::compile();
    if (returnVal != "OK") {
      Progress::emitEvent(100, "Error");
      wxMessageBox("There was an error while compiling:\n\n"
                       + returnVal, "Compile Error");
      return;
    }

    Progress::emitEvent(100, "Done.");
    wxMessageBox("Config Verified Successfully!", "Verify Config", wxOK | wxICON_INFORMATION);
  });
}

std::string Arduino::compile() {
  std::string output;
  char buffer[1024];

  std::string compileCommand = "compile ";
  compileCommand += "-b ";
  compileCommand += GeneralPage::instance->settings.board->GetSelection() == 0 ? ARDUINOCORE_PBV1 : GeneralPage::instance->settings.board->GetSelection() == 1 ? ARDUINOCORE_PBV2 : ARDUINOCORE_PBV3;
  compileCommand += " --board-options ";
  if (GeneralPage::instance->settings.massStorage->GetValue() && GeneralPage::instance->settings.webUSB->GetValue()) compileCommand += "usb=cdc_msc_webusb";
  else if (GeneralPage::instance->settings.webUSB->GetValue()) compileCommand += "usb=cdc_webusb";
  else if (GeneralPage::instance->settings.massStorage->GetValue()) compileCommand += "usb=cdc_msc";
  else compileCommand += "usb=cdc";
  if (GeneralPage::instance->settings.board->GetStringSelection() == "ProffieBoard V3") compileCommand +=",dosfs=sdmmc1";
  compileCommand += " " PROFFIEOS_PATH;
  FILE *arduinoCli = Arduino::CLI(compileCommand);

  while(fgets(buffer, 1024, arduinoCli) != NULL) {
    MainWindow::instance->progDialog->emitEvent(-1, ""); // Pulse
    if (std::strstr(buffer, "error")) {
      return Arduino::parseError(buffer);
    }
  }
  if (pclose(arduinoCli) != 0) {
    return "Unkown Compile Error";
  }

  return "OK";
}
std::string Arduino::parseError(const std::string& error) {
#define ERRCONTAINS(token) std::strstr(error.data(), token)
  if (ERRCONTAINS("select Proffieboard")) return "Please ensure you've selected the correct board in General";
  if (ERRCONTAINS("expected unqualified-id")) return "Please make sure there are no brackets in your styles!\nSuch as { or }";
  if (ERRCONTAINS("FLASH")) return "The specified config will not fit on Proffieboard.\n\nTry disabling diagnostic commands, disabling talkie, disabling prop features, or removing blade styles to make it fit.";
  else return ERRCONTAINS("error:");
#undef ERRCONTAINS
}
std::string Arduino::upload() {
  char buffer[1024];

  std::string uploadCommand = "upload ";
  uploadCommand += PROFFIEOS_PATH;
  uploadCommand += " --board-options ";
  if (GeneralPage::instance->settings.massStorage->GetValue() && GeneralPage::instance->settings.webUSB->GetValue()) uploadCommand += "usb=cdc_msc_webusb";
  else if (GeneralPage::instance->settings.webUSB->GetValue()) uploadCommand += "usb=cdc_webusb";
  else if (GeneralPage::instance->settings.massStorage->GetValue()) uploadCommand += "usb=cdc_msc";
  else uploadCommand += "usb=cdc";
  if (GeneralPage::instance->settings.board->GetStringSelection() == "ProffieBoard V3") uploadCommand +=",dosfs=sdmmc1";

  uploadCommand += " --fqbn ";
  uploadCommand += GeneralPage::instance->settings.board->GetSelection() == 0 ? ARDUINOCORE_PBV1 : GeneralPage::instance->settings.board->GetSelection() == 1 ? ARDUINOCORE_PBV2 : ARDUINOCORE_PBV3;
  uploadCommand += " --port ";
  uploadCommand += MainWindow::instance->devSelect->GetStringSelection();

  FILE *arduinoCli = Arduino::CLI(uploadCommand);

  while(fgets(buffer, 1024, arduinoCli) != NULL) {
    MainWindow::instance->progDialog->emitEvent(-1, ""); // Pulse
    if (std::strstr(buffer, "error")) {
      return buffer;
    }
  }
  if (pclose(arduinoCli) != 0) {
    return "Unknown Upload Error";
  }

  return "OK";
}
void Arduino::updateIno() {
  std::ifstream input(PROFFIEOS_PATH "/ProffieOS.ino");
  std::string fileData;
  std::vector<std::string> outputData;
  while(!input.eof()) {
    std::getline(input, fileData);
    outputData.push_back(fileData == "// #define CONFIG_FILE \"config/YOUR_CONFIG_FILE_NAME_HERE.h\"" ? "#define CONFIG_FILE \"config/ProffieConfig_autogen.h\"" : fileData);
  }
  input.close();
  std::ofstream output(PROFFIEOS_PATH "/ProffieOS.ino");
  for (const std::string& line : outputData) {
    output << line << std::endl;
  }
  output.close();
}

FILE* Arduino::CLI(const std::string& command) {
  std::string fullCommand = ARDUINO_PATH " ";
  fullCommand += command;
  fullCommand += " 2>&1";
  return popen(fullCommand.data(), "r");
}
