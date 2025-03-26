#include "changelog.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * launcher/update/changelog.cpp
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

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/scrolwin.h>

#include <log/logger.h>
#include <utils/paths.h>
#include <utils/crypto.h>

namespace Update {

} // namespace Update

[[nodiscard]] Update::Changelog Update::generateChangelog(const Data& data, const Version& currentVersion, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Update::generateChangelog()")};

    auto getVersionInCurrent{[&data, currentVersion](const ItemID& file) -> Version {
        auto bundleIt{data.bundles.find(currentVersion)};
        if (bundleIt == data.bundles.end()) return Version::invalidObject();

        for (const auto& [ id, fileVer, hash] : bundleIt->second.reqs) {
            if (id == file) return fileVer;
        }

        return Version::invalidObject();
    }};

    auto latestBundleIt{data.bundles.rbegin()};

    Changelog ret;
    ret.latestBundleVersion = latestBundleIt->first;
    ret.currentBundleVersion = currentVersion;

    for (const auto& [ id, version, hash] : latestBundleIt->second.reqs) {
        auto currentVersion{getVersionInCurrent(id)};
        if (currentVersion == version) continue;

        auto& changedFile{ret.changedFiles.emplace_back()};

        changedFile.currentVersion = currentVersion;
        changedFile.latestVersion = version;
        changedFile.hash = hash;
        changedFile.id = id;
    }

    if (currentVersion and data.bundles.find(currentVersion) != data.bundles.end()) {
        for (const auto& [ id, version, hash] : data.bundles.at(currentVersion).reqs) {
            bool newVersionHasFile{false};
            for (const auto& [itemID, version, hash] : data.bundles.find(currentVersion)->second.reqs) {
                if (itemID == id)  {
                    newVersionHasFile = true;
                    break;
                }
            }

            if (not newVersionHasFile) ret.removedFiles.emplace_back(id);
        }
    }

    return ret;
}

bool Update::promptWithChangelog(const Data& data, const Changelog& changelog, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Update::promptWithChangelog()")};

    enum {
        BUTTON_REMIND,
        BUTTON_INSTALL,
    };

    auto dlg{wxDialog(nullptr, wxID_ANY, "Update Available", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)};
    auto *sizer{new wxBoxSizer(wxHORIZONTAL)};

    auto *iconSizer{new wxBoxSizer(wxVERTICAL)};
#   ifdef __WXOSX__
    auto *iconElem{new wxStaticBitmap(&dlg, wxID_ANY, wxBitmap("icon", wxBITMAP_TYPE_ICON_RESOURCE))};
#   elif defined(__WIN32__)
    auto *iconElem{new wxStaticBitmap(&dlg, wxID_ANY, wxICON(ApplicationIcon))};
#   elif defined(__linux__)
    auto *iconElem{new wxStaticBitmap(&dlg, wxID_ANY, wxBitmap())};
#   endif
    iconElem->SetMaxSize({96, 96});
    iconSizer->Add(iconElem, wxSizerFlags().Border(wxLEFT | wxTOP | wxBOTTOM, 20));

    auto *mainSizer{new wxBoxSizer(wxVERTICAL)};

    auto *updateSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *updateText{new wxStaticText(&dlg, wxID_ANY, "ProffieConfig is ready for an update!")};
    auto *updateDeltaText{new wxStaticText(&dlg, wxID_ANY, 
            '(' + static_cast<string>(changelog.currentBundleVersion) + 
            " -> " + static_cast<string>(changelog.latestBundleVersion) + ')')};
    {
        auto font{updateText->GetFont()};
        font.SetPointSize(static_cast<int32_t>(font.GetPointSize() * 1.5));
        updateDeltaText->SetFont(font);
        font.SetWeight(wxFONTWEIGHT_BOLD);
        updateText->SetFont(font);
    }
    updateSizer->Add(updateText);
    updateSizer->AddSpacer(10);
    updateSizer->Add(updateDeltaText);
    mainSizer->Add(updateSizer, wxSizerFlags().Border(wxBOTTOM, 10));


    auto *whatNewText{new wxStaticText(&dlg, wxID_ANY, "What's New?")};
    {
        auto font{whatNewText->GetFont()};
        font.SetPointSize(static_cast<int32_t>(font.GetPointSize() * 1.2));
        whatNewText->SetFont(font);
    }
    mainSizer->Add(whatNewText, wxSizerFlags());

    auto *whatNewBorder{new wxStaticBoxSizer(wxVERTICAL, &dlg)};
    auto *whatNewPanel{new wxScrolledWindow(whatNewBorder->GetStaticBox())};
    auto *whatNewSizer{new wxBoxSizer(wxVERTICAL)};
    auto objFont{whatNewPanel->GetFont()};
    objFont.SetWeight(wxFONTWEIGHT_BOLD);
    objFont.SetPointSize(objFont.GetPointSize() + 2);
    auto versionFont{whatNewPanel->GetFont()};
    versionFont.SetStyle(wxFONTSTYLE_ITALIC);
    versionFont.SetPointSize(versionFont.GetPointSize() - 2);
    auto headFont{whatNewPanel->GetFont()};
    headFont.SetWeight(wxFONTWEIGHT_BOLD);
    auto listFont{whatNewPanel->GetFont()};

    whatNewSizer->AddSpacer(5);

    auto noteStartIt{changelog.currentBundleVersion ? std::next(data.bundles.find(changelog.currentBundleVersion)) : data.bundles.begin()};
    auto noteEndIt{std::next(data.bundles.find(changelog.latestBundleVersion))};
    for (auto noteIt{noteStartIt}; noteIt != noteEndIt; ++noteIt) {
        if (noteIt->second.note.empty()) continue;

        auto *noteText{new wxStaticText(whatNewPanel, wxID_ANY, noteIt->second.note)};
        noteText->SetFont(listFont);
        whatNewSizer->Add(noteText, wxSizerFlags());

        whatNewSizer->AddSpacer(10);
    }

    if (noteStartIt != noteEndIt) whatNewSizer->Add(new wxStaticLine(whatNewPanel), wxSizerFlags().Expand().Border(wxBOTTOM, 5));

    for (const auto& file : changelog.changedFiles) {
        if (file.id.ignored) continue;
        auto fileItem{data.items.at(file.id)};
        if (fileItem.hidden) continue;

        auto *objText{new wxStaticText(whatNewPanel, wxID_ANY, file.id.name)};
        objText->SetFont(objFont);
        whatNewSizer->Add(objText, wxSizerFlags());

        auto *versText{new wxStaticText(
                whatNewPanel, 
                wxID_ANY, 
                (file.currentVersion ? 
                 static_cast<string>(file.currentVersion) : 
                 "[NONE]") 
                + " -> " + static_cast<string>(file.latestVersion))};
        versText->SetFont(versionFont);
        whatNewSizer->Add(versText, wxSizerFlags().Border(wxBOTTOM, 5));

        auto outputSection{[&](const string& sectName, vector<string> ItemVersionData::*field){
            const auto versionsStart{
                file.currentVersion ? 
                    std::next(fileItem.versions.find(file.currentVersion)) : 
                    fileItem.versions.begin()
            };
            // Should always be end()
            const auto versionsEnd{std::next(fileItem.versions.find(file.latestVersion))};

            vector<string> itemBullets;
            for (auto versionIt{versionsStart}; versionIt != versionsEnd; ++versionIt) {
                const auto& versionBullets{versionIt->second.*field};
                itemBullets.reserve(itemBullets.size() + versionBullets.size());
                for (const auto& note : versionBullets) {
                    itemBullets.emplace_back(note);
                }
            }

            // Tab breaks this, so spaces are used instead!!
            if (not itemBullets.empty()) {
                logger.debug("Listing " + sectName + " for " + file.id.name + "...");
                auto *headText{new wxStaticText(whatNewPanel, wxID_ANY, "    " + sectName + ':')};
                headText->SetFont(headFont);
                whatNewSizer->Add(headText);

                for (const auto& bullet : itemBullets) {
                    logger.debug("- " + bullet);
                    auto *featureText{new wxStaticText(whatNewPanel, wxID_ANY, "        - " + bullet)};
                    featureText->SetFont(listFont);
                    whatNewSizer->Add(featureText);
                }

                whatNewSizer->AddSpacer(5);
            }
        }};

        outputSection("Features", &ItemVersionData::features);
        outputSection("Changes", &ItemVersionData::changes);
        outputSection("Bug Fixes", &ItemVersionData::fixes);

        whatNewSizer->AddSpacer(10);
    }
    whatNewPanel->SetSizerAndFit(whatNewSizer);
    whatNewSizer->FitInside(whatNewPanel);
    whatNewPanel->SetScrollRate(4, 4);
    whatNewBorder->Add(whatNewPanel, wxSizerFlags(1).Expand());
    mainSizer->Add(whatNewBorder, wxSizerFlags(1).Expand());

    mainSizer->AddSpacer(20);

    auto *buttonSizer{new wxBoxSizer(wxHORIZONTAL)};

    auto *remindButton{new wxButton(&dlg, BUTTON_REMIND, "Remind Me Later")};
    buttonSizer->Add(remindButton, wxSizerFlags());

    buttonSizer->AddStretchSpacer();

    auto *installButton{new wxButton(&dlg, BUTTON_INSTALL, "Update Now")};
    installButton->SetDefault();
    buttonSizer->Add(installButton, wxSizerFlags());

    mainSizer->Add(buttonSizer, wxSizerFlags(0).Expand());

    sizer->Add(iconSizer);
    sizer->Add(mainSizer, wxSizerFlags(1).Expand().Border(wxALL, 20));

    dlg.Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent&) {
        dlg.EndModal(false);
    });
    dlg.Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        dlg.EndModal(false);
    }, BUTTON_REMIND);
    dlg.Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        dlg.EndModal(true);
    }, BUTTON_INSTALL);

    dlg.SetSizerAndFit(sizer);
#   ifdef __linux__
    wxSize size{800, 600};
#   else
    wxSize size{600, 400};
#   endif
    sizer->SetMinSize(size);
    dlg.SetMinSize(wxSize{dlg.GetBestSize().x, size.y});
    dlg.SetSize(wxSize{-1, size.y});

    return dlg.ShowModal();
}

Update::Version Update::determineCurrentVersion(const Data& data, PCUI::ProgressDialog *prog, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("Update::determineCurrentVersion()")};

    // Ensure invalid
    Update::Version ret{Version::invalidObject()};
    std::map<filepath, string> hashCache;

    prog->Pulse("Determining current version...");
    for (auto bundleIt{data.bundles.rbegin()}; bundleIt != data.bundles.rend(); ++bundleIt) {
        const auto& [ version, bundle ]{*bundleIt};
        auto status{"Trying version " + static_cast<string>(version)};
        prog->Pulse(status);
        logger.info(status);

        bool filesMatch{true};
        for (const auto& [ id, fileVer, hash] : bundle.reqs) {
            if (id.ignored) continue;
            auto fileItem{data.items.at(id)};
            filepath itemPath;
            switch (id.type) {
                case ItemType::EXEC:
                    itemPath = Paths::binaries();
                    break;
                case ItemType::LIB:
                    itemPath = Paths::libraries();
                    break;
                case ItemType::COMP:
                    itemPath = Paths::components();
                    break;
                case ItemType::RSRC:
                    itemPath = Paths::resources();
                    break;
                case ItemType::TYPE_MAX:
                    break;
            }

#           ifdef __APPLE__
            if (fileItem.path == "ProffieConfig") {
                itemPath /= filepath{"ProffieConfig.app"} / "Contents" / "MacOS" / "ProffieConfig";
            } else {
                itemPath /= fileItem.path;
            }
#           else
            itemPath /= filepath{fileItem.path};
#           endif
            status = "Testing file " + id.name + ", " + static_cast<string>(fileVer);
            logger.debug(status + " at path: " + itemPath.string());
            prog->Pulse(status);

            auto cachedHash{hashCache.find(itemPath)};
            string itemHash;
            if (cachedHash == hashCache.end()) {
                itemHash = hashCache[itemPath] = Crypto::computeHash(itemPath);
            } else {
                itemHash = cachedHash->second;
            }

            if (itemHash != fileItem.versions.at(fileVer).hash) {
                status = "Hash check failed";
                logger.info(status);
                prog->Pulse(status);
                filesMatch = false;
                break;
            }
        }

        if (filesMatch) {
            status = "Bundle version " + static_cast<string>(version) + " matches installed files.";
            logger.info(status);
            prog->Update(90, status);
            ret = version;
            break;
        }
    }

    return ret;
}


