#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/utils/parent.hpp
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

#include "utils/types.hpp"

namespace utils {

namespace detail {

template <typename T>
struct MemberTraits;

template <class Parent, class Member>
struct MemberTraits<Member Parent::*>  {
    using ParentType = Parent;
    using MemberType = Member;
};

} // namespace detail

template <auto MEM_PTR>
typename detail::MemberTraits<decltype(MEM_PTR)>::ParentType& parent(
    typename detail::MemberTraits<decltype(MEM_PTR)>::MemberType& _
) {
    using ParentType = typename detail::MemberTraits<
        decltype(MEM_PTR)
    >::ParentType;

    ParentType *dummy{nullptr};
    const auto *offset{reinterpret_cast<uint8 *>(&(dummy->*MEM_PTR))};

    return *reinterpret_cast<ParentType *>(
        reinterpret_cast<uint8 *>(&_) - offset
    );
}

template <auto MEM_PTR>
typename detail::MemberTraits<decltype(MEM_PTR)>::ParentType& parent(
    const typename detail::MemberTraits<decltype(MEM_PTR)>::MemberType& _
) {
    using ParentType = typename detail::MemberTraits<
        decltype(MEM_PTR)
    >::ParentType;

    ParentType *dummy{nullptr};
    const auto *offset{reinterpret_cast<uint8 *>(&(dummy->*MEM_PTR))};

    return *reinterpret_cast<ParentType *>(
        const_cast<uint8 *>(reinterpret_cast<const uint8 *>(&_)) - offset
    );
}


} // namespace utils

