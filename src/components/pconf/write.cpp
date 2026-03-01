#include "write.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/pconf/write.cpp
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

#include "log/logger.hpp"

namespace {

bool writeEntry(
    std::ostream&,
    const pconf::EntryPtr& entry,
    int32 depth,
    logging::Branch&
);
bool writeSection(
    std::ostream&,
    const pconf::SectionPtr& section,
    int32 depth,
    logging::Branch&
);
std::ostream& writeWithDepth(std::ostream& outStream, int32 depth);

} // namespace

void pconf::write(
    std::ostream& outStream, const Data& pconfData, logging::Branch *lBranch
) {
    auto& logger{logging::Branch::optCreateLogger("pconf::write()", lBranch)};

    for (const auto& entry : pconfData) {
        if (entry.section()) {
            writeSection(
                outStream,
                entry.section(),
                0,
                *logger.bverbose("Writing Section: " + entry->name_)
            );
        } else {
            writeEntry(
                outStream,
                entry,
                0,
                *logger.bverbose("Writing entry: " + entry->name_)
            );
        }
    }
}

namespace {

bool writeEntry(
    std::ostream& outStream,
    const pconf::EntryPtr& entry,
    int32 depth,
    logging::Branch&
) {
    // auto& logger{lBranch.createLogger("pconf::writeEntry()")};
    
    writeWithDepth(outStream, depth) << entry->name_;
    if (entry->label_) outStream << "(\"" << entry->label_.value() << "\")";
    if (entry->labelNum_) outStream << '{' << entry->labelNum_.value() << '}';

    if (not entry->value_) {
        outStream << '\n';
        return true;
    }

    const auto& valueStr{entry->value_.value()};
    outStream << ": ";

    size_t lineBegin{0};
    size_t lineEnd{valueStr.find('\n')};
    bool containsQuotes{valueStr.find('"') != std::string::npos};
    if (lineEnd != std::string::npos or containsQuotes) {
        outStream << "{\n";
        writeWithDepth(outStream, depth + 1);
    }

    while (lineEnd != std::string::npos) {
        outStream << '"' << valueStr.substr(lineBegin, lineEnd - lineBegin) << "\"\n";
        writeWithDepth(outStream, depth + 1);
        lineBegin = lineEnd + 1;
        lineEnd = valueStr.find('\n', lineBegin);
    }

    outStream << '"' << valueStr.substr(lineBegin, lineEnd - lineBegin) << "\"\n";
    if (lineBegin != 0 or containsQuotes) writeWithDepth(outStream, depth) << "}\n";

    return true;
}

bool writeSection(
    std::ostream& outStream,
    const pconf::SectionPtr& section,
    int32 depth,
    logging::Branch& lBranch
) {
    auto& logger{lBranch.createLogger("pconf::writeSection()")};

    writeWithDepth(outStream, depth) << section->name_;
    if (section->label_) outStream << "(\"" << section->label_.value() << "\")";
    if (section->labelNum_) outStream << '{' << section->labelNum_.value() << '}';
    outStream << " {\n";

    for (const auto& entry : section->entries_) {
        if (entry.section()) {
            writeSection(
                outStream,
                entry.section(),
                depth + 1,
                *logger.binfo("Writing Section: " + entry->name_)
            );
        } else {
            writeEntry(
                outStream,
                entry,
                depth + 1,
                *logger.binfo("Writing entry: " + entry->name_)
            );
        }
    }

    writeWithDepth(outStream, depth) << "}\n";

    return true;
}

std::ostream& writeWithDepth(std::ostream& outStream, int32 depth) {
    for (auto i{0}; i < depth; i++) outStream << '\t';
    return outStream;
}

} // namespace

