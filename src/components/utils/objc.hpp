#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/utils/objc.hpp
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

#include <CoreFoundation/CoreFoundation.h>
#include <objc/objc-runtime.h>

#include "utils/types.hpp"

/*
 * A lot of the Objective-C runtime is very much written in assembly, it's not
 * convenient to use in C, much less C++, but this works well enough.
 *
 * Do not use a forwarding reference here, it will mess up the `Signature`, and
 * there's no references in (Objective-)C anyways. It'll always be ptrs or POD
 * types going through here.
 */
template <typename Ret = void, typename ...Args>
Ret objcMessage(id self, cstring op, Args... args) {
    using Signature = Ret (*)(id, SEL, Args...);
    auto *func{reinterpret_cast<Signature>(objc_msgSend)};
    return func(self, sel_registerName(op), args...);
}

