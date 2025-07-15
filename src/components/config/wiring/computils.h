#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/config/wiring/computils.h
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

#include <config/wiring/wiring.h>
#include <utils/types.h>

namespace Config::Wiring::Components {

template<PadType... TYPES>
struct StaticEmittedTypes {
    static constexpr array<PadType, sizeof...(TYPES)> EMIT_TYPES{TYPES...};
};

template<PadType... TYPES>
struct StaticReceivedTypes {
    static constexpr array<PadType, sizeof...(TYPES)> RECEIVE_TYPES{TYPES...};
};

template<FullPadID... PADS>
struct StaticReceivedPads {
    static constexpr array<FullPadID, sizeof...(PADS)> RECEIVE_PADS{PADS...};
};

template<class STATIC_EMIT_TYPES = StaticEmittedTypes<>, class STATIC_RECEIVE_TYPES = StaticReceivedTypes<>, class STATIC_RECEIVE_PADS = StaticReceivedPads<>>
PadTypesSet staticTypesGen(const Pad& pad) {
    PadTypesSet ret{pad};
    for (auto type : STATIC_EMIT_TYPES::EMIT_TYPES) {
        ret.emit.addType(type);
    }
    for (auto type : STATIC_RECEIVE_TYPES::RECEIVE_TYPES) {
        ret.receive.addType(type);
    }
    for (auto pad : STATIC_RECEIVE_PADS::RECEIVE_PADS) {
        ret.receive.addPad(pad);
    }
    return ret;
}

template<UID CONNECTOR_ID, class STATIC_EMIT_TYPES = StaticEmittedTypes<>, class STATIC_RECEIVE_TYPES = StaticReceivedTypes<>, class STATIC_RECEIVE_PADS = StaticReceivedPads<>>
PadTypesSet staticTypesGen(const Pad& pad) {
    PadTypesSet ret{staticTypesGen<STATIC_EMIT_TYPES, STATIC_RECEIVE_TYPES, STATIC_RECEIVE_PADS>(pad)};
    ret.emit.connectorID = CONNECTOR_ID;
    return ret;
}

} // namespace Config::Wiring::Components

