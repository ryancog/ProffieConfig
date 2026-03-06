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
    mPanel->SetSizer(mSizer);

    Bind(wxEVT_SIZE, [this](wxSizeEvent& evt) {
        int32 topBorder{};
        int32 otherBorder{};
        GetBordersForSizer(&topBorder, &otherBorder);

        auto size{GetSize()};
        wxPoint pos{0, 0};

        size.x -= PADDING * 2;
        size.x -= otherBorder * 2;

        size.y -= PADDING * 2;
        size.y -= otherBorder + topBorder;

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
    int32 topBorder{};
    int32 otherBorder{};
    GetBordersForSizer(&topBorder, &otherBorder);
    auto ret{mSizer->CalcMin()};
    ret.x += 2 * otherBorder;
    ret.y += otherBorder + topBorder;
    ret.x += PADDING * 2;
    ret.y += PADDING * 2;
    return ret;
}

