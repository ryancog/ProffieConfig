// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "pcspinctrldouble.h"

#include <wx/sizer.h>
#include <wx/tooltip.h>

pcSpinCtrlDouble::pcSpinCtrlDouble(wxWindow* _parent, int32_t _id, const wxString& _label, const wxPoint& _pos, const wxSize& _size, int32_t _style, double _min, double _max, double _initial, const wxOrientation& _orientation)
    : wxBoxSizer(_orientation),
      mEntry{new wxSpinCtrlDouble(_parent, _id, wxEmptyString, _pos, _size, _style, _min, _max, _initial)} {
  entry()->SetIncrement(0.1);
  entry()->SetDigits(1);

  if (!_label.empty()) {
    mText = new wxStaticText(_parent, wxID_ANY, _label);
    auto sizerFlags = wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 5);
    if (_orientation == wxHORIZONTAL) sizerFlags = sizerFlags.Center();
    Add(text(), sizerFlags);
  }
  Add(entry(), wxSizerFlags(1).Expand());
}

void pcSpinCtrlDouble::SetToolTip(wxToolTip* tip) {
  if (mText) mText->SetToolTip(new wxToolTip(tip->GetTip()));
  mEntry->SetToolTip(tip);
}

wxStaticText* pcSpinCtrlDouble::text() const {
  return mText;
}
wxSpinCtrlDouble* pcSpinCtrlDouble::entry() const {
  return mEntry;
}
