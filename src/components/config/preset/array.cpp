#include "array.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/config/preset/array.cpp
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

#include "config/config.h"

namespace Config {

} // namespace Config

void Config::PresetArray::addPreset() {
    auto choices{selection.choices()};
    choices.push_back(mPresets.emplace_back().name);
    selection.setChoices(std::move(choices));
}

void Config::PresetArray::removePreset(uint32 idx) {
    if (idx >= selection.choices().size()) return;

    mPresets.erase(std::next(mPresets.begin(), idx));

    auto choices{selection.choices()};
    choices.erase(std::next(choices.begin(), idx));
    selection.setChoices(std::move(choices));
}

void Config::PresetArray::movePresetUp(uint32 idx) {
    if (idx >= selection.choices().size()) return;
    if (idx == 0) return;

    mPresets.splice(
        std::next(mPresets.begin(), idx - 1),
        mPresets,
        std::next(mPresets.begin(), idx)
    );

    auto choices{selection.choices()};
    choices.insert(
        std::next(choices.begin(), idx - 1), 
        *choices.erase(std::next(choices.begin(), idx))
    );
    selection.setChoices(std::move(choices));
}

void Config::PresetArray::movePresetDown(uint32 idx) {
    if (idx >= selection.choices().size()) return;
    if (idx == selection.choices().size() - 1) return;

    mPresets.splice(
        std::next(mPresets.begin(), idx + 2),
        mPresets,
        std::next(mPresets.begin(), idx)
    );

    auto choices{selection.choices()};
    choices.insert(
        std::next(choices.begin(), idx + 1), 
        *choices.erase(std::next(choices.begin(), idx))
    );
    selection.setChoices(std::move(choices));
}

Config::PresetArrays::PresetArrays(Config& parent) : mParent{parent} {
    selection.setUpdateHandler([this](uint32 id) {
        if (id != selection.ID_SELECTION) return;

        if (selection == -1) {
            presetProxy.unbind();

            nameProxy.unbind();
            dirProxy.unbind();
            trackProxy.unbind();

            bladeProxy.unbind();

            commentProxy.unbind();
            styleProxy.unbind();
        } else {
            presetProxy.bind(std::next(mArrays.begin(), selection)->selection);
        }
    });
}

vector<string> Config::PresetArrays::presetArrayNames() const {
    vector<string> ret;
    ret.reserve(mArrays.size());
    for (const auto& array : mArrays) {
        ret.push_back(static_cast<string>(array.name));
    }
    return ret;
}

Config::PresetArray& Config::PresetArrays::addArray(string name) {
    auto& ret{mArrays.emplace_back()};
    ret.name = std::move(name);
    selection.setChoices(presetArrayNames());
    mParent.bladeArrays.refreshPresetArrays();
    return ret;
}

void Config::PresetArrays::removeArray(uint32 idx) {
    assert(idx < mArrays.size());
    mArrays.erase(std::next(mArrays.begin(), idx));
    selection.setChoices(presetArrayNames());
    mParent.bladeArrays.refreshPresetArrays();
}

void Config::PresetArrays::addInjection(const string& name) {
    mInjections.emplace_back(Injection{name});
    notifyData.notify(NOTIFY_INJECTIONS);
}

void Config::PresetArrays::removeInjection(const Injection& injection) {
    auto iter{mInjections.begin()};
    for (; iter != mInjections.end(); ++iter) {
        if (&*iter == &injection) break;
    }
    if (iter == mInjections.end()) return;

    mInjections.erase(iter);

    notifyData.notify(NOTIFY_INJECTIONS);
}

