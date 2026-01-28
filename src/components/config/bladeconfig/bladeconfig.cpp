#include "bladeconfig.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
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

Config::Blade::Blade(Config& config) :
    mConfig{config}, mPixelBlade(config) {
    type.setUpdateHandler([this](uint32 id) {
        if (
                id != pcui::Notifier::eID_Rebound and
                id != pcui::ChoiceData::eID_Selection
           ) return;
        if (id == pcui::ChoiceData::eID_Selection) {
            mConfig.presetArrays.syncStyleDisplay();
        }

        auto& bladeArrays{mConfig.bladeArrays};

        auto arraySelection{static_cast<int32>(bladeArrays.arraySelection)};
        if (arraySelection != -1) {
            auto& selectedArray{bladeArrays.array(arraySelection)};

            if (selectedArray.bladeSelection != -1) {
                auto& selectedBlade{
                    selectedArray.blade(selectedArray.bladeSelection)
                };

                if (&selectedBlade == this) {
                    bladeArrays.pixelBrightnessProxy.unbind();
                    bladeArrays.simpleBrightnessProxy.unbind();

                    if (type == WS281X) {
                        bladeArrays.pixelBrightnessProxy.bind(brightness);
                    } else if (type == SIMPLE) {
                        bladeArrays.simpleBrightnessProxy.bind(brightness);
                    }

                    bladeArrays.notifyData.notify(
                        BladeArrays::ID_BLADE_TYPE_SELECTION
                    );
                }
            }
        }
    });

    type.setChoices(Utils::createEntries({
        "WS281X",
        _("Simple"),
        _("Unassigned"),
    }));
    type = WS281X;

    brightness.setRange(0, 100);
    brightness.setValue(100);
}

Config::BladeConfig::BladeConfig(Config& config) : mConfig{config} {
    presetArray.setPersistence(pcui::ChoiceData::Persistence::Index);
    bladeSelection.setPersistence(pcui::ChoiceData::Persistence::Index);

    name.setUpdateHandler([this](uint32 id) {
        if (id != pcui::TextData::eID_Value) return;

        auto nameValue{static_cast<string>(name)};
        auto insertionPoint{name.getInsertionPoint()};
        uint32 numTrimmed{};
        Utils::trimCppName(
            nameValue,
            false,
            &numTrimmed,
            insertionPoint
        );
        std::ranges::transform(
            nameValue,
            nameValue.begin(),
            [](unsigned char chr){ return std::tolower(chr); }
        );

        // No further updates needed, can update things.
        if (static_cast<string>(name) == nameValue) {
            auto& bladeArrays{mConfig.bladeArrays};

            uint32 idx{0};
            auto arrayIter{bladeArrays.arrays().begin()};
            auto arrayEnd{bladeArrays.arrays().end()};
            for (; arrayIter != arrayEnd; ++arrayIter, ++idx) {
                if (this == &**arrayIter) break;
            }

            auto choices{bladeArrays.arraySelection.choices()};
            if (static_cast<string>(name).empty()) {
                choices[idx] = _("[default]").ToStdString();
            } else choices[idx] = static_cast<string>(name);
            bladeArrays.arraySelection.setChoices(std::move(choices));

            mConfig.presetArrays.syncStyleDisplay();

            notifyData.notify();
            if (
                    bladeArrays.arraySelection != -1 and
                    &bladeArrays.array(bladeArrays.arraySelection) == this
                ) {
                bladeArrays.arrayIssues = computeIssues();
                bladeArrays.notifyData.notify(BladeArrays::ID_ARRAY_ISSUES);
            }
            return;
        }
        name = std::move(nameValue);
        name.setInsertionPoint(insertionPoint - numTrimmed);
    });

    id.setUpdateHandler([this](uint32 id) {
        if (id != pcui::NumericData::eID_Value) return;

        auto& bladeArrays{mConfig.bladeArrays};

        notifyData.notify();
        if (
                bladeArrays.arraySelection != -1 and
                &bladeArrays.array(bladeArrays.arraySelection) == this
            ) {
            bladeArrays.arrayIssues = computeIssues();
            bladeArrays.notifyData.notify(BladeArrays::ID_ARRAY_ISSUES);
        }

       noBladeID = this->id == NO_BLADE;
    });

    noBladeID.setUpdateHandler([this](uint32 id) {
        if (id != pcui::ToggleData::eID_Value) return;

        if (not noBladeID and this->id == NO_BLADE) noBladeID = true;
        if (noBladeID) this->id = NO_BLADE;
    });

    presetArray.setUpdateHandler([this](uint32 id) {
        if (id != pcui::ChoiceData::eID_Selection) return;

        auto& bladeArrays{mConfig.bladeArrays};

        notifyData.notify();
        if (
                bladeArrays.arraySelection != -1 and
                &bladeArrays.array(bladeArrays.arraySelection) == this
            ) {
            bladeArrays.arrayIssues = computeIssues();
            bladeArrays.notifyData.notify(BladeArrays::ID_ARRAY_ISSUES);
        }
    });

    bladeSelection.setUpdateHandler([this](uint32 id) {
        if (
                id != pcui::Notifier::eID_Rebound and
                id != pcui::ChoiceData::eID_Selection
           ) return;

        auto& bladeArrays{mConfig.bladeArrays};

        if (bladeArrays.arraySelection == -1) return;
        auto& bladeArray{bladeArrays.array(bladeArrays.arraySelection)};
        if (&bladeArray != this) return;

        bladeArrays.powerPinNameEntry = "";
        bladeArrays.unbindBlade();

        if (bladeSelection != -1) {
            auto& selectedBlade{blade(bladeSelection)};
            auto& ws281x{selectedBlade.ws281x()};
            auto& simple{selectedBlade.simple()};

            bladeArrays.bladeTypeProxy.bind(selectedBlade.type);
            bladeArrays.powerPinProxy.bind(ws281x.powerPins);

            bladeArrays.colorOrder3Proxy.bind(ws281x.colorOrder3);
            bladeArrays.colorOrder4Proxy.bind(ws281x.colorOrder4);
            bladeArrays.hasWhiteProxy.bind(ws281x.hasWhite);
            bladeArrays.useRGBWithWhiteProxy.bind(ws281x.useRGBWithWhite);

            bladeArrays.dataPinProxy.bind(ws281x.dataPin);
            bladeArrays.lengthProxy.bind(ws281x.length);

            bladeArrays.splitSelectionProxy.bind(ws281x.splitSelect);

            auto& star1Proxy{bladeArrays.star1Proxy};
            star1Proxy.ledProxy.bind(simple.star1.led);
            star1Proxy.resistanceProxy.bind(simple.star1.resistance);
            star1Proxy.powerPinProxy.bind(simple.star1.powerPin);

            auto& star2Proxy{bladeArrays.star1Proxy};
            star2Proxy.ledProxy.bind(simple.star2.led);
            star2Proxy.resistanceProxy.bind(simple.star2.resistance);
            star2Proxy.powerPinProxy.bind(simple.star2.powerPin);

            auto& star3Proxy{bladeArrays.star1Proxy};
            star3Proxy.ledProxy.bind(simple.star3.led);
            star3Proxy.resistanceProxy.bind(simple.star3.resistance);
            star3Proxy.powerPinProxy.bind(simple.star3.powerPin);

            auto& star4Proxy{bladeArrays.star1Proxy};
            star4Proxy.ledProxy.bind(simple.star4.led);
            star4Proxy.resistanceProxy.bind(simple.star4.resistance);
            star4Proxy.powerPinProxy.bind(simple.star4.powerPin);
        }

        mConfig.bladeArrays.notifyData.notify(BladeArrays::ID_BLADE_SELECTION);
    });

    id.setRange(0, NO_BLADE);
}

Config::Blade& Config::BladeConfig::addBlade() {
    auto& ret{*mBlades.emplace_back(std::make_unique<Blade>(mConfig))};

    vector<string> choices;
    choices.reserve(mBlades.size());
    for (auto idx{0}; idx < mBlades.size(); ++idx) {
        choices.emplace_back(_("Blade ").ToStdString() + std::to_string(idx));
    }
    bladeSelection.setChoices(std::move(choices));

    mConfig.presetArrays.syncStyles();
    return ret;
}

void Config::BladeConfig::removeBlade(uint32 idx) {
    assert(idx < mBlades.size());
    mBlades.erase(std::next(mBlades.begin(), idx));

    vector<string> choices;
    choices.reserve(mBlades.size());
    for (auto idx{0}; idx < mBlades.size(); ++idx) {
        choices.emplace_back(_("Blade ").ToStdString() + std::to_string(idx));
    }
    bladeSelection.setChoices(std::move(choices));
    if (bladeSelection == idx) bladeSelection = -1;

    mConfig.presetArrays.syncStyleDisplay();
}

[[nodiscard]] uint32 Config::BladeConfig::computeIssues() const {
    uint32 ret{0};

    if (presetArray == -1) ret |= ISSUE_NO_PRESETARRAY;

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

[[nodiscard]] string Config::BladeConfig::issueString(uint32 issues) {
    string ret;
    if (issues & ISSUE_NO_PRESETARRAY) {
        ret += wxTRANSLATE("Blade Array is not linked to a Preset Array");
        ret += '\n';
    }
    if (issues & ISSUE_DUPLICATE_ID) {
        ret += wxTRANSLATE("Blade Array has duplicate ID");
        ret += '\n';
    }
    if (issues & ISSUE_DUPLICATE_NAME) {
        ret += wxTRANSLATE("Blade Array has duplicate name");
        ret += '\n';
    }

    // Pop off last newline
    if (not ret.empty()) ret.pop_back();

    return ret;
}

