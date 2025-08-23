#include "static_box.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025 Ryan Ogurek
 *
 * components/ui/static_box.cpp
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

#include <wx/panel.h>

namespace {

constexpr auto PADDING{10};

} // namespace

PCUI::StaticBox::StaticBox(wxOrientation orient, wxWindow *parent, const wxString& label) :
    wxStaticBox(parent, wxID_ANY, label) {
    mSizer = new wxBoxSizer(orient);
    mPanel = new wxPanel(
        this,
        wxID_ANY,
        wxDefaultPosition,
        wxDefaultSize,
        wxTAB_TRAVERSAL | wxNO_BORDER,
        "PCUI::StaticBox Inside"
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

        pos.x += PADDING;
        pos.y += PADDING;
#       ifdef __WXMSW__
        pos.x += otherBorder;
        pos.y += topBorder;
#       endif

        mPanel->SetSize(pos.x, pos.y, size.x, size.y);
        evt.Skip();
    });
}

wxSize PCUI::StaticBox::DoGetBestClientSize() const {
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

wxSizerItem *PCUI::StaticBox::Add(wxWindow *window, const wxSizerFlags& flags) {
    return mSizer->Add(window, flags);
}

wxSizerItem *PCUI::StaticBox::Add(wxSizer *sizer, const wxSizerFlags& flags) {
    return mSizer->Add(sizer, flags);
}

wxSizerItem *PCUI::StaticBox::AddSpacer(int32 size) {
    return mSizer->AddSpacer(size);
}

wxSizerItem *PCUI::StaticBox::AddStretchSpacer(int32 prop) {
    return mSizer->AddStretchSpacer(prop);
}

void PCUI::StaticBox::Clear(bool deleteWindows) {
    mSizer->Clear(deleteWindows);
}

bool PCUI::StaticBox::IsEmpty() {
    return mSizer->IsEmpty();
}

