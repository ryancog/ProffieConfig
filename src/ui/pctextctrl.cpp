#include "pctextctrl.h"

#include <wx/sizer.h>

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

wxStaticText* pcTextCtrl::text() const {
  return mText;
}

wxTextCtrl* pcTextCtrl::entry() const {
  return mEntry;
}
