#include "version.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/utils/version.cpp
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

#include <charconv>
#include <compare>
#include <string>
#include <sys/types.h>

#include "utils/string.hpp"

utils::Version::Version(std::string_view str) {
    if (str.empty()) {
        err_ = Err::Str_Empty;
        return;
    }

    std::string_view convStr{str};
    const cstring end{str.end()};

    auto parseNum{[this, &convStr, &end](VerNum& num) {
        auto res{std::from_chars(
            convStr.data(),
            convStr.data() + convStr.length(),
            num.val_
        )};

        if (res.ec != std::errc{}) {
            if (res.ec == std::errc::result_out_of_range) {
                err_ = Err::Num_Range;
            } else if (res.ec == std::errc::invalid_argument) {
                err_ = Err::Str_Invalid;
            } else {
                err_ = Err::Unknown;
            }
            return;
        }
        convStr = res.ptr;
    }};

    const auto parseLabel{[this, &convStr]() {
        // Jump over '-'
        convStr.remove_prefix(1);

        tag_.val_ = convStr;
        uint32 numTrimmed{};
        trim(
            tag_.val_,
            {.allowAlpha=true, .allowNum=true},
            &numTrimmed
        );

        if (numTrimmed) {
            err_ = Err::Str_Invalid;
            return;
        }
    }};


    parseNum(major_);
    if (err_ != Err::None or convStr.data() == end) return;

    if (convStr[0] == '.') {
        // Jump over '.'
        convStr.remove_prefix(1);
    } else if (convStr[0] == '-') {
        parseLabel();
        return;
    } else {
        err_ = Err::Str_Invalid;
        return;
    }

    parseNum(minor_);
    if (err_ != Err::None or convStr.data() == end) return;

    if (convStr[0] == '.') {
        // Jump over '.'
        convStr.remove_prefix(1);
    } else if (convStr[0] == '-') {
        parseLabel();
        return;
    } else {
        err_ = Err::Str_Invalid;
        return;
    }

    parseNum(bugfix_);
    if (err_ != Err::None or convStr.data() == end) return;
    if (convStr[0] != '-') {
        err_ = Err::Str_Invalid;
        return;
    }

    parseLabel();
}

std::strong_ordering utils::Version::compare(const Version& other) const {
    const auto compare{[this, &other](
        auto Version::* item
    ) -> std::strong_ordering {
        switch (static_cast<CompMode>((this->*item).mode_)) {
            case CompMode::Exact:
                return (this->*item).val_ <=> (other.*item).val_;
            case CompMode::Permissive:
                return std::strong_ordering::equal;
        }

        assert(0);
        __builtin_unreachable();
    }};

    auto majorRes{compare(&Version::major_)};
    if (majorRes != 0) return majorRes;

    auto minorRes{compare(&Version::minor_)};
    if (minorRes != 0) return minorRes;

    auto bugfixRes{compare(&Version::bugfix_)};
    if (bugfixRes != 0) return bugfixRes;

    auto tagRes{compare(&Version::tag_)};
    if (tagRes != 0) return tagRes;

    return std::strong_ordering::equal;
}

bool utils::Version::isExact() const {
    if (major_.mode_ != CompMode::Exact) return false;
    if (minor_.mode_ != CompMode::Exact) return false;
    if (bugfix_.mode_ != CompMode::Exact) return false;
    if (tag_.mode_ != CompMode::Exact) return false;
    return true;
}

std::string utils::Version::string() const {
    switch (err_) {
        using enum Err;
        case Invalid:
            return "Invalid";
        case Num_Range:
            return "Invalid Range";
        case Str_Invalid:
            return "Invalid Input";
        case Str_Empty:
            return "Empty Input";
        case Unknown:
            return "Unknown (Parse) Error";
        case None: break;
    }

    std::string ret;
    const auto outNum{[&ret] (const VerNum& num) {

    }};

    if (major_.mode_ == CompMode::Permissive) {
        ret += 'x';
    } else {
        ret += std::to_string(major_.val_);
    }

    if (minor_.mode_ == CompMode::Permissive) {
        ret += ".x";
    } else if (
        minor_.val_ != 0 or
        bugfix_.val_ != 0 or
        bugfix_.mode_ != CompMode::Exact
    ) {
        ret += '.';
        ret += std::to_string(minor_.val_);
    }

    if (bugfix_.mode_ == CompMode::Permissive) {
        ret += ".x";
    } else if (bugfix_.val_ != 0) {
        ret += '.';
        ret += std::to_string(bugfix_.val_);
    }

    if (tag_.mode_ == CompMode::Permissive) {
        ret += "-*";
    } else if (not tag_.val_.empty()) {
        ret += '-';
        ret += tag_.val_;
    }

    return ret;
}

utils::Version::operator std::string() const {
    return string();
}

utils::Version::operator bool() const { return err_ == Err::None; }

utils::Version utils::Version::invalid() {
    Version ret;
    ret.err_ = Err::Invalid;
    return ret;
}

std::strong_ordering utils::Version::RawComparator::operator()(
    const Version& lhs, const Version& rhs
) const {
    const auto comp{[this, &lhs, &rhs](auto Version::* item) {
        const auto modeComp{(lhs.*item).mode_ <=> (rhs.*item).mode_};
        if (modeComp != std::strong_ordering::equal) return modeComp;

        return (lhs.*item).val_ <=> (rhs.*item).val_;
    }};

    const auto majorComp{comp(&Version::major_)};
    if (majorComp != std::strong_ordering::equal) return majorComp;

    const auto minorComp{comp(&Version::minor_)};
    if (minorComp != std::strong_ordering::equal) return minorComp;

    const auto bugfixComp{comp(&Version::bugfix_)};
    if (bugfixComp != std::strong_ordering::equal) return bugfixComp;

    return comp(&Version::tag_);
}

bool utils::Version::RawOrderer::operator()(
    const Version& lhs, const Version& rhs
) const {
    return RawComparator{}(lhs, rhs) < 0;
}

