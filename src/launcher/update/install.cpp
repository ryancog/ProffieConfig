#include "install.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * launcher/update/install.cpp
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

#include <chrono>
#include <filesystem>
#include <future>
#include <thread>

#include <wx/webrequest.h>
#include <wx/filefn.h>
#include <wx/uri.h>

#include "app/app.hpp"
#include "log/logger.hpp"
#include "ui/dialogs/message.hpp"
#include "utils/files.hpp"
#include "utils/paths.hpp"
#include "utils/defer.hpp"

#include "update.hpp"

namespace {

inline fs::path stagingFolder() { return paths::dataDir() / "staging"; }

std::string convertSize(uint64 size);

} // namespace

bool Update::pullNewFiles(
    const Changelog& changelog,
    const Data& data,
    pcui::ProgressDialog *prog,
    logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("Update::pullNewFiles()")};
    std::error_code ec;

    prog->set(0, "Preparing to download new files...");
    std::promise<bool> requestPromise;
    fs::path downloadedFilename;
    Update::ItemType type{};

    // Allow this to fail?
    fs::remove_all(stagingFolder(), ec);

    auto stateHandler{[&](wxWebRequestEvent& evt) {
        const auto doHandleCompleted{[&] -> bool {
            auto filePath{stagingFolder() / downloadedFilename};

            fs::create_directories(filePath.parent_path(), ec);
            if (ec) {
                logger.error("Couldn't create " + filePath.parent_path().string() + ": " + ec.message());
                return false;
            }

            auto srcPath{evt.GetDataFile().utf8_string()};
            files::copyOverwrite(srcPath, filePath, ec);
            if (ec) {
                logger.error("Couldn't copy " + srcPath + " to " + filePath.string() + ": " + ec.message());
                return false;
            }

            if (type == Update::EXEC) {
                fs::permissions(
                    filePath,
                    fs::perms::owner_exec | fs::perms::others_exec,
                    fs::perm_options::add,
                    ec
                );

                if (ec) {
                    logger.error("Couldn't enable execution permission for " + filePath.string() + ": " + ec.message());
                    return false;
                }
            }

            return true;
        }};

        switch (evt.GetState()) {
            case wxWebRequest::State_Completed:
                requestPromise.set_value(doHandleCompleted());
                break;
            case wxWebRequest::State_Unauthorized:
            case wxWebRequest::State_Failed:
            case wxWebRequest::State_Cancelled:
                requestPromise.set_value(true);
                break;
            case wxWebRequestBase::State_Idle:
            case wxWebRequestBase::State_Active:
                break;
        }
    }};

    std::promise<void> bindPromise;
    const auto doBind{[&] {
        getEventHandler()->Bind(wxEVT_WEBREQUEST_STATE, stateHandler);
        bindPromise.set_value();
    }};
    wxTheApp->CallAfter(doBind);
    bindPromise.get_future().get();

    defer {
        std::promise<void> promise;
        const auto doUnbind{[&] {
            getEventHandler()->Unbind(wxEVT_WEBREQUEST_STATE, stateHandler);
            promise.set_value();
        }};
        wxTheApp->CallAfter(doUnbind);
        promise.get_future().get();
    };

    for (uint64 idx{0}; idx < changelog.changedFiles.size(); ++idx) {
        const auto& file{changelog.changedFiles[idx]};
        if (file.id.ignored) continue;
        const auto& item{data.items.at(file.id)};

        std::string itemURLString{paths::remoteUpdateAssets()};
        type = file.id.type;
        itemURLString += '/' + typeFolder(type).string() + '/';
        itemURLString += static_cast<std::string>(file.hash);
        downloadedFilename = typeFolder(file.id.type) / item.path;

        // Ugly way to reset the promise for reuse.
        requestPromise.~promise();
        new (&requestPromise) decltype(requestPromise);

        wxURI url{itemURLString};
        wxWebRequest request;

        std::promise<void> startPromise;
        const auto doStart{[&] {
            request = wxWebSession::GetDefault().CreateRequest(
                getEventHandler(), url.BuildURI()
            );
            request.SetStorage(wxWebRequestBase::Storage_File);
            request.Start();

            startPromise.set_value();
        }};
        wxTheApp->CallAfter(doStart);

        logger.info("Downloading " + file.id.name + " from \"" + url.BuildUnescapedURI().utf8_string() + "\"...");

        // Wait until actually started so the request object'll be valid when
        // querying received/expected to receive.
        startPromise.get_future().get();

        auto future{requestPromise.get_future()};
        const auto itvl{std::chrono::milliseconds(50)};
        while (std::future_status::ready != future.wait_for(itvl)) {
            auto dataReceived{request.GetBytesReceived()};
            auto dataTotal{request.GetBytesExpectedToReceive()};
            auto progress{dataReceived == 0 ? 0 : dataReceived * 100 / dataTotal};

            wxString statusMessage;
            if (file.id.type == ItemType::LIB) {
                statusMessage = "Downloading libraries... ";
            } else if (file.id.type == ItemType::RSRC) {
                statusMessage = "Downloading resources... ";
            } else {
                statusMessage = "Downloading " + file.id.name + "... ";
            }
            if (dataTotal != -1) {
                statusMessage += convertSize(dataReceived);
                statusMessage += " / ";
                statusMessage += convertSize(dataTotal);
                statusMessage += ' ';
            }
            statusMessage += '(' + std::to_string(idx + 1) + '/' + std::to_string(changelog.changedFiles.size()) + ')';

            if (dataTotal == -1) prog->pulse(statusMessage);
            else prog->set(static_cast<int32>(progress), statusMessage);

            if (prog->cancelled()) {
                logger.info("Downloads canceled.");

                std::promise<void> cancelPromise;
                const auto doCancel{[&] {
                    request.Cancel();
                    cancelPromise.set_value();
                }};
                wxTheApp->CallAfter(doCancel);
                cancelPromise.get_future().get();

                fs::remove_all(stagingFolder(), ec);

                prog->finish(false);
                return false;
            }
        }

        if (not future.get()) {
            // Error should've been logged above.
            prog->finish(true, _("Failed processing downloaded file."));

            fs::remove_all(stagingFolder(), ec);
            return false;
        }

        if (request.GetState() != wxWebRequestBase::State_Completed) {
            auto response{request.GetResponse()};
            auto statusText{response.GetStatusText()};
            logger.error("Download failed! " + (statusText.empty() ? "UError" : statusText.utf8_string()) + " (" + std::to_string(response.GetStatus()) + ')');

            prog->finish(true, _("Failed to download file."));

            fs::remove_all(stagingFolder(), ec);
            return false;
        }
    }

    return true;
}

bool Update::installFiles(
    const Changelog& changelog,
    const Data& data,
    pcui::ProgressDialog *prog,
    logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("Update::pullNewFiles()")};
    std::error_code ec;

    auto baseTypePath{[](ItemType type) -> fs::path {
        switch (type) {
        case ItemType::EXEC:
            return paths::binaryDir();
        case ItemType::LIB:
            return paths::libraryDir();
        case ItemType::COMP:
            return paths::componentDir();
        case ItemType::RSRC:
            return paths::resourceDir();
        case TYPE_MAX:
            break;
        }
        return {};
    }};

    for (const auto& file : changelog.removedFiles) {
        if (file.ignored) continue;
        const auto& item{data.items.at(file)};

        auto path{baseTypePath(file.type)};
        path /= item.path;

        fs::remove(path, ec);
        if (ec) {
            logger.error("Couldn't remove " + path.string() + ": " + ec.message());
            return false;
        }
    }

    for (const auto& file : changelog.changedFiles) {
        if (file.id.ignored) continue;
        const auto& item{data.items.at(file.id)};

        auto path{baseTypePath(file.id.type)};
#       ifdef __APPLE__
        if (file.id.type == ItemType::EXEC and item.path == "ProffieConfig") {
            auto execPath{paths::executable(paths::Executable::Main)};
            auto execParentPath{execPath.parent_path()};

            fs::create_directories(execParentPath, ec);
            if (ec) {
                logger.error("Couldn't create " + execParentPath.string() + ": " + ec.message());
                return false;
            }

            auto srcFile{stagingFolder() / typeFolder(file.id.type) / item.path};
            fs::copy_file(
                srcFile,
                execPath,
                fs::copy_options::overwrite_existing,
                ec
            );
            if (ec) {
                logger.error("Couldn't copy " + srcFile.string() + " to " + execPath.string() + ": " + ec.message());
                return false;
            }

            const auto resourcesPath{
                paths::executable(paths::Executable::Main)
                    .parent_path().parent_path() /
                "Resources"
            };
            fs::create_directories(resourcesPath, ec);
            if (ec) {
                logger.error("Couldn't create " + resourcesPath.string() + ec.message());
                return false;
            }

            const auto launcherIconPath{
                paths::executable(paths::Executable::Launcher)
                    .parent_path().parent_path() /
                "Resources" /
                "icon.icns"
            };
            const auto destIconPath{
                resourcesPath / "icon.icns"
            };
            fs::copy_file(
                launcherIconPath,
                destIconPath,
                fs::copy_options::overwrite_existing,
                ec
            );
            if (ec) {
                logger.error("Couldn't copy " + launcherIconPath.string() + " to " + destIconPath.string() + ": " + ec.message());
                return false;
            }

            const auto infoFilePath{
                paths::executable(paths::Executable::Main)
                    .parent_path().parent_path() /
                "Info.plist"
            };
            auto infoStream{files::openOutput(infoFilePath)};
            infoStream << 
                R"(<?xml version="1.0" encoding="UTF-8"?>)" "\n"
                R"(<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">)" "\n"
                R"(<plist version="1.0">)" "\n"
                "<dict>\n"
                "   <key>CFBundleDevelopmentRegion</key>\n"
                "   <string>en-US</string>\n"
                "   <key>CFBundleExecutable</key>\n"
                "   <string>ProffieConfig</string>\n"
                "   <key>CFBundleIconFile</key>\n"
                "   <string>icon.icns</string>\n"
                "   <key>CFBundleIdentifier</key>\n"
                "   <string>com.kafrenetrading.proffieconfig</string>\n"
                "   <key>CFBundlePackageType</key>\n"
                "   <string>APPL</string>\n"
                "   <key>CSResourcesFileMapped</key>\n"
                "   <true/>\n"
                "   <key>LSMinimumSystemVersion</key>\n"
                "   <string>11.0</string>\n"
                "   <key>NSHumanReadableCopyright</key>\n"
                "   <string>Copyright (C) 2024 Ryan Ogurek</string>\n"
                "</dict>\n"
                "</plist>\n";
            infoStream.close();

            if (infoStream.fail()) {
                logger.error("Failed to write executable info file at " + infoFilePath.string());
                return false;
            }
        }
        else
#       endif
        {
            path /= fs::path{item.path};

            // Allow this to fail.
            // I don't remember why it's explicitly removed before even with
            // copy_file set to overwrite but I think there was a reason.
            fs::remove(path, ec);

            fs::create_directories(path.parent_path(), ec);
            if (ec) {
                logger.error("Failed to create " + path.parent_path().string() + ": " + ec.message());
                return false;
            }

            auto srcPath{
                stagingFolder() / typeFolder(file.id.type) / item.path
            };
            files::copyOverwrite(
                srcPath,
                path,
                ec
            );
            if (ec) {
                logger.error("Failed to copy " + srcPath.string() + " to " + path.string() + ": " + ec.message());
                return false;
            }
        }
    }

    fs::remove_all(stagingFolder(), ec);
    if (ec) {
        logger.warn("Couldn't remove staging folder " + stagingFolder().string() + ": " + ec.message());
    }

    return true;
}

namespace {

std::string convertSize(uint64 size) {
    constexpr std::array<cstring, 4> SIZES{ "B", "KB", "MB", "GB" };
    auto scale{0};
    auto result{static_cast<float64>(size)};

    while (result >= 1000.0 and scale < (SIZES.size() - 1)) {
        result /= 1000.0;
        ++scale;
    }

    // 2 decimal places
    constexpr auto PRECISION{100};
    return {
        std::to_string(static_cast<int32>(result)) +
        '.' +
        std::to_string(static_cast<int32>(result * PRECISION) - (static_cast<int32>(result) * PRECISION)) +
        SIZES[scale]
    };
}

} // namespace

