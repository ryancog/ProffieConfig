#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/versions/detail/boards.hpp
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

#include <array>

#include "versions/os.hpp"

namespace versions::detail {

enum BoardIdx {
    eBoard_Proffie_V3,
    eBoard_Proffie_V2,
    eBoard_Proffie_V1,
};

const std::array<os::Board, 3> BOARDS{{
    {
        .name_="ProffieV3",
        .coreId_="proffieboard:stm32l4:ProffieboardV3-L452RE",
        .include_="\"proffieboard_v3_config.h\"",
    },
    {
        .name_="ProffieV2",
        .coreId_="proffieboard:stm32l4:ProffieboardV2-L433CC",
        .include_="\"proffieboard_v2_config.h\"",
    },
    {
        .name_="ProffieV1",
        .coreId_="proffieboard:stm32l4:Proffieboard-L433CC",
        .include_="\"proffieboard_v1_config.h\"",
    }
}};

} // namespace versions::detail

