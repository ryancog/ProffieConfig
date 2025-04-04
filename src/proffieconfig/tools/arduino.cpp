#include "arduino.h"
#include "wx/translation.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <cstring>
#include <filesystem>
#include <sstream>
#include <thread>
#include <unistd.h>

#ifdef __WINDOWS__
#include <windows.h>
#else
#include <termios.h>
#endif

#include <wx/gdicmn.h>
#include <wx/uri.h>
#include <wx/webrequest.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/zstream.h>

#include "log/context.h"
#include "log/logger.h"
#include "utils/paths.h"
#include "utils/types.h"
#include "utils/defer.h"

#include "../core/defines.h"
#include "../core/config/configuration.h"
#include "../core/config/propfile.h"
#include "../core/utilities/misc.h"
#include "../core/utilities/progress.h"
#include "../editor/editorwindow.h"
#include "../editor/pages/generalpage.h"
#include "../editor/pages/propspage.h"
#include "../editor/pages/bladespage.h"
#include "../editor/pages/presetspage.h"
#include "../editor/dialogs/bladearraydlg.h"

using namespace std::chrono_literals;

namespace Arduino {
    FILE *cli(const string& command);

    bool updateIno(string&, EditorWindow*, Log::Branch&);
#   ifdef __WINDOWS__
    optional<array<string, 2>> compile(string&, EditorWindow *, Progress *, Log::Branch&);
    bool upload(string&, EditorWindow *, Progress *, const array<string, 2>&, Log::Branch&);
#   else
    bool compile(string&, EditorWindow *, Progress *, Log::Branch&);
    bool upload(string&, EditorWindow *, Progress *, Log::Branch&);
#   endif
    string parseError(const string&, EditorWindow *);

    wxDEFINE_EVENT(EVT_INIT_DONE, Event);
    wxDEFINE_EVENT(EVT_APPLY_DONE, Event);
    wxDEFINE_EVENT(EVT_VERIFY_DONE, Event);
    wxDEFINE_EVENT(EVT_REFRESH_DONE, Event);
    wxDEFINE_EVENT(EVT_CLEAR_BLIST, Event);
    wxDEFINE_EVENT(EVT_APPEND_BLIST, Event);

    constexpr auto MAX_ERRMESSAGE_LENGTH{1024};

#   ifdef __WINDOWS__
    inline string windowModePrefix() { 
        return 
            "title ProffieConfig Worker & " + 
            (Paths::binaries() / "windowMode").string() + 
            R"( -title "ProffieConfig Worker" -mode force_minimized & )";
    }
#   endif

} // namespace Arduino

void Arduino::init(wxWindow *parent) {
    auto& logger{Log::Context::getGlobal().createLogger("Arduino::init()")};
    auto *progDialog{new Progress(parent)};
    progDialog->SetTitle("Dependency Installation");

    std::thread{[progDialog, parent, &logger]() {
        auto *const evt{new Arduino::Event(Arduino::EVT_INIT_DONE)};
        FILE *install{nullptr};
        string fulloutput;
        array<char, 32> buffer;

        constexpr cstring DOWNLOAD_MESSAGE{"Downloading ProffieOS..."};
        progDialog->emitEvent(5, DOWNLOAD_MESSAGE);
        logger.info(DOWNLOAD_MESSAGE);
        const auto uri{wxURI{Paths::remoteAssets() + "/ProffieOS/" wxSTRINGIZE(PROFFIEOS_VERSION) ".zip"}.BuildURI()};
        auto proffieOSRequest{wxWebSessionSync::GetDefault().CreateRequest(uri)};
        auto requestResult{proffieOSRequest.Execute()};

        if (not requestResult) {
            progDialog->emitEvent(100, "Error");
            const auto downloadFailMessage{"ProffieOS Download Failed\n" + requestResult.error.ToStdString()};
            evt->str = downloadFailMessage;
            logger.error(downloadFailMessage);
            wxQueueEvent(parent->GetEventHandler(), evt);
            return;
        }

        fs::remove_all(Paths::proffieos());

        wxZipInputStream zipStream{*proffieOSRequest.GetResponse().GetStream()};
        if (not zipStream.IsOk()) {
            progDialog->emitEvent(100, "Error");
            constexpr cstring ZIP_ERROR_MESSAGE{"Failed Opening ProffieOS ZIP"};
            evt->str = ZIP_ERROR_MESSAGE;
            logger.error(ZIP_ERROR_MESSAGE);
            wxQueueEvent(parent->GetEventHandler(), evt);
            return;
        }

        std::unique_ptr<wxZipEntry> entry;
        while (entry.reset(zipStream.GetNextEntry()), entry) {
            auto fileNameStr{(Paths::proffieos() / entry->GetName().ToStdWstring()).string()};
            if (fileNameStr.find("__MACOSX") != string::npos) continue;

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
                constexpr cstring READ_ERROR_MESSAGE{"ProffieOS Read Failed"};
                evt->str = READ_ERROR_MESSAGE;
                logger.error(READ_ERROR_MESSAGE);
                wxQueueEvent(parent->GetEventHandler(), evt);
                return;
            }
            
            wxFileOutputStream outStream{fileNameStr};
            if (not outStream.IsOk()) {
                progDialog->emitEvent(100, "Error");
                constexpr cstring WRITE_ERROR_MESSAGE{"ProffieOS Write Failed"};
                evt->str = WRITE_ERROR_MESSAGE;
                logger.error(WRITE_ERROR_MESSAGE);
                wxQueueEvent(parent->GetEventHandler(), evt);
                return;
            }
            
            zipStream.Read(outStream);
        }

        if (zipStream.GetLastError() != wxSTREAM_EOF) {
            progDialog->emitEvent(100, "Error");
            constexpr cstring PARSE_ERROR_MESSAGE{"Failed Parsing ProffieOS ZIP"};
            evt->str = PARSE_ERROR_MESSAGE;
            logger.error(PARSE_ERROR_MESSAGE);
            wxQueueEvent(parent->GetEventHandler(), evt);
            return;
        }
        
        progDialog->emitEvent(30, "Downloading dependencies...");
        install = Arduino::cli("core install proffieboard:stm32l4@" ARDUINO_PBPLUGIN_VERSION " --additional-urls https://profezzorn.github.io/arduino-proffieboard/package_proffieboard_index.json");
        while (fgets(buffer.data(), buffer.size(), install) != nullptr) { 
            progDialog->emitEvent(-1, ""); 
            fulloutput += buffer.data(); 
        }
        if (pclose(install)) {
            progDialog->emitEvent(100, "Error");
            const auto coreInstallErrorMessage{"Core install failed:\n\n" + fulloutput};
            evt->str = '\n' + coreInstallErrorMessage;
            logger.error(coreInstallErrorMessage);
            wxQueueEvent(parent->GetEventHandler(), evt);
            return;
        }

#       ifndef __WXOSX__
        constexpr cstring INSTALL_DRIVER_MESSAGE{"Installing drivers..."};
        progDialog->emitEvent(60, INSTALL_DRIVER_MESSAGE);
        logger.info(INSTALL_DRIVER_MESSAGE);

#       if defined(__linux__)
        install = popen("pkexec cp ~/.arduino15/packages/proffieboard/hardware/stm32l4/3.6/drivers/linux/*rules /etc/udev/rules.d", "r");
#       elif defined(__WINDOWS__)
        install = _wpopen((L"title ProffieConfig Worker & " + (Paths::binaries() / "proffie-dfu-setup.exe").native() + L" 2>&1").c_str(), L"r");
        // Really I should have a proper wait but I tried with an echo and that didn't work.
        // Could maybe revisit this in the future.
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        STARTUPINFOW startupInfo;
        PROCESS_INFORMATION procInfo;
        memset(&startupInfo, 0, sizeof startupInfo);
        startupInfo.cb = sizeof startupInfo;
        memset(&procInfo, 0, sizeof procInfo);

        const auto minimizeCommand{(Paths::binaries() / "windowMode").native() + LR"( -title "ProffieConfig Worker" -mode force_minimized)"};

        CreateProcessW(
            minimizeCommand.c_str(),
            nullptr,
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
        while (fgets(buffer.data(), buffer.size(), install) != nullptr) { 
            progDialog->emitEvent(-1, ""); 
            fulloutput += buffer.data(); 
        }
        if (pclose(install)) {
            progDialog->emitEvent(100, "Error");
            const auto driverInstallErrorMessage{"Driver install failed:\n\n" + fulloutput};
            std::cerr << fulloutput << std::endl;
            evt->str = '\n' + driverInstallErrorMessage;
            logger.error(driverInstallErrorMessage);
            wxQueueEvent(parent, evt);
            return;
        }
#       endif

        progDialog->emitEvent(100, "Done.");
        logger.info("Done");
        evt->succeeded = true;
        wxQueueEvent(parent->GetEventHandler(), evt);
    }}.detach(); // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
}

void Arduino::refreshBoards(MainMenu* window) {
    auto& logger{Log::Context::getGlobal().createLogger("Arduino::refreshBoards()")};

    auto *progDialog{new Progress(window)};
    progDialog->SetTitle("Device Update");

    auto lastSelection{window->boardSelect->entry()->GetStringSelection()};
    std::thread thread([progDialog, window, lastSelection, &logger]() {
        progDialog->emitEvent(0, "Initializing...");

        wxSetCursor(wxCURSOR_WAIT);
        Defer deferCursor{[]() { wxSetCursor(wxNullCursor); }};

        wxQueueEvent(window, new Event(EVT_CLEAR_BLIST));
        constexpr auto FETCH_MESSAGE{"Fetching Devices..."};
        progDialog->emitEvent(20, "Fetching devices...");
        auto boards{Arduino::getBoards(*logger.binfo(FETCH_MESSAGE))};
        for (const wxString& item : boards) {
            auto *evt{new Event(EVT_APPEND_BLIST)};
            evt->str = item.ToStdString();
            if (&item != &*boards.begin())logger.debug("Discovered board: " + item.ToStdString());
            wxQueueEvent(window, evt);
        }

        auto *evt{new Event(EVT_REFRESH_DONE)};
        evt->str = lastSelection.ToStdString();
        wxQueueEvent(window, evt);
        progDialog->emitEvent(100, "Done.");
        logger.info("Done");
    });
    thread.detach();
}

vector<wxString> Arduino::getBoards(Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Arduino::getBoards()")};

    vector<wxString> boards{"Select Board..."};
    array<char, 32> buffer;

    FILE *arduinoCli = Arduino::cli("board list");

    if (!arduinoCli) {
        return boards;
    }

    struct Result {
        Result(string port, bool isProffie) :
            port{std::move(port)}, isProffie{isProffie} {}
        string port;
        bool isProffie{false};
    };

    std::vector<Result> results;

    string output;
    while (fgets(buffer.data(), buffer.size(), arduinoCli) != nullptr) {
        output += buffer.data();
    }

    auto lineEndPos{output.find('\n')};
    while (not false) {
        const auto line{output.substr(0, lineEndPos)};

        if (line.find("No boards found.") != string::npos) {
            logger.info("No boards found.");
            break;
        }

        if (line[0] == ' ' or line[0] == '\t') {
            if (results.empty()) continue;

            if (line.find("proffieboard") != string::npos) {
                results.back().isProffie = true;
            }
        } else if (line.find("serial") != string::npos) {
            const auto port{line.substr(0, line.find_first_of(" \t"))};
            const auto isProffie{line.find("proffieboard") != string::npos};
            results.emplace_back(port, isProffie);
            logger.debug("Found board: " + port);
        } else if (line.find("dfu") != string::npos) {
            const auto port{line.substr(0, line.find_first_of(" \t"))};
            boards.emplace_back(_("BOOTLOADER") + '|' + port);
            logger.debug("Found board in bootloader mode: " + port);
        }

        if (lineEndPos == string::npos) break;
        output = output.substr(lineEndPos + 1);
        lineEndPos = output.find('\n');
    }

    for (const auto& result : results) {
        if (not result.isProffie) continue;

        logger.info("Reporting board: " + result.port);
        boards.emplace_back(result.port);
    }

#   ifdef __WINDOWS__
    boards.emplace_back(_("BOOTLOADER RECOVERY"));
#   endif
    return boards;
}

void Arduino::applyToBoard(MainMenu* window, EditorWindow* editor) {
    auto& logger{Log::Context::getGlobal().createLogger("Arduino::applyToBoard()")};

    auto *progDialog{new Progress(window)};
    progDialog->SetTitle("Applying Changes");

    editor->presetsPage->update();
    editor->bladesPage->update();
    editor->bladesPage->bladeArrayDlg->update();

    std::thread thread{[progDialog, window, editor, &logger]() {
        progDialog->emitEvent(0, "Initializing...");

        wxSetCursor(wxCURSOR_WAIT);
        Defer deferCursor{[]() { wxSetCursor(wxNullCursor); }};

        auto *evt{new Event(EVT_APPLY_DONE)};
        string returnVal;

        constexpr cstring CHECK_PRESENCE_MESSAGE{"Checking board presence..."};
        progDialog->emitEvent(10, CHECK_PRESENCE_MESSAGE);
        wxString lastSel = window->boardSelect->entry()->GetStringSelection();
        window->boardSelect->entry()->Clear();
        for (const wxString& item : Arduino::getBoards(*logger.binfo(CHECK_PRESENCE_MESSAGE))) {
            window->boardSelect->entry()->Append(item);
        }

        window->boardSelect->entry()->SetStringSelection(lastSel);
        if (window->boardSelect->entry()->GetSelection() == -1) {
            window->boardSelect->entry()->SetSelection(0);
            progDialog->emitEvent(100, "Error!");
            auto* msg{new Misc::MessageBoxEvent(wxID_ANY, _("Please make sure your board is connected and selected, then try again!"), _("Board Selection Error"), wxOK | wxICON_ERROR)};
            logger.warn("Board was not found.");
            wxQueueEvent(window, msg);
            wxQueueEvent(window, evt);
            return;
        }

        constexpr cstring GENERATE_MESSAGE{"Generating configuration file..."};
        progDialog->emitEvent(20, GENERATE_MESSAGE);
        const auto configPath{Paths::proffieos() / "config" / (string{editor->getOpenConfig()} + ".h")};
        if (not Configuration::outputConfig(configPath, editor, logger.binfo(GENERATE_MESSAGE), true)) {
            progDialog->emitEvent(100, "Error");
            // NO message here because outputConfig will handle it.
            wxQueueEvent(window, evt);
            return;
        }

        constexpr cstring UPDATE_INO_MESSAGE{"Updating ProffieOS file..."};
        progDialog->emitEvent(30, UPDATE_INO_MESSAGE);
        if (not Arduino::updateIno(returnVal, editor, *logger.binfo(UPDATE_INO_MESSAGE))) {
            progDialog->emitEvent(100, "Error");
            auto* msg{new Misc::MessageBoxEvent{
                wxID_ANY, 
                wxString::Format(_("There was an error while updating ProffieOS file:\n\n%s"), returnVal.substr(0, MAX_ERRMESSAGE_LENGTH)),
                _("Files Error")
            }};
            wxQueueEvent(window, msg);
            wxQueueEvent(window, evt);
            return;
        }

        constexpr cstring COMPILE_MESSAGE{"Compiling ProffieOS..."};
        progDialog->emitEvent(40, COMPILE_MESSAGE);
#       ifdef __WINDOWS__
        auto paths{Arduino::compile(returnVal, editor, progDialog, *logger.binfo(COMPILE_MESSAGE))};
        if (not paths) {
#       else
        if (not Arduino::compile(returnVal, editor, progDialog, *logger.binfo(COMPILE_MESSAGE))) {
#       endif
            progDialog->emitEvent(100, "Error");
            auto* msg{new Misc::MessageBoxEvent{
                wxID_ANY,
                wxString::Format(_("There was an error while compiling:\n\n%s"), returnVal.substr(0, MAX_ERRMESSAGE_LENGTH)),
                _("Compile Error")
            }};
            wxQueueEvent(window, msg);
            wxQueueEvent(window, evt);
            return;
        }

        constexpr cstring UPLOAD_MESSAGE{"Uploading to Proffieboard..."};
        progDialog->emitEvent(65, UPLOAD_MESSAGE);
#       ifdef __WINDOWS__
        if (not Arduino::upload(returnVal, editor, progDialog, *paths, *logger.binfo(UPLOAD_MESSAGE))) {
#       else
        if (not Arduino::upload(returnVal, editor, progDialog, *logger.binfo(UPLOAD_MESSAGE))) {
#       endif
            progDialog->emitEvent(100, "Error");
            auto* msg{new Misc::MessageBoxEvent{
                wxID_ANY, 
                wxString::Format(_("There was an error while uploading:\n\n%s"), returnVal.substr(0, MAX_ERRMESSAGE_LENGTH)),
                _("Upload Error"),
            }};
            wxQueueEvent(window, msg);
            wxQueueEvent(window, evt);
            return;
        }

        progDialog->emitEvent(100, "Done.");
        logger.info("Applied Successfully");

        auto* msg{new Misc::MessageBoxEvent{
            wxID_ANY,
            _("Changes Successfully Applied to ProffieBoard!"),
            _("Apply Changes to Board"),
            wxOK | wxICON_INFORMATION
        }};
        wxQueueEvent(window, msg);
        evt->succeeded = true;
        wxQueueEvent(window, evt);
    }};
    thread.detach();
}

void Arduino::verifyConfig(wxWindow* parent, EditorWindow* editor) {
    auto& logger{Log::Context::getGlobal().createLogger("Arduino::verifyConfig()")};

    auto *progDialog{new Progress(parent)};
    progDialog->SetTitle("Verify Config");

    editor->presetsPage->update();
    editor->bladesPage->update();
    editor->bladesPage->bladeArrayDlg->update();

    std::thread thread{[progDialog, editor, parent, &logger]() {
        progDialog->emitEvent(0, "Initializing...");

        wxSetCursor(wxCURSOR_WAIT);
        Defer deferCursor{[]() { wxSetCursor(wxNullCursor); }};

        auto *evt{new Event(EVT_VERIFY_DONE)};
        string returnVal;

        constexpr cstring GENERATE_MESSAGE{"Generating configuration file..."};
        progDialog->emitEvent(20, GENERATE_MESSAGE);
        const auto configPath{Paths::proffieos() / "config" / (string{editor->getOpenConfig()} + ".h")};
        if (not Configuration::outputConfig(configPath, editor, logger.binfo(GENERATE_MESSAGE), true)) {
            progDialog->emitEvent(100, "Error");
            // Outputconfig will handle error message
            wxQueueEvent(parent, evt);
            return;
        }

        constexpr cstring UPDATE_INO_MESSAGE{"Updating ProffieOS file..."};
        progDialog->emitEvent(30, UPDATE_INO_MESSAGE);
        if (not Arduino::updateIno(returnVal, editor, *logger.binfo(UPDATE_INO_MESSAGE))) {
            progDialog->emitEvent(100, "Error");
            auto* msg{new Misc::MessageBoxEvent{
                wxID_ANY, 
                wxString::Format(_("There was an error while updating ProffieOS file:\n\n%s"), returnVal.substr(0, MAX_ERRMESSAGE_LENGTH)),
                _("Files Error")
            }};
            wxQueueEvent(parent, msg);
            wxQueueEvent(parent, evt);
            return;
        }

        constexpr cstring COMPILE_MESSAGE{"Compiling ProffieOS..."};
        progDialog->emitEvent(40, COMPILE_MESSAGE);
        if (not Arduino::compile(returnVal, editor, progDialog, *logger.binfo(COMPILE_MESSAGE))) {
            progDialog->emitEvent(100, "Error");
            auto* msg{new Misc::MessageBoxEvent{
                wxID_ANY,
                wxString::Format(_("There was an error while compiling:\n\n%s"), returnVal.substr(0, MAX_ERRMESSAGE_LENGTH)),
                _("Compile Error")
            }};
            wxQueueEvent(parent, msg);
            wxQueueEvent(parent, evt);
            return;
        }

        constexpr string_view USED_PREFIX{"Sketch uses "};
        constexpr string_view MAX_PREFIX{"Maximum is "};
        const auto usedPos{returnVal.find(USED_PREFIX.data())};
        const auto maxPos{returnVal.find(MAX_PREFIX.data())};

        constexpr cstring SUCCESS_MESSAGE{wxTRANSLATE("Config Verified Successfully!")};
        wxString message;
        if (usedPos != string::npos and maxPos != string::npos) {
            const auto usedBytes{std::stoi(returnVal.substr(usedPos + USED_PREFIX.length()))};
            const auto maxBytes{std::stoi(returnVal.substr(maxPos + MAX_PREFIX.length()))};

            auto percent{std::round(usedBytes * 10000.0 / maxBytes) / 100.0};

            constexpr cstring USAGE_MESSAGE{wxTRANSLATE("The configuration uses %d%% of board space. (%d/%d)")};
            logger.info(wxString::Format(
                "%s %s",
                SUCCESS_MESSAGE,
                wxString::Format(
                    USAGE_MESSAGE,
                    percent,
                    usedBytes,
                    maxBytes
                )
            ).ToStdString());

            message = wxString::Format(
                "%s\n\n%s",
                wxGetTranslation(SUCCESS_MESSAGE),
                wxString::Format(
                    wxGetTranslation(USAGE_MESSAGE),
                    percent,
                    usedBytes,
                    maxBytes
                )
            );
        } else {
            message = wxGetTranslation(SUCCESS_MESSAGE);
            logger.info(SUCCESS_MESSAGE);
        }

        progDialog->emitEvent(100, "Done.");
        auto* msg{new Misc::MessageBoxEvent(wxID_ANY, message, _("Verify Config"), wxOK | wxICON_INFORMATION)};
        wxQueueEvent(parent, msg);
        evt->succeeded = true;
        wxQueueEvent(parent, evt);
    }};
    thread.detach();
}

#ifdef __WINDOWS__
optional<array<string, 2>> Arduino::compile(string& _return, EditorWindow* editor, Progress* progDialog, Log::Branch& lBranch) {
#else
bool Arduino::compile(string& _return, EditorWindow* editor, Progress* progDialog, Log::Branch& lBranch) {
#endif
    auto& logger{lBranch.createLogger("Arduino::compile()")};

    wxString output;
    array<char, 32> buffer;

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
    compileCommand += " \"" + Paths::proffieos().string() + "\" -v";
    FILE *arduinoCli = Arduino::cli(compileCommand);

    string error{};
#   ifdef __WINDOWS__
    array<string, 2> paths;
#   endif
    while(fgets(buffer.data(), buffer.size(), arduinoCli) != nullptr) {
        if (progDialog != nullptr) progDialog->emitEvent(-1, ""); // Pulse
        error += buffer.data();

    }

    if (error.find("error") != string::npos) {
        _return = Arduino::parseError(error, editor);
        logger.error(_return);
# 	    ifdef __WINDOWS__
        return nullopt;
# 	    else
        return false;
#       endif
    }

    if (pclose(arduinoCli) != 0) {
        _return = parseError(error, editor);
        logger.error("Error during pclose(): \n" + error);
# 	    ifdef __WINDOWS__
        return nullopt;
# 	    else
        return false;
#       endif
    }

#   ifdef __WINDOWS__
    if (
            error.find("ProffieOS.ino.dfu") != string::npos and
            error.find("stm32l4") != string::npos and
            error.find("C:\\") != string::npos
       ) {
        logger.debug("Parsing utility paths...");
        array<char, MAX_PATH> shortPath;

        constexpr string_view DFU_STRING{"ProffieOS.ino.dfu"};
        const auto dfuPos{error.rfind(DFU_STRING.data())};
        const auto dfuCPos{error.rfind("C:\\", dfuPos)};
        GetShortPathNameA(error.substr(dfuCPos, dfuPos - dfuCPos + DFU_STRING.length()).c_str(), shortPath.data(), shortPath.size());
        paths[0] = shortPath.data();
        logger.debug("Parsed dfu file: " + paths[0]);

        const auto dfuSuffixPos{error.rfind("//dfu-suffix.exe")};
        const auto dfuSuffixCPos{error.rfind("C:\\", dfuSuffixPos)};
        GetShortPathNameA(error.substr(dfuSuffixCPos, dfuSuffixPos - dfuSuffixCPos).c_str(), shortPath.data(), shortPath.size());
        paths[1] = string{shortPath.data()} + "\\stm32l4-upload.bat";
        logger.debug("Parsed upload file: " + paths[1]);
    }
#   endif

    _return = error;
    logger.info("Successful compile.");
# 	ifdef __WINDOWS__
    if (paths[0].empty() or paths[1].empty()) {
        logger.error("Failed to find utilities in output: " + error);
        _return = "Failed to find required utilities";
        return nullopt;
    }
    return paths;
# 	else
    return true;
#	endif
}

#ifdef __WINDOWS__
bool Arduino::upload(string& _return, EditorWindow *editor, Progress *progDialog, const array<string, 2>& paths, Log::Branch& lBranch) {
#else
bool Arduino::upload(string& _return, EditorWindow* editor, Progress* progDialog, Log::Branch& lBranch) {
#endif
    auto& logger{lBranch.createLogger("Arduino::upload()")};

    array<char, 32> buffer;

#   ifndef __WINDOWS__
    string uploadCommand = "upload ";
    uploadCommand += '"' + Paths::proffieos().native() + '"';
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
#   endif

#   ifdef __WINDOWS__
    auto boardPath{static_cast<MainMenu *>(editor->GetParent())->boardSelect->entry()->GetStringSelection()};
    if (boardPath != _("BOOTLOADER RECOVERY")) {
        // See https://learn.microsoft.com/en-us/windows/win32/fileio/naming-a-file#win32-device-namespaces
        boardPath = R"(\\.\)" + boardPath;
        progDialog->emitEvent(50, "Rebooting Proffieboard...");

        auto *serialHandle{CreateFileW(boardPath, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)};
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
#   else 
    const auto boardPath{static_cast<MainMenu*>(editor->GetParent())->boardSelect->entry()->GetStringSelection().ToStdString()};
    if (boardPath.find(_("BOOTLOADER").c_str()) == string::npos) {
        progDialog->emitEvent(-1, "Rebooting Proffieboard...");
        struct termios newtio;
        auto fd = open(boardPath.c_str(), O_RDWR | O_NOCTTY);
        if (fd < 0) {
            _return = "Failed to connect to board for reboot.";
            logger.error(_return);
            return false;
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
    }
#endif

#   ifndef __WINDOWS__
    FILE *arduinoCli = Arduino::cli(uploadCommand);

    string error;
    progDialog->emitEvent(-1, "Uploading to Proffieboard...");
    while(fgets(buffer.data(), buffer.size(), arduinoCli) != nullptr) {
        auto *const percentPtr{std::strstr(buffer.data(), "%")};
        if (percentPtr != nullptr and progDialog != nullptr and percentPtr - buffer.data() >= 3) {
            auto percent{strtol(percentPtr - 3, nullptr, 10)};
            progDialog->emitEvent(static_cast<int8>(percent), "");
        }

        error += buffer.data();
    }

    if (error.rfind("error") != string::npos || error.rfind("FAIL") != string::npos) {
        _return = Arduino::parseError(error, editor);
        logger.error(_return);
        return false;
    }

    if (pclose(arduinoCli) != 0) {
        _return = "Unknown Upload Error";
        logger.error("Error during pclose(): \n" + error);
        return false;
    }
#   else
    string commandString{windowModePrefix()};
    commandString += paths[1] + R"( 0x1209 0x6668 )" + paths[0] + R"( 2>&1)";
    logger.info("Uploading via: " + commandString);

    auto *upload{popen(commandString.c_str(), "r")};
    string error{};
    progDialog->emitEvent(-1, "");
    while (fgets(buffer.data(), buffer.size(), upload) != nullptr) {
        auto *const percentPtr{std::strstr(buffer.data(), "%")};
        if (percentPtr != nullptr and progDialog != nullptr and percentPtr - buffer.data() >= 3) {
            auto percent{strtol(percentPtr - 3, nullptr, 10)};
            progDialog->emitEvent(static_cast<int8>(percent), "");
        }

        error += buffer.data();
    }

    if (error.rfind("error") != string::npos || error.rfind("FAIL") != string::npos) {
        _return = Arduino::parseError(error, editor);
        logger.error(_return);
        return false;
    }

    if (pclose(upload) != 0) {
        _return = Arduino::parseError(error, editor);
        logger.error("Error during pclose(): \n" + error);
        return false;
    }

    if (error.find("File downloaded successfully") == string::npos) {
        progDialog->emitEvent(100, "Error");
        _return = Arduino::parseError(error, editor);
        logger.error(_return);
        return false;
    }
#   endif

    _return.clear();
    logger.info("Success");
    return true;
}

bool Arduino::updateIno(string& _return, EditorWindow* _editor, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Arduino::updateIno()")};

    const auto inoPath{Paths::proffieos() / "ProffieOS.ino"};
    std::ifstream input(inoPath);
    if (!input.is_open()) {
        constexpr cstring OPEN_READ_ERROR{"ERROR OPENING FOR READ"};
        logger.error(OPEN_READ_ERROR);
        _return = OPEN_READ_ERROR;
        return false;
    }

    string fileData;
    vector<wxString> outputData;
    while(!input.eof()) {
        getline(input, fileData);

        if (
                fileData.find(R"(// #define CONFIG_FILE "config/YOUR_CONFIG_FILE_NAME_HERE.h")") != string::npos or
                fileData.find(R"(#define CONFIG_FILE)") == 0
           ) {
            outputData.emplace_back("#define CONFIG_FILE \"config/" + wxString{_editor->getOpenConfig()} + ".h\"");
            if (fileData.find(R"(#define CONFIG_FILE)") == 0) continue;
        } 
        /* else if (fileData.find(R"(const char version[] = ")" ) != string::npos) {
            outputData.push_back(R"(const char version[] = ")" wxSTRINGIZE(PROFFIEOS_VERSION) R"(";)");
        } */ 
        else {
            outputData.emplace_back(fileData);
        }
    }
    input.close();


    std::ofstream output(inoPath);
    if (!output.is_open()) {
        constexpr cstring OPEN_WRITE_ERROR{"ERROR OPENING FOR WRITE"};
        logger.error(OPEN_WRITE_ERROR);
        _return = OPEN_WRITE_ERROR;
        return false;
    }

    for (const wxString& line : outputData) {
        output << line << std::endl;
    }
    output.close();

    _return.clear();
    return true;
}

string Arduino::parseError(const string& error, EditorWindow *editor) {
    std::cerr << "An arduino task failed with the following error: " << std::endl;
    std::cerr << error << std::endl;

#	define ERRCONTAINS(token) std::strstr(error.data(), token)
    if (ERRCONTAINS("select Proffieboard")) return "Please ensure you've selected the correct board in General";
    if (ERRCONTAINS("expected unqualified-id")) return "Please make sure there are no brackets in your styles (such as \"{\" or \"}\")\n and there is nothing missing or extra from your style! (such as parentheses or \"<>\")";
    if (ERRCONTAINS(/* region FLASH */"overflowed")) {
        const auto maxBytes = error.find("ProffieboardV3") != string::npos ? 507904 : 262144;
        constexpr string_view OVERFLOW_PREFIX{"region `FLASH' overflowed by "};

        const auto overflowPos{error.rfind(OVERFLOW_PREFIX.data())};
        std::ostringstream errMessage;
        if (overflowPos != string::npos) {
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

    auto *selectedProp{editor->propsPage->getSelectedProp()};
    if (selectedProp != nullptr) {
        for (const auto& [ arduino, display ] : selectedProp->getMappedErrors()) {
            if (error.find(arduino) != string::npos) return selectedProp->getName() + " prop error:\n" + display;
        }
    }

    if (ERRCONTAINS("error:")) {
        const auto errPos{error.find("error:")};
        const auto fileData{error.rfind('/', errPos)};
        return error.substr(fileData + 1);
    }

    return "Unknown error: " + error.substr(0, MAX_ERRMESSAGE_LENGTH);
#	undef ERRCONTAINS
}

FILE* Arduino::cli(const string& command) {
    auto& logger{Log::Context::getGlobal().createLogger("Arduino::cli()")};
    string fullCommand;
#   if defined(__WINDOWS__)
    fullCommand += windowModePrefix();
#   endif
    fullCommand += '"' + (Paths::binaries() / "arduino-cli").string() + '"';
    fullCommand += " " + command;
    fullCommand += " 2>&1";

    logger.debug("Executing command \"" + fullCommand + '"');
    return popen(fullCommand.data(), "r");
}
