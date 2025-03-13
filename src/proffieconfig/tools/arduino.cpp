#include "arduino.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <cstring>
#include <filesystem>
#include <limits>
#include <sstream>
#include <thread>

#ifdef __WINDOWS__
#include <windows.h>
#include <codecvt>
#include <locale>
#else
#include <termios.h>
#endif

#include <wx/uri.h>
#include <wx/webrequest.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/zstream.h>

#include "utils/paths.h"
#include "utils/types.h"
#include "log/context.h"
#include "log/logger.h"

#include "../core/defines.h"
#include "../core/config/configuration.h"
#include "../core/utilities/misc.h"
#include "../core/utilities/progress.h"
#include "../editor/editorwindow.h"
#include "../editor/pages/generalpage.h"
#include "../editor/pages/bladespage.h"
#include "../editor/pages/presetspage.h"
#include "../editor/dialogs/bladearraydlg.h"

using namespace std::chrono_literals;

namespace Arduino {
    FILE *cli(const string& command);

    bool updateIno(string&, EditorWindow*);
#   ifdef __WINDOWS__
    optional<array<string, 2>> compile(string&, EditorWindow *, Progress * = nullptr);
#   else
    bool compile(string&, EditorWindow *, Progress * = nullptr);
#   endif
    bool upload(string&, EditorWindow*, Progress* = nullptr);
    string parseError(const string&);

    wxDEFINE_EVENT(EVT_INIT_DONE, Event);
    wxDEFINE_EVENT(EVT_APPLY_DONE, Event);
    wxDEFINE_EVENT(EVT_VERIFY_DONE, Event);
    wxDEFINE_EVENT(EVT_REFRESH_DONE, Event);
    wxDEFINE_EVENT(EVT_CLEAR_BLIST, Event);
    wxDEFINE_EVENT(EVT_APPEND_BLIST, Event);
} // namespace Arduino

void Arduino::init(wxWindow *parent) {
    auto *progDialog{new Progress(parent)};
    progDialog->SetTitle("Dependency Installation");

    std::thread{[=]() {
        auto *const evt{new Arduino::Event(Arduino::EVT_INIT_DONE)};
        FILE *install{nullptr};
        std::string fulloutput;
        std::array<char, 128> buffer;

        progDialog->emitEvent(5, "Downloading ProffieOS...");
        const auto uri{wxURI{Paths::remoteAssets() + "/ProffieOS/" wxSTRINGIZE(PROFFIEOS_VERSION) ".zip"}.BuildURI()};
        auto proffieOSRequest{wxWebSessionSync::GetDefault().CreateRequest(uri)};
        auto requestResult{proffieOSRequest.Execute()};

        if (not requestResult) {
            progDialog->emitEvent(100, "Error");
            evt->str = "ProffieOS Download Failed\n" + requestResult.error.ToStdString();
            wxQueueEvent(parent->GetEventHandler(), evt);
            return;
        }

        fs::remove_all(Paths::proffieos());

        wxZipInputStream zipStream{*proffieOSRequest.GetResponse().GetStream()};
        if (not zipStream.IsOk()) {
            progDialog->emitEvent(100, "Error");
            evt->str = "Failed Opening ProffieOS ZIP";
            wxQueueEvent(parent->GetEventHandler(), evt);
            return;
        }

        std::unique_ptr<wxZipEntry> entry;
        while (entry.reset(zipStream.GetNextEntry()), entry) {
            auto fileNameStr{(Paths::proffieos() / entry->GetName().ToStdString()).string()};
            if (fileNameStr.find("__MACOSX") != std::string::npos) continue;

            auto permissionBits{entry->GetMode()};
            wxFileName fileName;
            
            if (entry->IsDir()) fileName.AssignDir(fileNameStr);
            else fileName.Assign(fileNameStr);
            
            if (!wxDirExists(fileName.GetPath())) {
                wxFileName::Mkdir(fileName.GetPath(), permissionBits, wxPATH_MKDIR_FULL);
            }
            
            if (entry->IsDir()) continue;
            
            if (not zipStream.CanRead()) {
                progDialog->emitEvent(100, "Error");
                evt->str = "ProffieOS Read Failed";
                wxQueueEvent(parent->GetEventHandler(), evt);
                return;
            }
            
            wxFileOutputStream outStream{fileNameStr};
            if (not outStream.IsOk()) {
                progDialog->emitEvent(100, "Error");
                evt->str = "ProffieOS Write Failed";
                wxQueueEvent(parent->GetEventHandler(), evt);
                return;
            }
            
            zipStream.Read(outStream);
        }

        if (zipStream.GetLastError() != wxSTREAM_EOF) {
            progDialog->emitEvent(100, "Error");
            evt->str = "Failed Parsing ProffieOS ZIP";
            wxQueueEvent(parent->GetEventHandler(), evt);
            return;
        }
        
        progDialog->emitEvent(30, "Downloading dependencies...");
        install = Arduino::cli("core install proffieboard:stm32l4@" ARDUINO_PBPLUGIN_VERSION " --additional-urls https://profezzorn.github.io/arduino-proffieboard/package_proffieboard_index.json");
        while (fgets(buffer.data(), buffer.size(), install) != nullptr) { progDialog->emitEvent(-1, ""); fulloutput += buffer.data(); }
        if (pclose(install)) {
            progDialog->emitEvent(100, "Error");
            std::cerr << fulloutput << std::endl;
            evt->str = "\nCore install failed:\n\n" + fulloutput;
            wxQueueEvent(parent->GetEventHandler(), evt);
            return;
        }

#       ifndef __WXOSX__
        progDialog->emitEvent(60, "Installing drivers...");

#       if defined(__linux__)
        install = popen("pkexec cp ~/.arduino15/packages/proffieboard/hardware/stm32l4/3.6/drivers/linux/*rules /etc/udev/rules.d", "r");
#       elif defined(__WINDOWS__)
        install = popen(("title ProffieConfig Worker & " + (Paths::binaries() / "proffie-dfu-setup.exe").string() + " 2>&1").c_str(), "r");
        // Really I should have a proper wait but I tried with an echo and that didn't work.
        // Could maybe revisit this in the future.
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        STARTUPINFOA startupInfo;
        PROCESS_INFORMATION procInfo;
        memset(&startupInfo, 0, sizeof startupInfo);
        startupInfo.cb = sizeof startupInfo;
        memset(&procInfo, 0, sizeof procInfo);

        constexpr const char MINIMIZE_COMMAND[]{R"(resources\windowmode -title "ProffieConfig Worker" -mode force_minimized)"};
        std::array<char, sizeof MINIMIZE_COMMAND + 1> minCmdInput;
        strncpy(minCmdInput.data(), MINIMIZE_COMMAND, minCmdInput.size());

        CreateProcessA(
            nullptr,
            minCmdInput.data(),
            nullptr,
            nullptr,
            false,
            0,
            nullptr,
            nullptr,
            &startupInfo,
            &procInfo
        );
#       endif

        fulloutput.clear();
        while (fgets(buffer.data(), buffer.size(), install) != nullptr) { progDialog->emitEvent(-1, ""); fulloutput += buffer.data(); }
        if (pclose(install)) {
            progDialog->emitEvent(100, "Error");
            std::cerr << fulloutput << std::endl;
            evt->str = "\nDriver install failed:\n\n" + fulloutput;
            wxQueueEvent(parent, evt);
            return;
        }
#       endif

        progDialog->emitEvent(100, "Done.");
        evt->succeeded = true;
        wxQueueEvent(parent->GetEventHandler(), evt);
    }}.detach(); // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void Arduino::refreshBoards(MainMenu* window) {
    auto *progDialog{new Progress(window)};
    progDialog->SetTitle("Device Update");

    auto lastSelection{window->boardSelect->entry()->GetStringSelection()};
    std::thread thread([=]() {
        progDialog->emitEvent(0, "Initializing...");
        wxQueueEvent(window, new Event(EVT_CLEAR_BLIST)); // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
        progDialog->emitEvent(20, "Fetching Devices...");
        for (const wxString& item : Arduino::getBoards()) {
            auto *evt{new Event(EVT_APPEND_BLIST)};
            evt->str = item.ToStdString();
            wxQueueEvent(window, evt);
        }

        auto *evt{new Event(EVT_REFRESH_DONE)};
        evt->str = lastSelection.ToStdString();
        wxQueueEvent(window, evt);
        progDialog->emitEvent(100, "Done.");
    });
    thread.detach();
}

std::vector<wxString> Arduino::getBoards() {
    std::vector<wxString> boards{"Select Board..."};
    char buffer[1024];

    FILE *arduinoCli = Arduino::cli("board list");

    if (!arduinoCli) {
        return boards;
    }

    struct Result {
        std::string port;
        bool isProffie{false};
    };

    std::vector<Result> results;

    while (fgets(buffer, 1024, arduinoCli) != nullptr) {
        if (std::strstr(buffer, "No boards found.") != nullptr) {
            break;
        }

        if (buffer[0] == ' ' or buffer[0] == '\t') {
            if (results.empty()) continue;

            if (std::strstr(buffer, "proffieboard") != nullptr) {
                results.back().isProffie = true;
            }

            continue;
        }

        if (std::strstr(buffer, "serial") != nullptr) {
            *std::strpbrk(buffer, " ") = '\0'; // End string at break to get dev path
            const auto isProffie{std::strstr(buffer, "proffieboard") != nullptr};
            results.emplace_back(Result{buffer, isProffie});
        } else if (std::strstr(buffer, "dfu") != nullptr) {
            *std::strpbrk(buffer, " ") = '\0';
            boards.emplace_back("BOOTLOADER|" + wxString(buffer));
        }
    }

    for (const auto& result : results) {
        if (!result.isProffie) continue;
        boards.emplace_back(result.port);
    }

#   ifdef __WINDOWS__
    boards.emplace_back("BOOTLOADER RECOVERY");
#   endif
    return boards;
}

void Arduino::applyToBoard(MainMenu* window, EditorWindow* editor) {
    auto *progDialog{new Progress(window)};
    progDialog->SetTitle("Applying Changes");

    editor->presetsPage->update();
    editor->bladesPage->update();
    editor->bladesPage->bladeArrayDlg->update();

    std::thread thread{[=]() {
        auto *evt{new Event(EVT_APPLY_DONE)};
        std::string returnVal;

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
            auto* msg{new Misc::MessageBoxEvent(wxID_ANY, "Please make sure your board is connected and selected, then try again!", "Board Selection Error", wxOK | wxICON_ERROR)};
            wxQueueEvent(window, msg);
            wxQueueEvent(window, evt);
            return;
        }

        progDialog->emitEvent(20, "Generating configuration file...");
        const auto configPath{Paths::proffieos() / "config" / (string{editor->getOpenConfig()} + ".h")};
        if (not Configuration::outputConfig(configPath, editor, true)) {
            progDialog->emitEvent(100, "Error");
            // NO message here because outputConfig will handle it.
            wxQueueEvent(window, evt);
            return;
        }

        progDialog->emitEvent(30, "Updating ProffieOS file...");
        if (!Arduino::updateIno(returnVal, editor)) {
            progDialog->emitEvent(100, "Error");
            auto* msg{new Misc::MessageBoxEvent(wxID_ANY, "There was an error while updating ProffieOS file:\n\n" + returnVal, "Files Error")};
            wxQueueEvent(window, msg);
            wxQueueEvent(window, evt);
            return;
        }

        progDialog->emitEvent(40, "Compiling ProffieOS...");
        if (!Arduino::compile(returnVal, editor)) {
            progDialog->emitEvent(100, "Error");
            auto* msg{new Misc::MessageBoxEvent(wxID_ANY, "There was an error while compiling:\n\n" + returnVal, "Compile Error")};
            wxQueueEvent(window, msg);
            wxQueueEvent(window, evt);
            return;
        }

#   ifdef __WINDOWS__
        if (window->boardSelect->entry()->GetStringSelection() != "BOOTLOADER RECOVERY") {
            progDialog->emitEvent(50, "Rebooting Proffieboard...");

            auto *serialHandle{CreateFileW(window->boardSelect->entry()->GetStringSelection().ToStdWstring().c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)};
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

                DWORD bytesHandled{};
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
        commandString += returnVal.substr(returnVal.find('|') + 1) + R"( 0x1209 0x6668 )" + returnVal.substr(0, returnVal.find('|')) + R"( 2>&1)";
        std::cerr << "UploadCommandString: " << commandString << std::endl;

        auto *upload{popen(commandString.c_str(), "r")};
        char buffer[128];
        std::string error{};
        while (fgets(buffer, sizeof(buffer), upload) != nullptr) {
            progDialog->emitEvent(-1, "");
            error += buffer;
        }

        if (error.find("File downloaded successfully") == std::string::npos) {
            progDialog->emitEvent(100, "Error");
            auto* msg{new Misc::MessageBoxEvent(wxID_ANY, "There was an error while uploading:\n\n" + Arduino::parseError(error), "Upload Error")};
            wxQueueEvent(window, msg);
            wxQueueEvent(window, evt);
            return;
        }

#   else
        if (not Arduino::upload(returnVal, editor)) {
            progDialog->emitEvent(100, "Error");
            Misc::MessageBoxEvent* msg = new Misc::MessageBoxEvent(wxID_ANY, "There was an error while uploading:\n\n" + returnVal, "Upload Error");
            wxQueueEvent(window, msg);
            wxQueueEvent(window, evt);
            return;
        }
#   endif

        progDialog->emitEvent(100, "Done.");

        auto* msg{new Misc::MessageBoxEvent(wxID_ANY, "Changes Successfully Applied to ProffieBoard!", "Apply Changes to Board", wxOK | wxICON_INFORMATION)};
        wxQueueEvent(window, msg);
        evt->succeeded = true;
        wxQueueEvent(window, evt);
    }};
    thread.detach();
}
void Arduino::verifyConfig(wxWindow* parent, EditorWindow* editor) {
    auto *progDialog{new Progress(parent)};
    progDialog->SetTitle("Verify Config");

    editor->presetsPage->update();
    editor->bladesPage->update();
    editor->bladesPage->bladeArrayDlg->update();

    std::thread thread{[=]() {
        auto *evt{new Event(EVT_VERIFY_DONE)};
        string returnVal;

        progDialog->emitEvent(20, "Generating configuration file...");
        const auto configPath{Paths::proffieos() / "config" / (string{editor->getOpenConfig()} + ".h")};
        if (not Configuration::outputConfig(configPath, editor, true)) {
            progDialog->emitEvent(100, "Error");
            // Outputconfig will handle error message
            wxQueueEvent(parent, evt);
            return;
        }

        progDialog->emitEvent(30, "Updating ProffieOS file...");
        if (not Arduino::updateIno(returnVal, editor)) {
            progDialog->emitEvent(100, "Error");
            auto* msg{new Misc::MessageBoxEvent(wxID_ANY, "There was an error while updating ProffieOS file:\n\n" + returnVal, "Files Error")};
            wxQueueEvent(parent, msg);
            wxQueueEvent(parent, evt);
            return;
        }

        progDialog->emitEvent(40, "Compiling ProffieOS...");
        if (not Arduino::compile(returnVal, editor)) {
            progDialog->emitEvent(100, "Error");
            auto* msg{new Misc::MessageBoxEvent(wxID_ANY, "There was an error while compiling:\n\n" + returnVal, "Compile Error")};
            wxQueueEvent(parent, msg);
            wxQueueEvent(parent, evt);
            return;
        }

        constexpr string_view USED_PREFIX{"Sketch uses "};
        constexpr string_view MAX_PREFIX{"Maximum is "};
        const auto usedPos{returnVal.find(USED_PREFIX)};
        const auto maxPos{returnVal.find(MAX_PREFIX)};

        string successMessage{"Config Verified Successfully!"};
        std::cout << "RetVal: " << returnVal << std::endl;
        if (usedPos != string::npos and maxPos != string::npos) {
            auto usedBytes{std::stoi(returnVal.substr(usedPos + USED_PREFIX.length()))};
            auto maxBytes{std::stoi(returnVal.substr(maxPos + MAX_PREFIX.length()))};
            auto percent{std::round(usedBytes * 10000.0 / maxBytes) / 100.0};

            std::stringstream successStream;
            successStream << "\n\nThe configuration uses " << percent << "% of board space. (" << usedBytes << '/' << maxBytes << ')';
            successMessage += successStream.str();
        }

        progDialog->emitEvent(100, "Done.");
        auto* msg{new Misc::MessageBoxEvent(wxID_ANY, successMessage, "Verify Config", wxOK | wxICON_INFORMATION)};
        wxQueueEvent(parent, msg);
        evt->succeeded = true;
        wxQueueEvent(parent, evt);
    }};
    thread.detach();
}

#ifdef __WINDOWS__
optional<array<string, 2>> Arduino::compile(string& _return, EditorWindow* editor, Progress* progDialog) {
#else
bool Arduino::compile(string& _return, EditorWindow* editor, Progress* progDialog) {
#endif
    string output;
    char buffer[1024];

    string compileCommand = "compile ";
    compileCommand += "-b ";
    switch (static_cast<Proffieboard>(editor->generalPage->board->entry()->GetSelection())) {
        case PROFFIEBOARDV3:
            compileCommand += ARDUINOCORE_PBV3;
            break;
        case PROFFIEBOARDV2:
            compileCommand += ARDUINOCORE_PBV2;
            break;
        case PROFFIEBOARDV1:
            compileCommand += ARDUINOCORE_PBV1;
            break;
        default: 
            abort();
    }
    compileCommand += " --board-options ";
    if (editor->generalPage->massStorage->GetValue() && editor->generalPage->webUSB->GetValue()) compileCommand += "usb=cdc_msc_webusb";
    else if (editor->generalPage->webUSB->GetValue()) compileCommand += "usb=cdc_webusb";
    else if (editor->generalPage->massStorage->GetValue()) compileCommand += "usb=cdc_msc";
    else compileCommand += "usb=cdc";
    if (editor->generalPage->board->entry()->GetSelection() == PROFFIEBOARDV3) compileCommand +=",dosfs=sdmmc1";
    compileCommand += " " + Paths::proffieos().string() + " -v";
    FILE *arduinoCli = Arduino::cli(compileCommand);

    string error{};
#   ifdef __WINDOWS__
    array<string, 2> paths;
#   endif
    while(fgets(buffer, sizeof(buffer), arduinoCli) != nullptr) {
        if (progDialog != nullptr) progDialog->emitEvent(-1, ""); // Pulse
        error += buffer;

        if (std::strstr(buffer, "error")) {
            _return = Arduino::parseError(error);
# 	        ifdef __WINDOWS__
            return nullopt;
# 	        else
            return false;
#           endif
        }

#   	ifdef __WINDOWS__
        if (std::strstr(buffer, "ProffieOS.ino.dfu") && std::strstr(buffer, "stm32l4") && std::strstr(buffer, "C:\\")) {
            array<string, 2> paths;
            array<char, MAX_PATH> shortPath;
            string pathsString{buffer};
            GetShortPathNameA(pathsString.substr(pathsString.rfind("C:\\"), pathsString.rfind("ProffieOS.ino.dfu") - pathsString.rfind("C:\\") + 17).c_str(), shortPath.data(), shortPath.size());
            paths[0] = shortPath.data();
            GetShortPathNameA(pathsString.substr(1, pathsString.find("windows") + 7 - 1).c_str(), shortPath.data(), shortPath.size());
            paths[1] = string{shortPath.data()} + R"(\\stm32l4-upload.bat)";
        }
#   	endif
    }
    if (pclose(arduinoCli) != 0) {
        _return = "Unknown Compile Error";
# 	    ifdef __WINDOWS__
        return nullopt;
# 	    else
        return false;
#       endif
    }

    _return = error;
# 	ifdef __WINDOWS__
    return paths;
# 	else
    return true;
#	endif
}

bool Arduino::upload(std::string& _return, EditorWindow* editor, Progress* progDialog) {
    char buffer[1024];

    string uploadCommand = "upload ";
    uploadCommand += Paths::proffieos().string();
    uploadCommand += " --board-options ";
    if (editor->generalPage->massStorage->GetValue() && editor->generalPage->webUSB->GetValue()) uploadCommand += "usb=cdc_msc_webusb";
    else if (editor->generalPage->webUSB->GetValue()) uploadCommand += "usb=cdc_webusb";
    else if (editor->generalPage->massStorage->GetValue()) uploadCommand += "usb=cdc_msc";
    else uploadCommand += "usb=cdc";
    if (editor->generalPage->board->entry()->GetStringSelection() == "ProffieBoard V3") uploadCommand +=",dosfs=sdmmc1";

    uploadCommand += " --fqbn ";
    switch (static_cast<Proffieboard>(editor->generalPage->board->entry()->GetSelection())) {
        case PROFFIEBOARDV3:
            uploadCommand += ARDUINOCORE_PBV3;
            break;
        case PROFFIEBOARDV2:
            uploadCommand += ARDUINOCORE_PBV2;
            break;
        case PROFFIEBOARDV1:
            uploadCommand += ARDUINOCORE_PBV1;
            break;
        default: 
            abort();
    }
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

    FILE *arduinoCli = Arduino::cli(uploadCommand);

    std::string error;
    while(fgets(buffer, sizeof(buffer), arduinoCli) != nullptr) {
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

bool Arduino::updateIno(std::string& _return, EditorWindow* _editor) {
    const auto inoPath{Paths::proffieos() / "ProffieOS.ino"};
    std::ifstream input(inoPath);
    if (!input.is_open()) {
        _return = "ERROR OPENING FOR READ";
        return false;
    }

    string fileData;
    vector<string> outputData;
    while(!input.eof()) {
        getline(input, fileData);

        if (
                fileData.find(R"(// #define CONFIG_FILE "config/YOUR_CONFIG_FILE_NAME_HERE.h")") != std::string::npos or
                fileData.find(R"(#define CONFIG_FILE)") == 0
           ) {
            outputData.emplace_back("#define CONFIG_FILE \"config/" + std::string{_editor->getOpenConfig()} + ".h\"");
            if (fileData.find(R"(#define CONFIG_FILE)") == 0) continue;
        } 
        /* else if (fileData.find(R"(const char version[] = ")" ) != std::string::npos) {
            outputData.push_back(R"(const char version[] = ")" wxSTRINGIZE(PROFFIEOS_VERSION) R"(";)");
        } */ 
        else {
            outputData.emplace_back(fileData);
        }
    }
    input.close();


    std::ofstream output(inoPath);
    if (!output.is_open()) {
        _return = "ERROR OPENING FOR WRITE";
        return false;
    }

    for (const string& line : outputData) {
        output << line << std::endl;
    }
    output.close();

    _return.clear();
    return true;
}

string Arduino::parseError(const string& error) {
    std::cerr << "An arduino task failed with the following error: " << std::endl;
    std::cerr << error << std::endl;

#	define ERRCONTAINS(token) std::strstr(error.data(), token)
    if (ERRCONTAINS("select Proffieboard")) return "Please ensure you've selected the correct board in General";
    if (ERRCONTAINS("expected unqualified-id")) return "Please make sure there are no brackets in your styles (such as \"{\" or \"}\")\n and there is nothing missing or extra from your style! (such as parentheses or \"<>\")";
    if (ERRCONTAINS(/* region FLASH */"overflowed")) {
        const auto maxBytes = error.find("ProffieboardV3") != std::string::npos ? 507904 : 262144;
        constexpr string_view OVERFLOW_PREFIX{"region `FLASH' overflowed by "};

        const auto overflowPos{error.rfind(OVERFLOW_PREFIX)};
        std::stringstream errMessage;
        if (overflowPos != std::string::npos) {
            const auto overflowBytes{std::stoi(error.substr(overflowPos + OVERFLOW_PREFIX.length()))};
            const auto percent{(overflowBytes * 100.0 / maxBytes) + 100.0};

            errMessage << "The specified config uses " << percent << "% of board space, and will not fit on the Proffieboard. (" << overflowBytes << " overflow)";
        } else {
            errMessage << "The specified config will not fit on the Proffieboard.";
        }

        errMessage << "\n\nTry disabling diagnostic commands, disabling talkie, disabling prop features, or removing blade styles to make it fit.";
        return errMessage.str();
    }
    if (ERRCONTAINS("Serial port busy")) return "The Proffieboard appears busy. \nPlease make sure nothing else is using it, then try again.";
    if (ERRCONTAINS("Buttons for operation")) return string{"Selected prop file "} + std::strstr(error.data(), "requires");
    if (ERRCONTAINS("1\n2\n3\n4\n5\n6\n7\n8\n9\n10")) return "Could not connect to Proffieboard for upload.";
    if (ERRCONTAINS("10\n9\n8\n7\n6\n5\n4\n3\n2\n1")) return "Could not connect to Proffieboard for upload.";
    if (ERRCONTAINS("No DFU capable USB device available")) return "No Proffieboard in BOOTLOADER mode found.";
    if (ERRCONTAINS("error:")) {
        const auto errPos{error.find("error:")};
        const auto fileData{error.rfind('/', errPos)};
        return error.substr(fileData + 1);
    }

    return "Unknown error: " + error;
#	undef ERRCONTAINS
}

FILE* Arduino::cli(const string& command) {
    auto& logger{Log::Context::getGlobal().createLogger("Arduino::cli()")};
    string fullCommand;
#   if defined(__WINDOWS__)
    fullCommand += "title ProffieConfig Worker & ";
    fullCommand += (Paths::binaries() / "windowMode").string() + R"( -title "ProffieConfig Worker" -mode force_minimized & )";
#   endif
    fullCommand += (Paths::binaries() / "arduino-cli").string();
    fullCommand += " " + command;
    fullCommand += " 2>&1";

    logger.debug("Executing command \"" + fullCommand + '"');
    return popen(fullCommand.data(), "r");
}
