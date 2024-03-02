// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include "pctextctrl.h"

#include <wx/sizer.h>
#include <wx/tooltip.h>

pcTextCtrl::pcTextCtrl(wxWindow* _parent, int32_t _id, const wxString& _label, const wxPoint& _pos, const wxSize& _size, int32_t _style, const wxString& _defaultVal, const wxOrientation& _orientation)
    : wxWindow(_parent, wxID_ANY),
      mEntry{new wxTextCtrl(this, _id, _defaultVal, _pos, _size, _style)} {
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

void pcTextCtrl::SetToolTip(wxToolTip* tip) {
  if (mText) mText->SetToolTip(new wxToolTip(tip->GetTip()));
  mEntry->SetToolTip(tip);
}


wxStaticText* pcTextCtrl::text() const {
  return mText;
}

wxTextCtrl* pcTextCtrl::entry() const {
  return mEntry;
}
