#pragma once
#include <vector>
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

  static bool updateIno(wxString&);
  static bool compile(wxString&);
  static bool upload(wxString&);
  static wxString parseError(const wxString&);
};
