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

Config::Blade::Blade(Config& config) : mConfig{config} {
    type.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::Notifier::ID_REBOUND and id != type.ID_SELECTION) return;
        if (id == type.ID_SELECTION) mConfig.presetArrays.syncStyleDisplay();

        auto arraySelection{static_cast<int32>(mConfig.bladeArrays.arraySelection)};
        if (arraySelection != -1) {
            auto& selectedArray{mConfig.bladeArrays.array(arraySelection)};
            if (selectedArray.bladeSelection != -1) {
                auto& selectedBlade{selectedArray.blade(selectedArray.bladeSelection)};
                if (&selectedBlade == this) {
                    mConfig.bladeArrays.notifyData.notify(BladeArrays::ID_BLADE_TYPE_SELECTION);
                }
            }
        }
    });

    type.setChoices(Utils::createEntries({
        "WS281X",
        _("Simple"),
    }));
    type = WS281X;
}

Config::BladeConfig::BladeConfig(Config& config) : mConfig{config} {
    presetArray.setPersistence(PCUI::ChoiceData::PERSISTENCE_INDEX);
    bladeSelection.setPersistence(PCUI::ChoiceData::PERSISTENCE_INDEX);

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

    bladeSelection.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::Notifier::ID_REBOUND and id != bladeSelection.ID_SELECTION) return;

        auto& bladeArray{mConfig.bladeArrays.array(mConfig.bladeArrays.arraySelection)};
        if (&bladeArray != this) return;

        mConfig.bladeArrays.powerPinNameEntry = "";

        if (bladeSelection == -1) {
            mConfig.bladeArrays.bladeTypeProxy.unbind();
            mConfig.bladeArrays.subBladeSelectionProxy.unbind();

            mConfig.bladeArrays.powerPinProxy.unbind();

            mConfig.bladeArrays.colorOrder3Proxy.unbind();
            mConfig.bladeArrays.colorOrder4Proxy.unbind();
            mConfig.bladeArrays.hasWhiteProxy.unbind();
            mConfig.bladeArrays.useRGBWithWhiteProxy.unbind();

            mConfig.bladeArrays.dataPinProxy.unbind();
            mConfig.bladeArrays.lengthProxy.unbind();

            mConfig.bladeArrays.subBladeTypeProxy.unbind();
            mConfig.bladeArrays.subBladeLengthProxy.unbind();
            mConfig.bladeArrays.subBladeSegmentsProxy.unbind();

            mConfig.bladeArrays.star1Proxy.ledProxy.unbind();
            mConfig.bladeArrays.star1Proxy.resistanceProxy.unbind();
            mConfig.bladeArrays.star1Proxy.powerPinProxy.unbind();

            mConfig.bladeArrays.star2Proxy.ledProxy.unbind();
            mConfig.bladeArrays.star2Proxy.resistanceProxy.unbind();
            mConfig.bladeArrays.star2Proxy.powerPinProxy.unbind();

            mConfig.bladeArrays.star3Proxy.ledProxy.unbind();
            mConfig.bladeArrays.star3Proxy.resistanceProxy.unbind();
            mConfig.bladeArrays.star3Proxy.powerPinProxy.unbind();

            mConfig.bladeArrays.star4Proxy.ledProxy.unbind();
            mConfig.bladeArrays.star4Proxy.resistanceProxy.unbind();
            mConfig.bladeArrays.star4Proxy.powerPinProxy.unbind();
        } else {
            auto& selectedBlade{blade(bladeSelection)};
            mConfig.bladeArrays.bladeTypeProxy.bind(selectedBlade.type);

            // mConfig.bladeArrays.subBladeSelectionProxy.bind();

            mConfig.bladeArrays.powerPinProxy.bind(selectedBlade.ws281x().powerPins);

            mConfig.bladeArrays.colorOrder3Proxy.bind(selectedBlade.ws281x().colorOrder3);
            mConfig.bladeArrays.colorOrder4Proxy.bind(selectedBlade.ws281x().colorOrder4);
            mConfig.bladeArrays.hasWhiteProxy.bind(selectedBlade.ws281x().hasWhite);
            mConfig.bladeArrays.useRGBWithWhiteProxy.bind(selectedBlade.ws281x().useRGBWithWhite);

            mConfig.bladeArrays.dataPinProxy.bind(selectedBlade.ws281x().dataPin);
            mConfig.bladeArrays.lengthProxy.bind(selectedBlade.ws281x().length);

            // mConfig.bladeArrays.subBladeTypeProxy.bind(selectedBlade.ws281x());
            // mConfig.bladeArrays.subBladeLengthProxy.bind();
            // mConfig.bladeArrays.subBladeSegmentsProxy.bind();

            mConfig.bladeArrays.star1Proxy.ledProxy.bind(selectedBlade.simple().star1.led);
            mConfig.bladeArrays.star1Proxy.resistanceProxy.bind(selectedBlade.simple().star1.resistance);
            mConfig.bladeArrays.star1Proxy.powerPinProxy.bind(selectedBlade.simple().star1.powerPin);

            mConfig.bladeArrays.star2Proxy.ledProxy.bind(selectedBlade.simple().star2.led);
            mConfig.bladeArrays.star2Proxy.resistanceProxy.bind(selectedBlade.simple().star2.resistance);
            mConfig.bladeArrays.star2Proxy.powerPinProxy.bind(selectedBlade.simple().star2.powerPin);

            mConfig.bladeArrays.star3Proxy.ledProxy.bind(selectedBlade.simple().star3.led);
            mConfig.bladeArrays.star3Proxy.resistanceProxy.bind(selectedBlade.simple().star3.resistance);
            mConfig.bladeArrays.star3Proxy.powerPinProxy.bind(selectedBlade.simple().star3.powerPin);

            mConfig.bladeArrays.star4Proxy.ledProxy.bind(selectedBlade.simple().star4.led);
            mConfig.bladeArrays.star4Proxy.resistanceProxy.bind(selectedBlade.simple().star4.resistance);
            mConfig.bladeArrays.star4Proxy.powerPinProxy.bind(selectedBlade.simple().star4.powerPin);
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
        choices.emplace_back("Blade " + std::to_string(idx));
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
        choices.emplace_back("Blade " + std::to_string(idx));
    }
    bladeSelection.setChoices(std::move(choices));
    if (bladeSelection == idx) bladeSelection = -1;

    mConfig.presetArrays.syncStyleDisplay();
}

void Config::BladeConfig::addSubBlade() {

    mConfig.presetArrays.syncStyles();
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

