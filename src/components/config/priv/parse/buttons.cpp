#include "buttons.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/parse/buttons.cpp
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

#include <sstream>

#include "config/buttons/button.hpp"
#include "config/priv/io.hpp"
#include "config/strings.hpp"
#include "data/context.hpp"
#include "utils/string.hpp"

std::optional<std::string> config::priv::parse::buttons(
    const std::string& buf, Config& config, logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("config::parseButtons()")};
    std::istringstream stream(buf);

    enum {
        eNone,
        eType,
        ePost_Type,
        eCpp_Name,
        ePost_Cpp_Name,
        eInner,
        ePost_Inner,
    } reading{eNone};

    std::string typeStr;
    std::string inner;
    char openChar{};

    while (stream.good()) {
        utils::CommentData commentData{
            .stream_=stream,
            .skipNewlines_=false,
            .skipSpaces_=false,
        };
        if (utils::extractComments(commentData)) {
            if (reading == eType) reading = ePost_Type;
            if (reading == eCpp_Name) reading = ePost_Cpp_Name;
        }

        const auto chr{stream.get()};
        if (not stream.good())
            return {};

        if (reading == eNone) {
            if (std::isgraph(chr)) {
                reading = eType;
                typeStr = static_cast<char>(chr);
                continue;
            }
        } else if (reading == eType) {
            if (std::isspace(chr)) {
                reading = ePost_Type;
                continue;
            }

            typeStr += static_cast<char>(chr);
        } else if (reading == ePost_Type) {
            if (std::isgraph(chr)) {
                reading = eCpp_Name;
                continue;
            }
        } else if (reading == eCpp_Name or reading == ePost_Cpp_Name) {
            if (chr == '{' or chr == '(') {
                openChar = static_cast<char>(chr);
                reading = eInner;
                continue;
            }

            if (reading == eCpp_Name) {
                if (std::isspace(chr)) reading = ePost_Cpp_Name;
            } else { // POST_CPP_NAME
                if (std::isspace(chr)) continue;

                // Not a space and not an opener:
                return errorMessage(logger, wxTRANSLATE("Unexpected character %#x post cpp name."), chr);
            }
        } else if (reading == eInner) {
            if (
                    openChar == '{' and chr == '}' or 
                    openChar == '(' and chr == ')'
               ) {
                reading = ePost_Inner;
                continue;
            }

            if (std::isgraph(chr)) {
                inner += static_cast<char>(chr);
            }
        } else if (reading == ePost_Inner) {
            if (std::isspace(chr)) continue;
            if (chr != ';') {
                return errorMessage(logger, wxTRANSLATE("Unexpected character %#x post inner (prior to ;)."), chr);
            }

            reading = eNone;

            auto buttons{data::context(config.buttons_)};
            auto& button{buttons.append<buttons::Button>(config)};

            int32 typeIdx{0};
            for (; typeIdx < BUTTON_TYPE_STRS.size(); ++typeIdx) {
                if (BUTTON_TYPE_STRS[typeIdx] == typeStr) break;
            }
            if (typeIdx == BUTTON_TYPE_STRS.size()) {
                return errorMessage(logger, wxTRANSLATE("Unknown button type: %s"), typeStr);
            } 

            button.type_.choose(typeIdx);

            auto eventCommaPos{inner.find(',')};
            if (eventCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("Missing comma for button event."));
            }

            auto eventStr{inner.substr(0, eventCommaPos)};
            inner.erase(0, eventCommaPos + 1);

            int32 evtIdx{-1};
            if (eventStr == "BUTTON_FIRE") evtIdx = eBtn_Evt_Up;
            else if (eventStr == "BUTTON_MODE_SELECT") evtIdx = eBtn_Evt_Down;
            else if (eventStr == "BUTTON_CLIP_DETECT") evtIdx = eBtn_Evt_Left;
            else if (eventStr == "BUTTON_RELOAD") evtIdx = eBtn_Evt_Right;
            else if (eventStr == "BUTTON_RANGE") evtIdx = eBtn_Evt_Select;

            if (evtIdx == -1) {
                evtIdx = 0;
                for (; evtIdx < BUTTON_EVENT_STRS.size(); ++evtIdx) {
                    if (BUTTON_EVENT_STRS[evtIdx] == eventStr) break;
                }
            }

            if (evtIdx == BUTTON_EVENT_STRS.size()) {
                logger.warn("Unknown button event: " + eventStr);
                button.event_.choose(0);
            } else {
                button.event_.choose(evtIdx);
            }

            auto pinCommaPos{inner.find(',')};
            if (pinCommaPos == std::string::npos) {
                return errorMessage(logger, wxTRANSLATE("Missing comma for button pin."));
            }

            auto pinStr{inner.substr(0, pinCommaPos)};
            inner.erase(0, pinCommaPos + 1);
            button.pin_.change(std::move(pinStr));

            if (typeIdx == eBtn_Type_Touch) {
                auto threshCommaPos{inner.find(',')};
                if (threshCommaPos == std::string::npos) {
                    return errorMessage(logger, wxTRANSLATE("Missing comma for touch button threshold."));
                }

                auto threshStr{inner.substr(0, threshCommaPos)};
                inner.erase(0, threshCommaPos + 1);

                auto thresh{utils::doStringMath(threshStr)};
                if (not thresh) {
                    logger.warn("Button threshold value \"" + threshStr + "\" could not be parsed.");
                } else {
                    button.touch_.set(static_cast<int32>(*thresh));
                }
            }

            if (inner.front() != '"' or inner.back() != '"') {
                logger.warn("Button name doesn't seem to be surrounded by quotes, is it malformatted?");
                button.name_.change(std::move(inner));
            } else {
                button.name_.change(inner.substr(1, inner.length() - 2));
            }

            inner.erase();
            continue;
        }
    }

    return std::nullopt;
}

