// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include <wx/textctrl.h>
#include <wx/stattext.h>

class pcTextCtrl : public wxWindow {
public:
  pcTextCtrl(
    wxWindow* parent,
    int32_t id = wxID_ANY,
    const wxString& label = wxEmptyString,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    int32_t style = 0,
    const wxString& defaultVal = wxEmptyString,
    const wxOrientation& orientation = wxVERTICAL
    );

  wxTextCtrl* entry() const;
  wxStaticText* text() const;

private:
  wxTextCtrl* mEntry{nullptr};
  wxStaticText* mText{nullptr};
};
