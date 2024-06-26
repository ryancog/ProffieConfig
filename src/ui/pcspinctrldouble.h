// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include <wx/spinctrl.h>
#include <wx/stattext.h>

class pcSpinCtrlDouble : public wxWindow {
public:
  pcSpinCtrlDouble(
    wxWindow* parent,
    int32_t id = wxID_ANY,
    const wxString& label = wxEmptyString,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    int32_t style = wxSP_ARROW_KEYS,
    double min = 0,
    double max = 10,
    double initial = 0,
    const wxOrientation& orientation = wxVERTICAL
    );

  void SetToolTip(wxToolTip*);

  wxSpinCtrlDouble* entry() const;
  wxStaticText* text() const;

private:
  wxSpinCtrlDouble* mEntry{nullptr};
  wxStaticText* mText{nullptr};
};
