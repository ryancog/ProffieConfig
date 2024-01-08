#include "pccombobox.h"
#include "wx/gtk/stattext.h"

#include <wx/stattext.h>
#include <wx/sizer.h>

pcComboBox::pcComboBox(wxWindow* _parent, int32_t _id, const wxString& _label, const wxPoint& _pos, const wxSize& _size, const wxArrayString& _choices, int32_t _style, const wxOrientation& _orientation) :
    wxWindow(_parent, wxID_ANY),
    mEntry{new wxComboBox(this, _id, (_choices.size() > 0) ? _choices.at(0) : "", _pos, _size, _choices, _style)} {

  auto sizer = new wxBoxSizer(_orientation);
  if (!_label.empty()) {
    mText = new wxStaticText(this, wxID_ANY, _label);
    sizer->Add(text(), wxSizerFlags(0).Border(wxLEFT | wxRIGHT, 5));
  } else mText = nullptr;
  sizer->Add(entry(), wxSizerFlags(0).Expand());

  SetSizerAndFit(sizer);
}

wxStaticText* pcComboBox::text() const {
  return const_cast<wxStaticText*>(mText);
}
wxComboBox* pcComboBox::entry() const {
  return const_cast<wxComboBox*>(mEntry);
}
