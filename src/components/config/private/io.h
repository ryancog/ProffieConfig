#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/config/private/io.h
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

#include <wx/translation.h>

#include "utils/types.h"

#include "../config.h"

namespace Config {

/**
 * Output a config to header on disk
 *
 * @return Error message on failure. nullopt on success
 */
optional<string> output(const filepath&, const Config&, Log::Branch *lBranch = nullptr);

/**
 * Parse a config from disk
 *
 * @return Error message on failure. nullopt on success.
 */
optional<string> parse(const filepath&, Config&, Log::Branch *lBranch = nullptr);

constexpr string_view INJECTION_STR{"injection"};
constexpr string_view PC_OPT_STR{"//PROFFIECONFIG "};
constexpr cstring MAX_LEDS_STR{"const unsigned int maxLedsPerStrip = "};
constexpr string_view DEFINE_STR{"#define "};
constexpr string_view INCLUDE_STR{"#include "};
constexpr string_view POWER_PINS_STR{"PowerPINS<"};

namespace Private {

template<typename T>
    requires (
        std::is_same_v<std::decay_t<T>, string> or
        std::is_same_v<std::decay_t<T>, wxString> or
        std::is_same_v<std::decay_t<T>, cstring>
    )
wxString maybeTranslate(const T& str) {
    return wxGetTranslation(str);
}

template<typename T>
T maybeTranslate(T&& val) {
    return std::forward<T>(val);
}

} // namespace Private

template<typename ...ARGS>
string errorMessage(Log::Logger& logger, const wxString& msg, ARGS&&... args) {
    logger.error(wxString::Format(msg, args...).ToStdString());
    return wxString::Format(
        wxGetTranslation(msg),
        Private::maybeTranslate(std::forward<ARGS>(args))...
    ).ToStdString();
}

} // namespace Config

