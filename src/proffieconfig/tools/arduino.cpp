#include "arduino.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * proffieconfig/tools/arduino.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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
#include <optional>
#include <unordered_set>
#include <variant>

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

#include "config/config.hpp"
#include "config/priv/io.hpp"
#include "config/misc/injection.hpp"
#include "config/styles/style.hpp"
#include "data/context.hpp"
#include "log/context.hpp"
#include "log/logger.hpp"
#include "log/branch.hpp"
#include "process/process.hpp"
#include "ui/dialogs/progress.hpp"
#include "utils/files.hpp"
#include "utils/paths.hpp"
#include "utils/types.hpp"
#include "versions/detail/boards.hpp"
#include "versions/detail/strings.hpp"

#include "serialmonitor.hpp"

using namespace std::chrono_literals;

namespace {

void cli(Process& proc, std::vector<std::string>& args);

struct CompileOutput {
    int32 used_;
    int32 total_;

    std::string dfuFile_;
    std::string suffixPath_;

    [[nodiscard]] float64 percent() const {
        return (static_cast<float64>(used_) / total_) * 100.0;
    }

    [[nodiscard]] wxString usageMessage() const {
        static constexpr cstring USAGE_MESSAGE{wxTRANSLATE(
            "The configuration uses %.2f%% of board space. (%d/%d)"
        )};

        return wxString::Format(
            wxGetTranslation(USAGE_MESSAGE),
            percent(),
            used_,
            total_
        );
    }
};

std::variant<CompileOutput, wxString> compile(
    const std::string&,
    const config::Config&,
    pcui::ProgressDialog&,
    logging::Branch&
);

std::optional<wxString> upload(
    const std::string& boardPath,
    const config::Config&,
    const CompileOutput&,
    pcui::ProgressDialog&,
    logging::Branch&
);

/**
 * Pre-checks specifically for compilation.
 * (May be fine for saving though)
 */
std::optional<wxString> precheckCompile(
    const config::Config&, logging::Branch&
);

wxString parseError(const std::string&, const config::Config&); 

std::optional<wxString> ensureCoreInstalled(
    const std::string& coreVersion,
    const std::string& coreURL,
    logging::Logger&,
    pcui::ProgressDialog *
);

constexpr auto MAX_ERRMESSAGE_LENGTH{1024};
constexpr cstring ARDUINOCORE_PBV1{"proffieboard:stm32l4:Proffieboard-L433CC"};
constexpr cstring ARDUINOCORE_PBV2{"proffieboard:stm32l4:ProffieboardV2-L433CC"};
constexpr cstring ARDUINOCORE_PBV3{"proffieboard:stm32l4:ProffieboardV3-L452RE"};

} // namespace

std::string arduino::version() {
    Process proc;
    std::vector<std::string> args{"version"};
    cli(proc, args);

    std::string output;
    while (auto buffer{proc.read()}) {
        output += *buffer;
    }

    proc.finish();

    constexpr cstring UNKNOWN_STR{wxTRANSLATE("Unknown")};
    constexpr std::string_view VERSION_TAG{"Version: "};
    const auto versionTagPos{output.find(VERSION_TAG)};
    if (versionTagPos == std::string::npos) {
        return wxGetTranslation(UNKNOWN_STR).ToStdString();
    }

    const auto versionStart{versionTagPos + VERSION_TAG.length()};
    const auto versionEnd{output.find(' ', versionStart)};
    if (versionEnd == std::string::npos) {
        return wxGetTranslation(UNKNOWN_STR).ToStdString();
    }

    return output.substr(versionStart, versionEnd - versionStart);
}

std::vector<std::string> arduino::getBoards(logging::Branch *lBranch) {
    auto& logger{logging::Branch::optCreateLogger("arduino::getBoards()", lBranch)};

    std::vector<std::string> boards;
    std::array<char, 32> buffer;

    Process proc;
    std::vector<std::string> args{
        "board",
        "list",
    };
    cli(proc, args);

    struct Result {
        Result(std::string port, bool isProffie) :
            port_{std::move(port)}, isProffie_{isProffie} {}

        std::string port_;
        bool isProffie_{false};
    };

    std::vector<Result> results;

    std::string output;
    while (auto buffer{proc.read()}) {
        output += *buffer;
    }

    proc.finish();

    auto lineEndPos{output.find('\n')};
    while (not false) {
        const auto line{output.substr(0, lineEndPos)};

        if (line.find("No boards found.") != std::string::npos) {
            logger.info("No boards found.");
            break;
        }

        if (line[0] == ' ' or line[0] == '\t') {
            if (results.empty()) continue;

            if (line.find("proffieboard") != std::string::npos) {
                results.back().isProffie_ = true;
            }
        } else if (line.find("serial") != std::string::npos) {
            const auto port{line.substr(0, line.find_first_of(" \t"))};
            const auto isProffie{
                line.find("proffieboard") != std::string::npos
            };

            results.emplace_back(port, isProffie);
            logger.debug("Found board: " + port);
        } else if (line.find("dfu") != std::string::npos) {
            const auto port{line.substr(0, line.find_first_of(" \t"))};

            boards.emplace_back(_("BOOTLOADER").ToStdString() + '|' + port);
            logger.debug("Found board in bootloader mode: " + port);
        }

        if (lineEndPos == std::string::npos) break;

        output = output.substr(lineEndPos + 1);
        lineEndPos = output.find('\n');
    }

    for (const auto& result : results) {
        if (not result.isProffie_) continue;

        logger.info("Reporting board: " + result.port_);
        boards.emplace_back(result.port_);
    }

    return boards;
}

void arduino::applyToBoard(
    const std::string& name,
    const std::string& boardPath,
    const config::Config& config,
    pcui::ProgressDialog& prog
) {
    auto& logger{logging::Context::getGlobal().createLogger("arduino::applyToBoard()")};

    auto res{compile(name, config, prog, *logger.binfo("Compiling..."))};
    if (auto *err{std::get_if<wxString>(&res)}) {
        prog.finish(true, *err);
        return;
    }

    const auto& compileOutput{std::get<CompileOutput>(res)};

    auto err{upload(
        boardPath,
        config,
        compileOutput,
        prog,
        *logger.binfo("Uploading...")
    )};
    if (err) {
        prog.finish(true, *err);
        return;
    }

    logger.info("Applied Successfully");

    wxString message{_("Config Verified Successfully!")};
    if (compileOutput.total_ != -1) {
        message += "\n\n";
        message += compileOutput.usageMessage();
    }

    prog.finish(true, message);
}

void arduino::verifyConfig(
    const std::string& name,
    const config::Config& config,
    pcui::ProgressDialog& prog
) {
    auto& logger{logging::Context::getGlobal().createLogger("arduino::verifyConfig()")};

    auto res{compile(name, config, prog, *logger.binfo("Compiling..."))};
    if (auto *err{std::get_if<wxString>(&res)}) {
        prog.finish(true, *err);
        return;
    }

    const auto& compileOutput{std::get<CompileOutput>(res)};

    logger.info("Verified Successfully");

    wxString message{_("Config Verified Successfully!")};
    if (compileOutput.total_ != -1) {
        message += "\n\n";
        message += compileOutput.usageMessage();
    }

    prog.finish(true, message);
}

namespace {

std::variant<CompileOutput, wxString> compile(
    const std::string& name,
    const config::Config& config,
    pcui::ProgressDialog& prog,
    logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("arduino::compile()")};
    std::optional<wxString> err;

    constexpr cstring PRECHK_MSG{wxTRANSLATE("Running compile prechecks...")};
    prog.set(5, wxGetTranslation(PRECHK_MSG));
    err = precheckCompile(config, *logger.binfo(PRECHK_MSG));
    if (err) return *err;

    err = ensureCoreInstalled(
        config.os()->coreVersion_,
        config.os()->coreUrl_,
        logger,
        &prog
    );
    if (err) return *err;

    const auto osPath{
        paths::osDir() / config.os()->version_.string() / "ProffieOS"
    };

    if (const auto *prop{config.prop()}) {
        constexpr cstring PROPINST_MSG{wxTRANSLATE("Installing Prop File...")};
        prog.set(20, wxGetTranslation(PROPINST_MSG));
        logger.info(PROPINST_MSG);

        std::error_code err;
        const auto sourcePropHeader{
            paths::propDir() / prop->installName_ / versions::detail::HEADER_FILE_STR
        };

        if (not prop->filename_.empty()) {
            if (not fs::exists(sourcePropHeader, err)) {
                logger.error("Prop doesn't have a header.");
                return _("Invalid Prop Selected");
            }

            const auto propHeaderDest{osPath / "props" / prop->filename_};
            auto res{files::copyOverwrite(
                sourcePropHeader, propHeaderDest, err
            )};

            if (not res) {
                logger.error("Failed to copy in prop header.");
                return _("OS FS Error");
            }
        }
    }

    const auto injectionsDest{osPath / "config" / config::priv::INJECTION_STR};
    auto injectionVec{data::context(config.injections_)};

    if (not injectionVec.children().empty()) {
        constexpr cstring PROPINST_MSG{wxTRANSLATE("Installing Injection Files...")};
        prog.set(25, wxGetTranslation(PROPINST_MSG));

        std::error_code err;
        if (not fs::create_directories(injectionsDest, err)) {
            logger.error("Failed to create injections dir: " + err.message());
            return _("OS FS Error");
        }
    }

    for (const auto& model : injectionVec.children()) {
        auto& injection{dynamic_cast<config::Injection&>(*model)};

        const auto srcPath{paths::injectionDir() / injection.filename_};
        const auto dstPath{injectionsDest / injection.filename_};

        std::error_code err;
        if (not files::copyOverwrite(srcPath, dstPath, err)) {
            logger.error("Failed to copy injection file \"" + srcPath.string() + "\" to \"" + dstPath.string() + "\": " + err.message());
            return _("OS FS Error");
        }
    }

    const auto outName{"ProffieConfig_" + name + ".h"};
    const auto configPath{osPath / "config" / outName};

    constexpr cstring GENERATE_MESSAGE{wxTRANSLATE("Generating configuration file...")};
    prog.set(30, wxGetTranslation(GENERATE_MESSAGE));
    err = config::generate(config, configPath, logger.binfo(GENERATE_MESSAGE));
    if (err) return *err;

    constexpr cstring UPDATE_INO_MESSAGE{wxTRANSLATE("Updating ProffieOS file...")};
    prog.set(35, wxGetTranslation(UPDATE_INO_MESSAGE));
    logger.info(UPDATE_INO_MESSAGE);

    const auto inoPath{osPath / "ProffieOS.ino"};
    const auto tmpInoPath{fs::temp_directory_path() / "ProffieOS.ino"};
    auto ino{files::openInput(inoPath)};
    if (ino.fail()) {
        logger.error("Failed to open ProffieOS INO");
        return _("OS Inaccessible or Corrupted");
    }

    auto tmpIno{files::openOutput(tmpInoPath)};
    if (tmpIno.fail()) {
        logger.error("Failed to open tmp ProffieOS INO");
        return _("Computer FS Error");
    }

    bool alreadyOutputConfigDefine{false};
    while (ino.good()) {
        std::string buffer;
        std::getline(ino, buffer);


        // This one doesn't need to be replaced, but I've been doing it for
        // a while now, no real reason to stop I guess.
        constexpr cstring COMMENTED_LINE{
            R"(// #define CONFIG_FILE "config/YOUR_CONFIG_FILE_NAME_HERE.h")"
        };
        constexpr cstring UNCOMMENTED_LINE{
            R"(#define CONFIG_FILE)"
        };

        if (
                buffer.starts_with(COMMENTED_LINE) or
                buffer.starts_with(UNCOMMENTED_LINE)
           ) {
            if (not alreadyOutputConfigDefine) {
                tmpIno << "#define CONFIG_FILE \"config/";
                tmpIno << outName << "\"\n";
                alreadyOutputConfigDefine = true;
            }
        } else if (buffer.starts_with(R"(const char version[] = ")")) {
            tmpIno << R"(const char version[] = ")";
            tmpIno << config.os()->version_.string() << "\";\n";
        } else {
            tmpIno << buffer << '\n';
        }
    }
    ino.close();
    tmpIno.close();

    std::error_code errCode;
    if (not files::copyOverwrite(tmpInoPath, inoPath, errCode)) {
        logger.error("Failed to copy in tmp ProffieOS INO: " + errCode.message());
        return _("Computer FS Error");
    }

    constexpr cstring COMPILE_MESSAGE{wxTRANSLATE("Compiling ProffieOS...")};
    prog.set(40, wxGetTranslation(COMPILE_MESSAGE));
    logger.info(COMPILE_MESSAGE);

    wxString output;
    std::array<char, 32> buffer;

    Process proc;
    std::vector<std::string> args{
        "compile",
        "-b",
    };

    const auto& board{*config.board()};

    args.push_back(board.coreId_);
    args.emplace_back("--board-options");

    std::string options;
    auto massStorage{data::context(config.settings_.massStorage_)};
    auto webUSB{data::context(config.settings_.webUsb_)};

    if (massStorage.val() and webUSB.val()) options = "usb=cdc_msc_webusb";
    else if (webUSB.val()) options = "usb=cdc_webusb";
    else if (massStorage.val()) options = "usb=cdc_msc";
    else options = "usb=cdc";

    using versions::detail::BOARDS;
    using enum versions::detail::BoardIdx;
    if (board.name_ == BOARDS[eBoard_Proffie_V3].name_) {
        options +=",dosfs=sdmmc1";
    }

    args.push_back(std::move(options));
    args.push_back(osPath.string());
    args.emplace_back("-v");
    cli(proc, args);

    std::string compileOutput{};
    while(auto buffer = proc.read()) {
        if (prog.cancelled()) {
            proc.interrupt();
            return _("Cancelled");
        }

        prog.pulse();
        compileOutput += *buffer;
    }

    auto res{proc.finish()};
    if (res.err_) {
        logger.error(
            "Process error: " + std::to_string(res.err_) + ':' +
            std::to_string(res.systemResult_) + "\n" +
            compileOutput
        );
        return parseError(compileOutput, config);
    }

    if (compileOutput.find("error") != std::string::npos) {
        logger.error(compileOutput);
        return parseError(compileOutput, config);
    }

    CompileOutput ret;

    constexpr std::string_view DFU_STR{"ProffieOS.ino.dfu"};
#   ifdef _WIN32
    constexpr std::string_view DFU_SUFFIX_STR{"dfu-suffix.exe"};
    constexpr std::string_view ROOT_SUFFIX_STR{"C:\\"};
    constexpr std::string_view ROOT_DFU_STR{"C:\\"};
#   else
    // Because "dfu-suffix" appears in other output and not just invocation,
    // search for "-v" as a hacky way of differentiation.
    constexpr std::string_view DFU_SUFFIX_STR{"dfu-suffix -v"};
    // Similarly the root on unix-like systems is not distinct like on
    // Windows, so a bit of finangling is needed.
    constexpr std::string_view ROOT_SUFFIX_STR{"\n/"};
    constexpr std::string_view ROOT_DFU_STR{" /"};
#   endif
    constexpr cstring UTIL_ERR{wxTRANSLATE("Failed to find required utilities")};
    auto dfuPos{compileOutput.rfind(DFU_STR)};
    auto dfuRootPos{compileOutput.rfind(ROOT_DFU_STR, dfuPos)};
    auto dfuSuffixPos{compileOutput.rfind(DFU_SUFFIX_STR)};
    auto dfuSuffixRootPos{compileOutput.rfind(ROOT_SUFFIX_STR, dfuSuffixPos)};
    if (
            dfuPos != std::string::npos and
            dfuRootPos != std::string::npos and
            dfuSuffixPos != std::string::npos and
            dfuSuffixRootPos != std::string::npos
       ) {
        logger.debug("Parsing utility paths...");

#       ifdef _WIN32
        std::array<char, MAX_PATH> shortPath;
        DWORD res{};
#       endif

        const auto dfuLongPath{compileOutput.substr(
            dfuRootPos,
            dfuPos - dfuRootPos + DFU_STR.length()
        )};

#       ifdef _WIN32
        res = GetShortPathNameA(
            dfuLongPath.c_str(), shortPath.data(), shortPath.size()
        );
        if (res == 0) {
            logger.error("Failed to find dfu file in output: " + compileOutput);
            return wxGetTranslation(UTIL_ERR);
        }

        ret.dfuFile_ = shortPath.data();
#       else
        ret.dfuFile_ = dfuLongPath;
#       endif

        logger.debug("Parsed dfu file: " + ret.dfuFile_);

        const auto dfuSuffixLongPath{compileOutput.substr(
            dfuSuffixRootPos, dfuSuffixPos - dfuSuffixRootPos
        )};

#       if _WIN32
        res = GetShortPathNameA(
            dfuSuffixLongPath.c_str(), shortPath.data(), shortPath.size()
        );
        if (res == 0) {
            logger.error("Failed to find dfu suffix in output: " + compileOutput);
            return wxGetTranslation(UTIL_ERR);
        }

        ret.suffixPath_ = std::string{shortPath.data()} + "stm32l4-upload.bat";
#       else
        ret.suffixPath_ = dfuSuffixLongPath + "stm32l4-upload";
        // Pop off `\n` the POSIX version needs to find the path root.
        ret.suffixPath_.erase(0, 1);
#       endif

        logger.debug("Parsed upload file: " + ret.suffixPath_);
    }

    if (ret.dfuFile_.empty() or ret.suffixPath_.empty()) {
        logger.error("Failed to find utilities in output: " + compileOutput);
        return wxGetTranslation(UTIL_ERR);
    }

    // Set to negatives to mark missing
    ret.used_ = -1;
    ret.total_ = -1;

    constexpr std::string_view USED_PREFIX{"Sketch uses "};
    constexpr std::string_view MAX_PREFIX{"Maximum is "};
    const auto usedPrefixPos{compileOutput.find(USED_PREFIX)};
    const auto maxPrefixPos{compileOutput.find(MAX_PREFIX)};
    if (
            usedPrefixPos != std::string::npos and
            maxPrefixPos != std::string::npos
       ) {
        const auto usedPos{usedPrefixPos + USED_PREFIX.length()};
        const auto maxPos{maxPrefixPos + MAX_PREFIX.length()};

        const auto used{strtoul(&compileOutput[usedPos], nullptr, 10)};
        const auto total{strtoul(&compileOutput[maxPos], nullptr, 10)};

        ret.used_ = static_cast<int32>(used);
        ret.total_ = static_cast<int32>(total);
    } else {
        logger.warn("Usage data not found in compilation output.");
    }

    logger.info("Success");
    return ret;
}

std::optional<wxString> upload(
    const std::string& boardPath,
    const config::Config& config,
    const CompileOutput& compileOutput,
    pcui::ProgressDialog& prog,
    logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("arduino::upload()")};

    bool isBootloader{boardPath == "BOOTLOADER"};
    if (not isBootloader) {
        constexpr cstring CHECK_PRESENCE_MESSAGE{wxTRANSLATE("Checking board presence...")};
        prog.set(10, wxGetTranslation(CHECK_PRESENCE_MESSAGE));

        auto boards{arduino::getBoards(logger.binfo(CHECK_PRESENCE_MESSAGE))};
        bool found{false};
        for (const auto& path : boards) {
            if (path == boardPath) {
                found = true;
                break;
            }
        }

        if (not found) {
            logger.warn("Board was not found.");
            return _("Please make sure your board is connected and selected, then try again!");
        }
    }

    if (not isBootloader) {
        prog.pulse("Rebooting Proffieboard...");

        SerialMonitor mon;

        if (auto err{mon.open(boardPath)}) {
            logger.warn("Could not open board port.");
            return wxString::Format(
                _("Board was not reachable for reboot (%d:%d)"),
                err.rsn_, err.code_
            );
        }

        if (auto err{mon.write("\r\nRebootDFU\r\n")}) {
            return wxString::Format(
                _("Board reboot failed (%d:%d)"),
                err.rsn_, err.code_
            );
        }

        mon.close();

        std::this_thread::sleep_for(5s);
    }

    prog.pulse(_("Uploading to Proffieboard..."));

    Process proc;
    std::array<std::string, 3> args{
        "0x1209",
        "0x6668",
        compileOutput.dfuFile_
    };
    proc.create(compileOutput.suffixPath_, args);

    std::string uploadOutput;
    while (auto buffer{proc.read()}) {
        const auto percentPos{buffer->find('%')};
        if (percentPos != std::string::npos and percentPos >= 3) {
            const auto percent{strtoul(
                &(*buffer)[percentPos - 3], nullptr, 10
            )};

            prog.set(percent);
            logger.verbose("Progress: " + std::to_string(percent) + '%');
        }

        uploadOutput += *buffer;
    }

    if (
            uploadOutput.rfind("error") != std::string::npos or
            uploadOutput.rfind("FAIL") != std::string::npos
       ) {
        logger.error(uploadOutput);
        return parseError(uploadOutput, config);
    }

    auto res{proc.finish()};
    if (res.err_) {
        logger.error(
            "Process error: " + std::to_string(res.err_) + ':' +
            std::to_string(res.systemResult_) +
            "\n" + uploadOutput
        );
        return _("Unknown Upload Error");
    }

    // TODO: Don't remember if this happens on non-msw so guard it for now
#   ifdef _WIN32
    if (uploadOutput.find("File downloaded successfully") == std::string::npos) {
        logger.error(uploadOutput);
        return parseError(uploadOutput, config);
    }
#   endif

    logger.info("Success");
    return std::nullopt;
}

std::optional<wxString> precheckCompile(
    const config::Config& config, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("arduino::precheckCompile()")};

    if (config.os() == nullptr) {
        logger.error("Configuration doesn't have an OS Version selected, cannot compile.");
        return _("Please select an OS Version");
    }

    if (config.board() == nullptr) {
        logger.error("Board not selected.");
        return _("Please select a board");
    }

    auto bladeConfigs{data::context(config.bladeConfigs_)};
    if (bladeConfigs.children().empty()) {
        logger.error("Config has no blade arrays, cannot compile.");
        return _("Config must have at least one blade array to compile.");
    }

    auto styles{data::context(config.styles_)};
    std::unordered_set<std::string> aliasNames;
    for (const auto& model : styles.children()) {
        auto& style{dynamic_cast<config::styles::Style&>(*model)};

        auto ctxt{data::context(style.name_)};
        const auto& name{ctxt.val()};

        if (aliasNames.contains(name)) {
            constexpr cstring MSG{wxTRANSLATE("Config has style aliases with duplicate name \"%s\".")};
            logger.error(wxString::Format(MSG, name).ToStdString());
            return wxString::Format(wxGetTranslation(MSG), name).ToStdString();
        }

        aliasNames.insert(name);
    }

    return std::nullopt;
}

wxString parseError(const std::string& err, const config::Config& config) {
    if (err.contains("select Proffieboard")) {
        return "Please ensure you've selected the correct board in General";
    }

    if (err.contains("expected unqualified-id")) {
        return _(
            "Please make sure there are no brackets in your styles (such as \"{\" or \"}\")\n"
            "and there is nothing missing or extra from your style! (such as parentheses or \"<>\")"
        );
    }

    if (err.contains(/* region FLASH */"overflowed")) {
        constexpr std::string_view OVERFLOW_PREFIX{"region `FLASH' overflowed by "};

        const auto maxBytes{err.find("ProffieboardV3") != std::string::npos
            ? 507904
            : 262144
        };

        const auto overflowPos{err.rfind(OVERFLOW_PREFIX)};
        wxString errMessage;
        if (overflowPos != std::string::npos) {
            const auto overflowBytes{strtoul(
                &err[overflowPos + OVERFLOW_PREFIX.length()], nullptr, 10
            )};
            const auto percent{
                (static_cast<float64>(overflowBytes) * 100.0 / maxBytes)
                + 100.0
            };

            errMessage = wxString::Format(
                _("The specified config uses %.2f%% of board space, and will not fit on the Proffieboard. (%d overflow)"),
                percent,
                overflowBytes
            );
        } else {
            errMessage = _("The specified config will not fit on the Proffieboard.");
        }

        errMessage += "\n\n";
        errMessage += _("Try disabling diagnostic commands, disabling talkie, disabling prop features, or removing blade styles to make it fit.");
        return errMessage;
    }

    if (err.contains("Serial port busy")) {
        return _(
            "The Proffieboard appears busy.\n"
            "Please make sure nothing else is using it, then try again."
        );
    }

    if (err.contains("Buttons for operation")) {
        return wxString::Format(
            _("%s prop file:\n%s"),
            config.prop()->name_,
            std::strstr(err.data(), "requires")
        );
    }

    if (err.contains("Cannot open DFU device")) {
        return _("Looks like there's some problems accessing the Proffieboard.") + '\n' +
            _("Try re-installing the Proffie driver, and make sure you don't have other software which might interfere.");
    }

    if (
        err.contains("\n1") and
        err.contains("\n2") and
        err.contains("\n3") and
        err.contains("\n4") and
        err.contains("\n5") and
        err.contains("\n6") and
        err.contains("\n7") and
        err.contains("\n8") and
        err.contains("\n9")
       ) {
        return _("Could not connect to Proffieboard for upload.");
    }

    if (err.contains("No DFU capable USB device available")) {
        return "No Proffieboard in BOOTLOADER mode found.";
    }

    if (const auto *prop{config.prop()}) {
        for (const auto& [ arduino, display ] : prop->errors()) {
            if (err.find(arduino) != std::string::npos) {
                return wxString::Format(
                    _("%s prop error:\n%s"),
                    prop->name_,
                    display
                );
            }
        }
    }

    if (err.contains("error:")) {
        const auto errPos{err.find("error:")};
        const auto fileData{err.rfind('/', errPos)};
        return err.substr(fileData + 1, MAX_ERRMESSAGE_LENGTH);
    }

    return wxString::Format(
        _("Unknown error:\n%s"),
        err.substr(0, MAX_ERRMESSAGE_LENGTH)
    );
}

std::optional<wxString> ensureCoreInstalled(
    const std::string& coreVersion,
    const std::string& coreURL,
    logging::Logger& logger,
    pcui::ProgressDialog *prog
) {
    constexpr cstring MSG{wxTRANSLATE("Ensuring Core Installation...")};
    if (prog) prog->set(15, wxGetTranslation(MSG));
    logger.info(MSG);

    Process proc;
    std::vector<std::string> args{
        "core",
        "install",
        "proffieboard:stm32l4@" + coreVersion,
        "--additional-urls",
        coreURL
    };
    cli(proc, args);

    std::string coreInstallOutput;
    while (auto buffer{proc.read()}) {
        if (prog) prog->pulse();
        coreInstallOutput += *buffer;
    }

    auto res{proc.finish()};
    if (res.err_) {
        logger.error(
            "Process error: " + std::to_string(res.err_) + ':' +
            std::to_string(res.systemResult_) +
            "\n" + coreInstallOutput
        );
        return _("Could Not Install Core");
    }

    return std::nullopt;
}

void cli(Process& proc, std::vector<std::string>& args) {
    args.emplace_back("--no-color");
    auto arduinoStr{(paths::binaryDir() / "arduino-cli").string()};
    proc.create(arduinoStr, args);
}

} // namespace

#if defined(_WIN32) or defined(__linux__)
bool arduino::runDriverInstallation() {
    auto& logger{logging::Context::getGlobal().createLogger("arduino::runDriverInstallation()")};
    logger.info("Installing drivers...");

#   if defined(__linux__)
    Process proc;

    auto err{ensureCoreInstalled(
        "3.6",
        "https://profezzorn.github.io/arduino-proffieboard/package_proffieboard_index.json",
        logger,
        nullptr
    )};
    if (err) return false;

    const auto rulesPath{
        paths::user() / ".arduino15" / "packages" / "proffieboard" /
        "hardware" / "stm32l4" / "3.6" / "drivers" / "linux"
    };

    std::vector<std::string> args;
    args.emplace_back("cp");

    std::error_code ec;
    fs::directory_iterator directoryIter{rulesPath, ec};
    if (ec) {
        logger.error("Could not access driver path " + rulesPath.string() + ": " + ec.message());
        return false;
    }

    for (const auto& entry : directoryIter) {
        if (not entry.is_regular_file(ec)) continue;
        if (not entry.path().string().ends_with("rules")) continue;

        args.emplace_back(entry.path().string());
    }

    args.emplace_back("/etc/udev/rules.d");

    proc.create("pkexec", args);

    auto result{proc.finish()};
#   elif defined(_WIN32)
    auto driverStr{(paths::binaryDir() / "proffie-dfu-setup.exe").string()};
    auto result{Process::elevatedProcess(driverStr.c_str())};
#   endif

    if (result.err_) {
        logger.error("Installation failed with error " + std::to_string(result.err_) + ":" + std::to_string(result.systemResult_));
        return false;
    }

    return true;
}
#endif

