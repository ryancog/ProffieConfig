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

#include "config/strings.hpp"
#include "config/settings/settings.hpp"
#include "data/hierarchy/node.hpp"
#include "data/number.hpp"
#include "data/selection.hpp"
#include "utils/string.hpp"

using namespace config::settings;

BladeAwareness::BladeAwareness(Settings& settings) :
    data::Node(&settings),
    // TODO: Is there a more legible way of doing this?
    bladeDetect_{
        .enable_=this,
        .pin_=this
    },
    bladeId_{
        .enable_=this,
        .pin_=this,
        .mode_=this,
        .bridgePin_=this,
        .pullup_=this,
        .powerForId_=this,
        .powerPins_=this,
        .continuous_={
            .enable_=this,
            .interval_=this,
            .times_=this
        }
    } {
    CreationScope createScope(*this);
    buildMap();
}

BladeAwareness::~BladeAwareness() = default;

bool BladeAwareness::enumerate(const EnumFunc& func) {
    for (auto& [id, data] : mMap) {
        auto& [str, model]{data};
        if (func(*model, id, str)) return true;
    }

    return false;
}

data::Model *BladeAwareness::find(uint64 id) {
    auto iter{mMap.find(id)};
    if (iter == mMap.end()) return nullptr;

    return iter->second.second;
}

void BladeAwareness::init() {
    bladeDetect_.enable_.responder().onSet_ = [](
        const data::Bool::ROContext& ctxt
    ) {
        auto& awareness{*ctxt.model().parent<BladeAwareness>()};
        data::String::Context pin{awareness.bladeDetect_.pin_};
        pin.enable(ctxt.val());
    };

    bladeId_.enable_.responder().onSet_ = [](
        const data::Bool::ROContext& ctxt
    ) {
        auto& bladeId{ctxt.model().parent<BladeAwareness>()->bladeId_};
        data::String::Context{bladeId.pin_}.enable(ctxt.val());
        data::Choice::Context{bladeId.mode_}.enable(ctxt.val());
        data::Bool::Context{bladeId.powerForId_}.enable(ctxt.val());
        data::String::Context{bladeId.bridgePin_}.enable(ctxt.val());
        data::Integer::Context{bladeId.pullup_}.enable(ctxt.val());
        data::Bool::Context{bladeId.continuous_.enable_}.enable(ctxt.val());
        // if (not set) bladeId_.continuousScanning = false;
    };

    { data::Choice::Context idMode{bladeId_.mode_};
        idMode.update(eBIDMode_Max);
        idMode.choose(0);
    }

    const auto bridgePinFilter{[](
        const data::String::ROContext&, std::string& str, size& pos
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
    bladeId_.bridgePin_.setFilter(bridgePinFilter);

    { data::Integer::Context times{bladeId_.continuous_.times_};
        times.update({.min_ = 1, .max_ = 100});
        times.set(8);
    }

    { data::Integer::Context interval{bladeId_.continuous_.interval_};
        interval.update({.min_ = 1, .max_ = 120000});
        interval.set(300);
    }

    (bladeId_.continuous_.enable_.responder().onSet_ = [](
        const data::Bool::ROContext& ctxt
    ) {
        auto& bladeId{ctxt.model().parent<BladeAwareness>()->bladeId_};
        data::Integer::Context itvl{bladeId.continuous_.interval_};
        itvl.enable(ctxt.val());
        data::Integer::Context times{bladeId.continuous_.times_};
        times.enable(ctxt.val());
    })(bladeId_.continuous_.enable_);

    (bladeId_.powerForId_.responder().onSet_ = [](
        const data::Bool::ROContext& ctxt
    ) {
        auto& bladeId{ctxt.model().parent<BladeAwareness>()->bladeId_};
        data::Selection::Context{bladeId.powerPins_}.enable(ctxt.val());
    })(bladeId_.powerForId_);

    const auto powerPinPruner{[](
        const data::Selection::ROContext&, uint32 idx
    ) {
        return idx >= 6;
    }};
    bladeId_.powerPins_.setPruner(powerPinPruner);

    { data::Selection::Context powerPins{bladeId_.powerPins_};
        powerPins.setItems({
            "bladePowerPin1",
            "bladePowerPin2",
            "bladePowerPin3",
            "bladePowerPin4",
            "bladePowerPin5",
            "bladePowerPin6",
        });
    }
}

void BladeAwareness::buildMap() {
    const auto process{[this] (std::string_view str, data::Model& model) {
        mMap[strID(str)] = {str, &model};
    }};

    process("BladeDetect.Enable", bladeDetect_.enable_);
    process("BladeDetect.Pin", bladeDetect_.pin_);

    process("BladeID.Enable", bladeId_.enable_);
    process("BladeID.Pin", bladeId_.pin_);

    process("BladeID.Mode", bladeId_.mode_);
    process("BladeID.BridgePin", bladeId_.bridgePin_);
    process("BladeID.Pullup", bladeId_.pullup_);

    process("BladeID.PowerForID", bladeId_.powerForId_);
    process("BladeID.PowerPins", bladeId_.powerPins_);

    auto& scan{bladeId_.continuous_};
    process("BladeID.Continuous.Enable", scan.enable_);
    process("BladeID.Continuous.Interval", scan.interval_);
    process("BladeID.Continuous.Times", scan.times_);
}

