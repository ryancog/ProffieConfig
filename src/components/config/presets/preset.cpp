#include "preset.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/config/presets/preset.hpp
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

#include "utils/string.hpp"

using namespace config::presets;

namespace {

constexpr std::string_view NAME_STR{"Name"};
constexpr std::string_view FONTDIR_STR{"FontDir"};
constexpr std::string_view TRACK_STR{"Track"};
constexpr std::string_view STYLES_STR{"Styles"};

constexpr std::string_view COMMENT_STR{"Comment"};
constexpr std::string_view CONTENT_STR{"Content"};

constexpr size MAX_NAME_LEN{24};

} // namespace

Preset::Preset(data::Node *parent) :
    data::Node(parent),
    name_(this),
    fontDir_(this),
    track_(this),
    styles_(this) {
    CreationScope createScope{*this};

    const auto nameFilter{[](
        const data::String::ROContext&, std::string& str, size& pos
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
        const data::String::ROContext&, std::string& str, size& pos
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

    data::String::Context{name_}.change("newpreset", 0);
}

Preset::Preset(const Preset& other, data::Node *parent) :
    data::Node(parent),
    name_(other.name_, this),
    fontDir_(other.fontDir_, this),
    track_(other.track_, this),
    styles_(other.styles_, this) {
    CreationScope createScope{*this};

    // The extra work here avoid truncating "copy", and instead tries to
    // truncate the old name.
    data::String::Context name{name_};
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

Preset::~Preset() = default;

bool Preset::enumerate(const EnumFunc& func) {
    if (func(name_, strID(NAME_STR), NAME_STR)) return true;
    if (func(fontDir_, strID(FONTDIR_STR), FONTDIR_STR)) return true;
    if (func(track_, strID(TRACK_STR), TRACK_STR)) return true;
    if (func(styles_, strID(STYLES_STR), STYLES_STR)) return true;
    return false;
}

data::Model *Preset::find(uint64 id) {
    if (id == strID(NAME_STR)) return &name_;
    if (id == strID(FONTDIR_STR)) return &fontDir_;
    if (id == strID(TRACK_STR)) return &track_;
    if (id == strID(STYLES_STR)) return &styles_;
    return nullptr;
}

std::unique_ptr<data::Model> Preset::clone(Node *parent) const {
    return std::make_unique<Preset>(*this, parent);
}

Style::Style(Node *parent) :
    data::Node(parent),
    comment_(this),
    content_(this) {
    CreationScope createScope{*this};

    const auto commentFilter{[](
        const data::String::ROContext&, std::string& str, size& pos
    ) {
        size_t illegalPos{};
        while (
                (illegalPos = str.find("/*")) != std::string::npos or
                (illegalPos = str.find("*/")) != std::string::npos
              ) {
            if (illegalPos < pos) pos -= std::max<size>(2, pos - illegalPos);
            str.erase(illegalPos, 2);
        }
    }};
    comment_.setFilter(commentFilter);

    const auto styleFilter{[] (
        const data::String::ROContext& ctxt, std::string& str, size& pos
    ) {
        // TODO: The replication in here is kind of ugly...
        auto& style{*ctxt.model().parent<Style>()};

        uint32 numTrimmed{};

        /*
         * - Only allow chars for the start of a block comment. No other need 
         *   for backslash
         *
         * - <>(), are self-explanatory
         *
         * - & for global style objects like the charging style.
         *
         * - : for scope resolution operator (there's scoped enums with effects)
         *
         * - "" may be used for the dynamic args defaults.
         *
         * - \n\t and ' ' are self-explanatory.
         *
         * - '-' for negative numbers.
         *
         * - '_' because it's just generally used.
         */
        utils::trim(
            str,
            {
                .allowAlpha=true,
                .allowNum=true,
                .safeList="/*<>(),&_:-\"\n\t "
            },
            &numTrimmed,
            pos
        );
        pos -= numTrimmed;

        data::String::Context comment{style.comment_};
        const auto addToComment{[&](std::string& addStr) {
            if (not comment.val().empty()) {
                addStr.insert(addStr.begin(), '\n');
            }

            auto commentStr{comment.val()};
            commentStr.append(addStr);

            const auto newPos{commentStr.length()};
            comment.change(std::move(commentStr), newPos);
        }};

        size_t illegalPos{0};
        bool commentMove{false};
        while (
                (illegalPos = str.find("/*", illegalPos)) 
                != std::string::npos
              ) {
            const auto terminatorPos{str.find("*/", illegalPos)};
            const auto eraseEnd{terminatorPos == std::string::npos
                ? std::string::npos 
                : terminatorPos + 2
            };

            if (eraseEnd < pos) {
                pos -= eraseEnd - illegalPos;
            } else if (illegalPos < pos) {
                pos = illegalPos;
            }

            const auto begin{illegalPos + 2};
            auto substr{str.substr(begin, terminatorPos - begin)};
            utils::trimSurroundingWhitespace(substr);
            if (not substr.empty()) {
                addToComment(substr);
            }

            str.erase(illegalPos, eraseEnd - illegalPos);
            commentMove = true;
        }

        // If comment terminator but no opener, move everything before
        // terminator into comment
        if (
                (illegalPos = str.rfind("*/")) != std::string::npos and 
                str.find("/*") == std::string::npos
           ) {
            auto substr{str.substr(0, illegalPos)};
            utils::trimSurroundingWhitespace(substr);
            if (not substr.empty()) {
                addToComment(substr);
            }
            const auto eraseEnd{illegalPos + 2};
            if (eraseEnd < pos) pos -= eraseEnd;
            else pos = 0;
            str.erase(0, eraseEnd);
        }

        illegalPos = 0;
        while (
                (illegalPos = str.find("//", illegalPos))
                != std::string::npos
              ) {
            const auto terminatorPos{str.find('\n', illegalPos)};
            const auto eraseEnd{terminatorPos == std::string::npos
                ? std::string::npos 
                : terminatorPos + 1
            };

            if (eraseEnd < pos) {
                pos -= eraseEnd - illegalPos;
            } else if (illegalPos < pos) {
                pos = illegalPos;
            }

            const auto begin{illegalPos + 2};
            auto substr{str.substr(begin, terminatorPos - begin)};
            utils::trimSurroundingWhitespace(substr);
            if (not substr.empty()) {
                addToComment(substr);
            }

            str.erase(illegalPos, eraseEnd - illegalPos);
            commentMove = true;
        }

        if ((illegalPos = str.find(')')) != std::string::npos) {
            str.erase(illegalPos + 1);
            pos = std::min<size_t>(pos, illegalPos + 1);
        }

        if (commentMove) comment.focus();
    }};
    content_.setFilter(styleFilter);

    data::String::Context{comment_}.change(
        "ProffieConfig Default Blue AudioFlicker", 0
    );
    data::String::Context{content_}.change(
        "StyleNormalPtr<AudioFlicker<Blue,DodgerBlue>,BLUE,300,800>()", 0
    );
}

Style::Style(const Style& other, data::Node *parent) :
    data::Node(parent),
    comment_(other.comment_, this),
    content_(other.content_, this) {}

bool Style::enumerate(const EnumFunc& func) {
    if (func(comment_, strID(COMMENT_STR), COMMENT_STR)) return true;
    if (func(content_, strID(CONTENT_STR), CONTENT_STR)) return true;
    return false;
}

data::Model *Style::find(uint64 id) {
    if (id == strID(COMMENT_STR)) return &comment_;
    if (id == strID(CONTENT_STR)) return &content_;
    return nullptr;
}

std::unique_ptr<data::Model> Style::clone(Node *parent) const {
    return std::make_unique<Style>(*this, parent);
}

