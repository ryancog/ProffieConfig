#include "trace.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/utils/trace.cpp
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

Trace::FilePos Trace::getFilePos(const string& fileBuffer, const uint64 index) {
    FilePos ret{};
    if (index > fileBuffer.size() - 1) return ret;
    uint64 newLinePos{};
    for (uint64 searchIdx{0}; searchIdx <= index; searchIdx++) {
        char chr{fileBuffer.at(searchIdx)};
        if (chr == '\n') {
            newLinePos = searchIdx;
            ret.pos = 0;
            ret.line++;
            continue;
        }

        ret.pos++;
    }
    if (newLinePos != fileBuffer.size() - 1) {
        auto lineStart{fileBuffer.find_first_not_of(" \t", newLinePos + 1)};
        if (lineStart != string::npos) ret.fullLine = fileBuffer.substr(lineStart, fileBuffer.find_first_of("\r\n", newLinePos + 1) - lineStart);
    }
    return ret;
}

Trace::FilePos::operator string() const {
    return "At line " + std::to_string(line + 1) + ", col " + std::to_string(pos + 1) + ": " + fullLine;
}


