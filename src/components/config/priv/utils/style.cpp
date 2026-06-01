#include "style.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/config/priv/utils/style.cpp
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

#include <sstream>
#include <stack>

#include "utils/string.hpp"

using namespace config::priv;

namespace {

struct Element {
    std::string comment_;

    /*
     * Pre and post children.
     */
    std::string pre_;
    std::string post_;

    std::vector<Element> children_;
};

std::optional<std::string> tryBuildCompact(Element&);

void doTrimming(Element&);

void formatMultiSingleComment(size, std::string&, const std::string&);

} // namespace

bool style::extractComments(
    std::string& content,
    size& pos,
    const data::base::String::Context& comments
) {
    const auto addToComment{[&](std::string& addStr) {
        if (not comments.val().empty()) {
            addStr.insert(addStr.begin(), '\n');
        }

        auto commentStr{comments.val()};
        commentStr.append(addStr);

        const auto newPos{commentStr.length()};
        comments.change(std::move(commentStr), newPos);
    }};

    size_t illegalPos{0};
    bool commentMove{false};
    while (
            (illegalPos = content.find("/*", illegalPos)) 
            != std::string::npos
          ) {
        const auto terminatorPos{content.find("*/", illegalPos)};
        const auto eraseEnd{terminatorPos == std::string::npos
            ? std::string::npos 
            : terminatorPos + 2
        };

        if (eraseEnd < pos) {
            pos -= eraseEnd - illegalPos;
        } else if (illegalPos < pos) {
            pos = illegalPos;
        }

        const auto begin{illegalPos + 2};
        auto substr{content.substr(begin, terminatorPos - begin)};
        utils::trimSurroundingWhitespace(substr);
        if (not substr.empty()) {
            addToComment(substr);
        }

        content.erase(illegalPos, eraseEnd - illegalPos);
        commentMove = true;
    }

    // If comment terminator but no opener, move everything before
    // terminator into comment
    if (
            (illegalPos = content.rfind("*/")) != std::string::npos and 
            content.find("/*") == std::string::npos
       ) {
        auto substr{content.substr(0, illegalPos)};
        utils::trimSurroundingWhitespace(substr);
        if (not substr.empty()) {
            addToComment(substr);
        }
        const auto eraseEnd{illegalPos + 2};

        if (eraseEnd < pos)
            pos -= eraseEnd;
        else
            pos = 0;

        content.erase(0, eraseEnd);
    }

    // Don't process line comments.

    return commentMove;
}

void style::commentFilter(
    const data::base::String::ROContext&, std::string& str, size& pos
) {
    size_t illegalPos{};
    while (
            (illegalPos = str.find("/*")) != std::string::npos or
            (illegalPos = str.find("*/")) != std::string::npos
          ) {
        if (illegalPos < pos) pos -= std::max<size>(2, pos - illegalPos);
        str.erase(illegalPos, 2);
    }
}

std::string style::format(
    const std::string& in, bool ignoreLength
) {
    std::istringstream stream(in);

    Element root;

    // First, extract out everything into `Element`s.
    std::stack<Element *> stack;
    stack.push(&root);
    while (stream.good()) {
        auto *current{stack.top()};

        utils::CommentData commentData{
            .stream_=stream,
            .skipSpaces_=false,
        };
        if (utils::extractComments(commentData)) {
            if (not current->comment_.empty())
                current->comment_ += '\n';

            current->comment_ += commentData.out_;
        }

        const auto chr{stream.get()};
        if (not stream.good())
            break;

        // Next child
        if (chr == ',') {
            stack.pop();

            // Handle an invalid input.
            if (stack.empty())
                return in;

            auto& child{stack.top()->children_.emplace_back()};
            stack.push(&child);
            continue;
        }

        // Handle children
        if (chr == '<') {
            current->pre_ += '<';

            auto& child{current->children_.emplace_back()};
            stack.push(&child);
            continue;
        }

        // At parent end.
        if (chr == '>') {
            stack.pop();

            // Handle an invalid input.
            if (stack.empty())
                return in;

            auto *parent{stack.top()};
            parent->post_ += '>';

            // If there's `<>`, make sure there's not an extra child.
            if (
                    parent->children_.size() == 1 and
                    parent->children_.back().pre_.empty()
               ) {
                parent->children_.clear();
            }

            continue;
        }

        if (current->post_.empty())
            current->pre_ += static_cast<char>(chr);
        else
            current->post_ += static_cast<char>(chr);
    }

    // Intentionally don't skip whitespace during the above parsing so that
    // this can maintain whitespace in quotes.
    doTrimming(root);

    // Now, go through and build the new string.
    std::string exploded;

    // Clear stack
    while (not stack.empty())
        stack.pop();

    stack.push(&root);
    while (not stack.empty()) {
        auto *current{stack.top()};

        auto depth{stack.size() - 1};
        auto spaces{depth * 4};

        auto compact{tryBuildCompact(*current)};
        if (compact and (ignoreLength or spaces + compact->size() <= 80)) {
            formatMultiSingleComment(depth, exploded, current->comment_);

            exploded.append(depth, '\t');
            exploded.append(*compact);
        } else {
            formatMultiSingleComment(depth, exploded, current->comment_);

            exploded.append(depth, '\t');
            exploded.append(current->pre_);

            if (not current->children_.empty()) {
                exploded.push_back('\n');
                stack.push(&current->children_.front());
                continue;
            }

            // No children.
            exploded.append(current->post_);
        }

        while (not false) {
            // Make sure current actually is current if looped.
            current = stack.top();
            stack.pop();

            if (stack.empty())
                break;

            auto *parent{stack.top()};

            // Find the current element in the parent children.
            auto iter{parent->children_.begin()};
            for (; &*iter != current; ++iter);
            // And then get the next child iter
            iter = std::next(iter);

            // No more children.
            if (iter == parent->children_.end()) {
                auto depth{stack.size() - 1};

                // Add newline after `current->post_`
                exploded.push_back('\n');

                // And then parent post on new line.
                exploded.append(depth, '\t');
                exploded.append(parent->post_);


                continue;
            }

            // For after post_. ">,\n"
            exploded.push_back(',');
            exploded.push_back('\n');

            // Push next child onto the stack and continue;
            stack.push(&*iter);
            break;
        }
    }

    return exploded;
}

namespace {

std::optional<std::string> tryBuildCompact(Element& element) {
    std::string ret;

    ret += element.pre_;

    for (auto& child : element.children_) {
        if (not child.comment_.empty())
            // Can't compact if a child has a comment.
            return std::nullopt;

        auto childCompact{tryBuildCompact(child)};
        if (not childCompact)
            return std::nullopt;

        ret += *childCompact;
        ret += ',';
    }

    // Pop off last ','
    if (not element.children_.empty())
        ret.pop_back();

    ret += element.post_;
    
    return ret;
}

void doTrimming(Element& element) {
    utils::trimWhitespaceOutsideString(element.pre_);
    utils::trimWhitespaceOutsideString(element.post_);

    for (auto& child : element.children_)
        doTrimming(child);
}

void formatMultiSingleComment(
    size depth, std::string& out, const std::string& comment
) {
    if (comment.empty())
        return;

    std::string_view view{comment};
    while (not false) {
        // Handle multiple single-line comments.
        auto lineEnd{view.find('\n')};
        out.append(depth, '\t');
        out.append("// ");
        out.append(view.substr(0, lineEnd));
        out.push_back('\n');

        if (lineEnd == std::string::npos)
            break;

        view.remove_prefix(lineEnd + 1);
    }
}

} // namespace

