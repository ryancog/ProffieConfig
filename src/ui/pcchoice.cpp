// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include "pcchoice.h"

#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/tooltip.h>

pcChoice::pcChoice(wxWindow* _parent, int32_t _id, const wxString& _label, const wxPoint& _pos, const wxSize& _size, const wxArrayString& _choices, int32_t _style, const wxOrientation& _orientation) :
    wxWindow(_parent, wxID_ANY),
    mEntry{new wxChoice(this, _id, _pos, _size, _choices, _style)} {

  auto sizer = new wxBoxSizer(_orientation);
  if (!_label.empty()) {
    mText = new wxStaticText(this, wxID_ANY, _label);
    auto sizerFlags = wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 5);
    if (_orientation == wxHORIZONTAL) sizerFlags = sizerFlags.Center();
    sizer->Add(text(), sizerFlags);
  }
  sizer->Add(entry(), wxSizerFlags(1).Expand());

  SetSizerAndFit(sizer);
}

void pcChoice::SetToolTip(wxToolTip* tip) {
  if (mText) mText->SetToolTip(new wxToolTip(tip->GetTip()));
  mEntry->SetToolTip(tip);
}

wxStaticText *pcChoice::text() const { return mText; }
wxChoice *pcChoice::entry() const { return mEntry; }

void pcChoice::SetValue(const wxString& str) { mEntry->SetSelection(mEntry->FindString(str, true)); }
wxString pcChoice::GetValue() const { return mEntry->GetString(mEntry->GetSelection()); }
