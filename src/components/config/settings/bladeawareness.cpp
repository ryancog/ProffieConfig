#include "bladeawareness.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/settings/bladeawareness.cpp
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

#include "config/blades/bladeconfig.hpp"
#include "config/strings.hpp"
#include "config/settings/settings.hpp"
#include "data/context.hpp"
#include "utils/string.hpp"

using namespace config::settings;

BladeAwareness::BladeAwareness(Settings& settings) :
    Model(settings.root()),
    // TODO: Is there a more legible way of doing this?
    bladeDetect_{
        .enable_=root(),
        .pin_=root()
    },
    bladeId_{
        .enable_=root(),
        .pin_=root(),
        .mode_=root(),
        .bridgePin_=root(),
        .pullup_=root(),
        .powerForId_=root(),
        .powerPins_=root(),
        .noBladeIdRange_={
            .enable_=root(),
            .low_=root(),
            .high_=root(),
        },
        .continuous_={
            .enable_=root(),
            .interval_=root(),
            .times_=root(),
            .stopWhenIgnited_=root(),
            .timeout_={
                .enable_=root(),
                .mins_=root(),
            },
        },
    } {
    CreationScope createScope(this);

    static const auto detectEnableTable{[] {
        data::hier::Bool::RecvTable table;
        table.onSet_ = data::map<&BladeAwareness::onDetectEnable>();
        return table;
    }()};
    respondWith(bladeDetect_.enable_, detectEnableTable);

    static const auto idEnableTable{[] {
        data::hier::Bool::RecvTable table;
        table.onSet_ = data::map<&BladeAwareness::onIDEnable>();
        return table;
    }()};
    respondWith(bladeId_.enable_, idEnableTable);

    static const auto noBladeIdRangeEnableTable{[] {
        data::hier::Bool::RecvTable table;
        table.onSet_ = data::map<&BladeAwareness::onNoBladeIdRangeEnable>();
        return table;
    }()};
    respondWith(bladeId_.noBladeIdRange_.enable_, noBladeIdRangeEnableTable);

    static const auto noBladeIdRangeLowTable{[] {
        data::hier::Integer::RecvTable table;
        table.onSet_ = data::map<&BladeAwareness::onNoBladeIdRangeLow>();
        return table;
    }()};
    respondWith(bladeId_.noBladeIdRange_.low_, noBladeIdRangeLowTable);

    static const auto noBladeIdRangeHighTable{[] {
        data::hier::Integer::RecvTable table;
        table.onSet_ = data::map<&BladeAwareness::onNoBladeIdRangeHigh>();
        return table;
    }()};
    respondWith(bladeId_.noBladeIdRange_.high_, noBladeIdRangeHighTable);

    static const auto continuousEnableTable{[] {
        data::hier::Bool::RecvTable table;
        table.onSet_ = data::map<&BladeAwareness::onContinuousEnable>();
        return table;
    }()};
    respondWith(bladeId_.continuous_.enable_, continuousEnableTable);

    static const auto continuousTimeoutEnableTable{[] {
        data::hier::Bool::RecvTable table;
        table.onSet_ = data::map<&BladeAwareness::onContinuousTimeoutEnable>();
        return table;
    }()};
    respondWith(bladeId_.continuous_.timeout_.enable_, continuousEnableTable);

    static const auto idPowerTable{[] {
        data::hier::Bool::RecvTable table;
        table.onSet_ = data::map<&BladeAwareness::onIDPower>();
        return table;
    }()};
    respondWith(bladeId_.powerForId_, idPowerTable);

    const auto pinFilter{[](
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
    bladeId_.pin_.setFilter(pinFilter);
    bladeId_.bridgePin_.setFilter(pinFilter);

    const auto powerPinPruner{[](
        const data::base::Selection::ROContext&, uint32 idx
    ) {
        return idx > 6;
    }};
    bladeId_.powerPins_.setPruner(powerPinPruner);
    const auto powerPinFilter{[](
        const data::base::Selection::ROContext&, std::string& str
    ) {
        utils::trimCppName(str, true);
    }};
    bladeId_.powerPins_.setAddFilter(powerPinFilter);

    bladeId_.mode_.update(eBIDMode_Max);
    bladeId_.mode_.choose(0);

    bladeId_.noBladeIdRange_.low_.update({
        .min_=0, .max_=config::blades::NO_BLADE
    });
    bladeId_.noBladeIdRange_.high_.update({
        .min_=0, .max_=config::blades::NO_BLADE
    });
    bladeId_.noBladeIdRange_.low_.set(100);
    bladeId_.noBladeIdRange_.high_.set(1000);
    
    bladeId_.continuous_.times_.update({.min_=1, .max_=100});
    bladeId_.continuous_.times_.set(8);

    bladeId_.continuous_.interval_.update({.min_=1, .max_=120000});
    bladeId_.continuous_.interval_.set(300);

    bladeId_.powerPins_.setItems({
        "bladePowerPin1",
        "bladePowerPin2",
        "bladePowerPin3",
        "bladePowerPin4",
        "bladePowerPin5",
        "bladePowerPin6",
    });
}

BladeAwareness::~BladeAwareness() = default;

void BladeAwareness::onActivate() {
    onDetectEnable();
    onIDEnable();
    onNoBladeIdRangeEnable();
    onContinuousEnable();
    onContinuousTimeoutEnable();
    onIDPower();
}

auto BladeAwareness::children() const -> std::vector<const Model *> {
    return {
		&bladeDetect_.enable_,
		&bladeDetect_.pin_,

		&bladeId_.enable_,
		&bladeId_.pin_,

		&bladeId_.mode_,
		&bladeId_.bridgePin_,
		&bladeId_.pullup_,

		&bladeId_.powerForId_,
		&bladeId_.powerPins_,

        &bladeId_.noBladeIdRange_.enable_,
        &bladeId_.noBladeIdRange_.low_,
        &bladeId_.noBladeIdRange_.high_,

		&bladeId_.continuous_.enable_,
		&bladeId_.continuous_.interval_,
		&bladeId_.continuous_.times_,
		&bladeId_.continuous_.stopWhenIgnited_,

		&bladeId_.continuous_.timeout_.enable_,
		&bladeId_.continuous_.timeout_.mins_,
    };
}

void BladeAwareness::onDetectEnable() {
    auto ctxt{data::context(bladeDetect_.enable_)};
    bladeDetect_.pin_.enable(ctxt.val());
}

void BladeAwareness::onIDEnable() {
    auto ctxt{data::context(bladeId_.enable_)};
    bladeId_.pin_.enable(ctxt.val());
    bladeId_.mode_.enable(ctxt.val());
    bladeId_.powerForId_.enable(ctxt.val());
    bladeId_.bridgePin_.enable(ctxt.val());
    bladeId_.pullup_.enable(ctxt.val());
    bladeId_.noBladeIdRange_.enable_.enable(ctxt.val());
    bladeId_.continuous_.enable_.enable(ctxt.val());

    onNoBladeIdRangeEnable();
    onContinuousEnable();
    onIDPower();
}

void BladeAwareness::onNoBladeIdRangeEnable() {
    auto id{data::context(bladeId_.enable_)};
    auto rng{data::context(bladeId_.noBladeIdRange_.enable_)};
    auto en{id.val() and rng.val()};

    bladeId_.noBladeIdRange_.low_.enable(en);
    bladeId_.noBladeIdRange_.high_.enable(en);
}

void BladeAwareness::onNoBladeIdRangeLow() {
    auto low{data::context(bladeId_.noBladeIdRange_.low_)};
    auto high{data::context(bladeId_.noBladeIdRange_.high_)};

    if (low.val() > high.val())
        high.set(low.val());
}

void BladeAwareness::onNoBladeIdRangeHigh() {
    auto low{data::context(bladeId_.noBladeIdRange_.low_)};
    auto high{data::context(bladeId_.noBladeIdRange_.high_)};

    if (low.val() > high.val())
        low.set(high.val());
}

void BladeAwareness::onContinuousEnable() {
    auto id{data::context(bladeId_.enable_)};
    auto cont{data::context(bladeId_.continuous_.enable_)};
    auto en{id.val() and cont.val()};

    bladeId_.continuous_.interval_.enable(en);
    bladeId_.continuous_.times_.enable(en);
    bladeId_.continuous_.stopWhenIgnited_.enable(en);
    bladeId_.continuous_.timeout_.enable_.enable(en);

    onContinuousTimeoutEnable();
}

void BladeAwareness::onContinuousTimeoutEnable() {
    auto id{data::context(bladeId_.enable_)};
    auto cont{data::context(bladeId_.continuous_.enable_)};
    auto timeout{data::context(bladeId_.continuous_.timeout_.enable_)};
    auto en{id.val() and cont.val() and timeout.val()};

    bladeId_.continuous_.timeout_.mins_.enable(en);
}

void BladeAwareness::onIDPower() {
    auto id{data::context(bladeId_.enable_)};
    auto pow{data::context(bladeId_.powerForId_)};
    auto en{id.val() and pow.val()};

    bladeId_.powerPins_.enable(en);
}

