// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once
#include <vector>
#include <string>
#include <wx/combobox.h>

class Arduino {
public:
  static void refreshBoards();
  static void updateList(wxComboBox*);

  static void applyToBoard();
  static void verifyConfig();

  static void init();
  static std::vector<wxString> getBoards();

private:
  Arduino();
  Arduino(const Arduino&) = delete;

  static FILE* CLI(const wxString& command);

  static wxString updateIno();
  static wxString compile();
  static wxString parseError(const wxString&);
  static wxString upload();
};
