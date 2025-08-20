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

PCUI::StaticBox::StaticBox(wxOrientation orient, wxWindow *parent, const wxString& label) :
    wxStaticBoxSizer(wxVERTICAL, parent, label) {
#   ifndef __WXOSX__
    mSizer = new wxBoxSizer(orient);
    wxStaticBoxSizer::Add(
        mSizer,
        wxSizerFlags(1)
            .Expand().Border(wxALL, 10)
    );
#   endif
}

#ifndef __WXOSX__
wxSizerItem *PCUI::StaticBox::Add(wxWindow *win, const wxSizerFlags& flags) {
    return mSizer->Add(win, flags);
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
#endif

