#include "bladeconfig.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/config/blades/bladeconfig.cpp
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

#include "config/blades/simple.hpp"
#include "config/blades/ws281x.hpp"
#include "config/config.hpp"
#include "data/number.hpp"
#include "utils/parent.hpp"
#include "utils/string.hpp"

namespace {

constexpr std::string_view BLADES_STR{"Blades"};
constexpr std::string_view NAME_STR{"Name"};
constexpr std::string_view PRESETARR_STR{"PresetArray"};
constexpr std::string_view ID_STR{"ID"};

} // namespace

using namespace config::blades;

Blade::Blade(data::Node *parent) : data::Node(parent) {
    data::Vector::Context types{mTypes};
    // Generic used as a dummy for unassigned.
    types.addCreate<WS281X>();
    types.addCreate<Simple>();
    types.addCreate<data::Generic>();

    data::Selector::Context{mType}.bind(&mTypes);
    data::Choice::Context{mType.choice_}.choose(0);

    data::Integer::Context brightness{brightness_};
    brightness.update({.min_=0, .max_=100});
    brightness.set(100);
}

Blade::~Blade() = default;

bool Blade::enumerate(const EnumFunc& func) {
    assert(0); // TODO
}

data::Model *Blade::find(uint64) {
    assert(0); // TODO
}

WS281X& Blade::ws281x() {
    data::Vector::Context types{mTypes};

    const auto& model{types.children()[eWS281X]};
    // NOLINTNEXTLINE suppress lifetimebound warning
    return static_cast<WS281X&>(*model);
}

Simple& Blade::simple() {
    data::Vector::Context types{mTypes};

    const auto& model{types.children()[eSimple]};
    // NOLINTNEXTLINE suppress lifetimebound warning
    return static_cast<Simple&>(*model);
}

const data::Selector& Blade::type() {
    return mType;
}

const data::Vector& Blade::types() {
    return mTypes;
}

BladeConfig::BladeConfig(data::Node *parent) :
    data::Node(parent),
    blades_(this),
    name_(this),
    presetArray_(this),
    id_(this) {
    const auto nameFilter{[](
        const data::String::ROContext&, std::string& str, size& pos
    ) {
        uint32 numTrimmed{};
        utils::trimCppName(
            str,
            false,
            &numTrimmed,
            pos
        );

        for (auto& chr : str) chr = static_cast<char>(std::tolower(chr));
        pos -= numTrimmed;
    }};
    name_.setFilter(nameFilter);

    name_.responder().onChange_ = [](const data::String::ROContext& ctxt) {
        ctxt.model().parent<BladeConfig>()->recomputeIssues();
    };

    id_.responder().onSet_ = [](const data::Integer::ROContext& ctxt) {
        auto& bladeConfig{*ctxt.model().parent<BladeConfig>()};
        bladeConfig.recomputeIssues();
        data::Bool::Context noBladeId{bladeConfig.noBladeId_};
        noBladeId.set(ctxt.val() == NO_BLADE);
    };

    const auto noBladeIdFilter{[](
        const data::Bool::ROContext& ctxt, bool& noBladeId
    ) {
        const auto& bladeConfig{utils::parent<&BladeConfig::noBladeId_>(
            static_cast<const data::Bool&>(ctxt.model())
        )};

        data::Integer::ROContext id{bladeConfig.id_};
        if (id.val() == NO_BLADE) noBladeId = true;
    }};
    noBladeId_.setFilter(noBladeIdFilter);

    noBladeId_.responder().onSet_ = [](const data::Bool::ROContext& ctxt) {
        auto& bladeConfig{*ctxt.model().parent<BladeConfig>()};
        if (ctxt.val()) {
            data::Integer::Context id{bladeConfig.id_};
            id.set(NO_BLADE);
        }
    };

    presetArray_.choice_.responder().onChoice_ = [](
        const data::Choice::ROContext& ctxt
    ) {
        auto& bladeConfig{*ctxt.model().parent<BladeConfig>()};
        bladeConfig.recomputeIssues();
    };

    blades_.responder().onInsert_ = [](
        const data::Vector::ROContext& ctxt, size
    ) {
        ctxt.model().root<Config>()->syncStyles();
    };

    data::Integer::Context{id_}.update({.min_=0, .max_=NO_BLADE});
}

BladeConfig::~BladeConfig() = default;

bool BladeConfig::enumerate(const EnumFunc& func) {
    if (func(blades_, strID(BLADES_STR), BLADES_STR)) return true;
    if (func(name_, strID(NAME_STR), NAME_STR)) return true;
    if (func(presetArray_, strID(PRESETARR_STR), PRESETARR_STR)) return true;
    if (func(id_, strID(ID_STR), ID_STR)) return true;

    return false;
}

data::Model *BladeConfig::find(uint64 id) {
    if (id == strID(BLADES_STR)) return &blades_;
    if (id == strID(NAME_STR)) return &name_;
    if (id == strID(PRESETARR_STR)) return &presetArray_;
    if (id == strID(ID_STR)) return &id_;

    return nullptr;
}

const data::Integer& BladeConfig::issues() const {
    return mIssues;
}

void BladeConfig::recomputeIssues() {
    int32 issues{0};

    if (data::Choice::Context{presetArray_.choice_}.choice() == -1) {
        issues |= eIssue_No_Preset_Array;
    }

    data::String::Context name{name_};
    data::Integer::Context id{id_};

    auto& config{*root<Config>()};
    data::Vector::Context bladeConfigs{config.bladeConfigs_};
    for (const auto& model : bladeConfigs.children()) {
        auto& bladeconfig{static_cast<BladeConfig&>(*model)};
        if (&bladeconfig == this) continue;

        data::String::Context otherName{bladeconfig.name_};
        if (name.val() == otherName.val()) {
            issues |= eIssue_Duplicate_Name;
        }

        data::Integer::Context otherID{bladeconfig.id_};
        if (id.val() == otherID.val()) {
            issues |= eIssue_Duplicate_ID;
        }
    }

    data::Integer::Context{mIssues}.set(issues);
}

