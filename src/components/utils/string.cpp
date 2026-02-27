#include "string.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/utils/string.cpp
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

#include "utils/types.hpp"

void utils::trimWhitespace(std::string& str) {
    // TODO: Does ranges actually work here? Does it break in unforeseen ways?
    str.erase(std::remove_if(str.begin(), str.end(), [](char chr) {
        return std::isspace(chr);
    }), str.end());
}

void utils::trimWhitespaceOutsideString(std::string& str) {
    bool inDoubleQuote{false};
    bool inSingleQuote{false};
    // TODO: Optimize
    for (auto idx{0}; idx != str.length(); ++idx) {
        const auto chr{str[idx]};
        if (chr == '"' and not inSingleQuote) {
            inDoubleQuote = not inDoubleQuote;
        } else if (chr == '\'' and not inDoubleQuote) {
            inSingleQuote = not inSingleQuote;
        } else if (std::isspace(chr)) {
            if (chr != ' ' or not (inSingleQuote or inDoubleQuote)) {
                str.erase(idx, 1);
                --idx;

            }
        }
    }
}

void utils::trimSurroundingWhitespace(std::string& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char chr) {
        return not std::isspace(chr);
    }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](char chr) {
        return not std::isspace(chr);
    }).base(), str.end());
}

void utils::trimCppName(
    std::string& str,
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

void utils::trim(
    std::string& str,
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
                rules.safeList.find(*iter) == std::string::npos
           ) {
            if (numTrimmed and countTrimIndex > idx) ++*numTrimmed;
            iter = str.erase(iter);
            continue;
        }

        ++iter;
        ++idx;
    }
}

void utils::trimForNumeric(
    std::string& str,
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

std::optional<std::string> utils::extractComment(std::istream& stream) {
    enum class Reading {
        None,
        Line_Comment,
        Long_Comment,
    } reading{Reading::None};

    const auto startPos{stream.tellg()};

    std::optional<std::string> ret;
    std::string comment;
    while (stream.good()) {
        const auto chr{stream.get()};

        if (chr < 0 or chr > 0xFF) continue;
        if (
                std::iscntrl(chr) and
                chr != '\n' and
                chr != '\t'
           ) continue;

        if (reading == Reading::None) {
            if (chr == '/') {
                if (stream.peek() == '*') {
                    reading = Reading::Long_Comment;
                    comment.clear();
                    stream.get();
                } else if (stream.peek() == '/') {
                    reading = Reading::Line_Comment;
                    comment.clear();
                    stream.get();
                }
            } else if (std::isgraph(chr)) {
                stream.unget();
                break;
            }
        } else if (reading == Reading::Line_Comment) {
            if (chr == '/' and comment.empty()) continue;
            if (chr == '\n') {
                if (not ret) ret.emplace();
                if (not ret->empty()) *ret += '\n';
                trimSurroundingWhitespace(comment);
                *ret += comment;
                comment.clear();
                reading = Reading::None;
                continue;
            }
            comment += static_cast<char>(chr);
        } else if (reading == Reading::Long_Comment) {
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
                    reading = Reading::None;
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

bool utils::skipComment(std::istream& stream, std::string *str) {
    enum class Reading {
        None,
        Line_Comment,
        Long_Comment,
    } reading{Reading::None};

    bool skipped{false};
    while (stream.good()) {
        const auto chr{stream.get()};

        if (reading == Reading::None) {
            if (chr == '/') {
                if (stream.peek() == '*') {
                    reading = Reading::Long_Comment;
                    if (str) {
                        skipped = true;
                        *str += static_cast<char>(chr);
                        *str += static_cast<char>(stream.get());
                    }
                    continue;
                }
                if (stream.peek() == '/') {
                    reading = Reading::Line_Comment;
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
        } else if (reading == Reading::Line_Comment) {
            if (chr == '\n') {
                reading = Reading::None;
            }
        } else if (reading == Reading::Long_Comment) {
            if (chr == '*' and stream.peek() == '/') {
                reading = Reading::None;
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

std::vector<std::string> utils::createEntries(
    const std::vector<wxString>& vec
) {
    std::vector<std::string> entries;
    entries.reserve(vec.size()); for (const auto& entry : vec) {
        entries.emplace_back(entry.ToStdString());
    }
    return entries;
}


std::vector<std::string> utils::createEntries(
    const std::initializer_list<wxString>& list
) {
    return utils::createEntries(static_cast<std::vector<wxString>>(list));
}

std::optional<float64> utils::doStringMath(const std::string& str) {
    std::istringstream ss{str};

    enum class Operator {
        ADD,
        SUB,
        MUL,
        DIV,

        NEG,

        PAREN_L,
        PAREN_R,
    };
    struct OpPair {
        float64 val_;
        std::optional<Operator> op_;
    };
    std::stack<std::vector<OpPair>> stack;

    const auto solveLayer{[](std::vector<OpPair>& layer) -> bool {
        // Can't solve without inputs
        if (layer.empty()) return false;
        // There can't be an operator at the back, e.g. 7 - (6 / 2 +), the plus
        // is nonsense.
        if (layer.back().op_) return false;

        const auto opPrio{[](Operator op) -> int32 {
            switch (op) {
                using enum Operator;
                case ADD:
                case SUB:
                return 1;
                case MUL:
                case DIV:
                return 2;
                case NEG:
                return 3;
                case PAREN_L:
                case PAREN_R:
                assert(0);
            }

            __builtin_unreachable();
        }};

        const auto apply{[](float64 a, Operator op, float64 b) -> float64 {
            switch (op) {
                using enum Operator;
                case ADD: return a + b;
                case NEG:
                case SUB: return a - b;
                case MUL: return a * b;
                case DIV: return a / b;
                case PAREN_L:
                case PAREN_R:
                    assert(0);
            }

            __builtin_unreachable();
        }};

        while (layer.size() > 1) {
            int32 highestPrio{-1};
            auto highestIter{layer.end()};
            
            for (auto iter{layer.begin()}; iter->op_; ++iter) {
                auto prio{opPrio(*iter->op_)};
                if (prio > highestPrio) {
                    highestPrio = prio;
                    highestIter = iter;
                }
            }

            auto nextIter{std::next(highestIter)};
            assert(
                highestIter != layer.end() and 
                nextIter != layer.end()
            );

            auto val{apply(
                highestIter->val_, *highestIter->op_, nextIter->val_
            )};
            nextIter->val_ = val;

            layer.erase(highestIter);
        }

        assert(not layer.back().op_);
        return true;
    }};

    const auto parseOp{[](char c) -> std::optional<Operator> {
        switch (c) {
            using enum Operator;
            case '+': return ADD;
            case '-': return SUB;
            case '*': return MUL;
            case '/': return DIV;
            case '(': return PAREN_L;
            case ')': return PAREN_R;
            default: return std::nullopt;
        }
    }};

    const auto parseNum{[](std::string& numStr) -> std::optional<float64> {
        char *end{nullptr};
        auto num{strtod(numStr.c_str(), &end)};
        if (num == HUGE_VAL) return std::nullopt;
        if (end != &numStr[numStr.size()]) return std::nullopt;

        numStr.clear();
        return num;
    }};

    const auto pushNum{[&](float64 num) -> bool {
        auto& layer{stack.top()};

        if (not layer.empty()) {
            // Another number cannot be pushed without an operator.
            if (not layer.back().op_) return false;
        }

        auto& opPair{layer.emplace_back()};
        opPair.val_ = num;

        return true;
    }};

    const auto pushOp{[&](Operator op) -> bool {
        auto& layer{stack.top()};

        if (op == Operator::PAREN_L) {
            if (not stack.top().empty() and not stack.top().back().op_) {
                // If this isn't the start, or there's no operator preceeding
                // the paren, then we have a situation like `5 (smth)` which,
                // while valid math notation, is not valid C++.
                //
                // Cannot start new layer/parens 
                return false;
            }

            // New layer
            stack.emplace();
            return true;
        }

        if (op == Operator::PAREN_R) {
            // There isn't an extra layer started by a previous '('
            if (stack.size() == 1) return false;

            if (not solveLayer(stack.top())) return false;

            auto val{stack.top().back().val_};
            stack.pop();

            auto opPair{stack.top().emplace_back()};
            opPair.val_ = val;

            return true;
        }

        // If there's nowhere to place the op.
        if (layer.empty() or layer.back().op_) {
            switch (op) {
                using enum Operator;
                case ADD:
                    // E.g. 7 - +6, add is noop
                    return true;
                case SUB:
                {
                    // Push new pair for high-priority negation op.
                    auto& opPair{layer.emplace_back()};
                    opPair.val_ = 0;
                    opPair.op_ = NEG;
                    return true;
                }
                case MUL:
                case DIV:
                    // MUL and DIV cannot appear before a num.
                    return false;
                case NEG:
                    // Cannot push a neg, it is synthesized automatically.
                case PAREN_L:
                case PAREN_R:
                    // Parens should've been handled above.
                    assert(0);
            }

            __builtin_unreachable();
        }

        layer.back().op_ = op;
        return true;
    }};

    // Create base operations layer;
    stack.emplace();

    std::string buf;
    int chr{};
    while (chr = ss.get(), not ss.eof()) {
        if (std::isspace(chr)) {
            if (buf.empty()) continue;

            auto num{parseNum(buf)};
            if (not num or not pushNum(*num)) return std::nullopt;

            continue;
        }

        if (std::isdigit(chr) or chr == '.' or chr == 'x' or chr == 'X') {
            // See https://en.cppreference.com/w/cpp/language/floating_literal.html
            // if I want to make this very ~~correct~~ dumb.
            buf += static_cast<char>(chr);
        } else if (auto op{parseOp(static_cast<char>(chr))}) {
            // Make sure number is parsed and pushed before processing op.
            if (not buf.empty()) {
                auto num{parseNum(buf)};
                if (not num or not pushNum(*num)) return std::nullopt;
            }

            if (not pushOp(*op)) return std::nullopt;
        } else return std::nullopt;
    }

    // Ensure any final number is processed.
    if (not buf.empty()) {
        auto num{parseNum(buf)};
        if (not num or not pushNum(*num)) return std::nullopt;
    }

    // Should have a single layer when all is said and done, so long as parens
    // were properly matched in the string.
    if (stack.size() != 1) return std::nullopt;

    // Condense to a single pair w/ value and null op.
    if (not solveLayer(stack.top())) return std::nullopt;

    return stack.top().back().val_;
}

