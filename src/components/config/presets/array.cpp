#include "array.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/config/presets/array.cpp
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

#include "config/config.hpp"
#include "utils/string.hpp"

using namespace config::presets;

Array::Array(data::Node *parent) :
    data::Node{parent} {
    const auto nameFilter{[](std::string& str, size& pos) {
        uint32 numTrimmed{};
        utils::trimCppName(
            str,
            false,
            &numTrimmed,
            pos
        );

        pos -= numTrimmed;
    }};

    presets_.responder().onInsert_ = [](
        const data::Vector::Context& ctxt, size
    ) {
        ctxt.model().root<Config>()->syncStyles();
    };
}

Array::~Array() = default;

bool Array::enumerate(const EnumFunc& func) {
    assert(0); // TODO
}

data::Model *Array::find(uint64 id) {
    assert(0); // TODO
}

/*
void Config::PresetArrays::addInjection(const string& name) {
    mInjections.emplace_back(std::make_unique<Injection>(name));
    notifyData.notify(NOTIFY_INJECTIONS);
}

void Config::PresetArrays::removeInjection(const Injection& injection) {
    auto iter{mInjections.begin()};
    for (; iter != mInjections.end(); ++iter) {
        if (&**iter == &injection) break;
    }
    if (iter == mInjections.end()) return;

    mInjections.erase(iter);

    notifyData.notify(NOTIFY_INJECTIONS);
}
*/

