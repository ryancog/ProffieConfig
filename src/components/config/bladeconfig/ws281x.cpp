#include "ws281x.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/config/bladeconfig/ws281x.cpp
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

#include "utils/string.h"

#include "../config.h"

Config::Split::Split(Config& config, WS281XBlade& parent) :
    mConfig{config}, mParent(parent), type{TYPE_MAX} {
    type.setUpdateHandler([this](uint32 id) {
        if (id != type.ID_SELECTION) return;

        auto usesLength{type == STANDARD or type == REVERSE or type == STRIDE or type == ZIG_ZAG};
        start.show(usesLength);
        end.show(usesLength);
        length.show(usesLength);

        auto usesSegments{type == STRIDE or type == ZIG_ZAG};
        segments.show(usesSegments);

        list.show(type == LIST);
        mConfig.bladeArrays.visualizerData.notify();
    });
    start.setUpdateHandler([this](uint32 id) {
        if (id != start.ID_VALUE) return;

        if (static_cast<uint32>(start) > static_cast<uint32>(end)) {
            end = static_cast<uint32>(start);
        }

        length = static_cast<uint32>(end) - static_cast<uint32>(start) + 1;
        mConfig.bladeArrays.visualizerData.notify();
    });
    end.setUpdateHandler([this](uint32 id) {
        if (id != end.ID_VALUE) return;

        if (static_cast<uint32>(start) > static_cast<uint32>(end)) {
            start = static_cast<uint32>(end);
        }

        length = static_cast<uint32>(end) - static_cast<uint32>(start) + 1;
        mConfig.bladeArrays.visualizerData.notify();
    });
    length.setUpdateHandler([this](uint32 id) {
        if (id != start.ID_VALUE) return;

        if (start + length > mParent.length) {
            length = mParent.length - start;
            return;
        }

        end = start + length - 1;
        mConfig.bladeArrays.visualizerData.notify();
    });

    segments.setRange(2, 6);
    type.setValue(0);
    length.setRange(0, std::numeric_limits<int32>::max());
    start.setRange(0, mParent.length);
    end.setRange(0, mParent.length);
    start.setValue(0);
}

Config::WS281XBlade::WS281XBlade(Config& config) : mConfig{config} {
    length.setUpdateHandler([this](uint32 id) {
        if (id != length.ID_VALUE) return;

        for (const auto& split : mSplits) {
            split->start.setRange(0, length);
            split->end.setRange(0, length);
        }
    });
    dataPin.setUpdateHandler([this](uint32 id) {
        if (id != dataPin.ID_VALUE) return;

        auto rawValue{static_cast<string>(dataPin)};
        uint32 numTrimmed{};
        auto insertionPoint{dataPin.getInsertionPoint()};
        Utils::trimUnsafe(
            rawValue,
            &numTrimmed,
            insertionPoint,
            {},
            true
        );

        if (rawValue == static_cast<string>(dataPin)) {
            return;
        }
        
        dataPin = std::move(rawValue);
        dataPin.setInsertionPoint(insertionPoint - numTrimmed);
    });
    hasWhite.setUpdateHandler([this](uint32 id) {
        if (id != hasWhite.ID_VALUE) return;

        colorOrder3.show(not hasWhite);
        if (not hasWhite) {
            auto newOrder3{static_cast<int32>(colorOrder4)};
            if (newOrder3 > ORDER4_WFIRST_START and newOrder3 < ORDER4_WFIRST_END) {
                newOrder3 -= ORDER4_WFIRST_START;
            }

            colorOrder3 = newOrder3;
        }
        colorOrder4.show(hasWhite);
        if (hasWhite) {
            colorOrder4 = static_cast<int32>(colorOrder3);
        }
        useRGBWithWhite.show(hasWhite);
    });
    powerPins.setUpdateHandler([this](uint32 id) {
        if (id != powerPins.ID_SELECTION) return;

        auto selected{static_cast<set<uint32>>(powerPins)};
        auto items{powerPins.items()};
        for (auto idx{6}; idx < items.size(); ++idx) {
            if (selected.find(idx) == selected.end()) {
                items.erase(std::next(items.begin(), idx));
                --idx;
            }
        }
        powerPins.setItems(std::move(items));
    });
    splitSelect.setUpdateHandler([this](uint32 id) {
        if (splitSelect.ID_CHOICES) {
            // splitSelect.show(not splitSelect.choices().empty());
            if (not splitSelect.choices().empty()) {
                if (splitSelect == -1) splitSelect = 0;
            } 
        }

        if (mConfig.bladeArrays.arraySelection == -1) return;
        auto& selectedArray{mConfig.bladeArrays.array(mConfig.bladeArrays.arraySelection)};
        if (selectedArray.bladeSelection == -1) return;
        auto& selectedBlade{selectedArray.blade(selectedArray.bladeSelection)};
        if (selectedBlade.type != Blade::WS281X) return;
        if (&selectedBlade.ws281x() != this) return;

        if (splitSelect != -1) {
            auto& selectedSplit{split(splitSelect)};
            mConfig.bladeArrays.splitTypeProxy.bind(selectedSplit.type);
            mConfig.bladeArrays.splitStartProxy.bind(selectedSplit.start);
            mConfig.bladeArrays.splitEndProxy.bind(selectedSplit.end);
            mConfig.bladeArrays.splitLengthProxy.bind(selectedSplit.length);
            mConfig.bladeArrays.splitSegmentsProxy.bind(selectedSplit.segments);
            mConfig.bladeArrays.splitListProxy.bind(selectedSplit.list);
        }

        mConfig.bladeArrays.notifyData.notify(BladeArrays::ID_SPLIT_SELECTION);
        mConfig.bladeArrays.visualizerData.notify();
    });

    powerPins.setItems(Utils::createEntries({
        "bladePowerPin1",
        "bladePowerPin2",
        "bladePowerPin3",
        "bladePowerPin4",
        "bladePowerPin5",
        "bladePowerPin6",
    }));
    dataPin.setDefaults(Utils::createEntries({
        "bladePin",
        "blade2Pin",
        "blade3Pin",
        "blade4Pin"
    }));
    length.setRange(0, 1000);
    length = 144;
    colorOrder3.setChoices(Utils::createEntries({
        _("GRB"),
        _("GBR"),
        _("BGR"),
        _("BRG"),
        _("RGB"),
        _("RBG"),
    }));
    colorOrder3 = GRB;
    colorOrder4.setChoices(Utils::createEntries({
        _("GRBW"),
        _("GBRW"),
        _("BGRW"),
        _("BRGW"),
        _("RGBW"),
        _("RBGW"),
        _("WGRB"),
        _("WGBR"),
        _("WBGR"),
        _("WBRG"),
        _("WRGB"),
        _("WRBG"),
    }));
    colorOrder4 = GRBW;

    hasWhite.setValue(false);
}

Config::Split& Config::WS281XBlade::addSplit() {
    auto& ret{*mSplits.emplace_back(std::make_unique<Split>(mConfig, *this))};

    vector<string> choices;
    for (auto idx{0}; idx < mSplits.size(); ++idx) {
        choices.emplace_back(_("SubBlade ").ToStdString() + std::to_string(idx));
    }
    splitSelect.setChoices(std::move(choices));

    return ret;
}

void Config::WS281XBlade::removeSplit(uint32 idx) {
    assert(idx < mSplits.size());

    mSplits.erase(std::next(mSplits.begin(), idx));

    vector<string> choices;
    for (auto idx{0}; idx < mSplits.size(); ++idx) {
        choices.emplace_back(_("SubBlade ").ToStdString() + std::to_string(idx));
    }
    splitSelect.setChoices(std::move(choices));
    if (splitSelect == idx) splitSelect = -1;
}

