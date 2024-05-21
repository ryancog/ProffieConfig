#include "parse.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * styles/parse.cpp
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

#include "elements/lockuptype.h"
#include "log/logger.h"

using namespace BladeStyles;

struct TokenizedStyle {
    std::string name;
    std::vector<std::string> args;
};
static std::optional<TokenizedStyle> tokenizeStyle(const std::string&);

static std::string typeToString(uint32_t);

BladeStyle* BladeStyles::parseString(const std::string& styleStr, bool* foundStyle) {
    auto styleTokens{tokenizeStyle(styleStr)};
    if (!styleTokens) return nullptr;

    if (foundStyle) *foundStyle = false;

    auto styleGen{get(styleTokens->name)};
    if (!styleGen) {
        Logger::error("Style not recognized: " + styleTokens->name);
        return nullptr;
    } else if (foundStyle) *foundStyle = true;

    auto style{styleGen(nullptr, {})};

    return style;
}

std::optional<std::string> BladeStyles::asString(BladeStyle& style) {
    std::string ret;
    return ret;
}

static std::optional<TokenizedStyle> tokenizeStyle(const std::string& styleStr) {
    auto styleBegin{styleStr.find_first_not_of(" &\t\n")};
    if (styleBegin == std::string::npos) {
        Logger::warn("Could not tokenize style!", false);
        return std::nullopt;
    }
    auto nameEnd{styleStr.find_first_of("<(", styleBegin)};
    auto styleName{styleStr.substr(styleBegin, nameEnd - styleBegin)};

    TokenizedStyle styleTokens;
    styleTokens.name = styleName;

    if (nameEnd > styleStr.length() || styleStr.at(nameEnd) != '<') return styleTokens;

    auto argSubStr{styleStr.substr(nameEnd + 1)};
    std::string buf;
    int32_t depth{1};
    char c;
    for (size_t i = 0; i < argSubStr.size(); i++) {
        c = argSubStr.at(i);
        if (c == ' ') continue;
        if (c == '\n') continue;
        if (c == '\t') continue;
        if (c == '\r') continue;

        if (c == '<') depth++;
        else if (c == '>') {
            depth--;
            if (depth < 0) {
                Logger::warn("Error parsing arguments for style: " + styleName);
                return std::nullopt;
            }
            if (depth == 0) {
                styleTokens.args.push_back(buf);
                break;
            }
        }

        else if (c == ',' && depth == 1) {
            styleTokens.args.push_back(buf);
            buf.clear();
            continue;
        }

        buf += c;
    }
    if (depth != 0) {
        Logger::warn("Mismatched <> in style: " + styleName);
        return std::nullopt;
    }

    return styleTokens;
}

static std::string typeToString(uint32_t type) {
    switch (type & FLAGMASK) {
    case WRAPPER:
        return "Wrapper";
    case BUILTIN:
        return "BuiltIn";
    case FUNCTION:
        return "Int";
    case BITS:
        return "Bits";
    case BOOL:
        return "Boolean";
    case COLOR:
        return "Color";
    case LAYER:
        return "Layer (Transparent Color)";
    case EFFECT:
        return "Effect";
    case LOCKUPTYPE:
        return "LockupType";
    }

    return "INVALID";
}
