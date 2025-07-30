#include "bladeconfig.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/config/bladeconfig/bladeconfig.cpp
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

#include <algorithm>

#include "ui/controls/numeric.h"
#include "utils/string.h"

#include "../config.h"

Config::Blade::Blade() {
    type.setChoices(Utils::createEntries({
        "WS281X",
        _("Simple"),
    }));

    // Yeah, we're going to be blissfully ignorant of this handler...
}

Config::BladeConfig::BladeConfig(Config& config) : mConfig{config} {
    presetArray.setPersistence(PCUI::ChoiceData::PERSISTENCE_INDEX);

    name.setUpdateHandler([this](uint32 id) {
        if (id != name.ID_VALUE) return;

        auto nameValue{static_cast<string>(name)};
        auto insertionPoint{name.getInsertionPoint()};
        uint32 numTrimmed;
        Utils::trimUnsafe(nameValue, &numTrimmed, insertionPoint);
        std::transform(
            nameValue.begin(),
            nameValue.end(),
            nameValue.begin(),
            [](unsigned char chr){ return std::tolower(chr); }
        );

        // No further updates needed, can update things.
        if (static_cast<string>(name) == nameValue) {
            uint32 idx{0};
            auto arrayIter{mConfig.bladeArrays.arrays().begin()};
            auto arrayEnd{mConfig.bladeArrays.arrays().end()};
            for (; arrayIter != arrayEnd; ++arrayIter, ++idx) {
                if (this == &**arrayIter) break;
            }

            auto choices{mConfig.bladeArrays.arraySelection.choices()};
            choices[idx] = name;
            mConfig.bladeArrays.arraySelection.setChoices(std::move(choices));

            mConfig.presetArrays.syncStyleDisplay();

            notifyData.notify();
            if (
                    mConfig.bladeArrays.arraySelection != -1 and
                    &mConfig.bladeArrays.array(mConfig.bladeArrays.arraySelection) == this
                ) {
                mConfig.bladeArrays.arrayIssues = computeIssues();
                mConfig.bladeArrays.notifyData.notify(BladeArrays::ID_ARRAY_ISSUES);
            }
            return;
        }
        name = std::move(nameValue);
        name.setInsertionPoint(insertionPoint - numTrimmed);
    });

    id.setUpdateHandler([this](uint32 id) {
        if (id != this->id.ID_VALUE) return;

        notifyData.notify();
        if (
                mConfig.bladeArrays.arraySelection != -1 and
                &mConfig.bladeArrays.array(mConfig.bladeArrays.arraySelection) == this
            ) {
            mConfig.bladeArrays.arrayIssues = computeIssues();
            mConfig.bladeArrays.notifyData.notify(BladeArrays::ID_ARRAY_ISSUES);
        }

       noBladeID = this->id == NO_BLADE;
    });

    noBladeID.setUpdateHandler([this](uint32 id) {
        if (id != noBladeID.ID_VALUE) return;

        if (not noBladeID and this->id == NO_BLADE) noBladeID = true;
        if (noBladeID) this->id = NO_BLADE;
    });

    presetArray.setUpdateHandler([this](uint32 id) {
        if (id != presetArray.ID_SELECTION) return;

        notifyData.notify();
        if (
                mConfig.bladeArrays.arraySelection != -1 and
                &mConfig.bladeArrays.array(mConfig.bladeArrays.arraySelection) == this
            ) {
            mConfig.bladeArrays.arrayIssues = computeIssues();
            mConfig.bladeArrays.notifyData.notify(BladeArrays::ID_ARRAY_ISSUES);
        }
    });

    id.setRange(0, NO_BLADE);
}

void Config::BladeConfig::addBlade() {

    mConfig.presetArrays.syncStyleDisplay();
}

void Config::BladeConfig::removeBlade(uint32 idx) {

    mConfig.presetArrays.syncStyleDisplay();
}

void Config::BladeConfig::addSubBlade() {

    mConfig.presetArrays.syncStyleDisplay();
}

void Config::BladeConfig::removeSubBlade(uint32 idx) {

    mConfig.presetArrays.syncStyleDisplay();
}

[[nodiscard]] uint32 Config::BladeConfig::computeIssues() const {
    uint32 ret{0};

    if (presetArray == -1) ret |= ISSUE_NO_PRESETARRAY;

    if (static_cast<string>(name).empty()) ret |= ISSUE_NO_NAME;

    for (const auto& array : mConfig.bladeArrays.arrays()) {
        if (&*array == this) continue;
        if (static_cast<string>(array->name) == static_cast<string>(name)) {
            ret |= ISSUE_DUPLICATE_NAME;
        }
        if (static_cast<uint32>(array->id) == static_cast<uint32>(id)) {
            ret |= ISSUE_DUPLICATE_ID;
        }
    }

    return ret;
}

[[nodiscard]] wxString Config::BladeConfig::issueString(Issue issue) {
    switch (issue) {
        case ISSUE_NONE:
            return _("No Issue");
        case ISSUE_NO_NAME:
            return _("Blade Array is unnamed");
        case ISSUE_NO_PRESETARRAY:
            return _("Blade Array is not linked to a Preset Array");
        case ISSUE_DUPLICATE_ID:
            return _("Blade Array has duplicate ID");
        case ISSUE_DUPLICATE_NAME:
            return _("Blade Array has duplicate name");
    }
}

