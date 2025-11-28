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
    segments.setRange(2, 6);
    length.setRange(1, std::numeric_limits<int32>::max());
    start.setRange(0, mParent.length - 1);
    end.setRange(0, mParent.length - 1);
    brightness.setRange(0, 100);
    brightness.setValue(100);

    type.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::RadiosData::ID_SELECTION) return;

        auto usesLength{type == STANDARD or type == REVERSE or type == STRIDE or type == ZIG_ZAG};
        start.show(usesLength);
        end.show(usesLength);
        length.show(usesLength);

        auto usesSegments{type == STRIDE or type == ZIG_ZAG};
        segments.show(usesSegments);
        if (segments.isShown()) segments.setValue(segments);
        else {
            length.setIncrement(1, false);
            end.setIncrement(1, false);
            length.setRange(1, std::numeric_limits<int32>::max());
        }

        list.show(type == LIST);

        mConfig.presetArrays.syncStyles();

        if (mConfig.bladeArrays.arraySelection == -1) return;
        auto& selectedArray{mConfig.bladeArrays.array(mConfig.bladeArrays.arraySelection)};
        if (selectedArray.bladeSelection == -1) return;
        auto& selectedBlade{selectedArray.blade(selectedArray.bladeSelection)};
        if (selectedBlade.type != Blade::WS281X) return;
        if (&selectedBlade.ws281x() != &mParent) return;

        mConfig.bladeArrays.visualizerData.notify();
        mConfig.bladeArrays.notifyData.notify(BladeArrays::ID_VISUAL_UPDATE);
    });
    start.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::NumericData::ID_VALUE) return;

        end.setOffset((start % segments) - 1, false);
        if (end.increment() != 1) {
            end = start + length - 1;
        } else {
            if (static_cast<uint32>(start) > static_cast<uint32>(end)) {
                end = static_cast<int32>(start);
            }

            length = static_cast<int32>(end) - static_cast<int32>(start) + 1;
        }

        if (mConfig.bladeArrays.arraySelection == -1) return;
        auto& selectedArray{mConfig.bladeArrays.array(mConfig.bladeArrays.arraySelection)};
        if (selectedArray.bladeSelection == -1) return;
        auto& selectedBlade{selectedArray.blade(selectedArray.bladeSelection)};
        if (selectedBlade.type != Blade::WS281X) return;
        if (&selectedBlade.ws281x() != &mParent) return;

        mConfig.bladeArrays.visualizerData.notify();
    });
    end.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::NumericData::ID_VALUE) return;

        if (static_cast<uint32>(start) > static_cast<uint32>(end)) {
            start = static_cast<int32>(end) - start.increment() + 1;
        }

        length = static_cast<int32>(end) - static_cast<int32>(start) + 1;

        if (mConfig.bladeArrays.arraySelection == -1) return;
        auto& selectedArray{mConfig.bladeArrays.array(mConfig.bladeArrays.arraySelection)};
        if (selectedArray.bladeSelection == -1) return;
        auto& selectedBlade{selectedArray.blade(selectedArray.bladeSelection)};
        if (selectedBlade.type != Blade::WS281X) return;
        if (&selectedBlade.ws281x() != &mParent) return;

        mConfig.bladeArrays.visualizerData.notify();
    });
    length.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::NumericData::ID_VALUE) return;

        if (length > mParent.length) {
            length = static_cast<int32>(mParent.length);
            return;
        }
        if (start + length > mParent.length) {
            start = mParent.length - length;
            return;
        }

        end = start + length - 1;

        if (mConfig.bladeArrays.arraySelection == -1) return;
        auto& selectedArray{mConfig.bladeArrays.array(mConfig.bladeArrays.arraySelection)};
        if (selectedArray.bladeSelection == -1) return;
        auto& selectedBlade{selectedArray.blade(selectedArray.bladeSelection)};
        if (selectedBlade.type != Blade::WS281X) return;
        if (&selectedBlade.ws281x() != &mParent) return;

        mConfig.bladeArrays.visualizerData.notify();
    });
    segments.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::NumericData::ID_VALUE) return;

        if (mParent.length < segments) {
            mParent.length = static_cast<int32>(segments);
        }
        length.setIncrement(segments, false);
        end.setIncrement(segments, false);
        end.setOffset((start % segments) - 1, false);
        length.setRange(segments, std::numeric_limits<int32>::max());

        mConfig.presetArrays.syncStyles();

        if (mConfig.bladeArrays.arraySelection == -1) return;
        auto& selectedArray{mConfig.bladeArrays.array(mConfig.bladeArrays.arraySelection)};
        if (selectedArray.bladeSelection == -1) return;
        auto& selectedBlade{selectedArray.blade(selectedArray.bladeSelection)};
        if (selectedBlade.type != Blade::WS281X) return;
        if (&selectedBlade.ws281x() != &mParent) return;

        mConfig.bladeArrays.visualizerData.notify();
    });
    list.setUpdateHandler([this](uint32) {
        auto str{static_cast<string>(list)};
        auto insertionPoint{list.getInsertionPoint()};

        for (auto idx{0}; idx < str.size(); ++idx) {
            if (
                    (not std::isdigit(str[idx]) and (idx == 0 or str[idx] != ',')) or
                    (idx != str.size() - 1 and (str[idx] == ',' and str[idx + 1] == ','))
               ) {
                if (idx < insertionPoint) --insertionPoint;
                str.erase(idx, 1);
                --idx;
            }
        }

        bool commaEnd{not str.empty() and str.back() == ','};
        vector<string> values;

        uint32 numStart{0};
        for (auto idx{0}; idx < str.size(); ++idx) {
            int32 end{-1};
            if (not commaEnd and idx == str.size() - 1) end = idx + 1;
            else if (str[idx] == ',') end = idx;

            if (end != -1) {
                auto substr{str.substr(numStart, end - numStart)};
                auto value{std::to_string(std::clamp<uint32>(std::stoi(substr), 0, mParent.length))};
                values.push_back(value);
                if (substr != value and insertionPoint >= numStart and insertionPoint < end) {
                    insertionPoint = end;
                }
                numStart = end + 1;
            }
        }

        str.clear();
        for (const auto& value : values) {
            str += value;
            str += ',';
        }
        if (not commaEnd and not str.empty()) str.pop_back();

        if (static_cast<string>(list) != str) {
            list = std::move(str);
            list.setInsertionPoint(insertionPoint);
            return;
        }

        if (mConfig.bladeArrays.arraySelection == -1) return;
        auto& selectedArray{mConfig.bladeArrays.array(mConfig.bladeArrays.arraySelection)};
        if (selectedArray.bladeSelection == -1) return;
        auto& selectedBlade{selectedArray.blade(selectedArray.bladeSelection)};
        if (selectedBlade.type != Blade::WS281X) return;
        if (&selectedBlade.ws281x() != &mParent) return;

        mConfig.bladeArrays.visualizerData.notify();
    });

    type.setValue(0);
}

vector<uint32> Config::Split::listValues() const {
    const auto str{static_cast<string>(list)};
    if (str.empty()) return {};

    vector<uint32> ret;
    bool commaEnd{str.back() == ','};

    uint32 numStart{0};
    for (auto idx{0}; idx < str.size(); ++idx) {
        int32 end{-1};
        if (not commaEnd and idx == str.size() - 1) end = idx + 1;
        else if (str[idx] == ',') end = idx;

        if (end != -1) {
            ret.push_back(std::stoi(str.substr(numStart, end - numStart)));
            numStart = end + 1;
        }
    }

    return ret;
}

Config::WS281XBlade::WS281XBlade(Config& config) : mConfig{config} {
    length.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::NumericData::ID_VALUE) return;

        for (const auto& split : mSplits) {
            if (split->segments > length) {
                split->type = Split::STANDARD;
            }

            split->start.setRange(0, length - 1);
            split->end.setRange(0, length - 1);
        }

        mConfig.bladeArrays.visualizerData.notify();
    });
    dataPin.setUpdateHandler([this](uint32 id) {
        if (id != PCUI::ComboBoxData::ID_VALUE) return;

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
        if (id != PCUI::ToggleData::ID_VALUE) return;

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
        if (id != PCUI::CheckListData::ID_SELECTION) return;

        auto selected{static_cast<set<uint32>>(powerPins)};
        auto items{powerPins.items()};
        for (auto idx{6}; idx < items.size(); ++idx) {
            if (not selected.contains(idx)) {
                items.erase(std::next(items.begin(), idx));
                --idx;
            }
        }
        powerPins.setItems(std::move(items));
    });
    splitSelect.setUpdateHandler([this](uint32 id) {
        if (id == PCUI::ChoiceData::ID_CHOICES) {
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
            mConfig.bladeArrays.splitBrightnessProxy.bind(selectedSplit.brightness);
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
    length.setRange(1, 1000);
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
    mConfig.presetArrays.syncStyles();

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
    mConfig.presetArrays.syncStyles();

    vector<string> choices;
    for (auto idx{0}; idx < mSplits.size(); ++idx) {
        choices.emplace_back(_("SubBlade ").ToStdString() + std::to_string(idx));
    }
    int32 oldSelect{splitSelect};
    splitSelect.setChoices(std::move(choices));
    splitSelect.setValue(std::clamp<int32>(oldSelect, 0, static_cast<int32>(splitSelect.choices().size()) - 1));
}

