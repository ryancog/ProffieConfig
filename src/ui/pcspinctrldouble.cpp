#include "pcspinctrldouble.h"

#include <wx/sizer.h>

pcSpinCtrlDouble::pcSpinCtrlDouble(wxWindow* _parent, int32_t _id, const wxString& _label, const wxPoint& _pos, const wxSize& _size, int32_t _style, double _min, double _max, double _initial, const wxOrientation& _orientation)
    : wxWindow(_parent, wxID_ANY),
      mEntry{new wxSpinCtrlDouble(this, _id, wxEmptyString, _pos, _size, _style, _min, _max, _initial)} {
  auto sizer = new wxBoxSizer(_orientation);

  if (!_label.empty()) {
    mText = new wxStaticText(this, wxID_ANY, _label);
    auto sizerFlags = wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 5);
    if (_orientation == wxHORIZONTAL) sizerFlags = sizerFlags.Center();
    sizer->Add(text(), sizerFlags);
  } else mText = nullptr;
  sizer->Add(entry(), wxSizerFlags(1).Expand());

  SetSizerAndFit(sizer);
}

wxStaticText* pcSpinCtrlDouble::text() const {
  return const_cast<wxStaticText*>(mText);
}
wxSpinCtrlDouble* pcSpinCtrlDouble::entry() const {
  return const_cast<wxSpinCtrlDouble*>(mEntry);
}
