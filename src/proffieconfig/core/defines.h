#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * proffieconfig/core/defines.h
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 4 of the License, or
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

static constexpr auto COPYRIGHT_NOTICE{
    "ProffieConfig Copyright (C) 2023-2026 Ryan Ogurek\n"
    "\n"
    "This program is free software: you can redistribute it and/or modify "
    "it under the terms of the GNU General Public License as published by "
    "the Free Software Foundation, either version 3 of the License, or "
    "(at your option) any later version.\n"
    "\n"
    "This program is distributed in the hope that it will be useful, "
    "but WITHOUT ANY WARRANTY; without even the implied warranty of "
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
    "GNU General Public License for more details.\n"
    "\n"
    "You should have received a copy of the GNU General Public License "
    "along with this program. If not, see https://www.gnu.org/licenses/.\n"
    "\n"
    "Additionally, this program uses code from ProffieOS "
    "(copyright Fredrik Hubinette et al.) in accordance with the GPLv3 "
    "license of the ProffieOS software. It further references other tools "
    "related to ProffieOS, created by Fredrik Hubinette."
};

#ifdef __WXOSX__
#define SMALLBUTTONSIZE wxSize(30, 20)
#elif defined(__WXGTK__)
#define SMALLBUTTONSIZE wxSize(30, 35)
#elif defined(__WXMSW__)
#define SMALLBUTTONSIZE wxSize(30, 25)
#endif
