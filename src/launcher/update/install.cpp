#include "install.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
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
#include <thread>

#include <wx/webrequest.h>
#include <wx/filefn.h>
#include <wx/uri.h>

#include <app/app.h>
#include <log/logger.h>
#include <utils/paths.h>
#include <utils/defer.h>

#include "update.h"
#include "wx/utils.h"

namespace Update {

inline filepath stagingFolder() { return Paths::data() / "staging"; }

string convertSize(uint64 size);

} // namespace Update

bool Update::pullNewFiles(const Changelog& changelog, const Data& data, PCUI::ProgressDialog *prog, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Update::pullNewFiles()")};

    prog->Update(0, "Preparing to download new files...");
    bool requestDone{};
    filepath downloadedFilename;
    Update::ItemType type;

    fs::remove_all(stagingFolder());

    auto stateHandler{[&](wxWebRequestEvent& evt) {
        if (evt.GetState() == wxWebRequest::State_Completed) {
            auto filePath{stagingFolder() / downloadedFilename};
            fs::create_directories(filePath.parent_path());
            fs::copy_file(evt.GetDataFile().ToStdString(), filePath);
            if (type == Update::EXEC) {
                fs::permissions(filePath, fs::perms::owner_exec | fs::perms::others_exec, fs::perm_options::add);
            }
        }

        switch (evt.GetState()) {
            case wxWebRequest::State_Unauthorized:
            case wxWebRequest::State_Completed:
            case wxWebRequest::State_Failed:
            case wxWebRequest::State_Cancelled:
                requestDone = true;
                break;
            default: break;
        }
    }};

    getEventHandler()->Bind(wxEVT_WEBREQUEST_STATE, stateHandler);
    Defer deferUnbind{[&](){ getEventHandler()->Unbind(wxEVT_WEBREQUEST_STATE, stateHandler); }};

    for (uint64 idx{0}; idx < changelog.changedFiles.size(); ++idx) {
        const auto& file{changelog.changedFiles[idx]};
        const auto& item{data.items.at(file.id)};

        string itemURLString{Paths::remoteUpdateAssets()};
#       ifdef __WIN32__
        itemURL += "/win32/";
#       elif defined(__APPLE__)
        itemURLString += "/macOS/";
#       elif defined(__linux__)
        itemURL += "/linux/";
#       endif
        type = file.id.type;
        itemURLString += typeFolder(type).string() + '/';
        itemURLString += file.id.name + '/' + string(file.latestVersion);
        wxURI url{itemURLString};
        auto request{wxWebSession::GetDefault().CreateRequest(getEventHandler(), url.BuildURI())};
        request.SetStorage(wxWebRequestBase::Storage_File);

        requestDone = false;
        downloadedFilename = typeFolder(file.id.type) / item.path;
        request.Start();

        logger.info("Downloading " + file.id.name + " from \"" + url.BuildURI().ToStdString() + "\"...");

        while (not requestDone) {
            auto dataReceived{request.GetBytesReceived()};
            auto dataTotal{request.GetBytesExpectedToReceive()};
            auto progress{dataReceived == 0 ? 0 : dataReceived * 100 / dataTotal};

            auto statusMessage{"Downloading " + file.id.name + "... "};
            if (dataTotal != -1) {
                statusMessage += convertSize(dataReceived);
                statusMessage += " / ";
                statusMessage += convertSize(dataTotal);
                statusMessage += ' ';
            }
            statusMessage += '(' + std::to_string(idx + 1) + '/' + std::to_string(changelog.changedFiles.size()) + ')';

            if (
                    dataTotal == -1 ? 
                    not prog->Pulse(statusMessage) : 
                    not prog->Update(static_cast<int32>(progress), statusMessage)
               ) {
                logger.info("Downloads canceled.");
                request.Cancel();
                fs::remove_all(stagingFolder());
                return false;
            }

            wxYield();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        if (request.GetState() != wxWebRequestBase::State_Completed) {
            logger.error("Download failed!");
            auto response{request.GetResponse()};
            auto statusText{response.GetStatusText().ToStdString()};
            PCUI::showMessage(
                    "Failed to download file.\n" +
                    (statusText.empty() ? "Error" : statusText) + " (" + std::to_string(response.GetStatus()) + ')',
                    App::getAppName());
            fs::remove_all(stagingFolder());
            return false;
        }
    }

    return true;
}

void Update::installFiles(const Changelog& changelog, const Data& data, PCUI::ProgressDialog *prog, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Update::pullNewFiles()")};

    auto baseTypePath{[](ItemType type) -> filepath {
        switch (type) {
        case ItemType::EXEC:
            return Paths::binaries();
        case ItemType::LIB:
            return Paths::libraries();
        case ItemType::COMP:
            return Paths::components();
        case ItemType::RSRC:
            return Paths::resources();
        case TYPE_MAX:
            break;
        }
        return {};
    }};

    for (const auto& file : changelog.removedFiles) {
        const auto& item{data.items.at(file)};

        auto path{baseTypePath(file.type)};
        path /= item.path;

        std::filesystem::remove(path);
    }

    for (const auto& file : changelog.changedFiles) {
        const auto& item{data.items.at(file.id)};

        auto path{baseTypePath(file.id.type)};
        path /= item.path;

        std::filesystem::remove(path);
        std::filesystem::create_directories(path.parent_path());
        std::filesystem::copy_file(stagingFolder() / typeFolder(file.id.type) / item.path, path);
    }

    std::filesystem::remove_all(stagingFolder());
}

string Update::convertSize(uint64 size) {              
    constexpr array<cstring, 4> SIZES{ "B", "KB", "MB", "GB" };
    auto scale{0};
    auto result{static_cast<float128>(size)};

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

