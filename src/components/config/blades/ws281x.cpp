#include "ws281x.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/config/blades/ws281x.cpp
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

#include "config/config.hpp"
#include "config/blades/bladeconfig.hpp"
#include "config/strings.hpp"
#include "data/base/models/number.hpp"
#include "data/base/models/selection.hpp"
#include "data/context.hpp"
#include "data/hierarchic/models/number.hpp"
#include "utils/parent.hpp"
#include "utils/string.hpp"

using namespace config::blades;

WS281X::WS281X(Blade& blade) :
    Model(blade.root()),
    length_(root()),
    dataPin_(root()),
    colorOrder3_(root()),
    colorOrder4_(root()),
    hasWhite_(root()),
    useRgbWithWhite_(root()),
    powerPins_(root()),
    splits_(root()),
    parent_(blade) {
    CreationScope createScope(this);

    static const auto lengthTable{[] {
        data::hier::Integer::RecvTable table;
        table.onSet_ = data::map(&WS281X::onLength);
        return table;
    }()};
    amend(length_, lengthTable);

    static const auto hasWhiteTable{[] {
        data::hier::Bool::RecvTable table;
        table.onSet_ = data::map(&WS281X::onHasWhiteSet);
        return table;
    }()};
    amend(hasWhite_, hasWhiteTable);

    static const auto splitsTable{[] {
        data::hier::Vector::RecvTable table;
        table.onInsert_ = data::map(&WS281X::onSplitsModify);
        table.onRemove_ = data::map(&WS281X::onSplitsModify);
        return table;
    }()};
    amend(splits_, splitsTable);

    const auto dataPinFilter{[](
        const data::base::String::ROContext&, std::string& str, size& pos
    ) {
        uint32 numTrimmed{};
        utils::trimCppName(
            str,
            true,
            &numTrimmed,
            pos
        );

        pos -= numTrimmed;
    }};
    dataPin_.setFilter(dataPinFilter);

    const auto powerPinPruner{[](
        const data::base::Selection::ROContext&, uint32 idx
    ) {
        return idx >= 6;
    }};
    powerPins_.setPruner(powerPinPruner);

    powerPins_.setItems({
        "bladePowerPin1",
        "bladePowerPin2",
        "bladePowerPin3",
        "bladePowerPin4",
        "bladePowerPin5",
        "bladePowerPin6",
    });

    length_.update({.min_=1, .max_=1000});
    length_.set(144);

    colorOrder3_.update(eOrder3_Max);
    colorOrder3_.choose(eOrder3_GRB);

    colorOrder4_.update(eOrder4_Max);
    colorOrder4_.choose(eOrder4_GRBW);

    dataPin_.change("bladePin");
}

auto WS281X::children() const -> std::vector<const Model *> {
    return {
        &length_,
        &dataPin_,
        &colorOrder3_,
        &colorOrder4_,
        &hasWhite_,
        &useRgbWithWhite_,
        &powerPins_,
        &splits_,
    };
}

void WS281X::onLength() {
    auto length{data::context(length_)};
    auto splits{data::context(splits_)};

    for (const auto& model : splits.children()) {
        auto& split{dynamic_cast<Split&>(*model)};

        auto segments{data::context(split.segments_)};
        if (segments.val() > length.val()) {
            data::context(split.type_)[Split::eStandard].set(true);
        }

        { auto start{data::context(split.start_)};
            auto params{start.params()};
            params.min_ = 0;
            params.max_ = length.val() - 1;
            start.update(params);
        }

        { auto end{data::context(split.end_)};
            auto params{end.params()};
            params.min_ = 0;
            params.max_ = length.val() - 1;
            end.update(params);
        }
    }
}

void WS281X::onHasWhiteSet() {
    auto hasWhite{data::context(hasWhite_)};

    if (hasWhite.val()) {
        auto order3{data::context(colorOrder3_)};
        colorOrder4_.choose(order3.idx());
        return;
    } 

    auto order4{data::context(colorOrder4_)};
    auto order4Val{static_cast<ColorOrder4>(order4.idx())};

    int32 newOrder3{static_cast<int32>(order4Val)};
    if (
            order4Val >= eOrder4_White_First_Start and
            order4Val <= eOrder4_White_First_End
       ) {
        newOrder3 -= eOrder4_White_First_Start;
    }

    colorOrder3_.choose(newOrder3);
}

void WS281X::onSplitsModify(size) {
    root<Config>().calcNumBlades();
}

WS281X::Split::Split(WS281X& ws281x) :
    Model(ws281x.root()),
    type_(root(), Type::eMax),
    start_(root()),
    end_(root()),
    length_(root()),
    segments_(root()),
    list_(root()),
    brightness_(root()),
    mParent{ws281x} {
    CreationScope createScope(this);

    static const auto osTable{[] {
        data::base::Choice::RecvTable table;
        table.onChoice_ = data::map(&Split::onOsChoice);
        return table;
    }()};
    amend(root<Config>().osChoice(), osTable);

    // The logic for the interactions between these controls is more fragile
    // than I'd like, but I don't have a great idea of how to make it better,
    // nor do I feel like spending a particularly large amount of time on it
    // right now.

    static const auto typeTable{[] {
        data::base::Selection::RecvTable table;
        table.onSelection_ = data::map(&Split::onType);
        return table;
    }()};
    amend(type_, typeTable);

    static const auto startTable{[] {
        data::base::Integer::RecvTable table;
        table.onSet_ = data::map(&Split::onStart);
        return table;
    }()};
    amend(start_, startTable);

    static const auto endTable{[] {
        data::base::Integer::RecvTable table;
        table.onSet_ = data::map(&Split::onEnd);
        return table;
    }()};
    amend(end_, endTable);

    static const auto lengthTable{[] {
        data::base::Integer::RecvTable table;
        table.onSet_ = data::map(&Split::onLength);
        return table;
    }()};
    amend(length_, lengthTable);

    static const auto segmentsTable{[] {
        data::base::Integer::RecvTable table;
        table.onSet_ = data::map(&Split::onSegments);
        return table;
    }()};
    amend(segments_, segmentsTable);

    length_.setFilter(lengthFilter);
    list_.setFilter(listFilter);

    auto parentLen{data::context(ws281x.length_)};

    segments_.update({.min_=2, .max_=6});
    length_.update({.min_=1, .max_=std::numeric_limits<int32>::max()});
    start_.update({.min_=0, .max_=parentLen.val() - 1});
    end_.update({.min_=0, .max_=parentLen.val() - 1});
    brightness_.update({.min_=0, .max_=100});
    brightness_.set(100);
}

void WS281X::Split::onActivate() {
    onOsChoice();
}

auto WS281X::Split::children() const -> std::vector<const Model *> {
    return {
        &type_,
		&start_,
		&end_,
		&length_,
		&segments_,
		&list_,
		&brightness_,
    };
}

std::vector<uint32> WS281X::Split::listValues() {
    auto list{data::context(list_)};
    const auto& str{list.val()};

    if (str.empty()) return {};

    std::vector<uint32> ret;
    bool commaEnd{str.back() == ','};

    uint32 numStart{0};
    for (auto idx{0}; idx < str.size(); ++idx) {
        int32 end{-1};
        if (not commaEnd and idx == str.size() - 1) end = idx + 1;
        else if (str[idx] == ',') end = idx;

        if (end == -1) continue;

        auto substr{str.substr(numStart, end - numStart)};

        // With the list stripped via update handler, this should never fail
        auto value{strtoul(substr.c_str(), nullptr, 10)};

        ret.push_back(value);
        numStart = end + 1;
    }

    return ret;
}

void WS281X::Split::onOsChoice() {
    auto *os{root<Config>().os()};
    const auto osOver8{
        os and
        utils::Version(8).compare(os->version_) <= 0
    };

    auto *list{dynamic_cast<data::base::Bool *>(type_.children()[eList])};
    auto ctxt{data::context(*list)};

    ctxt.enable(osOver8);

    // Can't use this on versions less than OS8, need to unset it.
    if (not osOver8 and ctxt.val()) {
        auto *standard{type_.children()[eStandard]};
        dynamic_cast<data::base::Bool *>(standard)->set(true);
    }
}

void WS281X::Split::onType(uint32 sel) {
    switch (static_cast<Type>(sel)) {
        case eStandard:
        case eReverse:
            { auto length{data::context(length_)};
                auto params{length.params()};
                params.min_ = 1;
                params.max_ = std::numeric_limits<int32>::max();
                params.inc_ = 1;
                length.update(params);
            }
            { auto end{data::context(end_)};
                auto params{end.params()};
                params.inc_ = 1;
                end.update(params);
            }
            break;
        case eStride: 
        case eZig_Zag:
            // Update offset/inc and stuff for these.
            onSegments();
            break;
        case eList:
            break;
        case eMax:
            assert(0);
            __builtin_unreachable();
    }

    root<Config>().calcNumBlades();
}

void WS281X::Split::onStart() {
    auto segments{data::context(segments_)};
    auto start{data::context(start_)};
    auto end{data::context(end_)};
    auto length{data::context(length_)};

    auto endParams{end.params()};
    endParams.off_ = (start.val() % segments.val()) - 1;
    end.update(endParams);

    if (endParams.inc_ != 1) {
        end.set(start.val() + length.val() - 1);
    } else {
        if (start.val() > end.val()) {
            end.set(start.val());
        }

        length.set(end.val() - start.val() + 1);
    }
}

void WS281X::Split::onEnd() {
    auto start{data::context(start_)};
    auto end{data::context(end_)};

    if (start.val() > end.val()) {
        start.set(end.val() - start.params().inc_ + 1);
    }

    length_.set(end.val() - start.val() + 1);
}

void WS281X::Split::onLength() {
    auto parentLen{data::context(mParent.length_)};

    auto start{data::context(start_)};
    auto length{data::context(length_)};

    if (start.val() + length.val() > parentLen.val()) {
        start.set(parentLen.val() - length.val());
        return;
    }

    auto end{data::context(end_)};
    end.set(start.val() + length.val() - 1);
}

void WS281X::Split::onSegments() {
    auto parentLen{data::context(mParent.length_)};

    auto start{data::context(start_)};
    auto end{data::context(end_)};
    auto segments{data::context(segments_)};
    auto length{data::context(length_)};

    if (parentLen.val() < segments.val()) {
        parentLen.set(segments.val());
    }

    { auto params{length.params()};
        params.min_ = segments.val();
        params.inc_ = segments.val();
        length.update(params);
    }

    { auto params{end.params()};
        params.inc_ = segments.val();
        params.off_ = (start.val() % segments.val()) - 1;
        end.update(params);
    }

    root<Config>().calcNumBlades();
}

void WS281X::Split::lengthFilter(
    const data::base::Integer::ROContext& ctxt, int32& len
) {
    auto& split{utils::parent<&Split::length_>(
        ctxt.model<data::hier::Integer>()
    )};
    auto parentLen{data::context(split.mParent.length_)};

    if (len > parentLen.val())
        len = static_cast<int32>(parentLen.val());
}

void WS281X::Split::listFilter(
    const data::base::String::ROContext& ctxt, std::string& str, size& pos
) {
    auto& split{utils::parent<&Split::list_>(
        ctxt.model<data::hier::String>()
    )};
    auto parentLen{data::context(split.mParent.length_)};

    for (auto idx{0}; idx < str.size(); ++idx) {
        if (
                (not std::isdigit(str[idx]) and 
                 (idx == 0 or str[idx] != ',')) or
                (idx != str.size() - 1 and
                 str[idx] == ',' and str[idx + 1] == ',')
           ) {
            if (idx < pos) --pos;
            str.erase(idx, 1);
            --idx;
        }
    }

    bool commaEnd{not str.empty() and str.back() == ','};
    std::vector<std::string> values;

    uint32 numStart{0};
    for (auto idx{0}; idx < str.size(); ++idx) {
        int32 end{-1};
        if (not commaEnd and idx == str.size() - 1) end = idx + 1;
        else if (str[idx] == ',') end = idx;

        if (end == -1) continue;

        auto substr{str.substr(numStart, end - numStart)};

        // With the stripping above, this should *never* fail!
        auto value{strtoul(substr.c_str(), nullptr, 10)};
        auto clampValue{std::clamp<uint32>(value, 0, parentLen.val())};
        auto clampValueStr{std::to_string(clampValue)};

        values.push_back(clampValueStr);

        if (
                substr != clampValueStr and
                pos >= numStart and
                pos < end
           ) {
            pos = end;
        }
        numStart = end + 1;
    }

    str.clear();
    for (const auto& value : values) {
        str += value;
        str += ',';
    }
    if (not commaEnd and not str.empty()) str.pop_back();
}

