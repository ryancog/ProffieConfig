#include "read.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/pconf/read.cpp
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

#include <charconv>
#include <iostream>
#include <stack>
#include <variant>

#include "log/logger.hpp"
#include "utils/string.hpp"

bool pconf::read(
    std::istream& inStream,
    Data& out,
    logging::Branch *lBranch
) {
    auto& logger{logging::Branch::optCreateLogger("pconf::read()", lBranch)};

    enum class LineType {
        Entry,
        Multiline,
    } type{LineType::Entry};

    std::stack<std::vector<EntryPtr> *> stack;

    out.clear();
    stack.push(&out);

    std::string line;
    for (size lineNo{0}; std::getline(inStream, line); ++lineNo) {
        // Handle comments
        [&] {
            bool inQuote{false};
            bool lastWasSlash{false};
            for (size idx{0}; idx < line.size(); ++idx) {
                auto chr{line[idx]};

                if (not inQuote and chr == '/') {
                    if (lastWasSlash) {
                        // Trim everything on line after "//"
                        line.erase(idx - 1);
                        break;
                    }

                    lastWasSlash = true;
                    continue;
                }

                lastWasSlash = false;

                if (chr == '"')
                    inQuote = not inQuote;
            }
        }();

        utils::trimSurroundingWhitespace(line);
        if (line.empty())
            continue;

        const auto expectMsg{[lineNo](
            std::string_view expect,
            std::variant<std::monostate, std::string_view, char> got = {}
        ) {
            std::string ret;
            ret += "Expected ";
            ret += expect;

            if (not std::holds_alternative<std::monostate>(got)) {
                ret += " but got ";

                if (auto *ptr{std::get_if<std::string_view>(&got)}) {
                    ret += *ptr;
                } else {
                    ret += '\'';
                    ret += std::get<char>(got);
                    ret += '\'';
                }
            }

            ret += " on line ";
            ret += std::to_string(lineNo);
            return ret;
        }};

        const auto earlyMsg{[lineNo](
            std::string_view when
        ) {
            std::string ret;
            ret += "Unexpected line end when ";
            ret += when;
            ret += " on line ";
            ret += std::to_string(lineNo);
            return ret;
        }};

        if (line.front() == '}') {
            switch (type) {
                case LineType::Entry:
                    // End of section
                    if (stack.size() == 1) {
                        logger.error(expectMsg("entry", "section-end"));
                        return false;
                    }

                    stack.pop();
                    break;
                case LineType::Multiline:
                    type = LineType::Entry;
                    break;
            }

            continue;
        }

        if (type == LineType::Multiline) {
            if (line.front() != '"') {
                logger.error(expectMsg("multiline start quote", line.front()));
                return false;
            }

            if (line.size() < 2) {
                logger.error(earlyMsg("parsing multiline"));
                return false;
            }

            if (line.back() != '"') {
                logger.error(expectMsg("multiline end quote", line.back()));
                return false;
            }

            auto& value{*stack.top()->back()->value_};

            if (not value.empty())
                value.push_back('\n');

            value.append(std::string_view(
                std::next(line.begin()),
                std::prev(line.end())
            ));

            continue;
        }

        enum class Reading {
            Name,
            Label_Start,
            Label,
            Label_End,
            NLabel,
            Post_Label,
            Value,
            Value_Unquoted,
            Value_In,
            Value_Out,
        } reading{Reading::Name};

        auto mark{line.begin()};
        const auto mkEnt{[&](decltype(line.begin()) end) {
            stack.top()->push_back(Entry::create(std::string(mark, end)));
            utils::trimSurroundingWhitespace(stack.top()->back()->name_);
        }};

        const auto convToSect{[&]() {
            auto& back{stack.top()->back()};

            back = Section::create(
                std::move(back->name_),
                std::move(back->label_),
                back->labelNum_
            );

            stack.push(&back.section()->entries_);
        }};

        for (auto iter{line.begin()}; iter != line.end(); ++iter) {
            switch (reading) {
                case Reading::Name:
                    if (*iter == '{') {
                        mkEnt(iter);

                        if (std::next(iter) == line.end()) {
                            convToSect();

                            // Make sure line-end handler below doesn't
                            // duplicate the entry creation.
                            reading = Reading::Value;

                            // Loop will exit on continue
                            continue;
                        }

                        mark = std::next(iter);
                        reading = Reading::NLabel;
                        continue;
                    }

                    if (*iter == '(') {
                        mkEnt(iter);
                        reading = Reading::Label_Start;
                        continue;
                    }

                    if (*iter == ':') {
                        mkEnt(iter);

                        // See comment in Post_Label case
                        stack.top()->back()->value_.emplace();
                        mark = std::next(iter);
                        reading = Reading::Value;
                        continue;
                    }
                    break;
                case Reading::Label_Start:
                    if (std::isspace(*iter))
                        continue;

                    if (*iter != '"') {
                        logger.error(expectMsg("label start quote", *iter));
                        return false;
                    }

                    mark = std::next(iter);
                    reading = Reading::Label;
                    break;
                case Reading::Label:
                    if (*iter == '"') {
                        reading = Reading::Label_End;
                        stack.top()->back()->label_ = {mark, iter};
                    }

                    break;
                case Reading::Label_End:
                    if (std::isspace(*iter))
                        continue;

                    if (*iter != ')') {
                        logger.error(expectMsg("label end paren", *iter));
                        return false;
                    }

                    reading = Reading::Post_Label;
                    break;
                case Reading::NLabel:
                    if (std::isdigit(*iter))
                        continue;

                    if (*iter == '}') {
                        auto res{std::from_chars(
                            mark.base(),
                            iter.base(),
                            stack.top()->back()->labelNum_.emplace()
                        )};

                        if (res.ec != std::errc{}) {
                            logger.error("Failed to parse label num (" + std::to_string(static_cast<size>(res.ec)) + ") on line " + std::to_string(lineNo));
                            return false;
                        }

                        reading = Reading::Post_Label;
                        continue;
                    }

                    logger.error(expectMsg("decimal digit for num label", *iter));
                    return false;
                case Reading::Post_Label:
                    if (std::isspace(*iter))
                        continue;

                    if (*iter == '{') {
                        if (std::next(iter) != line.end()) {
                            logger.error(expectMsg("section-start to be at line end"));
                            return false;
                        }

                        convToSect();
                        // Loop will exit on continue
                        continue;
                    }

                    if (*iter != ':') {
                        logger.error(expectMsg("value-separator", *iter));
                        return false;
                    }

                    // The presence of `:` means the value should be considered
                    // present, even if empty (and both single- and multi-line
                    // parsing relies on this).
                    stack.top()->back()->value_.emplace();
                    // Setup mark for unquoted value
                    mark = std::next(iter);
                    reading = Reading::Value;
                    break;
                case Reading::Value:
                    if (*iter == '{') {
                        if (std::next(iter) != line.end()) {
                            logger.error(expectMsg("multiline-start to be at line end"));
                            return false;
                        }

                        // The loop will exit on continue, and next reading
                        // will be for multiline value.
                        type = LineType::Multiline;
                        continue;
                    }

                    [[fallthrough]];
                case Reading::Value_Out:
                    if (std::isspace(*iter) or *iter == ',')
                        continue;

                    if (*iter == '"') {
                        reading = Reading::Value_In;
                        mark = std::next(iter);
                        continue;
                    }

                    if (reading != Reading::Value) {
                        logger.error(expectMsg("value start quote", *iter));
                        return false;
                    }

                    [[fallthrough]];
                case Reading::Value_Unquoted:
                    // This could move iter to the end and break early I guess,
                    // or maybe have some sort of checking for a valid subset
                    // of characters, but just leave it here and be permissive
                    // for now.
                    reading = Reading::Value_Unquoted;
                    break;
                case Reading::Value_In:
                    if (*iter == '"') {
                        auto& value{*stack.top()->back()->value_};

                        if (not value.empty())
                            value.push_back('\n');

                        value.append(std::string_view(mark, iter));
                        reading = Reading::Value_Out;
                    }
                    break;
            }
        }

        switch (reading) {
            case Reading::Name:
                // The line was already checked to not be empty.
                mkEnt(line.end());
                break;
            case Reading::Label_Start:
                logger.error(expectMsg("label start quote", "line end"));
                return false;
            case Reading::Label:
                logger.error(expectMsg("label end quote", "line end"));
                return false;
            case Reading::Label_End:
                logger.error(expectMsg("label end paren", "line end"));
                return false;
            case Reading::NLabel:
                logger.error(expectMsg("num label end brace", "line end"));
                return false;
            case Reading::Value_In:
                logger.error(expectMsg("value end quote", "line end"));
                return false;
            case Reading::Post_Label:
            case Reading::Value:
            case Reading::Value_Out:
                // All of these are find to end on, and ent has already been
                // made, so nothing more to do.
                break;
            case Reading::Value_Unquoted:
                stack.top()->back()->value_ = std::string(mark, line.end());
                break;
        }
    }

    return true;
}

