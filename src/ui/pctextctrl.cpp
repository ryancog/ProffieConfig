// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "pctextctrl.h"

#include <wx/sizer.h>
#include <wx/tooltip.h>

pcTextCtrl::pcTextCtrl(wxWindow* _parent, int32_t _id, const wxString& _label, const wxPoint& _pos, const wxSize& _size, int32_t _style, const wxString& _defaultVal, const wxOrientation& _orientation)
    : wxBoxSizer(_orientation),
      mEntry{new wxTextCtrl(_parent, _id, _defaultVal, _pos, _size, _style)} {
  if (!_label.empty()) {
    mText = new wxStaticText(_parent, wxID_ANY, _label);
    auto sizerFlags = wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 5);
    if (_orientation == wxHORIZONTAL) sizerFlags = sizerFlags.Center();
    Add(text(), sizerFlags);
  }
  Add(entry(), wxSizerFlags(1).Expand());
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
