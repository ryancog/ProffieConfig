#include "preset.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/config/presets/preset.cpp
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

#include <wx/translation.h>

#include "config/presets/style.hpp"
#include "data/context.hpp"
#include "config/config.hpp"
#include "utils/string.hpp"

using namespace config::presets;

namespace {

constexpr size MAX_NAME_LEN{24};

} // namespace

Preset::Preset(Config& config) :
    Model(config),
    name_(root()),
    fontDir_(root()),
    track_(root()),
    styles_(root()) {
    CreationScope createScope(this);

    const auto nameFilter{[](
        const data::base::String::ROContext&, std::string& str, size& pos
    ) {
        uint32 numTrimmed{};
        utils::trim(
            str,
            {.allowAlpha=true, .allowNum=true, .safeList="\\"},
            &numTrimmed,
            pos
        );
        if (str.length() > MAX_NAME_LEN) str.resize(MAX_NAME_LEN);

        // Only allow \n
        uint32 idx{0};
        for (auto iter{str.begin()}; iter != str.end();) {
            if (
                    *iter == '\\' and
                    (
                        std::next(iter) == str.end() or
                        *std::next(iter) != 'n'
                    )
               ) {
                iter = str.erase(iter);
                if (pos > idx) ++numTrimmed;
                continue;
            }

            ++iter;
            ++idx;
        }

        pos -= numTrimmed;
    }};
    name_.setFilter(nameFilter);

    const auto fontDirFilter{[](
        const data::base::String::ROContext&, std::string& str, size& pos
    ) {
        uint32 numTrimmed{};
        utils::trim(
            str,
            {.allowAlpha=true, .allowNum=true, .safeList="/;_"},
            &numTrimmed,
            pos
        );

        pos -= numTrimmed;
    }};
    fontDir_.setFilter(fontDirFilter);

    const auto trackFilter{[](std::string& str, size& pos) {
        uint32 numTrimmed{};
        utils::trim(
            str,
            {.allowAlpha=true, .allowNum=true, .safeList="./_"},
            &numTrimmed,
            pos
        );

        pos -= numTrimmed;
    }};

    name_.change("newpreset");
}

Preset::Preset(const Preset& other, Config& config) :
    Model(config),
    name_(other.name_, root()),
    fontDir_(other.fontDir_, root()),
    track_(other.track_, root()),
    styles_(root()) {
    CreationScope createScope(this);

    auto otherStyles{data::context(other.styles_)};
    for (const auto& model : otherStyles.children()) {
        auto *otherStyle{dynamic_cast<Style *>(model.get())};
        styles_.append(std::make_unique<Style>(*otherStyle, root<Config>()));
    }

    // The extra work here avoid truncating "copy", and instead tries to
    // truncate the old name.
    auto name{data::context(name_)};
    const auto formatStr{_("%s copy")};

    const auto newLen{std::min(
        // The - 2 accounts for the %s
        MAX_NAME_LEN - formatStr.length() - 2,
        name.val().length()
    )};

    name.change(wxString::Format(
        formatStr,
        name.val().substr(0, newLen)
    ).ToStdString());
}

auto Preset::children() const -> std::vector<const Model *> {
    return {
        &name_,
        &fontDir_,
        &track_,
        &styles_,
    };
}

