// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "pcchoice.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/tooltip.h>

pcChoice::pcChoice(wxWindow* _parent, int32_t _id, const wxString& _label, const wxPoint& _pos, const wxSize& _size, const wxArrayString& _choices, int32_t _style, const wxOrientation& _orientation) :
    wxBoxSizer(_orientation),
    mEntry{new wxChoice(_parent, _id, _pos, _size, _choices, _style)} {

    if (!_choices.empty()) mEntry->SetSelection(0);
    if (!_label.empty()) {
        mText = new wxStaticText(_parent, wxID_ANY, _label);
        auto sizerFlags = wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 5);
        if (_orientation == wxHORIZONTAL) sizerFlags = sizerFlags.Center();
        Add(mText, sizerFlags);
    }
    Add(mEntry, wxSizerFlags(1).Expand());
}

void pcChoice::SetToolTip(wxToolTip* tip) {
  if (mText) mText->SetToolTip(new wxToolTip(tip->GetTip()));
  mEntry->SetToolTip(tip);
}

wxStaticText *pcChoice::text() const { return mText; }
wxChoice *pcChoice::entry() const { return mEntry; }
