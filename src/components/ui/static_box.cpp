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
    mSizer = new SizerWrapper(this, orient);
    mPanel = new wxPanel(this);
    mPanel->SetSizer(mSizer);

    Bind(wxEVT_SIZE, [this](wxSizeEvent& evt) {
        int32 topBorder{};
        int32 otherBorder{};
        GetBordersForSizer(&topBorder, &otherBorder);

        mPanel->SetSize(GetSize() - wxSize{(PADDING * 2) + (otherBorder * 2), (PADDING * 2) + otherBorder + topBorder});
        mPanel->SetPosition({PADDING, PADDING});
        evt.Skip();
    });
}

// void PCUI::StaticBox::SizerWrapper::RepositionChildren(const wxSize& minSize) {
// void Layout() override;
//     wxBoxSizer::RepositionChildren(minSize - wxSize{20, 20});
//     int32 topBorder{};
//     int32 otherBorder{};
//     mBox->GetBordersForSizer(&topBorder, &otherBorder);
//     for (auto *item : GetChildren()) {
//         auto pos{item->GetPosition()};
//         auto size{item->GetSize()};
// #       ifndef __WXOSX__
//         pos.y += 10;
//         pos.x += 10;
// #       endif
//         item->SetDimension(pos, size);
//     }
// }

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

    // auto ret{wxBoxSizer::CalcMin()};

    // TODO
    // ret.x = std::max(ret.x, boxWidth);

    // return ret;
}

wxSizerItem *PCUI::StaticBox::SizerWrapper::DoInsert(size_t index, wxSizerItem *item) {
    const auto reparentChildren{[this](const auto& self, wxSizer *sizer) -> void {
        for (const auto *item : sizer->GetChildren()) {
            if (item->IsWindow()) {
                auto *window{item->GetWindow()};
                window->Reparent(mBox->mPanel);
            } else if (item->IsSizer()) {
                auto *sizer{item->GetSizer()};
                self(self, sizer);
            }
        }
    }};
    if (item->IsWindow()) {
        if (item->GetWindow()->GetParent() == mBox) {
            item->GetWindow()->Reparent(mBox->mPanel);
        }
    } else if (item->IsSizer()) {
        reparentChildren(reparentChildren, item->GetSizer());
    }
    return wxBoxSizer::DoInsert(index, item);
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

