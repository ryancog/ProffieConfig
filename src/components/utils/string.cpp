#include "string.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/utils/string.h
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

#include <cctype>

namespace Utils {

}

string Utils::extractComment(std::istream& stream) {
    enum {
        NONE,
        LINE_COMMENT,
        LONG_COMMENT,
    } reading{NONE};

    string ret;
    string comment;
    while (stream.good()) {
        const auto chr{stream.get()};
        if (chr < 0 or chr > 0xFF) continue;
        if (
                std::iscntrl(chr) and
                chr != '\n' and
                chr != '\t'
           ) continue;

        if (reading == NONE) {
            if (chr == '/') {
                if (stream.peek() == '*') {
                    reading = LONG_COMMENT;
                    comment.clear();
                    stream.get();
                } else if (stream.peek() == '/') {
                    reading = LINE_COMMENT;
                    comment.clear();
                    stream.get();
                }
            } else if (std::isgraph(chr)) {
                stream.unget();
                break;
            }
        } else if (reading == LINE_COMMENT) {
            if (chr == '/' and comment.empty()) continue;
            if (chr == '\n') {
                if (not ret.empty()) ret += '\n';
                trimSurroundingWhitespace(comment);
                ret += comment;
                reading = NONE;
                continue;
            }
            comment += static_cast<char>(chr);
        } else if (reading == LONG_COMMENT) {
            bool end{chr == '*' and stream.peek() == '/'};
            if (end or chr == '\n') {
                if (not ret.empty()) ret += '\n';
                trimSurroundingWhitespace(comment);
                while (not comment.empty() and comment.front() == '*') {
                    comment.erase(0, 1);
                }
                trimSurroundingWhitespace(comment);
                ret += comment;

                if (end) {
                    reading = NONE;
                    stream.get();
                }
                continue;
            }
            comment += static_cast<char>(chr);
        }
    }

    return ret;
}

bool Utils::skipComment(std::istream& stream, string *str) {
    enum {
        NONE,
        LINE_COMMENT,
        LONG_COMMENT,
    } reading{NONE};

    bool skipped{false};
    while (stream.good()) {
        const auto chr{stream.get()};

        if (reading == NONE) {
            if (chr == '/') {
                if (stream.peek() == '*') {
                    reading = LONG_COMMENT;
                    stream.get();
                } else if (stream.peek() == '/') {
                    reading = LINE_COMMENT;
                    stream.get();
                }
            } else if (std::isgraph(chr)) {
                stream.unget();
                break;
            }
        } else if (reading == LINE_COMMENT) {
            if (chr == '\n') {
                reading = NONE;
            }
        } else if (reading == LONG_COMMENT) {
            if (chr == '*' and stream.peek() == '/') {
                reading = NONE;
                stream.get();
            }
        }

        if (str) {
            skipped = true;
            *str += chr;
        }
    }

    return skipped;
}

void Utils::trimSurroundingWhitespace(string& str) {
    while (not str.empty() and std::isspace(str.front())) str.erase(0, 1);
    while (not str.empty() and std::isspace(str.back())) str.pop_back();
}

vector<string> Utils::createEntries(const std::vector<wxString>& vec) {
    vector<string> entries;
    entries.reserve(vec.size());
    for (const auto& entry : vec) {
        entries.emplace_back(entry.ToStdString());
    }
    return entries;
}


vector<string> Utils::createEntries(const std::initializer_list<wxString>& list) {
    return Utils::createEntries(static_cast<std::vector<wxString>>(list));
}

