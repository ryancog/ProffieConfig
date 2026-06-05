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

#include "config/blades/servo.hpp"
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
    id_(root()),
    noBladeId_(root()) {
    CreationScope createScope(this);

    static const auto nameTable{[] {
        data::hier::String::RecvTable table;
        table.onChange_ = data::map<&BladeConfig::onNameChange>();
        return table;
    }()};
    observeWith(name_, nameTable);

    static const auto idTable{[] {
        data::hier::Integer::RecvTable table;
        table.onSet_ = data::map<&BladeConfig::onID>();
        return table;
    }()};
    respondWith(id_, idTable);

    static const auto noBladeIDTable{[] {
        data::hier::Bool::RecvTable table;
        table.onSet_ = data::map<&BladeConfig::onNoBladeIDSet>();
        return table;
    }()};
    respondWith(noBladeId_, noBladeIDTable);

    static const auto presetArrayTable{[] {
        data::hier::Choice::RecvTable table;
        table.onChoice_ = data::map<&BladeConfig::onPresetArrayChoice>();
        return table;
    }()};
    observeWith(presetArray_.choice(), presetArrayTable);

    static const auto bladesTable{[] {
        data::hier::Vector::RecvTable table;
        table.onInsert_ = data::map<&BladeConfig::onBladesModify>();
        table.onRemove_ = data::map<&BladeConfig::onBladesModify>();
        return table;
    }()};
    respondWith(blades_, bladesTable);

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
            ctxt.model<data::hier::Bool>()
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

auto BladeConfig::children() const -> std::vector<const Model *> {
    return {
        &blades_,
        &name_,
        &presetArray_,
        &id_,
        // This is a hierarchic model so that whenever ID or this changes
        // there aren't issues with them needing to set each other.
        &noBladeId_
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

    static const auto osTable{[] {
        data::base::Choice::RecvTable table;
        table.onChoice_ = data::map<&Blade::onOsChoice>();
        return table;
    }()};
    respondWith(root<Config>().osChoice(), osTable);

    static const auto typeTable{[] {
        data::base::Choice::RecvTable table;
        table.onChoice_ = data::map<&Blade::onType>();
        return table;
    }()};
    respondWith(mType.choice(), typeTable);

    // Model used as a dummy for unassigned.
    mTypes.append(std::make_unique<WS281X>(*this));
    mTypes.append(std::make_unique<Simple>(*this));
    mTypes.append(std::make_unique<data::hier::Model>(root()));

    mType.bind(&mTypes);
    mType.choice().choose(eWS281X);

    brightness_.update({.min_=0, .max_=100});
    brightness_.set(100);
}

auto Blade::children() const -> std::vector<const Model *> {
    return {
        &brightness_,
        &mType,
        &mTypes,
    };
}

auto Blade::childrenToHash() const -> std::vector<const Model *> {
    auto type{data::context(mType)};

    // Only consider the currently-selected type/child.
    return {
        &brightness_,
        &mType,
        type.selected<data::hier::Model>(),
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

void Blade::onActivate() {
    onOsChoice();
}

void Blade::onOsChoice() {
    const auto *os{root<Config>().os()};
    const auto osIsOrOver8{
        os and
        utils::Version(8).compare(os->version_) <= 0
    };

    auto type{data::context(mType)};
    auto choice{data::context(mType.choice())};

    if (osIsOrOver8) {
        if (choice.num() != eOS8_Max) {
            // This comes after so the enumerations are consistent across
            // versions.
            // TODO: Maybe come up with a way to rearrange these things
            mTypes.append(std::make_unique<Servo>(*this));
        }
    } else {
        if (choice.num() != eOS7_Max) {
            // First, move the choice back before removing so that it's never
            // unset.
            if (choice.idx() == eServo)
                choice.choose(eUnassigned);

            mTypes.remove(eServo);
        }
    }
}

void Blade::onType() {
    root<Config>().calcNumBlades();
}

