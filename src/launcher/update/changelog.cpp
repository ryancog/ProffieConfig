#include "changelog.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
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

#include <fstream>
#include <future>
#include <ranges>

#include "log/logger.hpp"
#include "ui/build.hpp"
#include "ui/bitmap.hpp"
#include "ui/controls/button.hpp"
#include "ui/helpers/dialog_buttons.hpp"
#include "ui/helpers/foreach.hpp"
#include "ui/helpers/if.hpp"
#include "ui/layout/group.hpp"
#include "ui/layout/scrolled.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/divider.hpp"
#include "ui/static/image.hpp"
#include "ui/static/label.hpp"
#include "ui/font.hpp"
#include "ui/types.hpp"
#include "ui/utils.hpp"
#include "ui/values.hpp"
#include "utils/hash.hpp"
#include "utils/paths.hpp"

namespace {

pcui::DescriptorPtr ui(
    pcui::Dialog& dlg,
    const Update::Data& data,
    const Update::Changelog& log
);

} // namespace

[[nodiscard]] Update::Changelog Update::generateChangelog(
    const Data& data,
    const utils::Version& currentVersion,
    logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("Update::generateChangelog()")};

    auto getVersionInCurrent{[&data, currentVersion](
        const ItemID& file
    ) -> utils::Version {
        auto bundleIt{data.bundles.find(currentVersion)};
        if (bundleIt == data.bundles.end()) return utils::Version::invalid();

        for (const auto& [ id, fileVer, hash] : bundleIt->second.reqs) {
            if (id == file) return fileVer;
        }

        return utils::Version::invalid();
    }};

    auto latestBundleIt{data.bundles.rbegin()};

    Changelog ret;
    ret.latestBundleVersion = latestBundleIt->first;
    ret.currentBundleVersion = currentVersion;

    for (const auto& [ id, version, hash] : latestBundleIt->second.reqs) {
        auto currentVersion{getVersionInCurrent(id)};
        if (currentVersion.compare(version) == 0) continue;

        auto& changedFile{ret.changedFiles.emplace_back(
            id,
            *hash,
            currentVersion,
            version
        )};
    }

    if (currentVersion and data.bundles.contains(currentVersion)) {
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

bool Update::promptWithChangelog(
    const Data& data, const Changelog& log, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("Update::promptWithChangelog()")};

    std::promise<bool> promise;

    const auto prompt{[&promise, &data, &log] {
        pcui::Dialog dlg(
            nullptr,
            wxID_ANY,
            "Update Available",
            wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
        );

        pcui::build(&dlg, ui(dlg, data, log));

        switch (dlg.ShowModal()) {
            case wxID_OK:
                promise.set_value(true);
                return;
            case wxID_CANCEL:
                promise.set_value(false);
                return;
            default:
                exit(0);
        }
    }};
    pcui::safeCall(prompt);

    return promise.get_future().get();
}

utils::Version Update::determineCurrentVersion(
    const Data& data,
    pcui::ProgressDialog *prog,
    logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("Update::determineCurrentVersion()")};

    // Ensure invalid
    auto ret{utils::Version::invalid()};
    std::map<fs::path, std::optional<utils::hash::SHA256>> hashCache;

    prog->pulse("Determining current version...");

    for (
            auto bundleIt{data.bundles.rbegin()};
            bundleIt != data.bundles.rend();
            ++bundleIt
        ) {
        const auto& [ version, bundle ]{*bundleIt};
        auto status{"Trying version " + version.string()};
        prog->pulse(status);

        logger.info(status);

        bool filesMatch{true};
        for (const auto& [ id, fileVer, hash] : bundle.reqs) {
            if (id.ignored) continue;
            auto fileItem{data.items.at(id)};
            fs::path itemPath;
            switch (id.type) {
                case ItemType::EXEC:
                    itemPath = paths::binaryDir();
                    break;
                case ItemType::LIB:
                    itemPath = paths::libraryDir();
                    break;
                case ItemType::COMP:
                    itemPath = paths::componentDir();
                    break;
                case ItemType::RSRC:
                    itemPath = paths::resourceDir();
                    break;
                case ItemType::TYPE_MAX:
                    break;
            }

#           ifdef __APPLE__
            if (fileItem.path == "ProffieConfig") {
                itemPath /= fs::path{"ProffieConfig.app"} / "Contents" / "MacOS" / "ProffieConfig";
            } else {
                itemPath /= fileItem.path;
            }
#           else
            itemPath /= fs::path{fileItem.path};
#           endif
            status = "Testing file " + id.name + ", " + fileVer.string();
            logger.debug(status + " at path: " + itemPath.string());
            prog->pulse(status);

            auto cachedHash{hashCache.find(itemPath)};
            std::optional<utils::hash::SHA256> itemHash;
            if (cachedHash == hashCache.end()) {
                std::ifstream fileStream{itemPath, std::ios::binary};

                if (fileStream.fail()) {
                    status = "Could not open " + itemPath.string() + ", does it exist?";
                    logger.info(status);
                    prog->pulse(status);
                } else {
                    itemHash = utils::hash::SHA256::stream(fileStream);
                }

                // Place even if nullopt to be consistent w/ cache pulls
                hashCache.emplace(itemPath, itemHash);
            } else {
                itemHash = cachedHash->second;
            }

            if (itemHash != fileItem.versions.at(fileVer).hash) {
                status = "Hash check failed";
                logger.info(status);
                prog->pulse(status);
                filesMatch = false;
                break;
            }
        }

        if (filesMatch) {
            status = "Bundle version " + version.string() + " matches installed files.";
            logger.info(status);
            prog->set(90, status);
            ret = version;
            break;
        }
    }

    return ret;
}

namespace {

pcui::DescriptorPtr ui(
    pcui::Dialog& dlg,
    const Update::Data& data,
    const Update::Changelog& log
) {
    auto objFont{- pcui::Font::Normal};
    objFont.SetWeight(wxFONTWEIGHT_BOLD);
    objFont.SetPointSize(objFont.GetPointSize() + 2);

    auto versionFont{- pcui::Font::Normal};
    versionFont.SetStyle(wxFONTSTYLE_ITALIC);
    versionFont.SetPointSize(versionFont.GetPointSize() - 2);

    auto headFont{- pcui::Font::Normal};
    headFont.SetWeight(wxFONTWEIGHT_BOLD);

    auto listFont{- pcui::Font::Normal};

    // TODO: This logic seems a little silly.
    auto noteStartIt{log.currentBundleVersion
        ? std::next(data.bundles.find(log.currentBundleVersion))
        : data.bundles.begin()
    };
    auto noteEndIt{std::next(data.bundles.find(log.latestBundleVersion))};
    std::ranges::subrange notesRange{noteStartIt, noteEndIt};

    const auto makeNoteItem{[&](
        Update::Bundles::const_reference bundle
    ) {
        return pcui::If{
          .cond_=not bundle.second.note.empty(),
          .then_=pcui::Label{
            .win_={.base_={.border_={.size_=10, .dirs_=wxBOTTOM}}},
            .label_=bundle.second.note,
            .font_=listFont,
          }(),
        }();
    }};

    const auto makeFileItem{[&](
        const Update::Changelog::ChangedFile& file
    ) -> pcui::DynamicList {
        if (file.id.ignored) return {};

        auto fileItem{data.items.at(file.id)};
        if (fileItem.hidden) return {};

        auto section{[&](
            const std::string& sectName,
            std::vector<std::string> Update::ItemVersionData::*field
        ) -> pcui::DynamicList {
            const auto versionsStart{
                file.currentVersion ? 
                    std::next(fileItem.versions.find(file.currentVersion)) : 
                    fileItem.versions.begin()
            };

            // Should always be end()
            const auto versionsEnd{std::next(fileItem.versions.find(file.latestVersion))};

            std::vector<std::string> itemBullets;
            for (auto versionIt{versionsStart}; versionIt != versionsEnd; ++versionIt) {
                const auto& versionBullets{versionIt->second.*field};
                itemBullets.reserve(itemBullets.size() + versionBullets.size());
                for (const auto& note : versionBullets) {
                    itemBullets.emplace_back(note);
                }
            }

            if (itemBullets.empty()) return {};

            return {
              pcui::Label{
                // Tab breaks this, so spaces are used instead!!
                .label_="    " + sectName + ':',
                .font_=headFont,
              }(),
              pcui::ForEach{
                .of_=itemBullets,
                .do_=[&](const std::string& bullet) {
                  return pcui::Label{
                    .label_="        - " + bullet,
                    .font_=listFont,
                  }();
                }
              }(),
              pcui::Spacer{.size_=5}(),
            };
        }};

        return {
          pcui::Label{
            .label_=file.id.name,
            .font_=objFont,
          }(),
          pcui::Spacer{.size_=5}(),
          pcui::Label{
            .label_=(file.currentVersion
              ? file.currentVersion.string()
              : "[NONE]") + " -> " + file.latestVersion.string(),
            .font_=versionFont,
          }(),
          pcui::Spacer{.size_=10}(),
          section("Features", &Update::ItemVersionData::features),
          section("Changes", &Update::ItemVersionData::changes),
          section("Bug Fixes", &Update::ItemVersionData::fixes),
        };
    }};

    return pcui::Stack{
      .base_={.expand_=true, .proportion_=1},
      .orient_=wxHORIZONTAL,
      .children_={
        pcui::Stack{
          .children_={
            // TODO: Why is this in its own sizer?
            pcui::Image{
              .win_={
                .base_={
                  .border_={
                    .size_=20,
                    .dirs_=wxLEFT | wxTOP | wxBOTTOM
                  }
                },
                .maxSize_={96, 96}
              },
              .src_={
#               if defined(__APPLE__)
                pcui::Bitmap("icon", pcui::Bitmap::Type::Resource)
#               elif defined(_WIN32)
                wxICON(ApplicationIcon)
#               endif
              },
            }(),
          }
        }(),
        pcui::Stack{
          .base_={
            .minSize_={600, 400},
            .expand_=true,
            .proportion_=1,
            .border_={.size_=20, .dirs_=wxALL},
          },
          .children_={
            pcui::Stack{
              .orient_=wxHORIZONTAL,
              .children_={
                pcui::Label{
                  .label_="ProffieConfig is ready for an update!",
                  .font_=pcui::Font::Title,
                }(),
                pcui::Spacer{.size_=10}(),
                pcui::Label{
                  .label_={
                    '(' + log.currentBundleVersion.string() + " -> " +
                    log.latestBundleVersion.string() + ')'
                  },
                  .font_=[]() {
                    auto font{- pcui::Font::Title};
                    font.SetWeight(wxFONTWEIGHT_NORMAL);
                    return font;
                  }(),
                }(),
              }
            }(),
            pcui::Spacer{.size_=10}(),
            pcui::Label{
              .label_="What's New?",
              .font_=pcui::Font::Header,
            }(),
            pcui::Group{
              .win_={.base_={.expand_=true, .proportion_=1}},
              .children_={
                pcui::Scrolled{
                  .win_={
                    .base_={
                      .expand_=true,
                      .proportion_=1
                    }
                  },
                  .scrollRate_={.x_=4, .y_=4},
                  .child_=pcui::Stack{
                    .base_={
                      .expand_=true,
                      .border_={.size_=10, .dirs_=wxALL},
                    },
                    .orient_=wxVERTICAL,
                    .children_={
                      pcui::ForEach{
                        .of_=notesRange | std::views::reverse,
                        .do_=makeNoteItem,
                      }(),
                      pcui::If{
                        .cond_=not notesRange.empty(),
                        .then_={
                          pcui::Spacer{.size_=pcui::interControlSpacing()}(),
                          pcui::Divider{
                          .base_={.expand_=true},
                          .orient_=wxHORIZONTAL,
                          }(),
                          pcui::Spacer{.size_=pcui::interControlSpacing()}(),
                        }
                      }(),
                      pcui::ForEach{
                        .of_=log.changedFiles,
                        .do_=makeFileItem,
                      }(),
                    }
                  }(),
                }(),
              }
            }(),
            pcui::Spacer{.size_=20}(),
            pcui::DialogButtons{
              .ok_=pcui::Button{
                .label_="Update Now",
                .default_=true,
                .func_=[&dlg] {
                  dlg.EndModal(wxID_OK);
                }
              }(),
              .cancel_=pcui::Button{
                .label_="Remind Me Later",
                .func_=[&dlg] {
                  dlg.EndModal(wxID_CANCEL);
                }
              }(),
            }(),
          }
        }(),
      }
    }();
}

} // namespace

