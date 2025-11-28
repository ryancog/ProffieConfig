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

#ifdef _WIN32
#include <fileapi.h>
#include <handleapi.h>
#include <windows.h>
#include <synchapi.h>
#else
#include <thread>
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
#include "log/branch.h"
#include "process/process.h"
#include "utils/paths.h"
#include "utils/types.h"
#include "versions/versions.h"

#include "../core/utilities/progress.h"

using namespace std::chrono_literals;

namespace {

constexpr cstring DEFAULT_CORE_URL{"https://profezzorn.github.io/arduino-proffieboard/package_proffieboard_index.json"};

void cli(Process& proc, vector<string>& args);

struct CompileOutput {
    int32 used;
    int32 total;
#   ifdef _WIN32
    string tool1;
    string tool2;
#   endif
};

variant<CompileOutput, string> compile(
    const Config::Config&,
    Progress *,
    Log::Branch&
);

#ifdef _WIN32
optional<string> upload(
    const string& boardPath,
    const Config::Config&,
    const CompileOutput&,
    Progress *,
    Log::Branch&
);
#else
optional<string> upload(
    const string& boardPath,
    const Config::Config&,
    Progress *,
    Log::Branch&
);
#endif

/**
 * Pre-checks specifically for compilation.
 * (May be fine for saving though)
 */
optional<string> precheckCompile(const Config::Config&, Log::Branch&);

string parseError(const string&, const Config::Config&); 

optional<string> ensureCoreInstalled(
    const string& coreVersion,
    const string& coreURL,
    Log::Logger&,
    Progress * = nullptr
);

constexpr auto MAX_ERRMESSAGE_LENGTH{1024};
constexpr cstring ARDUINOCORE_PBV1{"proffieboard:stm32l4:Proffieboard-L433CC"};
constexpr cstring ARDUINOCORE_PBV2{"proffieboard:stm32l4:ProffieboardV2-L433CC"};
constexpr cstring ARDUINOCORE_PBV3{"proffieboard:stm32l4:ProffieboardV3-L452RE"};

} // namespace

string Arduino::version() {
    Process proc;
    vector<string> args{"version"};
    cli(proc, args);

    string output;
    while (auto buffer = proc.read()) {
        output += *buffer;
    }

    proc.finish();

    constexpr cstring UNKNOWN_STR{wxTRANSLATE("Unknown")};
    constexpr string_view VERSION_TAG{"Version: "};
    const auto versionTagPos{output.find(VERSION_TAG)};
    if (versionTagPos == string::npos) return wxGetTranslation(UNKNOWN_STR).ToStdString();

    const auto versionStart{versionTagPos + VERSION_TAG.length()};
    const auto versionEnd{output.find(' ', versionStart)};
    if (versionEnd == string::npos) return wxGetTranslation(UNKNOWN_STR).ToStdString();

    return output.substr(versionStart, versionEnd - versionStart);
}

vector<string> Arduino::getBoards(Log::Branch *lBranch) {
    auto& logger{Log::Branch::optCreateLogger("Arduino::getBoards()", lBranch)};

    vector<string> boards;
    array<char, 32> buffer;

    Process proc;
    vector<string> args{
        "board",
        "list",
    };
    cli(proc, args);

    struct Result {
        Result(string port, bool isProffie) : port{std::move(port)}, isProffie{isProffie} {}
        string port;
        bool isProffie{false};
    };

    std::vector<Result> results;

    string output;
    while (auto buffer = proc.read()) {
        output += *buffer;
    }

    proc.finish();

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
    if (auto *err = std::get_if<string>(&res)) {
        if (prog) prog->emitEvent(100, _("Done"));
        return *err;
    }
    const auto& compileOutput{std::get<CompileOutput>(res)};

#   ifdef _WIN32
    auto err{upload(boardPath, config, compileOutput, prog, *logger.binfo("Uploading..."))};
#   else
    auto err{upload(boardPath, config, prog, *logger.binfo("Uploading..."))};
#   endif
    if (prog) prog->emitEvent(100, _("Done"));
    if (err) return *err;

    logger.info("Applied Successfully");

    Arduino::Result ret{
        .used = compileOutput.used,
        .total = compileOutput.total,
    };
    return ret;
}

variant<Arduino::Result, string> Arduino::verifyConfig(const Config::Config& config, Progress *prog) {
    auto& logger{Log::Context::getGlobal().createLogger("Arduino::verifyConfig()")};

    auto res{compile(config, prog, *logger.binfo("Compiling..."))};
    if (prog) prog->emitEvent(100, _("Done"));

    if (auto *err = std::get_if<string>(&res)) return *err;

    const auto& compileOutput{std::get<CompileOutput>(res)};
    logger.info("Verified Successfully");

    Arduino::Result ret{
        .used = compileOutput.used,
        .total = compileOutput.total,
    };
    return ret;
}

namespace {

variant<CompileOutput, string> compile(
    const Config::Config& config,
    Progress *prog,
    Log::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("Arduino::compile()")};
    optional<string> err;

    constexpr cstring PRECHK_MSG{wxTRANSLATE("Running compile prechecks...")};
    if (prog) prog->emitEvent(5, wxGetTranslation(PRECHK_MSG));
    err = precheckCompile(config, *logger.binfo(PRECHK_MSG));
    if (err) {
        if (prog) prog->emitEvent(100, _("Error"));
        return *err;
    }

    constexpr cstring OSCHK_MSG{wxTRANSLATE("Checking OS Version...")};
    if (prog) prog->emitEvent(10, wxGetTranslation(OSCHK_MSG));
    logger.info(OSCHK_MSG);
    const auto osVersion{config.settings.getOSVersion()};
    const auto *const versionedOS{Versions::getVersionedOS(osVersion)};
    if (osVersion == Utils::Version::invalidObject() or not versionedOS) {
        if (prog) prog->emitEvent(100, _("Error"));
        logger.error("Configuration doesn't have a valid OS Version selected (" + static_cast<string>(osVersion) + "), cannot compile.");
        return _("Please select an OS Version").ToStdString();
    }

    err = ensureCoreInstalled(
        versionedOS->coreVersion.err
            ? static_cast<string>(Versions::getDefaultCoreVersion())
            : static_cast<string>(versionedOS->coreVersion),
        not versionedOS->coreURL.empty()
            ? versionedOS->coreURL
            : DEFAULT_CORE_URL,
        logger,
        prog
    );
    if (err) {
        return *err;
    }

    const auto osPath{Paths::os(osVersion)};

    if (config.propSelection != -1) {
        constexpr cstring PROPINST_MSG{wxTRANSLATE("Installing Prop File...")};
        if (prog) prog->emitEvent(25, wxGetTranslation(PROPINST_MSG));
        logger.info(PROPINST_MSG);
        auto [prop, reference]{config.propAndReference(config.propSelection)};
        if (not reference) {
            if (prog) prog->emitEvent(100, _("Error"));
            logger.error("Prop doesn't have a valid reference.");
            return _("Invalid Prop Selected").ToStdString();
        }

        std::error_code err;
        const auto sourcePropHeader{Paths::propDir() / reference->name / Versions::HEADER_FILE_STR};
        if (not reference->prop->filename.empty()) {
            if (not fs::exists(sourcePropHeader, err)) {
                if (prog) prog->emitEvent(100, _("Error"));
                logger.error("Prop doesn't have a header.");
                return _("Invalid Prop Selected").ToStdString();
            }

            auto res{Paths::copyOverwrite(sourcePropHeader, osPath / "props" / prop.filename, err)};
            if (not res) {
                if (prog) prog->emitEvent(100, _("Error"));
                logger.error("Failed to copy in prop header.");
                return _("OS FS Error").ToStdString();
            }
        }
    }

    const auto configPath{
        osPath / "config" / (static_cast<string>(config.name) + Config::RAW_FILE_EXTENSION)
    };

    constexpr cstring GENERATE_MESSAGE{wxTRANSLATE("Generating configuration file...")};
    if (prog) prog->emitEvent(30, wxGetTranslation(GENERATE_MESSAGE));
    err = config.save(configPath, logger.binfo(GENERATE_MESSAGE));
    if (err) {
        if (prog) prog->emitEvent(100, _("Error"));
        return *err;
    }

    constexpr cstring UPDATE_INO_MESSAGE{wxTRANSLATE("Updating ProffieOS file...")};
    if (prog) prog->emitEvent(35, wxGetTranslation(UPDATE_INO_MESSAGE));
    logger.info(UPDATE_INO_MESSAGE);
    const auto inoPath{osPath / "ProffieOS.ino"};
    const auto tmpInoPath{fs::temp_directory_path() / "ProffieOS.ino"};
    auto ino{Paths::openInputFile(inoPath)};
    auto tmpIno{Paths::openOutputFile(tmpInoPath)};
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
    while (ino.good()) {
        string buffer;
        std::getline(ino, buffer);

        if (
                buffer.starts_with(R"(// #define CONFIG_FILE "config/YOUR_CONFIG_FILE_NAME_HERE.h")") or 
                buffer.starts_with(R"(#define CONFIG_FILE)")
           ) {
            if (not alreadyOutputConfigDefine) {
                tmpIno << "#define CONFIG_FILE \"config/" << static_cast<string>(config.name) << ".h\"\n";
                alreadyOutputConfigDefine = true;
            }
        } else if (buffer.starts_with(R"(const char version[] = ")")) {
            tmpIno << R"(const char version[] = ")" + static_cast<string>(osVersion) +  "\";\n";
        } else {
            tmpIno << buffer << '\n';
        }
    }
    ino.close();
    tmpIno.close();
    std::error_code errCode;
    if (not Paths::copyOverwrite(tmpInoPath, inoPath, errCode)) {
        logger.error("Failed to copy in tmp ProffieOS INO: " + errCode.message());
        if (prog) prog->emitEvent(100, _("Error"));
        return _("Computer FS Error").ToStdString();
    }

    constexpr cstring COMPILE_MESSAGE{wxTRANSLATE("Compiling ProffieOS...")};
    if (prog) prog->emitEvent(40, wxGetTranslation(COMPILE_MESSAGE));
    logger.info(COMPILE_MESSAGE);

    wxString output;
    array<char, 32> buffer;

    Process proc;
    vector<string> args{
        "compile",
        "-b",
    };
    const auto boardVersion{
        static_cast<Config::BoardVersion>(static_cast<int32>(config.settings.board))
    };
    switch (boardVersion) {
        case Config::PROFFIEBOARDV3:
            args.push_back(versionedOS->coreBoardV3.empty() ? ARDUINOCORE_PBV3 : versionedOS->coreBoardV3);
            break;
        case Config::PROFFIEBOARDV2:
            args.push_back(versionedOS->coreBoardV2.empty() ? ARDUINOCORE_PBV2 : versionedOS->coreBoardV2);
            break;
        case Config::PROFFIEBOARDV1:
            args.push_back(versionedOS->coreBoardV1.empty() ? ARDUINOCORE_PBV1 : versionedOS->coreBoardV1);
            break;
        default: 
            assert(0);
    }
    args.emplace_back("--board-options");
    string options;
    if (config.settings.massStorage and config.settings.webUSB) options = "usb=cdc_msc_webusb";
    else if (config.settings.webUSB) options = "usb=cdc_webusb";
    else if (config.settings.massStorage) options = "usb=cdc_msc";
    else options = "usb=cdc";
    if (boardVersion == Config::PROFFIEBOARDV3) options +=",dosfs=sdmmc1";
    args.push_back(std::move(options));
    args.push_back(osPath.string());
    args.emplace_back("-v");
    cli(proc, args);

    string compileOutput{};
    while(auto buffer = proc.read()) {
        if (prog) prog->emitEvent(-1, ""); // Pulse
        compileOutput += *buffer;
    }

    auto res{proc.finish()};
    if (res.err) {
        logger.error(
            "Process error: " + std::to_string(res.err) + ':' + std::to_string(res.systemResult) +
            "\n" + compileOutput
        );
        if (prog) prog->emitEvent(100, _("Error"));
        return parseError(compileOutput, config);
    }

    if (compileOutput.find("error") != string::npos) {
        logger.error(compileOutput);
        if (prog) prog->emitEvent(100, _("Error"));
        return parseError(compileOutput, config);
    }

    CompileOutput ret;

#   ifdef _WIN32
    constexpr string_view DFU_STR{"ProffieOS.ino.dfu"};
    constexpr string_view DFU_SUFFIX_STR{"dfu-suffix.exe"};
    constexpr string_view ROOT_C_STR{"C:\\"};
    constexpr cstring UTIL_ERR{wxTRANSLATE("Failed to find required utilities")};
    size_t dfuPos{compileOutput.rfind(DFU_STR)};
    size_t dfuCPos{compileOutput.rfind(ROOT_C_STR, dfuPos)};
    size_t dfuSuffixPos{compileOutput.rfind(DFU_SUFFIX_STR)};
    size_t dfuSuffixCPos{compileOutput.rfind(ROOT_C_STR, dfuSuffixPos)};
    if (
            dfuPos != string::npos and
            dfuCPos != string::npos and
            dfuSuffixPos != string::npos and
            dfuSuffixCPos != string::npos
       ) {
        logger.debug("Parsing utility paths...");
        array<char, MAX_PATH> shortPath;
        DWORD res{};

        const auto dfuLongPath{compileOutput.substr(dfuCPos, dfuPos - dfuCPos + DFU_STR.length())};
        res = GetShortPathNameA(dfuLongPath.c_str(), shortPath.data(), shortPath.size());
        if (res == 0) {
            logger.error("Failed to find dfu util in output: " + compileOutput);
            if (prog) prog->emitEvent(100, _("Error"));
            return wxGetTranslation(UTIL_ERR).ToStdString();
        }

        ret.tool1 = shortPath.data();
        logger.debug("Parsed dfu file: " + ret.tool1);

        const auto dfuSuffixLongPath{compileOutput.substr(dfuSuffixCPos, dfuSuffixPos - dfuSuffixCPos)};
        res = GetShortPathNameA(dfuSuffixLongPath.c_str(), shortPath.data(), shortPath.size());
        if (res == 0) {
            logger.error("Failed to find dfu suffix in output: " + compileOutput);
            if (prog) prog->emitEvent(100, _("Error"));
            return wxGetTranslation(UTIL_ERR).ToStdString();
        }

        ret.tool2 = string{shortPath.data()} + "stm32l4-upload.bat";
        logger.debug("Parsed upload file: " + ret.tool2);
    }

    if (ret.tool1.empty() or ret.tool2.empty()) {
        logger.error("Failed to find utilities in output: " + compileOutput);
        if (prog) prog->emitEvent(100, _("Error"));
        return wxGetTranslation(UTIL_ERR).ToStdString();
    }
# 	endif

    constexpr string_view USED_PREFIX{"Sketch uses "};
    constexpr string_view MAX_PREFIX{"Maximum is "};
    const auto usedPos{compileOutput.find(USED_PREFIX)};
    const auto maxPos{compileOutput.find(MAX_PREFIX)};
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

#ifdef _WIN32
optional<string> upload(
    const string& boardPath,
    const Config::Config& config,
    const CompileOutput& compileOutput,
    Progress* prog,
    Log::Branch& lBranch
) {
#else
optional<string> upload(
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

#   ifdef _WIN32
    if (boardPath != _("BOOTLOADER RECOVERY")) {
        // See https://learn.microsoft.com/en-us/windows/win32/fileio/naming-a-file#win32-device-namespaces
        const auto safeBoardPath{R"(\\.\)" + boardPath};
        if (prog) prog->emitEvent(50, "Rebooting Proffieboard...");

        auto *serialHandle{CreateFileA(
            safeBoardPath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        )};
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
    if (boardPath.find(_("BOOTLOADER").c_str()) == string::npos) {
        prog->emitEvent(-1, "Rebooting Proffieboard...");
        struct termios newtio;
        auto fd = open(boardPath.c_str(), O_RDWR | O_NOCTTY);
        if (fd < 0) {
            if (prog) prog->emitEvent(100, "Error!");
            logger.warn("Could not open board port.");
            return _("Board was not reachable for reboot, please try again!").ToStdString();
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

        fsync(fd);

        // Ensure everything is flushed
        std::this_thread::sleep_for(50ms);
        close(fd);
        std::this_thread::sleep_for(5s);
    }
#   endif

    Process proc;

#   ifdef _WIN32
    array<string, 3> args{
        "0x1209",
        "0x6668",
        compileOutput.tool1
    };
    proc.create(compileOutput.tool2, args);
#   else
    vector<string> args{
        "upload",
    };
    const auto osVersion{config.settings.getOSVersion()};
    const auto osPath{Paths::os(osVersion)};
    args.push_back(osPath.string());
    args.emplace_back("--fqbn");
    const auto boardVersion{
        static_cast<Config::BoardVersion>(static_cast<int32>(config.settings.board))
    };
    switch (boardVersion) {
        case Config::PROFFIEBOARDV3:
            args.emplace_back(ARDUINOCORE_PBV3);
            break;
        case Config::PROFFIEBOARDV2:
            args.emplace_back(ARDUINOCORE_PBV2);
            break;
        case Config::PROFFIEBOARDV1:
            args.emplace_back(ARDUINOCORE_PBV1);
            break;
        default: 
            abort();
    }
    args.emplace_back("-v");
    cli(proc, args);
#   endif

    string uploadOutput;
    if (prog) prog->emitEvent(-1, wxGetTranslation(UPLOAD_MESSAGE));
    while (auto buffer = proc.read()) {
        const auto percentPos{buffer->find('%')};
        if (percentPos != string::npos and percentPos >= 3) {
            try {
                auto percent{std::stoi(buffer->substr(percentPos - 3))};
                if (prog) prog->emitEvent(static_cast<int8>(percent), {});
                logger.verbose("Progress: " + std::to_string(percent) + '%');
            } catch (const std::exception&) {
                uploadOutput += *buffer;
                continue;
            }
        }

        uploadOutput += *buffer;
    }

    if (uploadOutput.rfind("error") != string::npos or uploadOutput.rfind("FAIL") != string::npos) {
        logger.error(uploadOutput);
        return parseError(uploadOutput, config);
    }

    auto res{proc.finish()};
    if (res.err) {
        logger.error(
            "Process error: " + std::to_string(res.err) + ':' + std::to_string(res.systemResult) +
            "\n" + uploadOutput
        );
        return _("Unknown Upload Error").ToStdString();
    }

    // TODO: Don't remember if this happens on non-msw so guard it for now
#   ifdef _WIN32
    if (uploadOutput.find("File downloaded successfully") == string::npos) {
        logger.error(uploadOutput);
        return parseError(uploadOutput, config);
    }
#   endif

    logger.info("Success");
    return nullopt;
}

optional<string> precheckCompile(const Config::Config& config, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Arduino::precheckCompile()")};

    if (config.bladeArrays.arrays().empty()) {
        logger.error("Config has no blade arrays, cannot compile.");
        return _("Config must have at least one blade array to compile.").ToStdString();
    }

    return nullopt;
}

string parseError(const string& error, const Config::Config& config) {
#	define ERRCONTAINS(token) std::strstr(error.data(), token)
    if (ERRCONTAINS("select Proffieboard")) return "Please ensure you've selected the correct board in General";
    if (ERRCONTAINS("expected unqualified-id")) return "Please make sure there are no brackets in your styles (such as \"{\" or \"}\")\n and there is nothing missing or extra from your style! (such as parentheses or \"<>\")";
    if (ERRCONTAINS(/* region FLASH */"overflowed")) {
        const auto maxBytes = error.find("ProffieboardV3") != string::npos ? 507904 : 262144;
        constexpr string_view OVERFLOW_PREFIX{"region `FLASH' overflowed by "};

        const auto overflowPos{error.rfind(OVERFLOW_PREFIX)};
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
        return error.substr(fileData + 1, MAX_ERRMESSAGE_LENGTH);
    }

    return "Unknown error: " + error.substr(0, MAX_ERRMESSAGE_LENGTH);
#	undef ERRCONTAINS
}

optional<string> ensureCoreInstalled(
    const string& coreVersion,
    const string& coreURL,
    Log::Logger& logger,
    Progress *prog
) {
    constexpr cstring MSG{wxTRANSLATE("Ensuring Core Installation...")};
    if (prog) prog->emitEvent(15, wxGetTranslation(MSG));
    logger.info(MSG);

    Process proc;
    vector<string> args{
        "core",
        "install",
        "proffieboard:stm32l4@" + coreVersion,
        "--additional-urls",
        coreURL
    };
    cli(proc, args);

    string coreInstallOutput;
    while (auto buffer = proc.read()) {
        if (prog) prog->emitEvent(-1, ""); 
        coreInstallOutput += *buffer;
    }

    auto res{proc.finish()};
    if (res.err) {
        if (prog) prog->emitEvent(100, "Error");
        logger.error(
            "Process error: " + std::to_string(res.err) + ':' + std::to_string(res.systemResult) +
            "\n" + coreInstallOutput
        );
        return _("Could Not Install Core").ToStdString();
    }

    return nullopt;
}

void cli(Process& proc, vector<string>& args) {
    args.emplace_back("--no-color");
    auto arduinoStr{(Paths::binaryDir() / "arduino-cli").string()};
    proc.create(arduinoStr, args);
}

} // namespace

bool Arduino::ensureDefaultCoreInstalled(Log::Branch *lBranch) {
    auto& logger{Log::Branch::optCreateLogger("Arduino::ensureDefaultCoreInstalled()", lBranch)};

    auto err{ensureCoreInstalled(
        static_cast<string>(Versions::getDefaultCoreVersion()),
        DEFAULT_CORE_URL,
        logger
    )};

    return not err;
}

#if defined(_WIN32) or defined(__linux__)
bool Arduino::runDriverInstallation(Log::Branch *lBranch) {
    auto& logger{Log::Branch::optCreateLogger("Arduino::runDriverInstallation()", lBranch)};
    logger.info("Installing drivers...");

#   if defined(__linux__)
    Process proc;

    const auto rulesPath{
        Paths::user() / ".arduino15" / "packages" / "proffieboard" / "hardware" / "stm32l4" /
        static_cast<string>(Versions::getDefaultCoreVersion()) / "drivers" / "linux"
    };
    vector<string> args;
    args.emplace_back("cp");
    std::error_code err;
    fs::directory_iterator directoryIter{rulesPath, err};
    if (err) {
        logger.error("Could not access driver path " + rulesPath.string() + ": " + err.message());
        return false;
    }
    for (const auto& entry : directoryIter) {
        if (not entry.is_regular_file(err)) continue;
        if (not entry.path().string().ends_with("rules")) continue;

        args.emplace_back(entry.path().string());
    }
    args.emplace_back("/etc/udev/rules.d");
    proc.create("pkexec", args);

    auto result{proc.finish()};
#   elif defined(_WIN32)
    auto driverStr{(Paths::binaryDir() / "proffie-dfu-setup.exe").string()};
    auto result{Process::elevatedProcess(driverStr.c_str())};
#   endif

    if (result.err) {
        logger.error("Installation failed with error " + std::to_string(result.err) + ":" + std::to_string(result.systemResult));
        if (result.err == Process::Result::UNKNOWN) logger.error("System error: " + std::to_string(result.systemResult));
        return false;
    }
    return true;
}
#endif

