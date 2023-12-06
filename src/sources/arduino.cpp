#include "arduino.h"

#include "generalpage.h"
#include "mainwindow.h"
#include "configuration.h"
#include "defines.h"

#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <cstring>

#include <wx/wfstream.h>

void Arduino::init() {
  MainWindow::instance->progDialog = new Progress(MainWindow::instance);

  new ThreadRunner([&]() -> void {
    FILE* install;
    char buffer[128];

#ifndef __WXOSX__
#ifdef __WXGTK__
    Misc::MessageBoxEvent* passwordMessage = new Misc::MessageBoxEvent(Misc::EVT_MSGBOX, wxID_ANY, "You will be prompted for your password.", "Driver Install", wxOK);
    wxQueueEvent(MainWindow::instance->GetEventHandler(), passwordMessage);
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
    Misc::MessageBoxEvent* restartMessage = new Misc::MessageBoxEvent(Misc::EVT_MSGBOX, wxID_ANY, "Please restart your computer to apply changes.", "Dependency Installation Success", wxOK | wxICON_INFORMATION);
    wxQueueEvent(MainWindow::instance->GetEventHandler(), restartMessage);
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
    for (const wxString& item : Arduino::getBoards()) {
      MainWindow::instance->devSelect->Append(item);
    }

    MainWindow::instance->devSelect->SetValue(lastSel);
    if (MainWindow::instance->devSelect->GetSelection() == -1) MainWindow::instance->devSelect->SetSelection(0);
    Progress::emitEvent(100, "Done.");
    Progress::emitEvent(100, "Done."); // This has to be called twice to update on macOS?
  });
}
std::vector<wxString> Arduino::getBoards() {
  std::vector<wxString> boards{"Select Device..."};
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
      boards.push_back("BOOTLOADER|" + wxString(buffer));
    }
  }

  return boards;
}

void Arduino::applyToBoard() {
  MainWindow::instance->progDialog = new Progress(MainWindow::instance);
  MainWindow::instance->progDialog->SetTitle("Applying Changes");

  MainWindow::instance->thread = new ThreadRunner([&]() {
    wxString returnVal;
    Progress::emitEvent(0, "Initializing...");

    Progress::emitEvent(10, "Checking board presence...");
    wxString lastSel = MainWindow::instance->devSelect->GetStringSelection();
    MainWindow::instance->devSelect->Clear();
    for (const wxString& item : Arduino::getBoards()) {
      MainWindow::instance->devSelect->Append(item);
    }
    MainWindow::instance->devSelect->SetValue(lastSel);
    if (MainWindow::instance->devSelect->GetSelection() == -1) {
      MainWindow::instance->devSelect->SetSelection(0);
      Progress::emitEvent(100, "Error!");
      Misc::MessageBoxEvent* msg = new Misc::MessageBoxEvent(Misc::EVT_MSGBOX, wxID_ANY, "Please make sure your board is connected and selected, then try again!", "Board Selection Error", wxOK | wxICON_ERROR);
      wxQueueEvent(MainWindow::instance->GetEventHandler(), msg);
      return;
    }

    Progress::emitEvent(20, "Generating configuration file...");
    Configuration::instance->outputConfig();
    Arduino::updateIno();

    Progress::emitEvent(40, "Compiling ProffieOS...");
    returnVal = Arduino::compile();
    if (returnVal != "OK") {
      Progress::emitEvent(100, "Error");
      Misc::MessageBoxEvent* msg = new Misc::MessageBoxEvent(Misc::EVT_MSGBOX, wxID_ANY, "There was an error while compiling:\n\n" + returnVal, "Compile Error");
      wxQueueEvent(MainWindow::instance->GetEventHandler(), msg);
      return;
    }

    Progress::emitEvent(65, "Uploading to ProffieBoard...");
    returnVal = Arduino::upload();
    if (returnVal != "OK") {
      Progress::emitEvent(100, "Error");
      Misc::MessageBoxEvent* msg = new Misc::MessageBoxEvent(Misc::EVT_MSGBOX, wxID_ANY, "There was an error while uploading:\n\n" + returnVal, "Upload Error");
      wxQueueEvent(MainWindow::instance->GetEventHandler(), msg);
      return;
    }

    Progress::emitEvent(100, "Done.");
    Progress::emitEvent(100, "Done.");

    Misc::MessageBoxEvent* msg = new Misc::MessageBoxEvent(Misc::EVT_MSGBOX, wxID_ANY, "Changes Successfully Applied to ProffieBoard!", "Apply Changes to Board", wxOK | wxICON_INFORMATION);
    wxQueueEvent(MainWindow::instance->GetEventHandler(), msg);
  });
}
void Arduino::verifyConfig() {
  MainWindow::instance->progDialog = new Progress(MainWindow::instance);
  MainWindow::instance->progDialog->SetTitle("Verify Config");

  MainWindow::instance->thread = new ThreadRunner([&]() {
    wxString returnVal;
    Progress::emitEvent(20, "Generating configuration file...");

    if (!Configuration::instance->outputConfig()) {
      Progress::emitEvent(100, "Error");
      return;
    }

    returnVal = Arduino::updateIno();
    if (returnVal != "OK") {
      Progress::emitEvent(100, "Error");
      Misc::MessageBoxEvent* msg = new Misc::MessageBoxEvent(Misc::EVT_MSGBOX, wxID_ANY, "There was an error while updating files:\n\n"
                       + returnVal, "Files Error");
      wxQueueEvent(MainWindow::instance->GetEventHandler(), msg);
      return;
    }

    Progress::emitEvent(40, "Compiling ProffieOS...");
    returnVal = Arduino::compile();
    if (returnVal != "OK") {
      Progress::emitEvent(100, "Error");
      Misc::MessageBoxEvent* msg = new Misc::MessageBoxEvent(Misc::EVT_MSGBOX, wxID_ANY, "There was an error while compiling:\n\n"
                       + returnVal, "Compile Error");
      wxQueueEvent(MainWindow::instance->GetEventHandler(), msg);
      return;
    }

    Progress::emitEvent(100, "Done.");
    Misc::MessageBoxEvent* msg = new Misc::MessageBoxEvent(Misc::EVT_MSGBOX, wxID_ANY, "Config Verified Successfully!", "Verify Config", wxOK | wxICON_INFORMATION);
    wxQueueEvent(MainWindow::instance->GetEventHandler(), msg);
  });
}

wxString Arduino::compile() {
  wxString output;
  char buffer[1024];

  wxString compileCommand = "compile ";
  compileCommand += "-b ";
  compileCommand += GeneralPage::instance->board->GetSelection() == 0 ? ARDUINOCORE_PBV1 : GeneralPage::instance->board->GetSelection() == 1 ? ARDUINOCORE_PBV2 : ARDUINOCORE_PBV3;
  compileCommand += " --board-options ";
  if (GeneralPage::instance->massStorage->GetValue() && GeneralPage::instance->webUSB->GetValue()) compileCommand += "usb=cdc_msc_webusb";
  else if (GeneralPage::instance->webUSB->GetValue()) compileCommand += "usb=cdc_webusb";
  else if (GeneralPage::instance->massStorage->GetValue()) compileCommand += "usb=cdc_msc";
  else compileCommand += "usb=cdc";
  if (GeneralPage::instance->board->GetStringSelection() == "ProffieBoard V3") compileCommand +=",dosfs=sdmmc1";
  compileCommand += " " PROFFIEOS_PATH;
  FILE *arduinoCli = Arduino::CLI(compileCommand);

  while(fgets(buffer, 1024, arduinoCli) != NULL) {
    MainWindow::instance->progDialog->emitEvent(-1, ""); // Pulse
    if (std::strstr(buffer, "error")) {
      wxString error = buffer;
      /*
      while (fgets(buffer, 1024, arduinoCli) != NULL) {
        error += '\n';
        error += buffer;
      }
      */
      return Arduino::parseError(error);
    }
  }
  if (pclose(arduinoCli) != 0) {
    return "Unkown Compile Error";
  }

  return "OK";
}
wxString Arduino::upload() {
  char buffer[1024];

  wxString uploadCommand = "upload ";
  uploadCommand += PROFFIEOS_PATH;
  uploadCommand += " --board-options ";
  if (GeneralPage::instance->massStorage->GetValue() && GeneralPage::instance->webUSB->GetValue()) uploadCommand += "usb=cdc_msc_webusb";
  else if (GeneralPage::instance->webUSB->GetValue()) uploadCommand += "usb=cdc_webusb";
  else if (GeneralPage::instance->massStorage->GetValue()) uploadCommand += "usb=cdc_msc";
  else uploadCommand += "usb=cdc";
  if (GeneralPage::instance->board->GetStringSelection() == "ProffieBoard V3") uploadCommand +=",dosfs=sdmmc1";

  uploadCommand += " --fqbn ";
  uploadCommand += GeneralPage::instance->board->GetSelection() == 0 ? ARDUINOCORE_PBV1 : GeneralPage::instance->board->GetSelection() == 1 ? ARDUINOCORE_PBV2 : ARDUINOCORE_PBV3;
  uploadCommand += " --port ";
  uploadCommand += MainWindow::instance->devSelect->GetStringSelection();

  FILE *arduinoCli = Arduino::CLI(uploadCommand);

  while(fgets(buffer, 1024, arduinoCli) != NULL) {
    MainWindow::instance->progDialog->emitEvent(-1, ""); // Pulse
    if (std::strstr(buffer, "error")) {
      wxString error = buffer;
      /*
      while (fgets(buffer, 1024, arduinoCli) != NULL) {
        error += '\n';
        error += buffer;
      }
      */
      return Arduino::parseError(error);
    }
  }
  if (pclose(arduinoCli) != 0) {
    return "Unknown Upload Error";
  }

  return "OK";
}
wxString Arduino::updateIno() {
  std::ifstream input(PROFFIEOS_PATH "/ProffieOS.ino");
  if (!input.is_open()) return "ERROR OPENING FOR READ";

  std::string fileData;
  std::vector<wxString> outputData;
  while(!input.eof()) {
    getline(input, fileData);
    outputData.push_back(fileData == "// #define CONFIG_FILE \"config/YOUR_CONFIG_FILE_NAME_HERE.h\"" ? "#define CONFIG_FILE \"config/ProffieConfig_autogen.h\"" : fileData);
  }
  input.close();


  std::ofstream output(PROFFIEOS_PATH "/ProffieOS.ino");
  if (!output.is_open()) return "ERROR OPENING FOR WRITE";

  for (const wxString& line : outputData) {
    output << line << std::endl;
  }
  output.close();

  return "OK";
}

wxString Arduino::parseError(const wxString& error) {
#define ERRCONTAINS(token) std::strstr(error.data(), token)
  if (ERRCONTAINS("select Proffieboard")) return "Please ensure you've selected the correct board in General";
  if (ERRCONTAINS("expected unqualified-id")) return "Please make sure there are no brackets in your styles (such as \"{\" or \"}\")\n and there is nothing missing or extra from your style! (such as parentheses or \"<>\")";
  if (ERRCONTAINS("FLASH")) return "The specified config will not fit on Proffieboard.\n\nTry disabling diagnostic commands, disabling talkie, disabling prop features, or removing blade styles to make it fit.";
  if (ERRCONTAINS("Serial port busy")) return "The proffieboard appears busy. \nPlease make sure nothing else is using it, then try again.";
  else if (ERRCONTAINS("error:")) return ERRCONTAINS("error:");
  else return "Unknown error";
#undef ERRCONTAINS
}


FILE* Arduino::CLI(const wxString& command) {
  wxString fullCommand;
#if defined(__WXMSW__)
  fullCommand += "title ProffieConfig Worker && ";
  fullCommand += "resources\\windowmode -title \"ProffieConfig Worker\" -mode force_minimized && ";
#endif
  fullCommand += ARDUINO_PATH " ";
  fullCommand += command;
  fullCommand += " 2>&1";

  return popen(fullCommand.data(), "r");
}
