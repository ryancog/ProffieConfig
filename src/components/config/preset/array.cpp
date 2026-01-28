#include "array.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
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

pcui::TextData Config::PresetArrays::dummyCommentData;
pcui::TextData Config::PresetArrays::dummyStyleData;

Config::PresetArray::PresetArray(Config& config) :
    mConfig{config} {

    selection.setPersistence(pcui::ChoiceData::Persistence::Index);

    auto isCurrentArray{[this]() {
        if (mConfig.presetArrays.selection == -1) return false;

        auto& selectedArray{
            mConfig.presetArrays.array(mConfig.presetArrays.selection)
        };
        return &selectedArray == this;
    }};

    name.setUpdateHandler([this, isCurrentArray](uint32 id) {
        if (id != pcui::TextData::eID_Value) return;

        auto rawValue{static_cast<string>(name)};
        uint32 numTrimmed{};
        auto insertionPoint{name.getInsertionPoint()};
        Utils::trimCppName(
            rawValue,
            false,
            &numTrimmed,
            insertionPoint
        );

        if (rawValue == static_cast<string>(name)) {
            notifyData.notify();
            if (isCurrentArray()) {
                mConfig.presetArrays.notifyData.notify(
                    PresetArrays::NOTIFY_ARRAY_NAME
                );
            }

            auto idx{0};
            const auto& arrays{mConfig.presetArrays.arrays()};

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
        if (
                id != pcui::Notifier::eID_Rebound and 
                id != pcui::ChoiceData::eID_Selection
           ) return;

        if (not isCurrentArray()) return;

        if (selection == -1) {
            mConfig.presetArrays.nameProxy.unbind();
            mConfig.presetArrays.dirProxy.unbind();
            mConfig.presetArrays.trackProxy.unbind();
            mConfig.presetArrays.styleSelectProxy.unbind();
            mConfig.presetArrays.commentProxy.bind(
                PresetArrays::dummyCommentData
            );
            mConfig.presetArrays.styleProxy.bind(
                PresetArrays::dummyStyleData
            );
        } else {
            auto& preset{**std::next(mPresets.begin(), selection)};
            mConfig.presetArrays.nameProxy.bind(preset.name);
            mConfig.presetArrays.dirProxy.bind(preset.fontDir);
            mConfig.presetArrays.trackProxy.bind(preset.track);
            mConfig.presetArrays.styleSelectProxy.bind(preset.styleSelection);
        }

        mConfig.presetArrays.notifyData.notify(PresetArrays::NOTIFY_PRESETS);
    });
}

Config::Preset& Config::PresetArray::addPreset() {
    auto choices{selection.choices()};
    auto& preset{*mPresets.emplace_back(
        std::make_unique<Preset>(mConfig, *this)
    )};

    choices.push_back(preset.name);
    selection.setChoices(std::move(choices));
    mConfig.presetArrays.syncStyles();
    return preset;
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

    auto ptr{std::move(mPresets[idx])};
    mPresets.erase(std::next(mPresets.begin(), idx));
    mPresets.insert(std::next(mPresets.begin(), idx - 1), std::move(ptr));

    auto choices{selection.choices()};
    auto choiceIter{std::next(choices.begin(), idx)};
    auto choice{std::move(*choiceIter)};
    choices.erase(choiceIter);
    choices.insert(
        std::next(choices.begin(), idx - 1), 
        std::move(choice)
    );
    selection.setChoices(std::move(choices));
    selection = static_cast<int32>(idx - 1);
}

void Config::PresetArray::movePresetDown(uint32 idx) {
    if (idx >= selection.choices().size()) return;
    if (idx == selection.choices().size() - 1) return;

    auto ptr{std::move(mPresets[idx])};
    mPresets.erase(std::next(mPresets.begin(), idx));
    mPresets.insert(std::next(mPresets.begin(), idx + 1), std::move(ptr));

    auto choices{selection.choices()};
    auto choiceIter{std::next(choices.begin(), idx)};
    auto choice{std::move(*choiceIter)};
    choices.erase(choiceIter);
    choices.insert(
        std::next(choices.begin(), idx + 1), 
        std::move(choice)
    );
    selection.setChoices(std::move(choices));
    selection = static_cast<int32>(idx + 1);
}

Config::PresetArrays::PresetArrays(Config& parent) : mParent{parent} {
    // TODO: This should go somewhere else.
    dummyCommentData = _("Select or create preset and blade to edit style comments...").ToStdString();
    dummyCommentData.disable();
    dummyStyleData = _("Select or create preset and blade to edit style...").ToStdString();
    dummyStyleData.disable();

    commentProxy.bind(dummyCommentData);
    styleProxy.bind(dummyStyleData);

    selection.setPersistence(pcui::ChoiceData::Persistence::Index);
    styleDisplay.setPersistence(pcui::ChoiceData::Persistence::Index);
    styleSelectProxy.setPersistence(pcui::ChoiceDataProxy::Persistence::Index);

    selection.setUpdateHandler([this](uint32 id) {
        if (id == pcui::ChoiceData::eID_Choices) {
            if (not selection.choices().empty() and selection == -1) {
                selection = 0;
            }
            return;
        }
        if (id != pcui::ChoiceData::eID_Selection) return;

        if (selection == -1) {
            presetProxy.unbind();

            nameProxy.unbind();
            dirProxy.unbind();
            trackProxy.unbind();

            styleSelectProxy.unbind();

            commentProxy.bind(dummyCommentData);
            styleProxy.bind(dummyStyleData);
        } else {
            presetProxy.bind(
                (**std::next(mArrays.begin(), selection)).selection
            );

        }

        notifyData.notify(NOTIFY_SELECTION);
    });
    styleDisplay.setUpdateHandler([this](uint32 id) {
        if (id != pcui::ChoiceData::eID_Selection) return;

        syncStyleDisplay();
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

    if (selection == idx) selection = -1;
    mParent.bladeArrays.refreshPresetArrays(static_cast<int32>(idx));
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

void Config::PresetArrays::syncStyles() {
    const auto numBlades{mParent.bladeArrays.numBlades()};
    for (const auto& array : mArrays) {
        for (const auto& preset : array->presets()) {
            const auto newSize{
                std::max<uint32>(preset->mStyles.size(), numBlades)
            };

            preset->mStyles.reserve(newSize);
            for (auto idx{preset->mStyles.size()}; idx < newSize; ++idx) {
                preset->mStyles.emplace_back(
                    std::make_unique<Preset::Style>()
                );
            }
        }
    }
    syncStyleDisplay();
}

void Config::PresetArrays::syncStyleDisplay(int32 clearIdx) {
    styleDisplay.setChoices(
        vector{mParent.bladeArrays.arraySelection.choices()}
    );

    if (styleDisplay == clearIdx) styleDisplay = -1;

    vector<string> styleChoices;
    if (styleDisplay != -1) {
        auto numBlades{mParent.bladeArrays.numBlades()};
        auto& bladeArray{mParent.bladeArrays.array(styleDisplay)};

        auto count{0};
        auto mainIdx{0};
        auto subIdx{0};
        for (const auto& blade : bladeArray.blades()) {
            if (blade->type == Blade::Type::SIMPLE) {
                styleChoices.emplace_back(
                    wxString::Format(_("Blade %d"), mainIdx).ToStdString()
                );
                ++mainIdx;
                ++count;
            } else if (blade->type == Blade::Type::WS281X) {
                auto& ws281x{blade->ws281x()};
                if (ws281x.splits().empty()) {
                    styleChoices.emplace_back(
                        wxString::Format(_("Blade %d"), mainIdx).ToStdString()
                    );
                    ++count;
                } else {
                    subIdx = 0;
                    for (const auto& split : ws281x.splits()) {
                        const auto type{static_cast<Split::Type>(
                            static_cast<uint32>(split->type)
                        )};

                        switch (type) {
                            case Split::REVERSE:
                            case Split::STANDARD:
                            case Split::LIST:
                                styleChoices.emplace_back(
                                    wxString::Format(
                                        _("Blade %d:%d"),
                                        mainIdx,
                                        subIdx
                                    ).ToStdString()
                                );
                                ++count;
                                break;
                            case Split::STRIDE:
                            case Split::ZIG_ZAG:
                                for (
                                        auto idx{0};
                                        idx < split->segments;
                                        ++idx
                                    ) {
                                    styleChoices.emplace_back(
                                        wxString::Format(
                                            _("Blade %d:%d:%d"),
                                            mainIdx,
                                            subIdx,
                                            idx
                                        ).ToStdString()
                                    );
                                    ++count;
                                }
                                break;
                            case Split::TYPE_MAX:
                            default:
                                assert(0);
                        }
                        ++subIdx;
                    }
                }
                ++mainIdx;
            } else if (blade->type == Blade::Type::UNASSIGNED) {
                styleChoices.emplace_back(_("Unassigned").ToStdString());
                ++count;
            }
        }

        for (auto idx{count}; idx < numBlades; ++idx) {
            styleChoices.emplace_back(_("Unassigned").ToStdString());
        }
    }
    for (const auto& array : mArrays) {
        for (const auto& preset : array->presets()) {
            preset->styleSelection.setChoices(vector{styleChoices});
        }
    }
}

