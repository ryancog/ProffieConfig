// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include "arduino.h"

#include "core/defines.h"
#include "core/config/configuration.h"
#include "core/utilities/misc.h"
#include "core/utilities/progress.h"
#include "editor/editorwindow.h"
#include "editor/pages/generalpage.h"

#include <cstring>
#include <thread>

#ifdef __WINDOWS__
#include <windows.h>
#include <codecvt>
#include <locale>
#else
#include <termios.h>
#endif

using namespace std::chrono_literals;

namespace Arduino {
    FILE *CLI(const wxString& command);

    bool updateIno(wxString&, EditorWindow*);
    bool compile(wxString&, EditorWindow*, Progress* = nullptr);
    bool upload(wxString&, EditorWindow*, Progress* = nullptr);
    wxString parseError(const wxString&);

    wxDEFINE_EVENT(EVT_INIT_DONE, Event);
    wxDEFINE_EVENT(EVT_APPLY_DONE, Event);
    wxDEFINE_EVENT(EVT_VERIFY_DONE, Event);
    wxDEFINE_EVENT(EVT_REFRESH_DONE, Event);
    wxDEFINE_EVENT(EVT_CLEAR_BLIST, Event);
    wxDEFINE_EVENT(EVT_APPEND_BLIST, Event);
};

void Arduino::init(wxWindow* parent) {
    auto progDialog = new Progress(parent);
    progDialog->SetTitle("Dependency Installation");

    std::thread thread{[progDialog, parent]() {
        auto evt{new Arduino::Event(Arduino::EVT_INIT_DONE)};
        FILE* install;
        std::string fulloutput;
        char buffer[128];

        progDialog->emitEvent(5, "Downloading dependencies...");
        install = Arduino::CLI("core install proffieboard:stm32l4@" ARDUINO_PBPLUGIN_VERSION " --additional-urls https://profezzorn.github.io/arduino-proffieboard/package_proffieboard_index.json");
        while (fgets(buffer, 128, install) != nullptr) { progDialog->emitEvent(-1, ""); fulloutput += buffer; }
        if (pclose(install)) {
            progDialog->emitEvent(100, "Error");
            std::cerr << fulloutput << std::endl;
            evt->succeeded = false;
            wxQueueEvent(parent, evt);
            return;
        }

#   ifndef __WXOSX__
        progDialog->emitEvent(60, "Installing drivers...");
        install = DRIVER_INSTALL;
        while (fgets(buffer, 128, install) != nullptr) { progDialog->emitEvent(-1, ""); fulloutput += buffer; }
        if (pclose(install)) {
            progDialog->emitEvent(100, "Error");
            std::cerr << fulloutput << std::endl;
            wxQueueEvent(parent, evt);
            return;
        }
#   endif

        progDialog->emitEvent(100, "Done.");
        evt->succeeded = true;
        wxQueueEvent(parent, evt);
    }}; // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
    thread.detach();
}

void Arduino::refreshBoards(MainMenu* window) {
    auto progDialog = new Progress(window);
    progDialog->SetTitle("Device Update");

    auto lastSelection{window->boardSelect->entry()->GetStringSelection()};
    std::thread thread([=]() {
        progDialog->emitEvent(0, "Initializing...");
        wxQueueEvent(window, new Event(EVT_CLEAR_BLIST)); // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
        progDialog->emitEvent(20, "Fetching Devices...");
        for (const wxString& item : Arduino::getBoards()) {
            auto evt{new Event(EVT_APPEND_BLIST)};
            evt->str = item.ToStdString();
            wxQueueEvent(window, evt);
        }

        auto evt{new Event(EVT_REFRESH_DONE)};
        evt->str = lastSelection.ToStdString();
        wxQueueEvent(window, evt);
        progDialog->emitEvent(100, "Done.");
    });
    thread.detach();
}

std::vector<wxString> Arduino::getBoards() {
  std::vector<wxString> boards{"Select Board..."};
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

# ifdef __WINDOWS__
  boards.push_back("BOOTLOADER RECOVERY");
# endif
  return boards;
}

void Arduino::applyToBoard(MainMenu* window, EditorWindow* editor) {
    auto progDialog = new Progress(window);
    progDialog->SetTitle("Applying Changes");

    std::thread thread{[=]() {
        auto *evt{new Event(EVT_APPLY_DONE)};
        wxString returnVal;

        progDialog->emitEvent(0, "Initializing...");

        progDialog->emitEvent(10, "Checking board presence...");
        wxString lastSel = window->boardSelect->entry()->GetStringSelection();
        window->boardSelect->entry()->Clear();
        for (const wxString& item : Arduino::getBoards()) {
            window->boardSelect->entry()->Append(item);
        }
        window->boardSelect->entry()->SetStringSelection(lastSel);
        if (window->boardSelect->entry()->GetSelection() == -1) {
            window->boardSelect->entry()->SetSelection(0);
            progDialog->emitEvent(100, "Error!");
            Misc::MessageBoxEvent* msg = new Misc::MessageBoxEvent(wxID_ANY, "Please make sure your board is connected and selected, then try again!", "Board Selection Error", wxOK | wxICON_ERROR);
            wxQueueEvent(window, msg);
            wxQueueEvent(window, evt);
            return;
        }

        progDialog->emitEvent(20, "Generating configuration file...");
        if (!Configuration::outputConfig(editor)) {
            progDialog->emitEvent(100, "Error");
            // NO message here because outputConfig will handle it.
            wxQueueEvent(window, evt);
            return;
        }

        progDialog->emitEvent(30, "Updating ProffieOS file...");
        if (!Arduino::updateIno(returnVal, editor)) {
            progDialog->emitEvent(100, "Error");
            Misc::MessageBoxEvent* msg = new Misc::MessageBoxEvent(wxID_ANY, "There was an error while updating ProffieOS file:\n\n" + returnVal, "Files Error");
            wxQueueEvent(window, msg);
            wxQueueEvent(window, evt);
            return;
        }

        progDialog->emitEvent(40, "Compiling ProffieOS...");
        if (!Arduino::compile(returnVal, editor)) {
            progDialog->emitEvent(100, "Error");
            Misc::MessageBoxEvent* msg = new Misc::MessageBoxEvent(wxID_ANY, "There was an error while compiling:\n\n" + returnVal, "Compile Error");
            wxQueueEvent(window, msg);
            wxQueueEvent(window, evt);
            return;
        }

#   ifdef __WINDOWS__
        if (window->boardSelect->entry()->GetStringSelection() != "BOOTLOADER RECOVERY") {
            progDialog->emitEvent(50, "Rebooting Proffieboard...");

            auto serialHandle = CreateFileW(window->boardSelect->entry()->GetStringSelection().ToStdWstring().c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (serialHandle != INVALID_HANDLE_VALUE) {
                DCB dcbSerialParameters = {};
                dcbSerialParameters.DCBlength = sizeof(dcbSerialParameters);

                dcbSerialParameters.BaudRate = CBR_115200;
                dcbSerialParameters.ByteSize = 8;
                dcbSerialParameters.StopBits = ONESTOPBIT;
                dcbSerialParameters.Parity = NOPARITY;
                dcbSerialParameters.fRtsControl = RTS_CONTROL_ENABLE;
                dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

                SetCommState(serialHandle, &dcbSerialParameters);

                DWORD bytesHandled;
                const char* rebootCommand = "RebootDFU\r\n";
                WriteFile(serialHandle, rebootCommand, strlen(rebootCommand),  &bytesHandled, nullptr);

                CloseHandle(serialHandle);
                Sleep(5000);
            }
        }
#   endif

        progDialog->emitEvent(65, "Uploading to ProffieBoard...");
#   ifdef __WINDOWS__
        std::string commandString = R"(title ProffieConfig Worker & resources\windowmode -title "ProffieConfig Worker" -mode force_minimized & )";
        commandString += (returnVal.substr(returnVal.find("|") + 1) + R"( 0x1209 0x6668 )" + returnVal.substr(0, returnVal.find("|")) + R"( 2>&1)").ToStdString();
        std::cerr << "UploadCommandString: " << commandString << std::endl;

        auto upload = popen(commandString.c_str(), "r");
        char buffer[128];
        std::string error{};
        while (fgets(buffer, sizeof(buffer), upload) != nullptr) {
            progDialog->emitEvent(-1, "");
            error += buffer;
        }

        if (error.find("File downloaded successfully") == std::string::npos) {
            progDialog->emitEvent(100, "Error");
            Misc::MessageBoxEvent* msg = new Misc::MessageBoxEvent(wxID_ANY, "There was an error while uploading:\n\n" + Arduino::parseError(error), "Upload Error");
            wxQueueEvent(window, msg);
            wxQueueEvent(window, evt);
            return;
        }

#   else
        if (!Arduino::upload(returnVal, editor)) {
            progDialog->emitEvent(100, "Error");
            Misc::MessageBoxEvent* msg = new Misc::MessageBoxEvent(wxID_ANY, "There was an error while uploading:\n\n" + returnVal, "Upload Error");
            wxQueueEvent(window, msg);
            wxQueueEvent(window, evt);
            return;
        }
#   endif

        progDialog->emitEvent(100, "Done.");

        Misc::MessageBoxEvent* msg = new Misc::MessageBoxEvent(wxID_ANY, "Changes Successfully Applied to ProffieBoard!", "Apply Changes to Board", wxOK | wxICON_INFORMATION);
        wxQueueEvent(window, msg);
        evt->succeeded = true;
        wxQueueEvent(window, evt);
    }};
    thread.detach();
}
void Arduino::verifyConfig(wxWindow* parent, EditorWindow* editor) {
    auto progDialog = new Progress(parent);
    progDialog->SetTitle("Verify Config");

    std::thread thread{[=]() {
        auto *evt{new Event(EVT_VERIFY_DONE)};
        wxString returnVal;

        progDialog->emitEvent(20, "Generating configuration file...");
        if (!Configuration::outputConfig(editor)) {
            progDialog->emitEvent(100, "Error");
            // Outputconfig will handle error message
            wxQueueEvent(parent, evt);
            return; // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
        }

        progDialog->emitEvent(30, "Updating ProffieOS file...");
        if (!Arduino::updateIno(returnVal, editor)) {
            progDialog->emitEvent(100, "Error");
            Misc::MessageBoxEvent* msg = new Misc::MessageBoxEvent(wxID_ANY, "There was an error while updating ProffieOS file:\n\n"
                               + returnVal, "Files Error");
            wxQueueEvent(parent, msg);
            wxQueueEvent(parent, evt);
            return;
        }

        progDialog->emitEvent(40, "Compiling ProffieOS...");
        if (!Arduino::compile(returnVal, editor)) {
            progDialog->emitEvent(100, "Error");
            Misc::MessageBoxEvent* msg = new Misc::MessageBoxEvent(wxID_ANY, "There was an error while compiling:\n\n"
                               + returnVal, "Compile Error");
            wxQueueEvent(parent, msg);
            wxQueueEvent(parent, evt);
            return;
        }

        progDialog->emitEvent(100, "Done.");
        Misc::MessageBoxEvent* msg = new Misc::MessageBoxEvent(wxID_ANY, "Config Verified Successfully!", "Verify Config", wxOK | wxICON_INFORMATION);
        wxQueueEvent(parent, msg);
        evt->succeeded = true;
        wxQueueEvent(parent, evt);
    }};
    thread.detach();
}

bool Arduino::compile(wxString& _return, EditorWindow* editor, Progress* progDialog) {
  wxString output;
  char buffer[1024];

  wxString compileCommand = "compile ";
  compileCommand += "-b ";
  compileCommand += editor->generalPage->board->entry()->GetSelection() == PROFFIEBOARDV1 ? ARDUINOCORE_PBV1 : editor->generalPage->board->entry()->GetSelection() == PROFFIEBOARDV2 ? ARDUINOCORE_PBV2 : ARDUINOCORE_PBV3;
  compileCommand += " --board-options ";
  if (editor->generalPage->massStorage->GetValue() && editor->generalPage->webUSB->GetValue()) compileCommand += "usb=cdc_msc_webusb";
  else if (editor->generalPage->webUSB->GetValue()) compileCommand += "usb=cdc_webusb";
  else if (editor->generalPage->massStorage->GetValue()) compileCommand += "usb=cdc_msc";
  else compileCommand += "usb=cdc";
  if (editor->generalPage->board->entry()->GetSelection() == PROFFIEBOARDV3) compileCommand +=",dosfs=sdmmc1";
  compileCommand += " " PROFFIEOS_PATH " -v";
  FILE *arduinoCli = Arduino::CLI(compileCommand);

  std::string error{};
  std::wstring paths{};
  while(fgets(buffer, sizeof(buffer), arduinoCli) != NULL) {
    if (progDialog != nullptr) progDialog->emitEvent(-1, ""); // Pulse
    error += buffer;
    if (std::strstr(buffer, "error")) {
      _return = Arduino::parseError(error);
      return false;
    }
#   ifdef __WINDOWS__
    if (std::strstr(buffer, "ProffieOS.ino.dfu") && std::strstr(buffer, "stm32l4") && std::strstr(buffer, "C:\\")) {
      std::cerr << "ErrBufferFull: " << error << std::endl;
      error = buffer;
      std::cerr << "PathBuffer: " << error << std::endl;

      // Ugly code because Windows wants wchar_t*, which requires (ish) std::wstring's
      wchar_t shortPath[MAX_PATH];
      GetShortPathName(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(error.substr(error.rfind("C:\\"), error.rfind("ProffieOS.ino.dfu") - error.rfind("C:\\") + 17)).c_str(), shortPath, MAX_PATH);
      paths = shortPath;
      paths += L"|";
      GetShortPathName(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(error.substr(1, error.find("windows") + 7 - 1)).c_str(), shortPath, MAX_PATH);
      paths += shortPath;
      paths += LR"(\\stm32l4-upload.bat)";
      std::wcerr << "ParsedPaths: " << paths << std::endl;

      pclose(arduinoCli);
      _return = paths;
      return true;
    }
#   endif
  }
  if (pclose(arduinoCli) != 0) {
    _return = "Unknown Compile Error";
    return false;
  }


  _return = error;
# ifdef __WINDOWS__
  return false;
# else
  return true;
#endif
}
bool Arduino::upload(wxString& _return, EditorWindow* editor, Progress* progDialog) {
    char buffer[1024];

    wxString uploadCommand = "upload ";
    uploadCommand += PROFFIEOS_PATH;
    uploadCommand += " --board-options ";
    if (editor->generalPage->massStorage->GetValue() && editor->generalPage->webUSB->GetValue()) uploadCommand += "usb=cdc_msc_webusb";
    else if (editor->generalPage->webUSB->GetValue()) uploadCommand += "usb=cdc_webusb";
    else if (editor->generalPage->massStorage->GetValue()) uploadCommand += "usb=cdc_msc";
    else uploadCommand += "usb=cdc";
    if (editor->generalPage->board->entry()->GetStringSelection() == "ProffieBoard V3") uploadCommand +=",dosfs=sdmmc1";

    uploadCommand += " --fqbn ";
    uploadCommand += editor->generalPage->board->entry()->GetSelection() == 0 ? ARDUINOCORE_PBV1 : editor->generalPage->board->entry()->GetSelection() == 1 ? ARDUINOCORE_PBV2 : ARDUINOCORE_PBV3;
    uploadCommand += " -v";

#ifndef __WINDOWS__
    struct termios newtio;
    auto fd = open(static_cast<MainMenu*>(editor->GetParent())->boardSelect->entry()->GetStringSelection().data(), O_RDWR | O_NOCTTY);
    if (fd < 0) {
        std::cout << "err" << std::endl;
    }

    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = B115200 | CRTSCTS | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = (tcflag_t) NULL;
    newtio.c_lflag &= ~ICANON; /* unset canonical */
    newtio.c_cc[VTIME] = 1; /* 100 millis */

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

    char buf[255];
    while(read(fd, buf, 255));

    fsync(fd);
    write(fd, "\r\n", 2);
    write(fd, "\r\n", 2);
    write(fd, "RebootDFU\r\n", 11);

    // Ensure everything is flushed
    std::this_thread::sleep_for(50ms);
    close(fd);
    std::this_thread::sleep_for(5s);
#endif

    FILE *arduinoCli = Arduino::CLI(uploadCommand);

    wxString error{};
    while(fgets(buffer, sizeof(buffer), arduinoCli) != NULL) {
        if (progDialog != nullptr) progDialog->emitEvent(-1, ""); // Pulse
        error += buffer;
        if (std::strstr(buffer, "error") || std::strstr(buffer, "FAIL")) {
            _return = Arduino::parseError(error);
            return false;
        }
    }
    if (pclose(arduinoCli) != 0) {
        _return = "Unknown Upload Error";
        return false;
    }

    _return.clear();
    return true;
}
bool Arduino::updateIno(wxString& _return, EditorWindow* _editor) {
  std::ifstream input(PROFFIEOS_INO);
  if (!input.is_open()) {
    _return = "ERROR OPENING FOR READ";
    return false;
  }

  std::string fileData;
  std::vector<wxString> outputData;
  while(!input.eof()) {
    getline(input, fileData);
    if (fileData.find(R"(// #define CONFIG_FILE "config/YOUR_CONFIG_FILE_NAME_HERE.h")") != std::string::npos) outputData.push_back("#define CONFIG_FILE \"config/" + std::string{_editor->getOpenConfig()} + ".h\"");
    if (fileData.find(R"(#define CONFIG_FILE)") == 0) outputData.push_back("#define CONFIG_FILE \"config/" + std::string{_editor->getOpenConfig()} + ".h\"");
    else if (fileData.find(R"(const char version[] = ")" ) != std::string::npos) outputData.push_back(R"(const char version[] = ")" PROFFIEOS_VERSION R"(";)");
    else outputData.push_back(fileData);
  }
  input.close();


  std::ofstream output(PROFFIEOS_INO);
  if (!output.is_open()) {
    _return = "ERROR OPENING FOR WRITE";
    return false;
  }

  for (const wxString& line : outputData) {
    output << line << std::endl;
  }
  output.close();

  _return.clear();
  return true;
}

wxString Arduino::parseError(const wxString& error) {
  std::cerr << "An arduino task failed with the following error: " << std::endl;
  std::cerr << error << std::endl;

#define ERRCONTAINS(token) std::strstr(error.data(), token)
  if (ERRCONTAINS("select Proffieboard")) return "Please ensure you've selected the correct board in General";
  if (ERRCONTAINS("expected unqualified-id")) return "Please make sure there are no brackets in your styles (such as \"{\" or \"}\")\n and there is nothing missing or extra from your style! (such as parentheses or \"<>\")";
  if (ERRCONTAINS(/* region FLASH */"overflowed")) return "The specified config will not fit on Proffieboard.\n\nTry disabling diagnostic commands, disabling talkie, disabling prop features, or removing blade styles to make it fit.";
  if (ERRCONTAINS("Serial port busy")) return "The Proffieboard appears busy. \nPlease make sure nothing else is using it, then try again.";
  if (ERRCONTAINS("Buttons for operation")) return std::string("Selected prop file ") + std::strstr(error.data(), "requires");
  if (ERRCONTAINS("1\n2\n3\n4\n5\n6\n7\n8\n9\n10")) return "Could not connect to Proffieboard for upload.";
  if (ERRCONTAINS("10\n9\n8\n7\n6\n5\n4\n3\n2\n1")) return "Could not connect to Proffieboard for upload.";
  if (ERRCONTAINS("No DFU capable USB device available")) return "No Proffieboard in BOOTLOADER mode found.";
  if (ERRCONTAINS("error:")) return ERRCONTAINS("error:");

  return "Unknown error: " + error;
#undef ERRCONTAINS
}

FILE* Arduino::CLI(const wxString& command) {
  wxString fullCommand;
# if defined(__WINDOWS__)
  fullCommand += "title ProffieConfig Worker & ";
  fullCommand += R"(resources\windowmode -title "ProffieConfig Worker" -mode force_minimized & )";
# endif
  fullCommand += ARDUINO_PATH " ";
  fullCommand += command;
  fullCommand += " 2>&1";

  return popen(fullCommand.data(), "r");
}
