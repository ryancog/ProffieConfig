#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * launcher/update/changelog.h
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

#include <log/branch.h>

#include "update.h"

namespace Update {

struct Changelog {
    Version currentBundleVersion;
    Version latestBundleVersion;

    struct ChangedFile {
        ItemID id;
        string hash;
        Version currentVersion;
        Version latestVersion;
    };

    vector<ChangedFile> changedFiles;
    vector<ItemID> removedFiles;
};

[[nodiscard]] Changelog generateChangelog(const Data&, const Version& currentVersion, Log::Branch&);

[[nodiscard]] bool promptWithChangelog(const Data&, const Changelog&, Log::Branch&);

[[nodiscard]] Version determineCurrentVersion(const Data&, PCUI::ProgressDialog *, Log::Branch&);

} // namespace Update

