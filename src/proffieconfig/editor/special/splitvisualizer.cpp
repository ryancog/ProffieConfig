#include "splitvisualizer.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * proffieconfig/editor/special/splitvisualizer.cpp
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

#include <memory>

#include <wx/dc.h>
#include <wx/dcclient.h>
#include <wx/gdicmn.h>
#include <wx/graphics.h>

#include "utils/crypto.h"

vector<wxColour> SplitVisualizer::smIndexColors;

SplitVisualizer::SplitVisualizer(wxWindow *parent, Config::BladeArrays& bladeArrays) :
    wxWindow(parent, wxID_ANY), mBladeArrays{bladeArrays} {

    Bind(wxEVT_PAINT, &SplitVisualizer::paintEvent, this);

}

wxSize SplitVisualizer::GetMinSize() const {
    return {50, 200};
}

void SplitVisualizer::handleNotification(uint32) {

}

void SplitVisualizer::handleUnbound() {

}

void SplitVisualizer::paintEvent(wxPaintEvent&) {
    wxPaintDC dc{this};
    std::unique_ptr<wxGraphicsContext> gc{wxGraphicsContext::Create(dc)};

    auto size{GetSize()};

    wxPen pen{color(0)};
    wxBrush brush{color(0)};
    gc->SetBrush(brush);
    gc->SetPen(*wxTRANSPARENT_PEN);
    gc->DrawRectangle(0, 0, size.x, size.y);

    gc->SetBrush(*wxWHITE_BRUSH);
    
    gc->SetCompositionMode(wxCOMPOSITION_DEST_IN);
    gc->BeginLayer(1);
    gc->DrawRoundedRectangle(0, 0, size.x, size.y, 10);
    gc->EndLayer();
}

void SplitVisualizer::resize(wxSizeEvent&) {
    // Calculate sizes?

}

void SplitVisualizer::mouseMoved(wxMouseEvent&) {

}

void SplitVisualizer::mouseLeave(wxMouseEvent&) {

}

void SplitVisualizer::mouseClick(wxMouseEvent&) {

}

const wxColour& SplitVisualizer::color(uint32 idx) {
    if (idx < smIndexColors.size()) return smIndexColors[idx];

    while (idx >= smIndexColors.size()) {
        const auto hue{Crypto::random(0.0, 1.0)};
        const auto saturation{Crypto::random(0.4, 0.7)};
        const auto rgbValue{wxImage::HSVtoRGB({hue, saturation, 0.7})};
        smIndexColors.emplace_back(rgbValue.red, rgbValue.green, rgbValue.blue);
    }

    return smIndexColors[idx];
}

