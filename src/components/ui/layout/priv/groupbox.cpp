#include "groupbox.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/layout/priv/groupbox.cpp
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

#include <wx/colour.h>

#include "utils/types.hpp"

using namespace pcui::priv;

namespace {

constexpr auto PADDING{
#ifdef __WXOSX__
    0
#else
    10
#endif
};

} // namespace

GroupBox::GroupBox() = default;

GroupBox::GroupBox(
    wxOrientation orient, wxWindow *parent, const wxString& label
) { create(orient, parent, label); }

void GroupBox::create(
    wxOrientation orient,
    wxWindow *parent,
    const wxString& label
) {
    Create(parent, wxID_ANY, label);

    mSizer = new wxBoxSizer(orient);
    mPanel = new wxPanel(
        this,
        wxID_ANY,
        wxDefaultPosition,
        wxDefaultSize,
        wxTAB_TRAVERSAL | wxNO_BORDER,
        "GroupBox Inside"
    );
#   ifdef __WXOSX__
    mPanel->MacClipsToBounds(false);
    mPanel->SetBackgroundColour(wxTransparentColour);
#   endif
    mPanel->SetSizer(mSizer);

    Bind(wxEVT_SIZE, [this](wxSizeEvent& evt) {
        // The values returned by this function vary by OS, obviously, but also
        // vary in meaning.
        //
        // On macOS, the otherBorder is the only one that's meaningful. I'm not
        // quite sure what topBorder actually returns (could look in the
        // future), but it's 22px where the group borders on macOS are 11px,
        // and that's what otherBorder has. These are values on Sequoia, and it
        // surely varies across version, but I put the values there for
        // reference. The actual "padding" is 12px (it and 24px are values I've
        // seen referenced in other Cocoa discussions), if the 1px border drawn
        // is included. With a high-DPI display (virtually all of them), these
        // values are doubled.
        int32 topBorder{};
        int32 otherBorder{};
        GetBordersForSizer(&topBorder, &otherBorder);

        auto size{GetClientSize()};
        wxPoint pos{0, 0};

        size.x -= PADDING * 2;
        size.y -= PADDING * 2;

        size.x -= otherBorder * 2;
#       if defined(__WXOSX__)
        size.y -= otherBorder * 2;
#       else
        size.y -= otherBorder + topBorder;
#       endif

#       if defined(__WXMSW__)
        pos.x += otherBorder + PADDING;
        pos.y += topBorder + PADDING;
#       elif defined(__WXOSX__)
        pos.x += otherBorder + PADDING;
        pos.y += otherBorder + PADDING;
#       elif defined(__WXGTK__)
        pos.x += PADDING;
        pos.y += PADDING;
#       endif

        mPanel->SetSize(pos.x, pos.y, size.x, size.y);
        evt.Skip();
    });

    SetAutoLayout(true);
}

wxSizer *GroupBox::sizer() { return mSizer; }
wxWindow *GroupBox::childParent() { return mPanel; }

bool GroupBox::Layout() {
    mPanel->Layout();
    return true;
}

wxSize GroupBox::DoGetBestClientSize() const {
    // Early, during creation, this can be called by wxWidgets even though
    // nothing's established yet.
    if (not mSizer) return wxDefaultSize;

    int32 topBorder{};
    int32 otherBorder{};
    GetBordersForSizer(&topBorder, &otherBorder);

    auto ret{mSizer->CalcMin()};

    ret.x += 2 * otherBorder;
#   if defined(__WXOSX__)
    ret.y += otherBorder * 2;
#   else
    ret.y += otherBorder + topBorder;
#   endif

    ret.x += PADDING * 2;
    ret.y += PADDING * 2;

    // At least on macOS, this function, DoGetBestClientSize(), has a horribly
    // misleading name, apparently.
    //
    // If we return the client size, then in the wxEVT_SIZE, we'll find that
    // size was used for the entire window (including the label!), so we return
    // the window size here, and that makes things work out.
    return ClientToWindowSize(ret);
}

