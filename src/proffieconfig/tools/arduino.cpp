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
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <devguid.h>
#include <fileapi.h>
#include <handleapi.h>
#include <setupapi.h>
#include <synchapi.h>
#else
#include <thread>
#include <termios.h>
#endif

#include <wx/dialog.h>
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
#include "ui/build.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/text.hpp"
#include "ui/dialogs/message.hpp"
#include "ui/dialogs/progress.hpp"
#include "ui/helpers/dialog_buttons.hpp"
#include "ui/helpers/if.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "ui/values.hpp"
#include "utils/defer.hpp"
#include "utils/files.hpp"
#include "utils/paths.hpp"
#include "utils/rand.hpp"
#include "utils/types.hpp"
#include "ui/utils.hpp"
#include "versions/detail/boards.hpp"
#include "versions/detail/strings.hpp"

#include "serialmonitor.hpp"
#include "../core/state.hpp"

using namespace std::chrono_literals;

namespace {

struct Error {
    enum class Type {
        None,
        Simple,
        Detailed,
    };
    using enum Type;

    Error(Type type, wxString message = {}) :
        type_{type}, message_(std::move(message)) {}

    Type type_;
    wxString message_;

    explicit operator bool() const {
        return type_ != Type::None;
    }

    void show(pcui::ProgressDialog&) const;

private:
    void doShow(pcui::ProgressDialog&) const;
};

Error operator|(Error::Type type, wxString str) {
    return {type, std::move(str)};
}

void cli(Process& proc, std::vector<std::string>& args);

Error compile(
    const std::string&,
    const config::Config&,
    arduino::CompileOutput&,
    pcui::ProgressDialog&,
    logging::Branch&
);

void processCache(arduino::CompileInfo&, logging::Logger&);

Error upload(
    const arduino::Port&,
    const std::string& binPath,
    pcui::ProgressDialog&,
    logging::Branch&
);

/**
 * Pre-checks specifically for compilation.
 * (May be fine for saving though)
 */
Error precheckCompile(
    const config::Config&, logging::Branch&
);

Error parseError(const std::string&, const config::Config&); 

Error ensureCoreInstalled(
    const std::string& coreVersion,
    const std::string& coreURL,
    logging::Logger&,
    pcui::ProgressDialog *
);

std::string dfu_util_path;

constexpr cstring FALLBACK_CORE_VER{"3.6"};
constexpr cstring FALLBACK_CORE_URL{"https://profezzorn.github.io/arduino-proffieboard/package_proffieboard_index.json"};

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
        return wxGetTranslation(UNKNOWN_STR).utf8_string();
    }

    const auto versionStart{versionTagPos + VERSION_TAG.length()};
    const auto versionEnd{output.find(' ', versionStart)};
    if (versionEnd == std::string::npos) {
        return wxGetTranslation(UNKNOWN_STR).utf8_string();
    }

    return output.substr(versionStart, versionEnd - versionStart);
}

float64 arduino::CompileOutput::percent() const {
    return (static_cast<float64>(used_) / total_) * 100.0;
}

wxString arduino::CompileOutput::usageMessage() const {
    constexpr cstring USAGE_MESSAGE{wxTRANSLATE(
        "The configuration uses %.2f%% of board space. (%d/%d)"
    )};

    return wxString::Format(
        wxGetTranslation(USAGE_MESSAGE),
        percent(),
        used_,
        total_
    );
}

std::vector<arduino::Port> arduino::getBoards(logging::Branch *lBranch) {
    auto& logger{logging::Branch::optCreateLogger("arduino::getBoards()", lBranch)};

    bool found_bootloader{false};
    std::vector<arduino::Port> boards;

#   if _WIN32
    // Retrieve the device info list for all connected USB devices.
    HDEVINFO info_set{SetupDiGetClassDevsA(
        nullptr,
        "USB",
        nullptr,
        DIGCF_ALLCLASSES | DIGCF_PRESENT
    )};
    if (info_set == INVALID_HANDLE_VALUE) {
        std::error_code ec(
            static_cast<int32>(GetLastError()),
            std::system_category()
        );
        logger.error("Failed retrieving USB device info list: " + ec.message());
        return {};
    }

    // Make sure it's cleaned up once we're done here.
    defer {
        if (not SetupDiDestroyDeviceInfoList(info_set)) {
            std::error_code ec(
                static_cast<int32>(GetLastError()),
                std::system_category()
            );
            logger.warn("Failed destroying device info list: " + ec.message());
        }
    };

    SP_DEVINFO_DATA info_data;
    info_data.cbSize = sizeof info_data;
    for (DWORD idx{0}; SetupDiEnumDeviceInfo(info_set, idx, &info_data); ++idx) {
        const auto check_usb_id{[&](uint16 check_vid, uint16 check_pid) {
            BOOL reg_success{};

            // See how much to allocate to retrieve.
            DWORD reg_length{};
            reg_success = SetupDiGetDeviceRegistryPropertyA(
                info_set,
                &info_data,
                SPDRP_HARDWAREID,
                nullptr,
                nullptr,
                0,
                &reg_length
            );
            if (not reg_success) {
                // It seems counterintuitive, but this function still returns
                // this error even with the buffer null and size 0? Whatever...
                if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                    return false;
            }

            char *buf{new char[reg_length]};
            defer { delete[] buf; };

            // Fetch data
            DWORD reg_type{};
            reg_success = SetupDiGetDeviceRegistryPropertyA(
                info_set,
                &info_data,
                SPDRP_HARDWAREID,
                &reg_type,
                (PBYTE)buf,
                reg_length,
                nullptr
            );
            if (not reg_success)
                return false;

            // Sanity check the data type, so we know the null-termination is
            // there, otherwise Bad Things might happen.
            if (reg_type != REG_MULTI_SZ)
                return false;

            // Then test each entry to see if one matching the VID and PID
            // can be found.
            std::string_view view;
            for (
                    // Iteration begins at the buffer start, the first string
                    std::size_t buf_idx{0};
                    // Create a view, which will automatically find the null
                    // terminator, then test to see if the string is empty, if
                    // it is, that's the end.
                    view = &buf[buf_idx], not view.empty();
                    // Jump to the next string in the buffer
                    buf_idx += view.length() + 1
                ) {
                constexpr std::string_view VID_MARK{"VID_"};
                constexpr std::string_view PID_MARK{"&PID_"};

                const auto vid_mark_pos{view.find(VID_MARK)};
                if (vid_mark_pos == std::string::npos)
                    continue;

                const auto pid_mark_pos{view.find(PID_MARK)};
                if (pid_mark_pos == std::string::npos)
                    continue;

                const auto vid_pos{vid_mark_pos + VID_MARK.length()};
                const auto pid_pos{pid_mark_pos + PID_MARK.length()};

                const auto vid_end_pos{pid_mark_pos};
                // If there isn't another "&", then npos is the right value.
                const auto pid_end_pos{view.find('&', pid_pos)};

                const auto from_chars{[&](
                    std::size_t begin,
                    std::size_t end,
                    uint16& v
                ) {
                    auto [ptr, ec]{std::from_chars(
                        &view[begin],
                        end == std::string::npos
                            ? &*view.end()
                            : &view[end],
                        v,
                        16
                    )};
                    return ec == std::errc();
                }};

                uint16 vid{};
                if (not from_chars(vid_pos, vid_end_pos, vid))
                    continue;

                uint16 pid{};
                if (not from_chars(pid_pos, pid_end_pos, pid))
                    continue;

                if (vid == check_vid and pid == check_pid)
                    return true;
            }

            return false;
        }};

        // If this USB device is a COM port, check if its hardware ID matches
        // the ProffieOS Serial ID.
        if (
                info_data.ClassGuid == GUID_DEVCLASS_PORTS and
                check_usb_id(0x1209, 0x6668)
           ) {
            HKEY hkey{SetupDiOpenDevRegKey(
                info_set,
                &info_data,
                DICS_FLAG_GLOBAL,
                0,
                DIREG_DEV,
                KEY_READ
            )};
            if (hkey == INVALID_HANDLE_VALUE) {
                std::error_code ec(
                    static_cast<int32>(GetLastError()),
                    std::system_category()
                );
                logger.info("Failed opening device registry: " + ec.message());
                continue;
            }

            std::error_code ec;
            DWORD reg_length{};
            char *buf{nullptr};
            defer { delete[] buf; };

            auto get_value{[&] -> std::error_code {
                auto err{RegGetValueA(
                    hkey,
                    nullptr,
                    "PortName",
                    RRF_RT_REG_SZ,
                    nullptr,
                    buf,
                    &reg_length
                )};
                return {
                    static_cast<int32>(err),
                    std::system_category()
                };
            }};
            if (
                    // Read to get length, alloc buf, then read again
                    (ec = get_value()) or
                    (buf = new char[reg_length], ec = get_value())
               ) logger.info("Failed reading device port: " + ec.message());

            auto& port{boards.emplace_back()};
            port.type_ = Port::Type::Proffie;
            port.serial_path_ = buf;
            continue;
        }

        // The generic USBDevice setup class GUID isn't defined anywhere?
        constexpr GUID GUID_DEVCLASS_USBDEVICE{0x88bae032L, 0x5a81, 0x49f0, {0xbc, 0x3d, 0xa4, 0xff, 0x13, 0x82, 0x16, 0xd6}};

        // If this USB device is a generic USB device, check if its hardware ID
        // matches the STM32 BOOTLOADER ID and that bootloader hasn't been found
        // yet.
        if (
                info_data.ClassGuid == GUID_DEVCLASS_USBDEVICE and
                check_usb_id(0x0483, 0xDF11) and
                not found_bootloader
           ) {
            auto& port{boards.emplace_back()};
            port.type_ = Port::Type::Bootloader;
            found_bootloader = true;
            continue;
        }
    }

    std::error_code ec(
        static_cast<int32>(GetLastError()),
        std::system_category()
    );
    if (ec.value() != ERROR_NO_MORE_ITEMS)
        logger.warn("Failed enumerating USB devices: " + ec.message());
#   endif

    logger.info("Discovered " + std::to_string(boards.size()) + " boards.");
    return boards;
}

void arduino::applyToBoard(
    const std::string& name,
    const arduino::Port& port,
    CompileInfo& info,
    pcui::ProgressDialog& prog
) {
    auto& logger{logging::Context::getGlobal().createLogger("arduino::applyToBoard()")};

    if (info.out_) {
        logger.info("Using cached binary: " + info.out_->dfuFile_);
    } else {
        auto err{compile(
            name,
            info.source_,
            info.out_.emplace(),
            prog,
            *logger.binfo("Compiling...")
        )};

        if (err) {
            info.out_.reset();
            err.show(prog);
            return;
        }

        processCache(info, logger);
    }

    auto err{upload(
        port,
        info.out_->dfuFile_,
        prog,
        *logger.binfo("Uploading...")
    )};
    if (err) {
        err.show(prog);
        return;
    }

    logger.info("Applied Successfully");

    wxString message{_("Config Applied Successfully!")};
    if (info.out_->total_ != -1) {
        message += "\n\n";
        message += info.out_->usageMessage();
    }

    prog.finish(true, message);
}

void arduino::verifyConfig(
    const std::string& name,
    CompileInfo& info,
    pcui::ProgressDialog& prog
) {
    auto& logger{logging::Context::getGlobal().createLogger("arduino::verifyConfig()")};

    if (info.out_) {
        logger.info("Using cached binary: " + info.out_->dfuFile_);
    } else {
        auto err{compile(
            name,
            info.source_,
            info.out_.emplace(),
            prog,
            *logger.binfo("Compiling...")
        )};

        if (err) {
            info.out_.reset();
            err.show(prog);
            return;
        }

        processCache(info, logger);
    }

    logger.info("Verified Successfully");

    wxString message{_("Config Verified Successfully!")};
    if (info.out_->total_ != -1) {
        message += "\n\n";
        message += info.out_->usageMessage();
    }

    prog.finish(true, message);
}

arduino::CompileInfo& arduino::getCacheInfo(
    config::Config& config, bool clean
) {
    CompileInfo *info{nullptr};

    if (not clean)
        info = static_cast<CompileInfo *>(config.cache());

    if (info == nullptr) {
        info = new CompileInfo(config);
        config.cache(std::unique_ptr<CompileInfo>(info));
    }

    return *info;
}

#if defined(_WIN32) or defined(__linux__)
bool arduino::runDriverInstallation() {
    auto& logger{logging::Context::getGlobal().createLogger("arduino::runDriverInstallation()")};
    logger.info("Installing drivers...");

#   if defined(__linux__)
    Process proc;

    auto err{ensureCoreInstalled(
        FALLBACK_CORE_VER,
        FALLBACK_CORE_URL,
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

namespace {

Error compile(
    const std::string& name,
    const config::Config& config,
    arduino::CompileOutput& out,
    pcui::ProgressDialog& prog,
    logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("arduino::compile()")};
    std::optional<wxString> err;

    constexpr cstring PRECHK_MSG{wxTRANSLATE("Running compile prechecks...")};
    prog.set(5, wxGetTranslation(PRECHK_MSG));

    if (auto err{precheckCompile(config, *logger.binfo(PRECHK_MSG))})
        return err;

    if (auto err{ensureCoreInstalled(
                config.os()->coreVersion_,
                config.os()->coreUrl_,
                logger,
                &prog
                )}) return err;

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
                return Error::Simple | _("Invalid Prop Selected");
            }

            const auto propHeaderDest{osPath / "props" / prop->filename_};
            auto res{files::copyOverwrite(
                sourcePropHeader, propHeaderDest, err
            )};

            if (not res) {
                logger.error("Failed to copy in prop header.");
                return Error::Simple | _("OS FS Error");
            }
        }
    }

    const auto injectionsDest{osPath / "config" / config::priv::INJECTION_STR};
    auto injectionVec{data::context(config.injections_)};

    if (not injectionVec.children().empty()) {
        constexpr cstring PROPINST_MSG{wxTRANSLATE("Installing Injection Files...")};
        prog.set(25, wxGetTranslation(PROPINST_MSG));

        std::error_code ec;
        fs::create_directories(injectionsDest, ec);
        if (ec) {
            logger.error("Failed to create injections dir: " + ec.message());
            return Error::Simple | _("OS FS Error");
        }
    }

    for (const auto& model : injectionVec.children()) {
        auto& injection{dynamic_cast<config::Injection&>(*model)};

        const auto srcPath{paths::injectionDir() / injection.filename_};
        const auto dstPath{injectionsDest / injection.filename_};

        std::error_code err;
        if (not files::copyOverwrite(srcPath, dstPath, err)) {
            logger.error("Failed to copy injection file \"" + srcPath.string() + "\" to \"" + dstPath.string() + "\": " + err.message());
            return Error::Simple | _("OS FS Error");
        }
    }

    // Use a prefix on the name, "ProffieConfig" so that the core config
    // headers can't be overwritten. I could check for this elsewhere, but it's
    // a lot easier to just prevent it this way.
    const auto outName{"ProffieConfig " + name + ".h"};
    const auto configPath{osPath / "config" / outName};

    constexpr cstring GENERATE_MESSAGE{wxTRANSLATE("Generating configuration file...")};
    prog.set(30, wxGetTranslation(GENERATE_MESSAGE));
    if (auto err{config::generate(
                config,
                configPath,
                logger.binfo(GENERATE_MESSAGE)
                )}) {
        return Error::Simple | wxString::Format(
            _("Configuration Generation Error:\n%s"),
            *err
        );
    }

    constexpr cstring UPDATE_INO_MESSAGE{wxTRANSLATE("Updating ProffieOS file...")};
    prog.set(35, wxGetTranslation(UPDATE_INO_MESSAGE));
    logger.info(UPDATE_INO_MESSAGE);

    const auto inoPath{osPath / "ProffieOS.ino"};
    const auto tmpInoPath{fs::temp_directory_path() / "ProffieOS.ino"};
    auto ino{files::openInput(inoPath)};
    if (ino.fail()) {
        logger.error("Failed to open ProffieOS INO");
        return Error::Simple | _("OS Inaccessible or Corrupted");
    }

    auto tmpIno{files::openOutput(tmpInoPath)};
    if (tmpIno.fail()) {
        logger.error("Failed to open tmp ProffieOS INO");
        return Error::Simple | _("OS FS Error");
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
        return Error::Simple | _("OS FS Error");
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
    while (auto buffer = proc.read()) {
        if (prog.cancelled()) {
            proc.interrupt();
            return Error::Simple | _("Cancelled");
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

    constexpr std::string_view DFU_FILE_STR{"ProffieOS.ino.dfu"};
#   ifdef _WIN32
    constexpr std::string_view DFU_SUFFIX_STR{"dfu-suffix.exe"};
    constexpr std::string_view DFU_FILE_ROOT_STR{"C:\\"};
    constexpr std::string_view DFU_SUFFIX_ROOT_STR{"C:\\"};
#   else
    // Because "dfu-suffix" appears in other output and not just invocation,
    // search for "-v" as a hacky way of differentiation.
    constexpr std::string_view DFU_SUFFIX_STR{"dfu-suffix -v"};
    // Similarly the root on unix-like systems is not distinct like on
    // Windows, so a bit of finangling is needed.
    constexpr std::string_view DFU_FILE_ROOT_STR{" /"};
    constexpr std::string_view DFU_SUFFIX_ROOT_STR{"\n/"};
#   endif
    constexpr cstring UTIL_ERR{wxTRANSLATE("Failed to find required utilities")};

    const auto dfu_file_pos{compileOutput.rfind(DFU_FILE_STR)};
    const auto dfu_file_root_pos{compileOutput.rfind(
        DFU_FILE_ROOT_STR,
        dfu_file_pos
    )};
    const auto dfu_suffix_pos{compileOutput.rfind(DFU_SUFFIX_STR)};
    const auto dfu_suffix_root_pos{compileOutput.rfind(
        DFU_SUFFIX_ROOT_STR,
        dfu_suffix_pos
    )};

    if (
            dfu_file_pos != std::string::npos and
            dfu_file_root_pos != std::string::npos and
            dfu_suffix_pos != std::string::npos and
            dfu_suffix_root_pos != std::string::npos
       ) {
        logger.debug("Parsing utility paths...");

#       ifdef _WIN32
        std::array<char, MAX_PATH> short_path;
        DWORD res{};
#       endif

        const auto dfu_file_long_path{compileOutput.substr(
            dfu_file_root_pos,
            dfu_file_pos - dfu_file_root_pos + DFU_FILE_STR.length()
        )};

#       ifdef _WIN32
        res = GetShortPathNameA(
            dfu_file_long_path.c_str(),
            short_path.data(),
            short_path.size()
        );
        if (res == 0) {
            logger.error("Failed to convert DFU file long path: " + dfu_file_long_path);
            return Error::Simple | wxGetTranslation(UTIL_ERR);
        }

        out.dfuFile_ = short_path.data();
#       else
        out.dfuFile_ = dfu_file_long_path;
        // Pop off ' ' the POSIX version needs to find path root.
        out.dfuFile_.erase(0, 1);
#       endif

        logger.debug("Parsed dfu file: " + out.dfuFile_);

        const auto dfu_suffix_long_path{compileOutput.substr(
            dfu_suffix_root_pos,
            dfu_suffix_pos - dfu_suffix_root_pos
        )};

#       ifdef _WIN32
        res = GetShortPathNameA(
            dfu_suffix_long_path.c_str(),
            short_path.data(),
            short_path.size()
        );
        if (res == 0) {
            logger.error("Failed to convert dfu-suffix long path: " + dfu_suffix_long_path);
            return Error::Simple | wxGetTranslation(UTIL_ERR);
        }

        dfu_util_path = std::string{short_path.data()} + "dfu-util.exe";
#       else
        dfu_util_path = dfu_suffix_long_path + "dfu-util";
        // Pop off `\n` the POSIX version needs to find the path root.
        dfu_util_path.erase(0, 1);
#       endif

        logger.debug("Updated DFU util Path: " + dfu_util_path);
    }

    if (out.dfuFile_.empty() or dfu_util_path.empty()) {
        logger.error("Failed to find utilities in output: " + compileOutput);
        return Error::Simple | wxGetTranslation(UTIL_ERR);
    }

    // Set to negatives to mark missing
    out.used_ = -1;
    out.total_ = -1;

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

        out.used_ = static_cast<int32>(used);
        out.total_ = static_cast<int32>(total);
    } else {
        logger.warn("Usage data not found in compilation output.");
    }

    logger.info("Success");
    return Error::None;
}

void processCache(arduino::CompileInfo& info, logging::Logger& logger) {
    std::error_code ec;
    auto path{fs::temp_directory_path(ec)};
    if (ec) {
        logger.warn("Couldn't get temp directory: " + ec.message());
        return;
    }

    path /= "ProffieConfig_DFUCache";
    fs::create_directories(path, ec);

    if (ec) {
        logger.warn("Couldn't create cache directory: " + ec.message());
        return;
    }

    // The hash doesn't account for the name, so just come up with a
    // probably-not-yet-existent ID for the path.
    path /= std::to_string(utils::rand::get<uint64>());

    fs::rename(info.out_->dfuFile_, path, ec);
    if (ec) {
        logger.warn("Couldn't cache binary: " + ec.message());
        return;
    }

    info.out_->dfuFile_ = path.string();
}

Error upload(
    const arduino::Port& port,
    const std::string& binPath,
    pcui::ProgressDialog& prog,
    logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("arduino::upload()")};

    std::size_t num_tries{1};
    if (port.type_ == arduino::Port::Type::Proffie) {
        num_tries = 10;

        prog.pulse("Rebooting Proffieboard...");

        SerialMonitor mon;
        if (auto err{mon.open(port.serial_path_)}) {
            logger.warn("Could not open board port.");

            return Error::Simple | wxString::Format(
                _("Board was not reachable for reboot (%d:%d)"),
                err.rsn_, err.code_
            );
        }

        if (auto err{mon.write("\r\nRebootDFU\r\n")}) {
            return Error::Simple | wxString::Format(
                _("Board reboot failed (%d:%d)"),
                err.rsn_, err.code_
            );
        }

        mon.close();
    }

    prog.pulse(_("Uploading to Proffieboard..."));

    for (; num_tries; --num_tries) {
        // A short delay to backoff and/or let the board actually reboot.
        std::this_thread::sleep_for(1'000ms);

        Process proc;
        std::string args[]{
            // Device w/ ProffieOS Serial Runtime ID and STM32 BOOTLOADER DFU mode
            // ID. Afaics, the runtime ID is just used to validate the DFU file.
            "-d", "1209:6668,0483:df11",
            // Alt Zero, Internal Flash
            "-a", "0",
            // DfuSe needs the address specified, despite the warning in dfu-util's
            // manpage giving general advice otherwise.
            // `:leave` triggers a reset to normal application code after download.
            "-s", "0x08000000:leave",
            // "Download" this DFU binary
            "-D", binPath,
        };
        proc.create(dfu_util_path, args);

        std::string output;
        while (auto buffer{proc.read()}) {
            const auto percent_pos{buffer->find('%')};
            if (percent_pos != std::string::npos and percent_pos >= 3) {
                const auto percent{strtoul(
                    &(*buffer)[percent_pos - 3], nullptr, 10
                )};

                prog.set(percent);
                logger.verbose("Progress: " + std::to_string(percent) + '%');
            }

            output += *buffer;
        }

        auto res{proc.finish()};
        if (res.err_ and res.err_ != Process::Result::eExited_With_Failure) {
            logger.error(
                "Process error: " + std::to_string(res.err_) + ':' +
                std::to_string(res.systemResult_) +
                "\n" + output
            );
            return Error::Simple | _("Upload process failed");
        }

        if (output.contains("File downloaded successfully")) {
            logger.info("Success");
            return Error::None;
        }

        if (output.contains("No DFU capable USB device available"))
            continue;

        if (output.contains("Cannot open DFU device")) {
            return Error::Simple | _("Looks like there's some problems accessing the Proffieboard.") + '\n' +
                _("Try re-installing the Proffie driver, and make sure you don't have other software which might interfere.");
        }

        logger.error("Unknown error: " + output);
        return Error::Simple | _("Unknown error during upload");
    }

    return Error::Simple | _("Could not connect to Proffieboard for upload.");
}

Error precheckCompile(
    const config::Config& config, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("arduino::precheckCompile()")};

    if (config.os() == nullptr) {
        logger.error("Configuration doesn't have an OS version selected, cannot compile.");
        return Error::Simple | _("Please select an OS version in the editor");
    }

    if (config.board() == nullptr) {
        logger.error("Board version not selected.");
        return Error::Simple | _("Please select a board version in the editor");
    }

    auto bladeConfigs{data::context(config.bladeConfigs_)};
    if (bladeConfigs.children().empty()) {
        logger.error("Config has no blade arrays, cannot compile.");
        return Error::Simple | _("Config must have at least one blade array to compile.");
    }

    auto styles{data::context(config.styles_)};
    std::unordered_set<std::string> aliasNames;
    for (const auto& model : styles.children()) {
        auto& style{dynamic_cast<config::styles::Style&>(*model)};

        auto ctxt{data::context(style.name_)};
        const auto& name{ctxt.val()};

        if (aliasNames.contains(name)) {
            constexpr cstring MSG{wxTRANSLATE("Config has style aliases with duplicate name \"%s\".")};
            logger.error(wxString::Format(MSG, name).utf8_string());
            return Error::Simple | wxString::Format(wxGetTranslation(MSG), name).utf8_string();
        }

        aliasNames.insert(name);
    }

    if (const auto *prop{config.prop()}) {
        auto buttonsCtxt{data::context(config.buttons_)};
        const auto numButtons{buttonsCtxt.children().size()};
        const auto *buttons{prop->buttons(numButtons)};
        if (not buttons) {
            constexpr cstring MSG{wxTRANSLATE("Prop %s does not support %zu buttons.")};
            logger.error(wxString::Format(MSG, prop->name_, numButtons).utf8_string());
            return Error::Simple | wxString::Format(wxGetTranslation(MSG), prop->name_, numButtons).utf8_string();
        }
    }

    return Error::None;
}

Error parseError(const std::string& err, const config::Config& config) {
    // Don't handle errors which are location-sensitive.
    // I.e. keep any syntax-type errors that ProffieConfig pre-check doesn't
    // catch to make sure the full error is presented to the user.
    //
    // If there are specific cases I want to test for like that, they need to
    // happen during pre-checking where I can provide in-ProffieConfig
    // reference to the error "location," not here.

    if (err.contains("select Proffieboard")) {
        return Error::Simple | _("Please ensure you've selected the correct board in General");
    }

    if (err.contains("out of memory allocating")) {
        return Error::Simple | _(
            "The compiler ran out of memory. Your config must be very large.\n"
            "If you're not sure what to do, reach out to me or post on The Crucible with your config."
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
        return Error::Simple | errMessage;
    }

    if (err.contains("Serial port busy")) {
        return Error::Simple | _(
            "The Proffieboard appears busy.\n"
            "Please make sure nothing else is using it, then try again."
        );
    }

    if (err.contains("Buttons for operation")) {
        return Error::Simple | wxString::Format(
            _("%s prop file:\n%s"),
            config.prop()->name_,
            std::strstr(err.data(), "requires")
        );
    }

    if (const auto *prop{config.prop()}) {
        for (const auto& [ arduino, display ] : prop->errors()) {
            if (err.find(arduino) != std::string::npos) {
                return Error::Simple | wxString::Format(
                    _("%s prop error:\n%s"),
                    prop->name_,
                    display
                );
            }
        }
    }

    if (auto err_pos{err.find("error:")}; err_pos != std::string::npos) {
        // Reverse search to find where the error line started, first looking
        // for the ProffieOS directory, and then the config prefix.
        // This should provide a relatively correct printout starting at the
        // actual error text. And the full output is in the log file in any
        // case.

#       if _WIN32
#           define SEP "\\"
#       else
#           define SEP "/"
#       endif
        constexpr std::string_view DIR_PREFIX{SEP "ProffieOS" SEP};
        constexpr std::string_view CONFIG_PREFIX{SEP "ProffieConfig "};
#       undef SEP

        const auto dir_pos{err.rfind(DIR_PREFIX, err_pos)};
        if (dir_pos == std::string::npos)
            return Error::Detailed | err.substr(err_pos);

        // Find the config prefix within the subview that wraps the dir
        // prefix and the error marker
        const auto dir_substr{std::string_view(err).substr(
            dir_pos,
            err_pos - dir_pos
        )};
        const auto config_prefix_pos{dir_substr.find(CONFIG_PREFIX)};
        if (config_prefix_pos == std::string::npos)
            return Error::Detailed | err.substr(dir_pos);

        // Calculate position where config name starts, note that
        // config_prefix_pos is an index into the subview.
        const auto config_pos{
            dir_pos +
            config_prefix_pos +
            CONFIG_PREFIX.length()
        };

        return Error::Detailed | err.substr(config_pos);
    }

    return Error::Detailed | err;
}

Error ensureCoreInstalled(
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
        return Error::Simple | _("Could Not Install Core");
    }

    return Error::None;
}

void cli(Process& proc, std::vector<std::string>& args) {
    args.emplace_back("--no-color");
    auto arduinoStr{(paths::binaryDir() / "arduino-cli").string()};
    proc.create(arduinoStr, args);
}

void Error::show(pcui::ProgressDialog& prog) const {
    pcui::safeCall([err=*this, &prog] {
        err.doShow(prog);
    });
}

void Error::doShow(pcui::ProgressDialog& prog) const {
    if (type_ == Error::Simple) {
        prog.finish(true, message_);
        return;
    }

    auto *dlg{new pcui::Dialog(
        prog.parent(),
        wxID_ANY,
        prog.title(),
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
    )};
    dlg->Bind(wxEVT_SHOW, [dlg](wxShowEvent& evt) {
        evt.Skip();

        if (evt.GetEventObject() != dlg)
            return;

        if (not evt.IsShown())
            dlg->Destroy();
    });

    prog.finish(false);

    pcui::Stack stack{
      .base_={
        .expand_=true,
        .proportion_=1,
        .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
      },
      .orient_=wxVERTICAL,
      .children_={
        pcui::If{
          .cond_=type_ == Type::Detailed,
          .then_={
            pcui::Label{
              .label_=_("Unknown Error:")
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
          }
        }(),
        pcui::Text{
          .win_={
            .base_={
              .minSize_={600, 350},
              .expand_=true,
              .proportion_=1
            },
          },
          .data_=message_,
          .autoLink_=true,
          .font_=type_ == Type::Detailed
              ? pcui::Font::Monospace
              : pcui::Font::Normal,
          .style_=pcui::Text::MultiLine{
            .wrap_=pcui::Text::Wrap::None,
          },
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::DialogButtons{
          .ok_=pcui::Button{}(),
          .apply_=pcui::Button{
            .label_=_("Show Log"),
            .func_=[] {
                wxLaunchDefaultApplication((paths::logDir() / "ProffieConfig.log").string());
            },
          }(),
        }(),
      }
    };
    pcui::build(dlg, stack());

    dlg->Show();
    dlg->Raise();
}

} // namespace

