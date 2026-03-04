#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/config/priv/io.hpp
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

#include <string>
#include <string_view>

#include <wx/translation.h>

#include "log/logger.hpp"
#include "utils/types.hpp"

namespace config::priv {

constexpr std::string_view INJECTION_STR{"injection"};
constexpr std::string_view PC_OPT_STR{"//PROFFIECONFIG "};
constexpr std::string_view PC_OPT_NOCOMMENT_STR{"PROFFIECONFIG "};
constexpr std::string_view DEFINE_STR{"#define "};
constexpr std::string_view INCLUDE_STR{"#include "};
constexpr std::string_view POWER_PINS_STR{"PowerPINS<"};
constexpr cstring MAX_LEDS_STR{"const unsigned int maxLedsPerStrip = "};

template<typename T>
    requires (
        std::is_same_v<std::decay_t<T>, std::string> or
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

template<typename ...ARGS>
std::string errorMessage(
    logging::Logger& logger, const wxString& msg, ARGS&&... args
) {
    logger.error(wxString::Format(msg, args...).ToStdString());
    return wxString::Format(
        wxGetTranslation(msg),
        maybeTranslate(std::forward<ARGS>(args))...
    ).ToStdString();
}

} // namespace config::priv

