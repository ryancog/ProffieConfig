// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "pccombobox.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/tooltip.h>

pcComboBox::pcComboBox(wxWindow* _parent, int32_t _id, const wxString& _label, const wxPoint& _pos, const wxSize& _size, const wxArrayString& _choices, int32_t _style, const wxOrientation& _orientation) :
    wxBoxSizer(_orientation),
    mEntry{new wxComboBox(_parent, _id, {}, _pos, _size, _choices, _style)} {

    if (!_label.empty()) {
        mText = new wxStaticText(_parent, wxID_ANY, _label);
        auto sizerFlags = wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 5);
        if (_orientation == wxHORIZONTAL) sizerFlags = sizerFlags.Center();
        Add(text(), sizerFlags);
    }
    Add(entry(), wxSizerFlags(1).Expand());
}

void pcComboBox::SetToolTip(wxToolTip* tip) {
  if (mText) mText->SetToolTip(new wxToolTip(tip->GetTip()));
  mEntry->SetToolTip(tip);
}

wxStaticText *pcComboBox::text() const { return mText; }
wxComboBox *pcComboBox::entry() const { return mEntry; }
