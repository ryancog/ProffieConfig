#include "arduino.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2025 Ryan Ogurek
 *
 * proffieconfig/tools/arduino.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 4 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <cstring>
#include <filesystem>
#include <sstream>
#include <thread>

#ifdef __WINDOWS__
#include <fileapi.h>
#include <handleapi.h>
#else
#include <termios.h>
#endif

#include <wx/gdicmn.h>
#include <wx/translation.h>
#include <wx/uri.h>
#include <wx/webrequest.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/zstream.h>

#include "config/config.h"
#include "log/context.h"
#include "log/logger.h"
#include "utils/paths.h"
#include "utils/types.h"
#include "versions/versions.h"

#include "../core/utilities/progress.h"

using namespace std::chrono_literals;

namespace Arduino {

FILE *cli(const string& command);

struct CompileOutput {
    int32 used;
    int32 total;
#   ifdef __WXMSW__
    string tool1;
    string tool2;
#   endif
};

variant<CompileOutput, string> compile(
    const Config::Config&,
    Progress *,
    Log::Branch&
);

optional<string> upload(
    const string& boardPath,
    const Config::Config&,
    Progress *,
    Log::Branch&
);

string parseError(const string&, const Config::Config&); 

#ifdef __WINDOWS__
inline string windowModePrefix() { 
    return 
        "title ProffieConfig Worker & " + 
        (Paths::binaries() / "windowMode").string() + 
        R"( -title "ProffieConfig Worker" -mode force_minimized & )";
}
#endif

constexpr auto MAX_ERRMESSAGE_LENGTH{1024};
constexpr cstring ARDUINOCORE_PBV1{"proffieboard:stm32l4:Proffieboard-L433CC"};
constexpr cstring ARDUINOCORE_PBV2{"proffieboard:stm32l4:ProffieboardV2-L433CC"};
constexpr cstring ARDUINOCORE_PBV3{"proffieboard:stm32l4:ProffieboardV3-L452RE"};

} // namespace Arduino

string Arduino::version() {
    auto *arduinoCli{cli("version")};

    array<char, 32> buffer;
    string output;
    while (fgets(buffer.data(), buffer.size(), arduinoCli) != nullptr) {
        output += buffer.data();
    }

    constexpr cstring UNKNOWN_STR{wxTRANSLATE("Unknown")};
    constexpr string_view VERSION_TAG{"Version: "};
    const auto versionTagPos{output.find(VERSION_TAG)};
    if (versionTagPos == string::npos) return wxGetTranslation(UNKNOWN_STR).ToStdString();

    const auto versionStart{versionTagPos + VERSION_TAG.length()};
    const auto versionEnd{output.find(' ', versionStart)};
    if (versionEnd == string::npos) return wxGetTranslation(UNKNOWN_STR).ToStdString();

    return output.substr(versionStart, versionEnd - versionStart);
}

// void Arduino::init(wxWindow *parent) {
//     auto& logger{Log::Context::getGlobal().createLogger("Arduino::init()")};
//     auto *progDialog{new Progress(parent)};
//     progDialog->SetTitle("Dependency Installation");
// 
//     std::thread{[progDialog, parent, &logger]() {
//         auto *const evt{new Arduino::Event(Arduino::EVT_INIT_DONE)};
//         FILE *install{nullptr};
//         string fulloutput;
//         array<char, 32> buffer;
// 
//         // constexpr cstring DOWNLOAD_MESSAGE{"Downloading ProffieOS..."};
//         // progDialog->emitEvent(5, DOWNLOAD_MESSAGE);
//         // logger.info(DOWNLOAD_MESSAGE);
//         // const auto uri{wxURI{Paths::remoteAssets() + "/ProffieOS/" wxSTRINGIZE(PROFFIEOS_VERSION) ".zip"}.BuildURI()};
//         // auto proffieOSRequest{wxWebSessionSync::GetDefault().CreateRequest(uri)};
//         // auto requestResult{proffieOSRequest.Execute()};
// 
//         // if (not requestResult) {
//         //     progDialog->emitEvent(100, "Error");
//         //     const auto downloadFailMessage{"ProffieOS Download Failed\n" + requestResult.error.ToStdString()};
//         //     evt->str = downloadFailMessage;
//         //     logger.error(downloadFailMessage);
//         //     wxQueueEvent(parent->GetEventHandler(), evt);
//         //     return;
//         // }
// 
//         // fs::remove_all(Paths::proffieos());
// 
//         // wxZipInputStream zipStream{*proffieOSRequest.GetResponse().GetStream()};
//         // if (not zipStream.IsOk()) {
//         //     progDialog->emitEvent(100, "Error");
//         //     constexpr cstring ZIP_ERROR_MESSAGE{"Failed Opening ProffieOS ZIP"};
//         //     evt->str = ZIP_ERROR_MESSAGE;
//         //     logger.error(ZIP_ERROR_MESSAGE);
//         //     wxQueueEvent(parent->GetEventHandler(), evt);
//         //     return;
//         // }
// 
//         // std::unique_ptr<wxZipEntry> entry;
//         // while (entry.reset(zipStream.GetNextEntry()), entry) {
//         //     auto fileNameStr{(Paths::proffieos() / entry->GetName().ToStdWstring()).string()};
//         //     if (fileNameStr.find("__MACOSX") != string::npos) continue;
// 
//         //     auto permissionBits{entry->GetMode()};
//         //     wxFileName fileName;
//         //     
//         //     if (entry->IsDir()) fileName.AssignDir(fileNameStr);
//         //     else fileName.Assign(fileNameStr);
//         //     
//         //     if (!wxDirExists(fileName.GetPath())) {
//         //         wxFileName::Mkdir(fileName.GetPath(), permissionBits, wxPATH_MKDIR_FULL);
//         //     }
//         //     
//         //     if (entry->IsDir()) continue;
//         //     
//         //     if (not zipStream.CanRead()) {
//         //         progDialog->emitEvent(100, "Error");
//         //         constexpr cstring READ_ERROR_MESSAGE{"ProffieOS Read Failed"};
//         //         evt->str = READ_ERROR_MESSAGE;
//         //         logger.error(READ_ERROR_MESSAGE);
//         //         wxQueueEvent(parent->GetEventHandler(), evt);
//         //         return;
//         //     }
//         //     
//         //     wxFileOutputStream outStream{fileNameStr};
//         //     if (not outStream.IsOk()) {
//         //         progDialog->emitEvent(100, "Error");
//         //         constexpr cstring WRITE_ERROR_MESSAGE{"ProffieOS Write Failed"};
//         //         evt->str = WRITE_ERROR_MESSAGE;
//         //         logger.error(WRITE_ERROR_MESSAGE);
//         //         wxQueueEvent(parent->GetEventHandler(), evt);
//         //         return;
//         //     }
//         //     
//         //     zipStream.Read(outStream);
//         // }
// 
//         // if (zipStream.GetLastError() != wxSTREAM_EOF) {
//         //     progDialog->emitEvent(100, "Error");
//         //     constexpr cstring PARSE_ERROR_MESSAGE{"Failed Parsing ProffieOS ZIP"};
//         //     evt->str = PARSE_ERROR_MESSAGE;
//         //     logger.error(PARSE_ERROR_MESSAGE);
//         //     wxQueueEvent(parent->GetEventHandler(), evt);
//         //     return;
//         // }
//         
//         progDialog->emitEvent(30, "Downloading dependencies...");
//         install = Arduino::cli("core install proffieboard:stm32l4@" ARDUINO_PBPLUGIN_VERSION " --additional-urls https://profezzorn.github.io/arduino-proffieboard/package_proffieboard_index.json");
//         while (fgets(buffer.data(), buffer.size(), install) != nullptr) { 
//             progDialog->emitEvent(-1, ""); 
//             fulloutput += buffer.data(); 
//         }
//         if (pclose(install)) {
//             progDialog->emitEvent(100, "Error");
//             const auto coreInstallErrorMessage{"Core install failed:\n\n" + fulloutput};
//             evt->str = '\n' + coreInstallErrorMessage;
//             logger.error(coreInstallErrorMessage);
//             wxQueueEvent(parent->GetEventHandler(), evt);
//             return;
//         }
// 
// #       ifndef __WXOSX__
//         constexpr cstring INSTALL_DRIVER_MESSAGE{"Installing drivers..."};
//         progDialog->emitEvent(60, INSTALL_DRIVER_MESSAGE);
//         logger.info(INSTALL_DRIVER_MESSAGE);
// 
// #       if defined(__linux__)
//         install = popen("pkexec cp ~/.arduino15/packages/proffieboard/hardware/stm32l4/3.6/drivers/linux/*rules /etc/udev/rules.d", "r");
// #       elif defined(__WINDOWS__)
//         install = _wpopen((L"title ProffieConfig Worker & " + (Paths::binaries() / "proffie-dfu-setup.exe").native() + L" 2>&1").c_str(), L"r");
//         // Really I should have a proper wait but I tried with an echo and that didn't work.
//         // Could maybe revisit this in the future.
//         std::this_thread::sleep_for(std::chrono::milliseconds(500));
// 
//         STARTUPINFOW startupInfo;
//         PROCESS_INFORMATION procInfo;
//         memset(&startupInfo, 0, sizeof startupInfo);
//         startupInfo.cb = sizeof startupInfo;
//         memset(&procInfo, 0, sizeof procInfo);
// 
//         const auto minimizeCommand{(Paths::binaries() / "windowMode").native() + LR"( -title "ProffieConfig Worker" -mode force_minimized)"};
// 
//         CreateProcessW(
//             minimizeCommand.c_str(),
//             nullptr,
//             nullptr,
//             nullptr,
//             false,
//             0,
//             nullptr,
//             nullptr,
//             &startupInfo,
//             &procInfo
//         );
// #       endif
// 
//         fulloutput.clear();
//         while (fgets(buffer.data(), buffer.size(), install) != nullptr) { 
//             progDialog->emitEvent(-1, ""); 
//             fulloutput += buffer.data(); 
//         }
//         if (pclose(install)) {
//             progDialog->emitEvent(100, "Error");
//             const auto driverInstallErrorMessage{"Driver install failed:\n\n" + fulloutput};
//             std::cerr << fulloutput << std::endl;
//             evt->str = '\n' + driverInstallErrorMessage;
//             logger.error(driverInstallErrorMessage);
//             wxQueueEvent(parent, evt);
//             return;
//         }
// #       endif
// 
//         progDialog->emitEvent(100, "Done.");
//         logger.info("Done");
//         evt->succeeded = true;
//         wxQueueEvent(parent->GetEventHandler(), evt);
//     }}.detach(); // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
// }

vector<string> Arduino::getBoards(Log::Branch *lBranch) {
    auto& logger{Log::Branch::optCreateLogger("Arduino::getBoards()", lBranch)};

    vector<string> boards;
    array<char, 32> buffer;

    FILE *arduinoCli = Arduino::cli("board list");
    if (not arduinoCli) {
        return boards;
    }

    struct Result {
        Result(string port, bool isProffie) : port{std::move(port)}, isProffie{isProffie} {}
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
            boards.emplace_back(_("BOOTLOADER").ToStdString() + '|' + port);
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

    return boards;
}

variant<Arduino::Result, string> Arduino::applyToBoard(
    const string& boardPath,
    const Config::Config& config,
    Progress *prog
) {
    auto& logger{Log::Context::getGlobal().createLogger("Arduino::applyToBoard()")};

    auto res{compile(config, prog, *logger.binfo("Compiling..."))};
    if (auto *err = std::get_if<string>(&res)) return *err;
    const auto& compileOutput{std::get<CompileOutput>(res)};

    auto err{upload(boardPath, config, prog, *logger.binfo("Uploading..."))};
    if (err) return *err;

    if (prog) prog->emitEvent(100, _("Done"));
    logger.info("Applied Successfully");

    Arduino::Result ret{
        .total = compileOutput.total,
        .used = compileOutput.used,
    };
    return ret;
}

variant<Arduino::Result, string> Arduino::verifyConfig(const Config::Config& config, Progress *prog) {
    auto& logger{Log::Context::getGlobal().createLogger("Arduino::verifyConfig()")};

    auto res{compile(config, prog, *logger.binfo("Compiling..."))};
    if (auto *err = std::get_if<string>(&res)) return *err;
    const auto& compileOutput{std::get<CompileOutput>(res)};

    if (prog) prog->emitEvent(100, _("Done"));
    logger.info("Verified Successfully");

    Arduino::Result ret{
        .total = compileOutput.total,
        .used = compileOutput.used,
    };
    return ret;
}

variant<Arduino::CompileOutput, string> Arduino::compile(
    const Config::Config& config,
    Progress *prog,
    Log::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("Arduino::compile()")};
    optional<string> err;

    if (prog) prog->emitEvent(10, _("Checking OS Version..."));
    if (config.settings.osVersion == -1) {
        if (prog) prog->emitEvent(100, _("Error"));
        logger.error("Configuration doesn't have an OS Version selected, cannot compile.");
        return _("Please select an OS Version").ToStdString();
    }

    constexpr cstring GENERATE_MESSAGE{wxTRANSLATE("Generating configuration file...")};
    if (prog) prog->emitEvent(20, wxGetTranslation(GENERATE_MESSAGE));

    const auto osVersion{Versions::getOSVersions()[config.settings.osVersion].verNum};
    const auto osPath{Paths::os(osVersion)};
    const auto configPath{
        osPath / "config" / (static_cast<string>(config.name) + Config::RAW_FILE_EXTENSION)
    };
    err = config.save(configPath, logger.binfo(GENERATE_MESSAGE));
    if (err) {
        if (prog) prog->emitEvent(100, _("Error"));
        return *err;
    }

    constexpr cstring UPDATE_INO_MESSAGE{wxTRANSLATE("Updating ProffieOS file...")};
    if (prog) prog->emitEvent(30, wxGetTranslation(UPDATE_INO_MESSAGE));
    const auto inoPath{osPath / "ProffieOS.ino"};
    const auto tmpInoPath{fs::temp_directory_path() / "ProffieOS.ino"};
    std::ifstream ino(inoPath);
    std::ofstream tmpIno(tmpInoPath);
    if (not ino.is_open()) {
        logger.error("Failed to open ProffieOS INO");
        if (prog) prog->emitEvent(100, _("Error"));
        return _("OS Inaccessible or Corrupted").ToStdString();
    }
    if (not tmpIno.is_open()) {
        logger.error("Failed to open tmp ProffieOS INO");
        if (prog) prog->emitEvent(100, _("Error"));
        return _("Computer FS Error").ToStdString();
    }

    bool alreadyOutputConfigDefine{false};
    while(ino.good()) {
        string buffer;
        std::getline(ino, buffer);

        if (
                buffer.find(R"(// #define CONFIG_FILE "config/YOUR_CONFIG_FILE_NAME_HERE.h")") == 0 or 
                buffer.find(R"(#define CONFIG_FILE)") == 0
           ) {
            if (not alreadyOutputConfigDefine) {
                tmpIno << "#define CONFIG_FILE \"config/" << static_cast<string>(config.name) << ".h\"";
                alreadyOutputConfigDefine = true;
            }
        /* else if (fileData.find(R"(const char version[] = ")" ) != string::npos) {
            outputData.push_back(R"(const char version[] = ")" wxSTRINGIZE(PROFFIEOS_VERSION) R"(";)");
        */
        } else {
            tmpIno << buffer;
        }
    }
    ino.close();
    tmpIno.close();
    std::error_code errCode;
    if (not fs::copy_file(tmpInoPath, inoPath, fs::copy_options::overwrite_existing, errCode)) {
        logger.error("Failed to copy in tmp ProffieOS INO: " + errCode.message());
        if (prog) prog->emitEvent(100, _("Error"));
        return _("Computer FS Error").ToStdString();
    }

    constexpr cstring COMPILE_MESSAGE{"Compiling ProffieOS..."};
    if (prog) prog->emitEvent(40, COMPILE_MESSAGE);

    wxString output;
    array<char, 32> buffer;

    string compileCommand = "compile ";
    compileCommand += "-b ";
    const auto boardVersion{
        static_cast<Config::Settings::BoardVersion>(static_cast<int32>(config.settings.board))
    };
    switch (boardVersion) {
        case Config::Settings::PROFFIEBOARDV3:
            compileCommand += ARDUINOCORE_PBV3;
            break;
        case Config::Settings::PROFFIEBOARDV2:
            compileCommand += ARDUINOCORE_PBV2;
            break;
        case Config::Settings::PROFFIEBOARDV1:
            compileCommand += ARDUINOCORE_PBV1;
            break;
        default: 
            assert(0);
    }
    compileCommand += " --board-options ";
    if (config.settings.massStorage and config.settings.webUSB) compileCommand += "usb=cdc_msc_webusb";
    else if (config.settings.webUSB) compileCommand += "usb=cdc_webusb";
    else if (config.settings.massStorage) compileCommand += "usb=cdc_msc";
    else compileCommand += "usb=cdc";
    if (boardVersion == Config::Settings::PROFFIEBOARDV3) compileCommand +=",dosfs=sdmmc1";
    compileCommand += " \"" + osPath.string() + "\" -v";
    FILE *arduinoCli = Arduino::cli(compileCommand);

    string compileOutput{};
    while(fgets(buffer.data(), buffer.size(), arduinoCli) != nullptr) {
        if (prog) prog->emitEvent(-1, ""); // Pulse
        compileOutput += buffer.data();
    }

    if (pclose(arduinoCli) != 0) {
        logger.error("Error during pclose():\n" + compileOutput);
        return parseError(compileOutput, config);
    }

    if (compileOutput.find("error") != string::npos) {
        logger.error(compileOutput);
        return parseError(compileOutput, config);
    }

    CompileOutput ret;

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

# 	ifdef __WINDOWS__
    if (paths[0].empty() or paths[1].empty()) {
        logger.error("Failed to find utilities in output: " + error);
        _return = "Failed to find required utilities";
        return nullopt;
    }
    return paths;
# 	endif

    constexpr string_view USED_PREFIX{"Sketch uses "};
    constexpr string_view MAX_PREFIX{"Maximum is "};
    const auto usedPos{compileOutput.find(USED_PREFIX.data())};
    const auto maxPos{compileOutput.find(MAX_PREFIX.data())};
    if (usedPos != string::npos and maxPos != string::npos) {
        try {
            ret.used = std::stoi(compileOutput.substr(usedPos + USED_PREFIX.length()));
            ret.total = std::stoi(compileOutput.substr(maxPos + MAX_PREFIX.length()));
        } catch (const std::exception& e) {
            ret.used = -1;
            ret.total = -1;
            logger.warn("Usage data not found in compilation output.");
        }
    }

    logger.info("Success");
    return ret;
}

#ifdef __WINDOWS__
bool Arduino::upload(string& _return, EditorWindow *editor, Progress *progDialog, const array<string, 2>& paths, Log::Branch& lBranch) {
#else
optional<string> Arduino::upload(
    const string& boardPath,
    const Config::Config& config,
    Progress* prog,
    Log::Branch& lBranch
) {
#endif
    auto& logger{lBranch.createLogger("Arduino::upload()")};

    bool isBootloader{boardPath.find(_("BOOTLOADER").ToStdString()) != string::npos};
    if (not isBootloader) {
        constexpr cstring CHECK_PRESENCE_MESSAGE{wxTRANSLATE("Checking board presence...")};
        if (prog) prog->emitEvent(10, wxGetTranslation(CHECK_PRESENCE_MESSAGE));

        auto boards{Arduino::getBoards(logger.binfo(CHECK_PRESENCE_MESSAGE))};
        bool found{false};
        for (const auto& path : boards) {
            if (path == boardPath) {
                found = true;
                break;
            }
        }

        if (not found) {
            if (prog) prog->emitEvent(100, "Error!");
            logger.warn("Board was not found.");
            return _("Please make sure your board is connected and selected, then try again!").ToStdString();
        }
    }

    constexpr cstring UPLOAD_MESSAGE{wxTRANSLATE("Uploading to Proffieboard...")};
    if (prog) prog->emitEvent(65, wxGetTranslation(UPLOAD_MESSAGE));

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
    // const auto boardPath{static_cast<MainMenu*>(editor->GetParent())->boardSelect->entry()->GetStringSelection().ToStdString()};
    // if (boardPath.find(_("BOOTLOADER").c_str()) == string::npos) {
    //     progDialog->emitEvent(-1, "Rebooting Proffieboard...");
    //     struct termios newtio;
    //     auto fd = open(boardPath.c_str(), O_RDWR | O_NOCTTY);
    //     if (fd < 0) {
    //         _return = "Failed to connect to board for reboot.";
    //         logger.error(_return);
    //         return false;
    //     }

    //     memset(&newtio, 0, sizeof(newtio));

    //     newtio.c_cflag = B115200 | CRTSCTS | CS8 | CLOCAL | CREAD;
    //     newtio.c_iflag = IGNPAR;
    //     newtio.c_oflag = (tcflag_t) NULL;
    //     newtio.c_lflag &= ~ICANON; /* unset canonical */
    //     newtio.c_cc[VTIME] = 1; /* 100 millis */

    //     tcflush(fd, TCIFLUSH);
    //     tcsetattr(fd, TCSANOW, &newtio);

    //     char buf[255];
    //     while(read(fd, buf, 255));

    //     fsync(fd);
    //     write(fd, "\r\n", 2);
    //     write(fd, "\r\n", 2);
    //     write(fd, "RebootDFU\r\n", 11);

    //     // Ensure everything is flushed
    //     std::this_thread::sleep_for(50ms);
    //     close(fd);
    //     std::this_thread::sleep_for(5s);
    // }
#   endif

#   ifndef __WINDOWS__
    string uploadCommand = "upload ";
    const auto osVersion{Versions::getOSVersions()[config.settings.osVersion].verNum};
    const auto osPath{Paths::os(osVersion)};
    uploadCommand += '"' + osPath.string() + '"';
    uploadCommand += " --fqbn ";
    const auto boardVersion{
        static_cast<Config::Settings::BoardVersion>(static_cast<int32>(config.settings.board))
    };
    switch (boardVersion) {
        case Config::Settings::PROFFIEBOARDV3:
            uploadCommand += ARDUINOCORE_PBV3;
            break;
        case Config::Settings::PROFFIEBOARDV2:
            uploadCommand += ARDUINOCORE_PBV2;
            break;
        case Config::Settings::PROFFIEBOARDV1:
            uploadCommand += ARDUINOCORE_PBV1;
            break;
        default: 
            abort();
    }
    uploadCommand += " -v";
    FILE *arduinoCli = Arduino::cli(uploadCommand);

    string error;
    array<char, 32> buffer;
    if (prog) prog->emitEvent(-1, _("Uploading to Proffieboard..."));
    while(fgets(buffer.data(), buffer.size(), arduinoCli) != nullptr) {
        auto *const percentPtr{std::strstr(buffer.data(), "%")};
        if (percentPtr != nullptr and percentPtr - buffer.data() >= 3) {
            auto percent{strtol(percentPtr - 3, nullptr, 10)};
            if (prog) prog->emitEvent(static_cast<int8>(percent), "");
            logger.verbose("Progress: " + std::to_string(percent) + '%');
        }

        error += buffer.data();
    }

    if (error.rfind("error") != string::npos or error.rfind("FAIL") != string::npos) {
        logger.error(error);
        return parseError(error, config);
    }

    if (pclose(arduinoCli) != 0) {
        logger.error("Error during pclose(): \n" + error);
        return _("Unknown Upload Error").ToStdString();
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

    logger.info("Success");
    return nullopt;
}

string Arduino::parseError(const string& error, const Config::Config& config) {
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

    if (config.propSelection != -1) {
        auto& selectedProp{*config.props()[config.propSelection]};
        for (const auto& [ arduino, display ] : selectedProp.errors()) {
            if (error.find(arduino) != string::npos) return selectedProp.name + " prop error:\n" + display;
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
    fullCommand += '"' + (Paths::binaryDir() / "arduino-cli").string() + '"';
    fullCommand += " " + command;
    fullCommand += " 2>&1";

    logger.debug("Executing command \"" + fullCommand + '"');
    return popen(fullCommand.data(), "r");
}
