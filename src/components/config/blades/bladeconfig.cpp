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
#include "data/context.hpp"
#include "utils/parent.hpp"
#include "utils/string.hpp"

using namespace config::blades;

BladeConfig::BladeConfig(Config& config) :
    Model(config),
    blades_(root()),
    name_(root()),
    presetArray_(root()),
    id_(root()) {
    CreationScope createScope(this);

    static const auto nameTable{[] {
        data::hier::String::RecvTable table;
        table.onChange_ = data::map(&BladeConfig::onNameChange);
        return table;
    }()};
    amend(name_, nameTable);

    static const auto idTable{[] {
        data::hier::Integer::RecvTable table;
        table.onSet_ = data::map(&BladeConfig::onID);
        return table;
    }()};
    amend(id_, idTable);

    static const auto noBladeIDTable{[] {
        data::hier::Bool::RecvTable table;
        table.onSet_ = data::map(&BladeConfig::onNoBladeIDSet);
        return table;
    }()};
    amend(noBladeId_, noBladeIDTable);

    static const auto presetArrayTable{[] {
        data::hier::Choice::RecvTable table;
        table.onChoice_ = data::map(&BladeConfig::onPresetArrayChoice);
        return table;
    }()};
    amend(presetArray_.choice(), presetArrayTable);

    static const auto bladesTable{[] {
        data::hier::Vector::RecvTable table;
        table.onInsert_ = data::map(&BladeConfig::onBladesModify);
        table.onRemove_ = data::map(&BladeConfig::onBladesModify);
        return table;
    }()};
    amend(blades_, bladesTable);

    const auto nameFilter{[](
        const data::base::String::ROContext&, std::string& str, size& pos
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

    const auto noBladeIdFilter{[](
        const data::base::Bool::ROContext& ctxt, bool& noBladeId
    ) {
        const auto& bladeConfig{utils::parent<&BladeConfig::noBladeId_>(
            ctxt.model<data::prim::Bool>()
        )};

        auto id{data::context(bladeConfig.id_)};
        if (id.val() == NO_BLADE)
            noBladeId = true;
    }};
    noBladeId_.setFilter(noBladeIdFilter);

    id_.update({.min_=0, .max_=NO_BLADE});
    presetArray_.bind(&config.presetArrays_);
}

void BladeConfig::onActivate() {
    recomputeIssues();
}

auto BladeConfig::children() -> std::vector<Model *> {
    return {
        &blades_,
        &name_,
        &presetArray_,
        &id_,
    };
}

const data::base::Integer& BladeConfig::issues() const {
    return mIssues;
}

void BladeConfig::recomputeIssues() {
    int32 issues{0};

    auto presetArray{data::context(presetArray_)};
    if (presetArray.choiceIdx() == -1) {
        issues |= eIssue_No_Preset_Array;
    }

    auto name{data::context(name_)};
    auto id{data::context(id_)};

    auto bladeConfigs{data::context(root<Config>().bladeConfigs_)};
    for (const auto& model : bladeConfigs.children()) {
        auto& bladeConfig{dynamic_cast<BladeConfig&>(*model)};
        if (&bladeConfig == this) continue;

        auto otherName{data::context(bladeConfig.name_)};
        if (name.val() == otherName.val()) {
            issues |= eIssue_Duplicate_Name;
        }

        auto otherID{data::context(bladeConfig.id_)};
        if (id.val() == otherID.val()) {
            issues |= eIssue_Duplicate_ID;
        }
    }

    mIssues.set(issues);
}

void BladeConfig::onNameChange() {
    recomputeIssues();
}

void BladeConfig::onID() {
    recomputeIssues();

    auto id{data::context(id_)};
    noBladeId_.set(id.val() == NO_BLADE);
}

void BladeConfig::onNoBladeIDSet() {
    auto noBladeID{data::context(noBladeId_)};

    if (noBladeID.val())
        id_.set(NO_BLADE);
}

void BladeConfig::onPresetArrayChoice() {
    recomputeIssues();
}

void BladeConfig::onBladesModify(size) {
    root<Config>().calcNumBlades();
}

Blade::Blade(Config& config) :
    Model(config),
    brightness_(root()),
    mType(root()),
    mTypes(root()) {
    CreationScope createScope(this);

    // Model used as a dummy for unassigned.
    mTypes.append(std::make_unique<WS281X>(*this));
    mTypes.append(std::make_unique<Simple>(*this));
    mTypes.append(std::make_unique<data::hier::Model>(root()));

    mType.bind(&mTypes);
    mType.choice().choose(0);

    brightness_.update({.min_=0, .max_=100});
    brightness_.set(100);
}

auto Blade::children() -> std::vector<Model *> {
    return {
        &brightness_,
        &mType,
        &mTypes,
    };
}

const data::base::Selector& Blade::type() {
    return mType;
}

const data::base::Vector& Blade::types() {
    return mTypes;
}

WS281X& Blade::ws281x() {
    auto types{data::context(mTypes)};

    const auto& model{types.children()[eWS281X]};
    // NOLINTNEXTLINE suppress lifetimebound warning
    return dynamic_cast<WS281X&>(*model);
}

Simple& Blade::simple() {
    auto types{data::context(mTypes)};

    const auto& model{types.children()[eSimple]};
    // NOLINTNEXTLINE suppress lifetimebound warning
    return dynamic_cast<Simple&>(*model);
}

