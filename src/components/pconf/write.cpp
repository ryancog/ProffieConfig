#include "write.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
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

#include "log/logger.h"

#include "pconf.h"

namespace PConf {

bool writeEntry(std::ostream&, const std::shared_ptr<Entry>& entry, int32 depth, Log::Branch&);
bool writeSection(std::ostream&, const std::shared_ptr<Section>& section, int32 depth, Log::Branch&);
std::ostream& writeWithDepth(std::ostream& outStream, int32_t depth);

}

void PConf::write(std::ostream& outStream, const Data& pconfData, Log::Branch *lBranch) {
    auto& logger{Log::Branch::optCreateLogger("PConf::write()", lBranch)};

    for (const auto& entry : pconfData) {
        switch (entry->getType()) {
            case Type::ENTRY:
                writeEntry(outStream, entry, 0, *logger.binfo("Writing entry: " + entry->name));
                break;
            case Type::SECTION:
                writeSection(outStream, std::static_pointer_cast<Section>(entry), 0, *logger.binfo("Writing Section: " + entry->name));
                break;
        }
    }
}


bool PConf::writeEntry(std::ostream& outStream, const std::shared_ptr<Entry>& entry, int32_t depth, Log::Branch&) {
    // auto& logger{lBranch.createLogger("PConf::writeEntry()")};
    
    writeWithDepth(outStream, depth) << entry->name;
    if (entry->label) outStream << "(\"" << entry->label.value() << "\")";
    if (entry->labelNum) outStream << '{' << entry->labelNum.value() << '}';

    if (not entry->value) {
        outStream << '\n';
        return true;
    }

    const auto& valueStr{entry->value.value()};
    outStream << ": ";

    size_t lineBegin{0};
    size_t lineEnd{valueStr.find('\n')};
    bool containsQuotes{valueStr.find('"') != string::npos};
    if (lineEnd != string::npos or containsQuotes) {
        outStream << "{\n";
        writeWithDepth(outStream, depth + 1);
    }

    while (lineEnd != string::npos) {
        outStream << '"' << valueStr.substr(lineBegin, lineEnd - lineBegin) << "\"\n";
        writeWithDepth(outStream, depth + 1);
        lineBegin = lineEnd + 1;
        lineEnd = valueStr.find('\n', lineBegin);
    }

    outStream << '"' << valueStr.substr(lineBegin, lineEnd - lineBegin) << "\"\n";
    if (lineBegin != 0 or containsQuotes) writeWithDepth(outStream, depth) << "}\n";

    return true;
}

bool PConf::writeSection(std::ostream& outStream, const std::shared_ptr<Section>& section, int32_t depth, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PConf::writeSection()")};

    writeWithDepth(outStream, depth) << section->name;
    if (section->label) outStream << "(\"" << section->label.value() << "\")";
    if (section->labelNum) outStream << '{' << section->labelNum.value() << '}';
    outStream << " {\n";

    for (const auto& entry : section->entries) {
        switch (entry->getType()) {
            case Type::ENTRY:
                writeEntry(outStream, entry, depth + 1, *logger.bverbose("Writing entry: " + entry->name));
                break;
            case Type::SECTION:
                writeSection(outStream, std::static_pointer_cast<Section>(entry), depth + 1, *logger.bverbose("Writing section: " + entry->name));
                break;
        }
    }

    writeWithDepth(outStream, depth) << "}\n";

    return true;
}

std::ostream& PConf::writeWithDepth(std::ostream& outStream, int32_t depth) {
    for (int32_t i{0}; i < depth; i++) outStream << '\t';
    return outStream;
}

