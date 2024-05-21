#include "block.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * ui/blocks/block.cpp
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

#include <wx/settings.h>
#include <wx/event.h>
#include <wx/gdicmn.h>

using namespace PCUI;

static bool initialized{false};
const wxColour* Block::textColor;
const wxColour* Block::faceColor;
const wxColour* Block::bgColor;
const wxColour* Block::dimColor;
const wxColour* Block::hlColor;

const wxColour* Block::builtinColor;
const wxColour* Block::function3DColor;
const wxColour* Block::timeFuncColor;
const wxColour* Block::wrapperColor;
const wxColour* Block::transitionColor;
const wxColour* Block::functionColor;
const wxColour* Block::colorColor;
const wxColour* Block::layerColor;
const wxColour* Block::colorLayerColor;
const wxColour* Block::effectColor;
const wxColour* Block::lockupTypeColor;
const wxColour* Block::argumentColor;

Block::Block(wxWindow* parent, uint32_t type) :
    type(type) {

#	ifndef __WXGTK__
    // current wx build complains about GDK compositing with this.
    // Required on Win32. I think it's responsible for eliminating
    // the flickers, since what it's really doing is enabling compositing.
    SetBackgroundStyle(wxBG_STYLE_TRANSPARENT);
# 	endif

    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, "Block");

    if (initialized) return;
    // textColor = new wxColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
    textColor = new wxColour(255, 255, 255);
    faceColor = new wxColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    bgColor   = new wxColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    dimColor  = new wxColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    hlColor   = new wxColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));

    builtinColor    = new wxColour(144, 129, 114); // Grey 
    function3DColor = new wxColour( 50, 116,  63); // Green
    timeFuncColor   = new wxColour( 50, 116,  63); // Green
    wrapperColor    = new wxColour(144, 129, 114); // Grey
    transitionColor = new wxColour( 47, 101, 179); // Blue
    functionColor   = new wxColour( 50, 116,  63); // Green
    colorColor      = new wxColour(217,  64,  64); // Red
    layerColor      = new wxColour(214, 102,   0); // Orange
    colorLayerColor = new wxColour(170,  56,   0);
    effectColor     = new wxColour(209, 132, 153); // Pink
    lockupTypeColor = new wxColour(214,  53,  97); // Different Pink
    argumentColor   = new wxColour(101,  45, 144); // Deep Purple
    initialized = true;
}
