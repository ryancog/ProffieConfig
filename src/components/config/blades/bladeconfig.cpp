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
#include "utils/string.hpp"
#include "wx/translation.h"

using namespace config::blades;

Blade::Blade(data::Node *parent) : data::Node(parent) {
    data::Vector::Context types{types_};
    // Generic used as a dummy for unassigned.
    types.addCreate<WS281X>();
    types.addCreate<Simple>();
    types.addCreate<data::Generic>();

    data::Selector::Context{type_}.bind(&types_);
    data::Choice::Context{type_.choice_}.choose(0);

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
    data::Vector::Context types{types_};

    const auto& model{types.children()[eWS281X]};
    // NOLINTNEXTLINE suppress lifetimebound warning
    return static_cast<WS281X&>(*model);
}

Simple& Blade::simple() {
    data::Vector::Context types{types_};

    const auto& model{types.children()[eSimple]};
    // NOLINTNEXTLINE suppress lifetimebound warning
    return static_cast<Simple&>(*model);
}

BladeConfig::BladeConfig(data::Node *parent) : data::Node(parent) {
    const auto nameFilter{[](
        const data::String::Context&, std::string& str, size& pos
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

    name_.responder().onChange_ = [](const data::String::Context& ctxt) {
        ctxt.model().parent<BladeConfig>()->recomputeIssues();
    };

    id_.responder().onSet_ = [](const data::Integer::Context& ctxt) {
        auto& bladeConfig{*ctxt.model().parent<BladeConfig>()};
        bladeConfig.recomputeIssues();
        data::Bool::Context noBladeId{bladeConfig.noBladeId_};
        noBladeId.set(ctxt.val() == NO_BLADE);
    };

    const auto noBladeIdFilter{[](
        const data::Bool::Context& ctxt, bool& noBladeId
    ) {
        auto& bladeConfig{*ctxt.model().parent<BladeConfig>()};
        data::Integer::Context id{bladeConfig.id_};
        if (id.val() == NO_BLADE) noBladeId = true;
    }};
    noBladeId_.setFilter(noBladeIdFilter);

    noBladeId_.responder().onSet_ = [](const data::Bool::Context& ctxt) {
        auto& bladeConfig{*ctxt.model().parent<BladeConfig>()};
        if (ctxt.val()) {
            data::Integer::Context id{bladeConfig.id_};
            id.set(NO_BLADE);
        }
    };

    presetArray_.choice_.responder().onChoice_ = [](
        const data::Choice::Context& ctxt
    ) {
        auto& bladeConfig{*ctxt.model().parent<BladeConfig>()};
        bladeConfig.recomputeIssues();
    };

    blades_.responder().onInsert_ = [](
        const data::Vector::Context& ctxt, size
    ) {
        ctxt.model().root<Config>()->syncStyles();
    };

    data::Integer::Context{id_}.update({.min_=0, .max_=NO_BLADE});
}

BladeConfig::~BladeConfig() = default;

bool BladeConfig::enumerate(const EnumFunc& func) {
    assert(0); // TODO
}

data::Model *BladeConfig::find(uint64 id) {
    assert(0); // TODO
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

    data::Integer::Context{issues_}.set(issues);
}

std::string BladeConfig::issueString(uint32 issues) {
    std::string ret;
    if (issues & eIssue_No_Preset_Array) {
        ret += wxTRANSLATE("Blade Array is not linked to a Preset Array");
        ret += '\n';
    }
    if (issues & eIssue_Duplicate_ID) {
        ret += wxTRANSLATE("Blade Array has duplicate ID");
        ret += '\n';
    }
    if (issues & eIssue_Duplicate_Name) {
        ret += wxTRANSLATE("Blade Array has duplicate name");
        ret += '\n';
    }

    // Pop off last newline
    if (not ret.empty()) ret.pop_back();

    return ret;
}

