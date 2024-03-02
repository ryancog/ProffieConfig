// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include <wx/spinctrl.h>
#include <wx/stattext.h>

class pcSpinCtrl : public wxWindow {
public:
  pcSpinCtrl(
    wxWindow* parent,
    int32_t id = wxID_ANY,
    const wxString& label = wxEmptyString,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    int32_t style = wxSP_ARROW_KEYS,
    int32_t min = 0,
    int32_t max = 100,
    int32_t initial = 0,
    const wxOrientation& orientation = wxVERTICAL
    );

  void SetToolTip(wxToolTip*);

  wxSpinCtrl* entry() const;
  wxStaticText* text() const;

private:
  wxSpinCtrl* mEntry{nullptr};
  wxStaticText* mText{nullptr};
};
