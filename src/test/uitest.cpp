#include "uitest.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * test/uitest.cpp
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

#include <iostream>

#include <wx/frame.h>
#include <wx/sizer.h>
#include <wx/button.h>

#include "styleeditor/blocks/styleblock.h"
#include "styles/bladestyle.h"
#include "ui/movable.h"
#include "styleeditor/blocks/block.h"

PCUI::Frame* Test::init() {
    auto frame{new PCUI::Frame(nullptr, wxID_ANY, "TestFrame")};
    auto frameSizer{new wxBoxSizer(wxVERTICAL)};
    auto moveArea{new PCUI::MoveArea(frame, wxID_ANY)};
    frameSizer->Add(moveArea, wxSizerFlags(1).Expand());
    auto areaSizer{new wxBoxSizer(wxVERTICAL)};
    auto panel{new PCUI::MovePanel(moveArea, moveArea, wxID_ANY)};
    areaSizer->Add(panel, wxSizerFlags(1).Expand());

    // auto style{BladeStyles::get("Cylon")({})};
    // PCUI::Block* block{new PCUI::StyleBlock(moveArea, panel, *style)};
    auto style = BladeStyles::get("StylePtr")(nullptr, {});
    auto block = new PCUI::StyleBlock(moveArea, panel, style);
    style = BladeStyles::get("TrWipeSparkTipX")(nullptr, {});
    block = new PCUI::StyleBlock(moveArea, panel, style);
    // style = BladeStyles::get("AudioFlickerL")({});
    // block = new PCUI::StyleBlock(moveArea, panel, *style);
    // style = BladeStyles::get("AudioFlickerL")({});
    // block = new PCUI::StyleBlock(moveArea, panel, *style);
    // style = BladeStyles::get("BlastF")({});
    // block = new PCUI::StyleBlock(moveArea, panel, *style);
    // style = BladeStyles::get("&style_charging")({});
    // block = new PCUI::StyleBlock(moveArea, panel, *style);

    
    moveArea->SetSizerAndFit(areaSizer);
    frame->SetSizer(frameSizer);

    return frame;
}
