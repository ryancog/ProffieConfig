#include "static_box.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
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

constexpr auto PADDING{
#ifdef __WXOSX__
    0
#else
    10
#endif
};

} // namespace

pcui::StaticBox::StaticBox(
    wxOrientation orient, wxWindow *parent, const wxString& label
) : wxStaticBox(parent, wxID_ANY, label) {
    mSizer = new wxBoxSizer(orient);
    mPanel = new wxPanel(
        this,
        wxID_ANY,
        wxDefaultPosition,
        wxDefaultSize,
        wxTAB_TRAVERSAL | wxNO_BORDER,
        "pcui::StaticBox Inside"
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

#       if defined (__WXMSW__)
        pos.x += otherBorder + PADDING;
        pos.y += topBorder + PADDING;
#       elif defined (__WXOSX__)
        pos.x += otherBorder + PADDING;
        pos.y += otherBorder + PADDING;
#       else
        pos.x += PADDING;
        pos.y += PADDING;
#       endif

        mPanel->SetSize(pos.x, pos.y, size.x, size.y);
        evt.Skip();
    });

    SetAutoLayout(true);
}

bool pcui::StaticBox::Layout() {
    mPanel->Layout();
    return true;
}

wxSize pcui::StaticBox::DoGetBestClientSize() const {
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

wxSizerItem *pcui::StaticBox::Add(
    wxWindow *window, const wxSizerFlags& flags
) {
    return mSizer->Add(window, flags);
}

wxSizerItem *pcui::StaticBox::Add(wxSizer *sizer, const wxSizerFlags& flags) {
    return mSizer->Add(sizer, flags);
}

wxSizerItem *pcui::StaticBox::AddSpacer(int32 size) {
    return mSizer->AddSpacer(size);
}

wxSizerItem *pcui::StaticBox::AddStretchSpacer(int32 prop) {
    return mSizer->AddStretchSpacer(prop);
}

void pcui::StaticBox::Clear(bool deleteWindows) {
    mSizer->Clear(deleteWindows);
}

bool pcui::StaticBox::IsEmpty() {
    return mSizer->IsEmpty();
}

