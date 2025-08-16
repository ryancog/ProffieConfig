#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * components/utils/position.h
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

#include <wx/gdicmn.h>

#include "types.h"
#include "utils_export.h"

struct UTILS_EXPORT Point {
    Point() = default;
    constexpr Point(int32 x, int32 y) : x(x), y(y) {}

    template<typename T>
    explicit Point(T point) : x(point.x), y (point.y) {}

    Point(wxSize point) : x(point.x), y(point.y) {}
    Point(wxPoint point) : x(point.x), y(point.y) {}

    template<typename T>
    explicit operator T() const { return { x, y }; }

    operator wxSize() const { return { x, y }; }
    operator wxPoint() const { return { x, y }; }

    bool operator==(const Point& rhs) const noexcept = default;
    auto operator<=>(const Point& rhs) const {
        if (*this == rhs) return std::partial_ordering::equivalent;
        if (x > rhs.x and y > rhs.y) return std::partial_ordering::greater;
        if (x < rhs.x and y < rhs.y) return std::partial_ordering::less;
        return std::partial_ordering::unordered;
    };

    template<typename T>
    [[nodiscard]] inline constexpr Point operator+(const T& rhs) const { 
        return Point{ static_cast<int32>(std::round(x + rhs.x)), static_cast<int32>(std::round(y + rhs.y)) }; 
    }

    template<typename T>
    [[nodiscard]] inline constexpr Point operator-(const T& rhs) const { 
        return Point{ static_cast<int32>(std::round(x - rhs.x)), static_cast<int32>(std::round(y - rhs.y)) };
    }

    template<typename T>
    [[nodiscard]] inline constexpr Point operator*(T scale) const { 
        return Point{ static_cast<int32>(std::round(x * scale)), static_cast<int32>(std::round(y * scale)) }; 
    }

    template<typename T>
    [[nodiscard]] inline constexpr Point operator/(T scale) const { 
        return Point{ static_cast<int32>(std::round(x / scale)), static_cast<int32>(std::round(y / scale)) }; 
    }

    template<typename T>
    inline constexpr Point& operator+=(const T& rhs) { return *this = (*this + rhs); }
    template<typename T>
    inline constexpr Point& operator-=(const T& rhs) { return *this = (*this - rhs); }
    template<typename T>
    inline constexpr Point& operator*=(const T& rhs) { return *this = (*this * rhs); }
    template<typename T>
    inline constexpr Point& operator/=(const T& rhs) { return *this = (*this / rhs); }

    int32 x{0};
    int32 y{0};
};


