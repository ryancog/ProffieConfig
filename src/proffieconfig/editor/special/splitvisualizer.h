#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * proffieconfig/editor/special/splitvisualizer.h
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

#include "ui/notifier.h"
#include "config/bladeconfig/arrays.h"

class SplitVisualizer : public wxWindow, PCUI::NotifyReceiver {
public:
    SplitVisualizer(wxWindow *parent, Config::BladeArrays& bladeArrays);

private:
    void handleNotification(uint32) final;

    wxSize calculateSizes();
    void paintEvent(wxPaintEvent&);
    void resize(wxSizeEvent&);

    void mouseMoved(wxMouseEvent&);
    void mouseLeave(wxMouseEvent&);
    void mouseClick(wxMouseEvent&);

    /**
     * Sure let's regenerate this every program run...
     */
    static vector<wxColour> smIndexColors;
    static const wxColour& color(uint32 idx);

    Config::BladeArrays& mBladeArrays;

    struct SplitSize {
        float64 start;
        float64 length;
        uint32 splitIdx;
        uint32 segments{0};
        float64 segmentSize;

        bool overlapStart{false};
        bool overlapEnd{false};
    };
    vector<SplitSize> mSizes;
    int32 selectedSplit{-1};
    int32 hoveredSplit{-1};
};

