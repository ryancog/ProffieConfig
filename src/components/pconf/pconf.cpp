#include "pconf.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/pconf/private/pconf.cpp
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
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include <log/branch.h>
#include <log/logger.h>

namespace PConf {

// All these return false on a critical error condition.
bool parseName              (const std::string& line, std::optional<std::string>& out, Log::Branch&);
bool parseLabel             (const std::string& line, std::optional<std::string>& out, Log::Branch&);
bool parseLabelNum          (const std::string& line, std::optional<int32_t>& out, Log::Branch&);
bool parseValue             (const std::string& line, std::istream&, std::optional<std::string>& out, Log::Branch&);
bool parseSinglelineValue   (const std::string& line, std::optional<std::string>& out, Log::Branch&);
bool parseMultilineValue    (std::istream&, std::optional<std::string>& out, Log::Branch&);

bool readEntry(std::istream&, std::shared_ptr<Entry>& out, bool& isSect, Log::Branch&);
bool readSection(std::istream&, std::shared_ptr<Section>& out, Log::Branch&);

bool writeEntry(std::ostream&, const std::shared_ptr<Entry>& entry, int32_t depth, Log::Branch&);
bool writeSection(std::ostream&, const std::shared_ptr<Section>& section, int32_t depth, Log::Branch&);

bool readline(std::istream&, string&);

} // namespace PConf

PConf::Entry::Entry(
        std::string name, 
        std::optional<std::string> value,
        std::optional<std::string> label,
        std::optional<int32_t> labelNum
        ) : name(std::move(name)), value(std::move(value)), label(std::move(label)), labelNum(labelNum) {}

PConf::Section::Section(
        std::string name, 
        std::optional<std::string> label, 
        std::optional<int32_t> labelNum,
        Data entries
        ) : Entry(std::move(name), std::nullopt, std::move(label), labelNum), entries(std::move(entries)) {}

bool PConf::read(std::istream& inStream, Data& out, Log::Branch *lBranch) {
    auto& logger{Log::Branch::optCreateLogger("PConf::read()", lBranch)};

    out.clear();
    bool isSect{};
    while (inStream.good() and not inStream.eof()) {
        std::shared_ptr<Entry> entry;
        if (not readEntry(inStream, entry, isSect, *logger.bverbose("Reading line (for entry)..."))) {
            logger.error("Failed reading pconf.");
            return false;
        }
        if (not entry and not isSect) continue;

        if (entry) {
            out.push_back(entry);
        } else {
            std::shared_ptr<Section> sect;
            if (not readSection(inStream, sect, *logger.bverbose("Reading section..."))) {
                logger.error("Failed reading pconf.");
                return false;
            }
            if (not sect) continue;
            out.push_back(sect);
        }
    }

    return true;
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

PConf::HashedData PConf::hash(const Data& data) {
    HashedData ret;
    for (const auto& entry : data) {
        ret.emplace(entry->name, entry);
    }

    return ret;
}

bool PConf::readEntry(std::istream& inStream, std::shared_ptr<Entry>& out, bool& isSect, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PConf::readEntry()")};

    out.reset();
    isSect = false;
    std::string line;
    auto prevLinePos{inStream.tellg()};

    if (not readline(inStream, line)) {
        logger.info("Reached end of stream.");
        return true;
    }

    auto numOpenBraces{std::count(line.begin(), line.end(), '{')};
    auto numCloseBraces{std::count(line.begin(), line.end(), '}')};
    auto openBracePos{line.find('{')};
    auto closeBracePos{line.find('}')};
    auto hasSeperator{line.find(':') != std::string::npos};

    bool correctNumBraces{
        ((numOpenBraces == 0 or numOpenBraces == 1) and (numCloseBraces == numOpenBraces - 1 or numCloseBraces == numOpenBraces)) ||
        ((numOpenBraces == 2) and (numCloseBraces == numOpenBraces - 1))
    };
    if (not correctNumBraces) {
        auto closeCondition{numCloseBraces == 1 and numOpenBraces == 0};
        if (closeCondition) {
            logger.verbose("Found close condition.");
            return true;
        } 

        logger.error("Bad brace setup!");
        return false;
    }

    isSect = (numOpenBraces - numCloseBraces == 1) and (openBracePos <= closeBracePos) and not hasSeperator;
    if (isSect) {
        inStream.seekg(prevLinePos);
        logger.debug("Found section.");
        return true;
    }

    std::optional<std::string> name;
    if (not parseName(line, name, *logger.bverbose("Attempting to parse entry name..."))) return false;
    if (not name) {
        logger.verbose("Line not an entry/missing name.");
        return true;
    }

    out = std::make_shared<Entry>();
    out->name = name.value();
    if (not parseValue(line, inStream, out->value, *logger.bdebug("Parsing entry \"" + name.value() + "\" value..."))) return false;
    if (not parseLabel(line, out->label, *logger.bdebug("Parsing entry \"" + name.value() + "\" label..."))) return false;
    if (not parseLabelNum(line, out->labelNum, *logger.bdebug("Parsing entry \"" + name.value() + "\" number label..."))) return false;

    return true;
}

bool PConf::readSection(std::istream& inStream, std::shared_ptr<Section>& out, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PConf::readSection()")};

    out.reset();
    std::string line;
    if (not readline(inStream, line)) return true;

    std::optional<std::string> name;
    if (not parseName(line, name, *logger.bverbose("Attempting to parse section name..."))) return false;
    if (not name) {
        logger.error("Read this as a section, but missing name. Must be a formatting error!");
        return false;
    }

    out = std::make_shared<Section>();
    out->name = name.value();
    if (not parseLabel(line, out->label, *logger.bdebug("Parsing section \"" + name.value() + "\" label..."))) return false;
    if (not parseLabelNum(line, out->labelNum, *logger.bdebug("Parsing section \"" + name.value() + "\" number label..."))) return false;

    bool foundSect{false};
    while (not inStream.eof()) {
        auto startPos{inStream.tellg()};
        std::shared_ptr<Entry> entry;
        if (not readEntry(inStream, entry, foundSect, *logger.bverbose("Reading line (for entry)..."))) return false;
        if (foundSect) {
            std::shared_ptr<Section> subSect;
            if (not readSection(inStream, subSect, *logger.bverbose("Reading section..."))) return false;
            if (not subSect) continue;
            out->entries.push_back(subSect);
            continue;
        }
        if (not entry) {
            // Check for section end
            inStream.seekg(startPos);
            readline(inStream, line);
            if (line.find('}') != std::string::npos) return true;

            continue;
        }
        out->entries.push_back(entry);
    }

    logger.error("Reached end of stream before section closing, formatting error!");
    return false;
}

bool PConf::parseName(const std::string& line, std::optional<std::string>& out, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PConf::parseName()")};

    out = std::nullopt;
    auto bracePos{line.find('{')};
    auto parenPos{line.find('(')};
    auto seperatorPos{line.find(':')};

    auto nameBegin{line.find_first_not_of(" \t")};
    auto spacePos{line.find_first_of(" \t{(", nameBegin)};
    if (nameBegin == spacePos) {
        logger.debug("Could not find name.");
        return true;
    }

    auto nameEnd{std::min(std::min(seperatorPos, spacePos), std::min(bracePos, parenPos))};
    out = line.substr(nameBegin, nameEnd - nameBegin);
    return true;
}

bool PConf::parseValue(const std::string& line, std::istream& inStream, std::optional<std::string>& out, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PConf::parseValue()")};

    out = std::nullopt;
    auto seperatorPos{line.find(':')};
    bool hasSeperator{seperatorPos != std::string::npos};

    if (not hasSeperator) {
        logger.verbose("No separator, no value.");
        return true;
    }

    auto bracePos{line.find('{')};
    auto isMultiline{bracePos != std::string::npos and seperatorPos < bracePos};
    if (isMultiline) {
        return parseMultilineValue(inStream, out, *logger.bdebug("Value is multiline, parsing..."));
    }

    return parseSinglelineValue(line.substr(seperatorPos + 1), out, *logger.bdebug("Value is single-line, parsing..."));
}

bool PConf::parseSinglelineValue(const std::string& line, std::optional<std::string>& out, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PConf::parseSingleLineValue()")};

    bool usesQuotes{line.find('"') != std::string::npos};
    bool record{false};

    size_t lineIdx{0};
    out = "";
    while (lineIdx < line.size()) {
        auto buf{line[lineIdx++]};
        if (buf == EOF) {
            if (usesQuotes and record) {
                logger.warn("Entry value w/ quotes not terminated before EOL! (" + line + ")");
                out = std::nullopt;
                return false;
            }
            return true;
        }

        if (usesQuotes) {
            if (buf == '"') {
                record = not record;
                if (record and not out->empty()) *out += '\n';
                continue;
            }
        } else {
            if (buf == ',') {
                *out += '\n';
                continue;
            }
            record = buf != ' ' and buf != '\t';
        }

        if (record) *out += buf;
    }

    return true;
}

bool PConf::parseMultilineValue(std::istream& inStream, std::optional<std::string>& out, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PConf::parseMultilineValue()")};

    out = "";
    std::string buf;
    while (readline(inStream, buf)) {
        auto quoteBegin{buf.find('"')};
        auto quoteEnd{buf.rfind('"')};
        if (quoteBegin == std::string::npos or quoteBegin == quoteEnd) {
            if (buf.find('}') != std::string::npos) {
                // Pop off trailing newline
                out->pop_back();
                return true;
            }
            continue;
        }

        *out += buf.substr(quoteBegin + 1, quoteEnd - quoteBegin - 1) + '\n';
    }

    logger.error("Reached end of segment before closed, formatting issue!");
    return false;
}

bool PConf::parseLabel(const string& line, optional<string>& out, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PConf::parseLabel()")};

    out = "";
    bool inParens{false};
    bool inQuotes{false};

    for (char character : line) {
        if (character == ':') {
            out = nullopt;
            return true;
        }
        if (character == '"' and not inParens) {
            out = nullopt;
            return true;
        }

        if (character == '(') {
            inParens = true;
            continue;
        }
        if (character == '"' and inParens) {
            if (inQuotes) return true;
            inQuotes = true;
            continue;
        }

        if (inQuotes) *out += character;
    }

    if (inQuotes) {
        logger.error("Quotes not closed before EOL! (" + line + ')');
        return false;
    } 

    if (out->empty()) out = nullopt;
    return true;
}

bool PConf::parseLabelNum(const std::string& line, std::optional<int32_t>& out, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PConf::parseLabelNum()")};

    out = std::nullopt;
    auto numCloseBraces{std::count(line.begin(), line.end(), '}')};
    if (numCloseBraces != 1) return true;

    auto openBracePos{line.find('{')};
    auto closeBracePos{line.find('}')};
    bool bracesInOrder{openBracePos < closeBracePos};
    if (not bracesInOrder or closeBracePos == std::string::npos) {
        logger.error("Entry has malplaced numeric label braces! (" + line + ")");
        return false;
    }

    out = std::strtol(line.substr(openBracePos + 1, closeBracePos - openBracePos - 1).c_str(), nullptr, 10);
    return true;
}

static std::ostream& writeWithDepth(std::ostream& outStream, int32_t depth) {
    for (int32_t i{0}; i < depth; i++) outStream << '\t';
    return outStream;
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
    if (lineEnd != std::string::npos) {
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
    if (lineBegin != 0) writeWithDepth(outStream, depth) << "}\n";

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

bool PConf::readline(std::istream& stream, string& out) {
    if (not std::getline(stream, out)) return false;

    out = out.substr(0, out.find("//"));
    return true;
}

