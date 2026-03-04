#include "bladeawareness.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/settings/bladeawareness.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 4 of the License, or
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

#include "config/priv/strings.hpp"
#include "config/settings/settings.hpp"
#include "data/hierarchy/node.hpp"
#include "data/number.hpp"
#include "data/selection.hpp"
#include "utils/string.hpp"

using namespace config::settings;

BladeAwareness::BladeAwareness(Settings& settings) :
    data::Node(&settings) {
    using namespace priv;

    bladeDetect_.enable_.responder().onSet_ = [](
        const data::Bool::Context& ctxt
    ) {
        auto& awareness{*ctxt.model().parent<BladeAwareness>()};
        data::String::Context pin{awareness.bladeDetect_.pin_};
        pin.enable(ctxt.val());
    };

    bladeId_.enable_.responder().onSet_ = [](
        const data::Bool::Context& ctxt
    ) {
        auto& bladeId{ctxt.model().parent<BladeAwareness>()->bladeId_};
        data::String::Context{bladeId.pin_}.enable(ctxt.val());
        data::Choice::Context{bladeId.mode_}.enable(ctxt.val());
        data::Bool::Context{bladeId.powerForId_}.enable(ctxt.val());
        data::String::Context{bladeId.bridgePin_}.enable(ctxt.val());
        data::Integer::Context{bladeId.pullup_}.enable(ctxt.val());
        data::Bool::Context{bladeId.continuousScanning_}.enable(ctxt.val());
        // if (not set) bladeId_.continuousScanning = false;
    };

    { data::Choice::Context idMode{bladeId_.mode_};
        idMode.update(eBIDMode_Max);
        idMode.choose(0);
    }

    const auto bridgePinFilter{[](
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
    bladeId_.bridgePin_.setFilter(bridgePinFilter);

    { data::Integer::Context continuousTimes{bladeId_.continuousTimes_};
        continuousTimes.update({.min_ = 1, .max_ = 100});
        continuousTimes.set(8);
    }

    { data::Integer::Context continuousInterval{bladeId_.continuousInterval_};
        continuousInterval.update({.min_ = 1, .max_ = 120000});
        continuousInterval.set(300);
    }

    (bladeId_.continuousScanning_.responder().onSet_ = [](
        const data::Bool::Context& ctxt
    ) {
        auto& bladeId{ctxt.model().parent<BladeAwareness>()->bladeId_};
        data::Integer::Context itvl{bladeId.continuousInterval_};
        itvl.enable(ctxt.val());
        data::Integer::Context times{bladeId.continuousTimes_};
        times.enable(ctxt.val());
    })(bladeId_.continuousScanning_);

    (bladeId_.powerForId_.responder().onSet_ = [](
        const data::Bool::Context& ctxt
    ) {
        auto& bladeId{ctxt.model().parent<BladeAwareness>()->bladeId_};
        data::Selection::Context{bladeId.powerPins_}.enable(ctxt.val());
    })(bladeId_.powerForId_);

    bladeId_.powerPins_.responder().onSelection_ = [](
        const data::Selection::Context& ctxt, uint32 idx
    ) {
        if (idx < 6) return;
        if (ctxt.selected()[idx]) return;

        ctxt.remove(idx);
    };

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

BladeAwareness::~BladeAwareness() = default;

bool BladeAwareness::enumerate(const EnumFunc&) {
    assert(0); // TODO
}

data::Model *BladeAwareness::find(uint64) {
    assert(0); // TODO
}


