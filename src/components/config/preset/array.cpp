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
#include "utils/string.h"

namespace Config {

} // namespace Config

PCUI::TextData Config::PresetArrays::dummyCommentData;
PCUI::TextData Config::PresetArrays::dummyStyleData;

Config::PresetArray::PresetArray(Config& config) :
    mConfig{config} {

    selection.setPersistence(PCUI::ChoiceData::PERSISTENCE_INDEX);

    auto isCurrentArray{[this]() {
        if (mConfig.presetArrays.selection == -1) return false;
        auto& selectedArray{mConfig.presetArrays.array(mConfig.presetArrays.selection)};
        return &selectedArray == this;
    }};

    name.setUpdateHandler([this, isCurrentArray](uint32 id) {
        if (id != name.ID_VALUE) return;

        auto rawValue{static_cast<string>(name)};
        uint32 numTrimmed{};
        auto insertionPoint{name.getInsertionPoint()};
        Utils::trimUnsafe(
            rawValue,
            &numTrimmed,
            insertionPoint
        );

        if (rawValue == static_cast<string>(name)) {
            notifyData.notify();
            if (isCurrentArray()) mConfig.presetArrays.notifyData.notify(PresetArrays::NOTIFY_ARRAY_NAME);

            auto idx{0};
            auto& arrays{mConfig.presetArrays.arrays()};
            auto arrayIter{arrays.begin()};
            auto arrayEnd{arrays.end()};
            for (; idx < arrays.size(); ++idx, ++arrayIter) {
                if (&**arrayIter == this) break;
            }
            auto choices{mConfig.presetArrays.selection.choices()};
            if (idx != arrays.size() and idx != choices.size()) {
                choices[idx] = name;
                mConfig.presetArrays.selection.setChoices(std::move(choices));
                mConfig.bladeArrays.refreshPresetArrays();
            }

            return;
        }

        name = std::move(rawValue);
        name.setInsertionPoint(insertionPoint - numTrimmed);
    });
    selection.setUpdateHandler([this, isCurrentArray](uint32 id) {
        if (selection == -1) {
            mConfig.presetArrays.nameProxy.unbind();
            mConfig.presetArrays.dirProxy.unbind();
            mConfig.presetArrays.trackProxy.unbind();
            mConfig.presetArrays.styleSelectProxy.unbind();
        } else {
            auto& preset{*std::next(mPresets.begin(), selection)};
            mConfig.presetArrays.nameProxy.bind(preset.name);
            mConfig.presetArrays.dirProxy.bind(preset.fontDir);
            mConfig.presetArrays.trackProxy.bind(preset.track);
            mConfig.presetArrays.styleDisplayProxy.bind(preset.styleDisplay);
            mConfig.presetArrays.styleSelectProxy.bind(preset.styleSelection);
        }

        if (isCurrentArray()) mConfig.presetArrays.notifyData.notify(PresetArrays::NOTIFY_PRESETS);
    });
}

void Config::PresetArray::addPreset() {
    auto choices{selection.choices()};
    choices.push_back(mPresets.emplace_back(mConfig, *this).name);
    selection.setChoices(std::move(choices));
}

void Config::PresetArray::removePreset(uint32 idx) {
    if (idx >= selection.choices().size()) return;

    mPresets.erase(std::next(mPresets.begin(), idx));

    auto choices{selection.choices()};
    choices.erase(std::next(choices.begin(), idx));
    selection.setChoices(std::move(choices));
    selection = -1;
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
    auto move{std::next(choices.begin(), idx)};
    auto moveValue{*move};
    choices.erase(move);
    choices.insert(
        std::next(choices.begin(), idx - 1), 
        moveValue
    );
    selection.setChoices(std::move(choices));
    selection = idx - 1;
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
    auto move{std::next(choices.begin(), idx)};
    auto moveValue{*move};
    choices.erase(move);
    choices.insert(
        std::next(choices.begin(), idx + 1), 
        moveValue
    );
    selection.setChoices(std::move(choices));
    selection = idx + 1;
}

Config::PresetArrays::PresetArrays(Config& parent) : mParent{parent} {
    dummyCommentData = _("Select or create preset and blade to edit style comments...").ToStdString();
    dummyCommentData.disable();
    dummyStyleData = _("Select or create preset and blade to edit style...").ToStdString();
    dummyStyleData.disable();

    commentProxy.bind(dummyCommentData);
    styleProxy.bind(dummyStyleData);

    selection.setPersistence(PCUI::ChoiceData::PERSISTENCE_INDEX);

    selection.setUpdateHandler([this](uint32 id) {
        if (id != selection.ID_SELECTION) return;

        if (selection == -1) {
            presetProxy.unbind();

            nameProxy.unbind();
            dirProxy.unbind();
            trackProxy.unbind();

            styleSelectProxy.unbind();

            commentProxy.bind(dummyCommentData);
            styleProxy.bind(dummyStyleData);
        } else {
            presetProxy.bind((**std::next(mArrays.begin(), selection)).selection);
        }

        notifyData.notify(NOTIFY_SELECTION);
    });
}

Config::PresetArray& Config::PresetArrays::addArray(string name) {
    auto& ret{mArrays.emplace_back(std::make_unique<PresetArray>(mParent))};
    ret->name = std::move(name);
    vector<string> choices;
    choices.reserve(mArrays.size());
    for (const auto& array : mArrays) choices.push_back(array->name);
    selection.setChoices(std::move(choices));
    mParent.bladeArrays.refreshPresetArrays();
    return *ret;
}

void Config::PresetArrays::removeArray(uint32 idx) {
    assert(idx < mArrays.size());
    mArrays.erase(std::next(mArrays.begin(), idx));
    vector<string> choices;
    choices.reserve(mArrays.size());
    for (const auto& array : mArrays) choices.push_back(array->name);
    selection.setChoices(std::move(choices));
    selection = -1;
    mParent.bladeArrays.refreshPresetArrays(idx);
}

void Config::PresetArrays::addInjection(const string& name) {
    mInjections.emplace_back(std::make_unique<Injection>(name));
    notifyData.notify(NOTIFY_INJECTIONS);
}

void Config::PresetArrays::removeInjection(const Injection& injection) {
    auto iter{mInjections.begin()};
    for (; iter != mInjections.end(); ++iter) {
        if (&**iter == &injection) break;
    }
    if (iter == mInjections.end()) return;

    mInjections.erase(iter);

    notifyData.notify(NOTIFY_INJECTIONS);
}

