#pragma once

#include <wx/gdicmn.h>
#include <wx/string.h>
#include <wx/combobox.h>
#include <wx/stattext.h>

class pcComboBox : public wxWindow {
public:
  pcComboBox(
      wxWindow* parent,
      int32_t id = wxID_ANY,
      const wxString& label = wxEmptyString,
      const wxPoint& position = wxDefaultPosition,
      const wxSize& size = wxDefaultSize,
      const wxArrayString& choices = {},
      int32_t style = 0,
      const wxOrientation& orientation = wxVERTICAL
      );

  void SetToolTip(wxToolTip*);

  wxComboBox* entry() const;
  wxStaticText* text() const;

private:
  wxComboBox* mEntry{nullptr};
  wxStaticText* mText{nullptr};
};
