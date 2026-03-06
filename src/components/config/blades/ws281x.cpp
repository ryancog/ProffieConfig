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
#include "config/priv/strings.hpp"
#include "data/number.hpp"
#include "data/vector.hpp"
#include "utils/string.hpp"

using namespace config::blades;

WS281X::WS281X(data::Node *parent) : data::Node{parent} {
    using namespace priv;

    length_.responder().onSet_ = [](const data::Integer::Context& ctxt) {
        auto& ws281x{*ctxt.model().parent<WS281X>()};
        data::Vector::Context splits{ws281x.splits_};

        for (const auto& model : splits.children()) {
            auto& split{static_cast<Split&>(*model)};

            if (data::Integer::Context{split.segments_}.val() > ctxt.val()) {
                data::Bool::Context{
                    split.type_[Split::eStandard]
                }.set(true);
            }

            data::Integer::Context{split.start_}.update({
                .min_=0, .max_=ctxt.val() - 1
            });
            data::Integer::Context{split.end_}.update({
                .min_=0, .max_=ctxt.val() - 1
            });
        }
    };

    const auto dataPinFilter{[](
        const data::String::Context&, std::string& str, size& pos
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

    hasWhite_.responder().onSet_ = [](const data::Bool::Context& ctxt) {
        auto& ws281x{*ctxt.model().parent<WS281X>()};

        if (ctxt.val()) {
            data::Choice::Context{ws281x.colorOrder4_}.choose(
                data::Choice::Context{ws281x.colorOrder3_}.choice()
            );
            return;
        } 

        auto order4{static_cast<ColorOrder4>(
            data::Choice::Context{ws281x.colorOrder4_}.choice()
        )};

        int32 newOrder3{static_cast<int32>(order4)};
        if (
                order4 > eOrder4_White_First_Start and
                order4 < eOrder4_White_First_End
           ) {
            newOrder3 -= eOrder4_White_First_Start;
        }

        data::Choice::Context{ws281x.colorOrder3_}.choose(newOrder3);
    };

    powerPins_.responder().onSelection_ = [](
        const data::Selection::Context& ctxt, uint32 idx
    ) {
        if (idx < 6) return;
        if (ctxt.selected()[idx]) return;

        ctxt.remove(idx);
    };

    splits_.responder().onInsert_ = [](
        const data::Vector::Context& ctxt, size
    ) {
        ctxt.model().root<Config>()->syncStyles();
    };

    data::Selection::Context{powerPins_}.setItems({
        "bladePowerPin1",
        "bladePowerPin2",
        "bladePowerPin3",
        "bladePowerPin4",
        "bladePowerPin5",
        "bladePowerPin6",
    });

    { data::Integer::Context length{length_};
        length.update({.min_=1, .max_=1000});
        length.set(144);
    }

    { data::Choice::Context order3{colorOrder3_};
        order3.update(eOrder3_Max);
        order3.choose(eOrder3_GRB);
    }

    { data::Choice::Context order3{colorOrder4_};
        order3.update(eOrder4_Max);
        order3.choose(eOrder4_GRBW);
    }
}

WS281X::~WS281X() = default;

bool WS281X::enumerate(const EnumFunc& func) {
    assert(0); // TODO
}

data::Model *WS281X::find(uint64) {
    assert(0); // TODO
}

WS281X::Split::Split(data::Node *parent) :
    data::Node(parent),
    type_(Type::eMax, this) {

    type_.responder().onSelection_ = [](
        const data::Exclusive::Context& ctxt, size sel
    ) {
        auto& split{*ctxt.model().parent<Split>()};

        switch (static_cast<Type>(sel)) {
            case eStandard:
            case eReverse:
                data::Integer::Context{split.length_}.update({
                    .min_=1,
                    .max_=std::numeric_limits<int32>::max(),
                    .inc_=1
                });
                data::Integer::Context{split.end_}.update({
                    .inc_=1
                });
                break;
            case eStride: 
            case eZig_Zag:
                // If segments is enabled, it re-called the handler...
            case eList:
                break;
            default:
                assert(0);
                __builtin_unreachable();
        }

        ctxt.model().root<Config>()->syncStyles();
    };

    start_.responder().onSet_ = [](const data::Integer::Context& ctxt) {
        auto& split{*ctxt.model().parent<Split>()};

        const auto segVal{data::Integer::Context{split.segments_}.val()};
        const auto startVal{ctxt.val()};

        data::Integer::Context end{split.end_};
        end.update({
            .off_=(startVal % segVal) - 1
        });

        data::Integer::Context length{split.length_};
        if (end.params().inc_ != 1) {
            const auto lenVal{length.val()};
            end.set(startVal + lenVal - 1);
        } else {
            if (startVal > end.val()) {
                end.set(startVal);
            }

            length.set(end.val() - startVal + 1);
        }
    };

    end_.responder().onSet_ = [](const data::Integer::Context& ctxt) {
        auto& split{*ctxt.model().parent<Split>()};

        const auto endVal{ctxt.val()};

        data::Integer::Context start{split.start_};
        if (start.val() > endVal) {
            start.set(endVal - start.params().inc_ + 1);
        }

        data::Integer::Context length{split.length_};
        length.set(endVal - start.val() + 1);
    };

    const auto lenFilter{[](const data::Integer::Context& ctxt, int32& len) {
        auto parentLen{data::Integer::Context{
            ctxt.model().parent<Split>()->parent<WS281X>()->length_
        }.val()};

        if (len > parentLen) len = static_cast<int32>(parentLen);
    }};
    length_.setFilter(lenFilter);

    length_.responder().onSet_ = [](const data::Integer::Context& ctxt) {
        auto& split{*ctxt.model().parent<Split>()};
        auto parentLen{data::Integer::Context{
            split.parent<WS281X>()->length_
        }.val()};

        data::Integer::Context start{split.start_};
        const auto lengthVal{ctxt.val()};

        if (start.val() + lengthVal > parentLen) {
            start.set(parentLen - lengthVal);
            return;
        }

        data::Integer::Context end{split.end_};
        end.set(start.val() + lengthVal - 1);
    };

    segments_.responder().onSet_ = [](const data::Integer::Context& ctxt) {
        auto& split{*ctxt.model().parent<Split>()};
        auto parentLen{data::Integer::Context{
            split.parent<WS281X>()->length_
        }};

        const auto numSeg{ctxt.val()};

        if (parentLen.val() < numSeg) {
            parentLen.set(numSeg);
        }

        const auto startVal{data::Integer::Context{split.start_}.val()};

        data::Integer::Context{split.length_}.update({
            .min_=numSeg,
            .max_=std::numeric_limits<int32>::max(),
            .inc_=numSeg,
        });
        data::Integer::Context{split.end_}.update({
            .inc_=numSeg, .off_=(startVal % numSeg) - 1
        });

        split.root<Config>()->syncStyles();
    };

    const auto listFilter{[](
        const data::String::Context& ctxt, std::string& str, size& pos
    ) {
        auto& split{*ctxt.model().parent<Split>()};
        auto parentLen{data::Integer::Context{
            split.parent<WS281X>()->length_
        }.val()};

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
            auto clampValue{std::clamp<uint32>(value, 0, parentLen)};
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
    }};
    list_.setFilter(listFilter);

    data::Integer::Context ws281xLen{parent->parent<WS281X>()->length_};

    data::Integer::Context{segments_}.update({.min_=2, .max_=6});
    data::Integer::Context{length_}.update({
        .min_=1, .max_=std::numeric_limits<int32>::max()
    });
    data::Integer::Context{start_}.update({
        .min_=0, .max_=ws281xLen.val() - 1
    });
    data::Integer::Context{end_}.update({
        .min_=0, .max_=ws281xLen.val() - 1
    });
    data::Integer::Context{brightness_}.update({
        .min_=0, .max_=100
    });
    data::Integer::Context{brightness_}.set(100);
}

WS281X::Split::~Split() = default;

bool WS281X::Split::enumerate(const EnumFunc& func) {
    assert(0); // TODO
}

data::Model *WS281X::Split::find(uint64 id) {
    assert(0); // TODO
}

std::vector<uint32> WS281X::Split::listValues() {
    const auto str{data::String::Context{list_}.val()};
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

