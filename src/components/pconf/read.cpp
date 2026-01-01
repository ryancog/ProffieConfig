#include "pconf.h"
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

#include <algorithm>

#include "log/logger.h"

namespace {

// All these return false on a critical error condition.
void parseName           (const string& line, optional<string>& out, Log::Branch&);
bool parseLabel          (const string& line, optional<string>& out, Log::Branch&);
bool parseLabelNum       (const string& line, optional<uint32>& out, Log::Branch&);
bool parseValue          (const string& line, std::istream&, optional<string>& out, Log::Branch&);
bool parseSinglelineValue(const string& line, optional<string>& out, Log::Branch&);
bool parseMultilineValue (std::istream&, optional<string>& out, Log::Branch&);

bool readEntry(std::istream&, PConf::EntryPtr& out, bool& isSect, Log::Branch&);
bool readSection(std::istream&, PConf::SectionPtr& out, Log::Branch&);

bool readline(std::istream&, string&);

} // namespace

bool PConf::read(std::istream& inStream, Data& out, Log::Branch *lBranch) {
    auto& logger{Log::Branch::optCreateLogger("PConf::read()", lBranch)};

    out.clear();
    bool isSect{};
    while (inStream.good() and not inStream.eof()) {
        EntryPtr entry;
        if (not readEntry(inStream, entry, isSect, *logger.bverbose("Reading line (for entry)..."))) {
            logger.error("Failed reading pconf.");
            return false;
        }
        if (not entry and not isSect) continue;

        if (entry) {
            out.push_back(std::move(entry));
        } else {
            SectionPtr sect;
            if (not readSection(inStream, sect, *logger.bverbose("Reading section..."))) {
                logger.error("Failed reading pconf.");
                return false;
            }
            if (not sect) continue;
            out.emplace_back(sect);
        }
    }

    return true;
}

namespace {

bool readEntry(std::istream& inStream, PConf::EntryPtr& out, bool& isSect, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PConf::readEntry()")};

    out.reset();
    isSect = false;
    string line;
    auto prevLinePos{inStream.tellg()};

    if (not readline(inStream, line)) {
        logger.verbose("Reached end of stream.");
        return true;
    }

    auto numOpenBraces{std::count(line.begin(), line.end(), '{')};
    auto numCloseBraces{std::count(line.begin(), line.end(), '}')};
    auto openBracePos{line.find('{')};
    auto closeBracePos{line.find('}')};
    auto hasSeperator{line.find(':') != string::npos};

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
        logger.verbose("Found section.");
        return true;
    }

    optional<string> name;
    parseName(line, name, *logger.bverbose("Attempting to parse entry name..."));
    if (not name) {
        logger.verbose("Line not an entry/missing name.");
        return true;
    }

    out = PConf::Entry::create(*name);
    if (not parseValue(line, inStream, out->value, *logger.bverbose("Parsing entry \"" + name.value() + "\" value..."))) return false;
    if (not parseLabel(line, out->label, *logger.bverbose("Parsing entry \"" + name.value() + "\" label..."))) return false;
    if (not parseLabelNum(line, out->labelNum, *logger.bverbose("Parsing entry \"" + name.value() + "\" number label..."))) return false;

    return true;
}

bool readSection(std::istream& inStream, PConf::SectionPtr& out, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PConf::readSection()")};

    out.reset();
    string line;
    if (not readline(inStream, line)) return true;

    optional<string> name;
    parseName(line, name, *logger.bverbose("Attempting to parse section name..."));
    if (not name) {
        logger.error("Read this as a section, but missing name. Must be a formatting error!");
        return false;
    }

    out = PConf::Section::create(*name);
    if (not parseLabel(line, out->label, *logger.bverbose("Parsing section \"" + name.value() + "\" label..."))) return false;
    if (not parseLabelNum(line, out->labelNum, *logger.bverbose("Parsing section \"" + name.value() + "\" number label..."))) return false;

    bool foundSect{false};
    while (inStream.good() and not inStream.eof()) {
        auto startPos{inStream.tellg()};
        PConf::EntryPtr entry;
        if (not readEntry(inStream, entry, foundSect, *logger.bverbose("Reading line (for entry)..."))) return false;
        if (foundSect) {
            std::shared_ptr<PConf::Section> subSect;
            if (not readSection(inStream, subSect, *logger.bverbose("Reading section..."))) return false;
            if (not subSect) continue;
            out->entries.emplace_back(subSect);
            continue;
        }
        if (not entry) {
            // Check for section end
            inStream.seekg(startPos);
            readline(inStream, line);
            if (line.find('}') != string::npos) return true;

            continue;
        }
        out->entries.push_back(entry);
    }

    logger.error("Reached end of stream before section closing, formatting error!");
    return false;
}

void parseName(const string& line, std::optional<string>& out, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PConf::parseName()")};

    out = nullopt;
    auto bracePos{line.find('{')};
    auto parenPos{line.find('(')};
    auto seperatorPos{line.find(':')};

    auto nameBegin{line.find_first_not_of(" \t")};
    auto spacePos{line.find_first_of(" \t{(", nameBegin)};
    if (nameBegin == spacePos) {
        logger.verbose("Could not find name.");
        return;
    }

    auto nameEnd{std::min({seperatorPos, spacePos, bracePos, parenPos})};
    out = line.substr(nameBegin, nameEnd - nameBegin);
}

bool parseValue(const string& line, std::istream& inStream, optional<string>& out, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PConf::parseValue()")};

    out = nullopt;
    auto seperatorPos{line.find(':')};
    bool hasSeperator{seperatorPos != string::npos};

    if (not hasSeperator) {
        logger.verbose("No separator, no value.");
        return true;
    }

    auto bracePos{line.find('{')};
    auto isMultiline{bracePos != string::npos and seperatorPos < bracePos};
    if (isMultiline) {
        return parseMultilineValue(inStream, out, *logger.bverbose("Value is multiline, parsing..."));
    }

    return parseSinglelineValue(line.substr(seperatorPos + 1), out, *logger.bverbose("Value is single-line, parsing..."));
}

bool parseSinglelineValue(const string& line, std::optional<string>& out, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PConf::parseSingleLineValue()")};

    bool usesQuotes{line.find('"') != string::npos};
    bool record{false};

    out = "";
    for (auto idx{0}; idx < line.length(); ++idx) {
        const char chr{line[idx]};
        if (usesQuotes and chr == '"') {
            record = not record;
            continue;
        } 
        
        if (not record and chr == ',') {
            *out += '\n';
            continue;
        }

        // If `not record` is true, it doesn't matter if we're using quotes or not.
        // `not record` is always true when not using quotes
        if (not record and std::isspace(chr)) continue;

        if (not usesQuotes or record) *out += chr;
    }

    if (usesQuotes and record) {
        logger.warn("Entry value w/ quotes not terminated before EOL! (" + line + ")");
        return false;
    }

    if (out->empty()) out = nullopt;

    return true;
}

bool parseMultilineValue(std::istream& inStream, std::optional<string>& out, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PConf::parseMultilineValue()")};

    out = "";
    string buf;
    while (readline(inStream, buf)) {
        auto quoteBegin{buf.find('"')};
        auto quoteEnd{buf.rfind('"')};
        if (quoteBegin == string::npos or quoteBegin == quoteEnd) {
            if (buf.find('}') != string::npos) {
                // Pop off trailing newline
                // Don't trim whitespace. That's intentional for a multiline
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

bool parseLabel(const string& line, optional<string>& out, Log::Branch& lBranch) {
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

        if (not inParens and character == '(') {
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

bool parseLabelNum(const string& line, optional<uint32>& out, Log::Branch& lBranch) {
    auto& logger{lBranch.createLogger("PConf::parseLabelNum()")};

    out = nullopt;
    auto numCloseBraces{std::count(line.begin(), line.end(), '}')};
    if (numCloseBraces != 1) return true;

    auto openBracePos{line.find('{')};
    auto closeBracePos{line.find('}')};
    bool bracesInOrder{openBracePos < closeBracePos};
    if (not bracesInOrder or closeBracePos == string::npos) {
        logger.error("Entry has malplaced numeric label braces! (" + line + ")");
        return false;
    }

    const auto parseString{line.substr(
        openBracePos + 1,
        closeBracePos - openBracePos - 1
    )};
    if (parseString.empty() or not std::isdigit(parseString[0])) {
        return true;
    }

    out = strtoul(parseString.c_str(), nullptr, 0);
    return true;
}

bool readline(std::istream& stream, string& out) {
    if (not std::getline(stream, out)) return false;

    out = out.substr(0, out.find("//"));
    return true;
}

} // namespace

