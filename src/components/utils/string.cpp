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

#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <stack>

#include "utils/types.h"

void Utils::trimWhitespace(string& str) {
    // TODO: Does ranges actually work here? Does it break in unforeseen ways?
    str.erase(std::remove_if(str.begin(), str.end(), [](char chr) {
        return std::isspace(chr);
    }), str.end());
}

void Utils::trimWhitespaceOutsideString(string& str) {
    bool inDoubleQuote{false};
    bool inSingleQuote{false};
    // TODO: Optimize
    for (auto idx{0}; idx != str.length(); ++idx) {
        const auto chr{str[idx]};
        if (chr == '"' and not inSingleQuote) inDoubleQuote = not inDoubleQuote;
        else if (chr == '\'' and not inDoubleQuote) inSingleQuote = not inSingleQuote;
        else if (std::isspace(chr)) {
            if (chr != ' ' or not (inSingleQuote or inDoubleQuote)) {
                str.erase(idx, 1);
                --idx;

            }
        }
    }
}

void Utils::trimSurroundingWhitespace(string& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char chr) {
        return not std::isspace(chr);
    }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](char chr) {
        return not std::isspace(chr);
    }).base(), str.end());
}

void Utils::trimCppName(
    string& str,
    bool allowNum,
    uint32 *numTrimmed,
    uint32 countTrimIndex
) {
    if (numTrimmed) *numTrimmed = 0;

    if (not allowNum) {
        while (not str.empty() and std::isdigit(str[0])) {
            if (numTrimmed and countTrimIndex > 0) {
                ++*numTrimmed;
            }
            str.erase(0, 1);
        }
    } else if (not str.empty() and std::isdigit(str[0])) {
        uint32 idx{1};
        auto iter{std::next(str.begin())};
        while (iter != str.end()) {
            if (not std::isdigit(*iter)) {
                if (numTrimmed and countTrimIndex > idx) ++*numTrimmed;
                iter = str.erase(iter);
                continue;
            }

            ++iter;
            ++idx;
        }
    }

    uint32 idx{0};
    auto iter{str.begin()};
    while (iter != str.end()) {
        if (
                not std::isalnum(*iter) and
                *iter != '_'
           ) {
            if (numTrimmed and countTrimIndex > idx) ++*numTrimmed;
            iter = str.erase(iter);
            continue;
        }

        ++iter;
        ++idx;
    }
}

void Utils::trim(
    string& str,
    TrimRules rules,
    uint32 *numTrimmed,
    uint32 countTrimIndex
) {
    if (numTrimmed) *numTrimmed = 0;

    uint32 idx{0};
    auto iter{str.begin()};
    while (iter != str.end()) {
        if (
                not (rules.allowAlpha and std::isalpha(*iter)) and
                not (rules.allowNum or std::isdigit(*iter)) and
                rules.safeList.find(*iter) == string::npos
           ) {
            if (numTrimmed and countTrimIndex > idx) ++*numTrimmed;
            iter = str.erase(iter);
            continue;
        }

        ++iter;
        ++idx;
    }
}

void Utils::trimForNumeric(
    string& str,
    uint32 *numTrimmed,
    uint32 countTrimIndex
) {
    if (numTrimmed) *numTrimmed = 0;

    // Special case
    if (str.size() == 1 and str[0] == '0') return;

    bool foundNonZero{false};
    uint32 idx{0};
    auto iter{str.begin()};
    while (iter != str.end()) {
        if (not std::isdigit(*iter)) {
            if (numTrimmed and countTrimIndex > idx) ++*numTrimmed;
            iter = str.erase(iter);
            continue;
        }

        if (not foundNonZero) {
            if (*iter == '0') {
                if (numTrimmed and countTrimIndex > idx) ++*numTrimmed;
                iter = str.erase(iter);
                continue;
            }
            foundNonZero = true;
        }

        ++iter;
        ++idx;
    }
}

optional<string> Utils::extractComment(std::istream& stream) {
    enum {
        NONE,
        LINE_COMMENT,
        LONG_COMMENT,
    } reading{NONE};

    const auto startPos{stream.tellg()};

    optional<string> ret;
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
                if (not ret) ret.emplace();
                if (not ret->empty()) *ret += '\n';
                trimSurroundingWhitespace(comment);
                *ret += comment;
                comment.clear();
                reading = NONE;
                continue;
            }
            comment += static_cast<char>(chr);
        } else if (reading == LONG_COMMENT) {
            bool end{chr == '*' and stream.peek() == '/'};
            if (end or chr == '\n') {
                if (not ret) ret.emplace();
                if (not ret->empty()) *ret += '\n';
                trimSurroundingWhitespace(comment);
                while (not comment.empty() and comment.front() == '*') {
                    comment.erase(0, 1);
                }
                trimSurroundingWhitespace(comment);
                *ret += comment;
                comment.clear();

                if (end) {
                    reading = NONE;
                    stream.get();
                }
                continue;
            }
            comment += static_cast<char>(chr);
        }
    }

    if (not ret) stream.seekg(startPos);
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
                    if (str) {
                        skipped = true;
                        *str += static_cast<char>(chr);
                        *str += static_cast<char>(stream.get());
                    }
                    continue;
                }
                if (stream.peek() == '/') {
                    reading = LINE_COMMENT;
                    if (str) {
                        skipped = true;
                        *str += static_cast<char>(chr);
                        *str += static_cast<char>(stream.get());
                    }
                    continue;
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
                if (str) {
                    skipped = true;
                    *str += static_cast<char>(chr);
                    *str += static_cast<char>(stream.get());
                }
                continue;
            }
        }

        if (str) {
            skipped = true;
            *str += static_cast<char>(chr);
        }
    }

    return skipped;
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

// Pulled and adapted from:
// C++ Program to illustrate how to evalauate a mathematical expression that is stored as string
// https://www.geeksforgeeks.org/cpp/how-to-parse-mathematical-expressions-in-cpp
//
// I should probably review it in depth at some point but for now it's good enough.
float64 Utils::doStringMath(const string& str) {
    const auto isOperator{[](char c) {
        return c == '+' or c == '-' or c == '*' or c == '/' or c == '^';
    }};
    const auto precedence{[](char op) -> int32 {
        if (op == '+' or op == '-') return 1;
        if (op == '*' or op == '/') return 2;
        if (op == '^') return 3;
        return 0;
    }};

    const auto applyOp{[](float64 a, float64 b, char op) -> float64 {
        // Applies the operator to the operands and returns the
        // result
        switch (op) {
            case '+': return a + b;
            case '-': return a - b;
            case '*': return a * b;
            case '/': return a / b;
            case '^': return pow(a, b);
            default: return 0;
        }
    }};

    std::stack<char> operators;
    std::stack<float64> operands;

    std::istringstream ss{str};

    string token;
    while (std::getline(ss, token, ' ')) {
        if (token.empty()) continue;
        if (std::isdigit(token[0])) {
            float64 num{};
            std::istringstream{token} >> num;
            operands.push(num);
        } else if (isOperator(token[0])) {
            const char op{token[0]};

            while (not operators.empty() and precedence(operators.top()) >= precedence(op)) {
                const auto operandB{operands.top()};
                operands.pop();
                const auto operandA{operands.top()};
                operands.pop();
                const auto op{operators.top()};
                operators.pop();

                operands.push(applyOp(operandA, operandB, op));
            }

            operators.push(op);
        } else if (token[0] == '(') { 
            operators.push('(');
        } else if (token[0] == ')') {
            while (not operators.empty() and operators.top() != '(') {
                const auto operandB{operands.top()};
                operands.pop();
                const auto operandA{operands.top()};
                operands.pop();
                const auto op{operators.top()};
                operators.pop();

                operands.push(applyOp(operandA, operandB, op));
            }

            // Pop the opening parenthesis
            operators.pop();
        }
    }

    while (not operators.empty()) {
        const auto operandB{operands.top()};
        operands.pop();
        const auto operandA{operands.top()};
        operands.pop();
        const auto op{operators.top()};
        operators.pop();

        operands.push(applyOp(operandA, operandB, op));
    }

    // The result is at the top of the operand stack
    return operands.empty() ? 0 : operands.top();
}

