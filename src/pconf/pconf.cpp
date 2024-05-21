#include "pconf.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * pconf/pconf.cpp
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

#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <algorithm>

#include "log/logger.h"

static std::optional<std::string> parseName(const std::string& line);
static std::optional<std::string> parseValue(const std::string& line, std::istream& inStream);
static std::optional<std::string> parseMultilineValue(std::istream& inStream);
static std::optional<std::string> parseSinglelineValue(const std::string& lineEnd);
static std::optional<std::string> parseLabel(const std::string& line);
static std::optional<int32_t> parseLabelNum(const std::string& line);

PConf::Entry* PConf::readEntry(std::istream& inStream, bool& isSect) {
    isSect = false;
    std::string line;
    std::streampos prevLinePos{inStream.tellg()};
    if (!std::getline(inStream, line)) return nullptr;

    auto numOpenBraces{std::count(line.begin(), line.end(), '{')};
    auto numCloseBraces{std::count(line.begin(), line.end(), '}')};
    auto openBracePos{line.find('{')};
    auto closeBracePos{line.find('}')};
    auto hasSeperator{line.find(':') != std::string::npos};

    isSect = (numOpenBraces - numCloseBraces == 1) && (openBracePos <= closeBracePos) && !hasSeperator;
    if (isSect) {
        inStream.seekg(prevLinePos);
        return nullptr;
    }

    auto name{parseName(line)};
    if (!name) return nullptr;

    auto ret{new Entry};
    ret->name = name.value();
    ret->value = parseValue(line, inStream);
    ret->label = parseLabel(line);
    ret->labelNum = parseLabelNum(line);

    return ret;
}

PConf::Section* PConf::readSection(std::istream& inStream) {
    std::string line;
    if (!getline(inStream, line)) return nullptr;

    auto name{parseName(line)};
    if (!name) return nullptr;

    auto sect{new Section};
    sect->name = name.value();
    sect->label = parseLabel(line);
    sect->labelNum = parseLabelNum(line);

    bool foundSect;
    while (inStream.peek() != EOF) {
        auto entry{readEntry(inStream, foundSect)};
        if (foundSect) {
            auto subSect{readSection(inStream)};
            if (!subSect) continue;
            sect->entries.push_back(subSect);
            continue;
        }
        if (!entry) {
            // Check for section end
            char checkBuf;
            inStream.unget(); // \n
            do {
                inStream.unget();
                checkBuf = inStream.peek();
            } while ((checkBuf != '\n' && checkBuf != '}') && checkBuf != EOF);
            inStream.get(); // \n
            getline(inStream, line); // ungot line

            if (checkBuf == '}') return sect;
            continue;
        }
        sect->entries.push_back(entry);
    }

    return sect;
}

std::unordered_set<std::string> PConf::setFromValue(const std::optional<std::string>& value) {
    std::unordered_set<std::string> ret;
    size_t startPos{0};
    size_t endPos;
    const std::string& val{value.value_or("")};
    do {
        endPos = val.find('\n', startPos);
        ret.emplace(val.substr(startPos, endPos));
        startPos = endPos + 1;
    } while (endPos != std::string::npos);

    return ret;
}

static std::optional<std::string> parseName(const std::string& line) {
    auto bracePos{line.find('{')};
    auto parenPos{line.find('(')};
    auto seperatorPos{line.find(':')};

    if (bracePos == std::string::npos && parenPos == std::string::npos && seperatorPos == std::string::npos) return std::nullopt;

    auto nameBegin{line.find_first_not_of(" \t")};
    auto spacePos{line.find_first_of(" \t{(", nameBegin)};
    auto nameEnd{std::min(std::min(seperatorPos, spacePos), std::min(bracePos, parenPos))};
    return line.substr(nameBegin, nameEnd - nameBegin);
}

static std::optional<std::string> parseValue(const std::string& line, std::istream& inStream) {
    auto seperatorPos{line.find(':')};
    bool hasSeperator{seperatorPos != std::string::npos};

    if (!hasSeperator) return std::nullopt;

    auto bracePos{line.find('{')};
    auto isMultiline{bracePos != std::string::npos && seperatorPos < bracePos};
    if (isMultiline) return parseMultilineValue(inStream);
    else return parseSinglelineValue(line.substr(seperatorPos + 1));
}

static std::optional<std::string> parseSinglelineValue(const std::string& lineEnd) {
    std::istringstream lineStream(lineEnd);

    bool usesQuotes{lineEnd.find('"') != std::string::npos};
    bool record{false};

    char buf;
    std::string ret;

    while (!false) {
        buf = lineStream.get();
        if (buf == EOF) {
            if (usesQuotes && record) {
                Logger::warn("Entry value w/ quotes not terminated before EOL! (" + lineEnd + ")", false);
                return std::nullopt;
            }
            return ret;
        }

        if (usesQuotes) {
            if (buf == '"') {
                record = !record;
                if (record == true && !ret.empty()) ret += '\n';
                continue;
            }
        } else {
            if (buf == ',') {
                ret += '\n';
                continue;
            }
            if (buf == ' ' || buf == '\t') record = false;
            else record = true;
        }

        if (record) ret += buf;
    }
}

static std::optional<std::string> parseMultilineValue(std::istream& inStream) {
    std::string ret;

    std::string buf;
    while (std::getline(inStream, buf)) {
        auto quoteBegin{buf.find('"')};
        auto quoteEnd{buf.rfind('"')};
        if (quoteBegin == std::string::npos || quoteBegin == quoteEnd) {
            if (buf.find('}')) return ret;
            continue;
        }

        ret += buf.substr(quoteBegin + 1, quoteEnd - quoteBegin - 1) + '\n';
    }

    Logger::warn("Reached end of segment before brace closed, likely a formatting issue!", false);
    return std::nullopt;
}

static std::optional<std::string> parseLabel(const std::string& line) {
    std::string label;
    bool inParens{false};
    bool inQuotes{false};

    for (char c : line) {
        if (c == ':') return std::nullopt;
        if (c == '"' && !inParens) return std::nullopt;

        if (c == '(') {
            inParens = true;
            continue;
        }
        if (c == '"' && inParens) {
            if (inQuotes) return label;
            inQuotes = true;
            continue;
        }

        if (inQuotes) label += c;
    }

    return std::nullopt;
}

static std::optional<int32_t> parseLabelNum(const std::string& line) {
    auto numOpenBraces{std::count(line.begin(), line.end(), '{')};
    auto numCloseBraces{std::count(line.begin(), line.end(), '}')};
    bool correctNumBraces{(numOpenBraces == 1 || numOpenBraces == 2) && numCloseBraces == 1};
    if (!correctNumBraces) return std::nullopt;

    auto openBracePos{line.find('{')};
    auto closeBracePos{line.find('}')};
    bool bracesInOrder{openBracePos < closeBracePos};
    if (!bracesInOrder) {
        Logger::warn("Entry has malplaced numeric label braces! (" + line + ")", false);
        return std::nullopt;
    }

    auto numBeginIt{std::find_if(line.begin() + openBracePos, line.end(), [](char c) { return std::isdigit(c); })};
    if (numBeginIt == line.end()) {
        Logger::warn("Entry has empty/malformed numeric label! (" + line + ")", false);
        return std::nullopt;
    }
    if ((numBeginIt - 1).base()[0] == '-') numBeginIt--;

    return std::stoi(numBeginIt.base());
}

static std::ostream& writeWithDepth(std::ostream& outStream, int32_t depth) {
    for (int32_t i{0}; i < depth; i++) outStream << '\t';
    return outStream;
}

void PConf::writeEntry(std::ostream& outStream, const Entry& entry, const int32_t depth) {
    writeWithDepth(outStream, depth) << entry.name;
    if (entry.label) outStream << "(\"" << entry.label.value() << "\")";
    if (entry.labelNum) outStream << '{' << entry.labelNum.value() << '}';

    if (!entry.value) {
        outStream << std::endl;
        return;
    }

    const auto& valueStr{entry.value.value()};
    outStream << ": ";

    auto lineBegin{0};
    auto lineEnd{valueStr.find('\n')};
    if (lineEnd != std::string::npos) {
        outStream << '{' << std::endl;
        writeWithDepth(outStream, depth + 1);
    }

    while (lineEnd != std::string::npos) {
        outStream << '"' << valueStr.substr(lineBegin, lineEnd - lineBegin) << '"' << std::endl;
        writeWithDepth(outStream, depth + 1);
        lineBegin = lineEnd + 1;
        lineEnd = valueStr.find('\n', lineBegin);
    }

    outStream << '"' << valueStr.substr(lineBegin, lineEnd - lineBegin) << '"' << std::endl;
    if (lineBegin != 0) writeWithDepth(outStream, depth) << '}' << std::endl;
}

void PConf::writeSection(std::ostream& outStream, const Section& section, const int32_t depth) {
    writeWithDepth(outStream, depth) << section.name;
    if (section.label) outStream << "(\"" << section.label.value() << "\")";
    if (section.labelNum) outStream << '{' << section.labelNum.value() << '}';
    outStream << " {" << std::endl;

    for (const auto& entry : section.entries) {
        if (entry->getType() == DataType::ENTRY) writeEntry(outStream, *entry, depth + 1);
        else if (entry->getType() == DataType::SECTION) writeSection(outStream, *static_cast<Section*>(entry), depth + 1);
    }

    writeWithDepth(outStream, depth) << '}' << std::endl;
}
