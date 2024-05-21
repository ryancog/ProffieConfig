#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * styleeditor/blocks/block.h
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


#include <wx/window.h>
#include <wx/dcclient.h>
#include <wx/dcbuffer.h>

#include "styles/bladestyle.h"

namespace PCUI {

class Block : public virtual wxWindow {
public:
    Block(wxWindow* parent, uint32_t type);

    const uint32_t type;
    virtual wxSize DoGetBestClientSize() const override { return size; }

    static const wxColour* textColor;
    static const wxColour* faceColor;
    static const wxColour* bgColor;
    static const wxColour* dimColor;
    static const wxColour* hlColor;

    static const wxColour* builtinColor;
    static const wxColour* function3DColor;
    static const wxColour* timeFuncColor;
    static const wxColour* wrapperColor;
    static const wxColour* transitionColor;
    static const wxColour* functionColor;
    static const wxColour* layerColor;
    static const wxColour* colorColor;
    static const wxColour* colorLayerColor;
    static const wxColour* effectColor;
    static const wxColour* lockupTypeColor;
    static const wxColour* argumentColor;

    static constexpr int32_t edgePadding{10};
    static constexpr int32_t borderThickness{4};
    static constexpr int32_t internalPadding{5};

protected:
    virtual void paintEvent(wxPaintEvent&) {
        wxPaintDC dc(this);
        render(dc);
    }

    virtual void paintNow() {
        wxClientDC dc(this);
        render(dc);
    }

    virtual void render(wxDC&) = 0;

    wxSize size{0, 0};
};

}

