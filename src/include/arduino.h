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

  static void updateIno();
  static std::string compile();
  static std::string parseError(const std::string&);
  static std::string upload();

  static FILE* CLI(const std::string& command);

  static void init();
  static std::vector<std::string> getBoards();

private:
  Arduino();
};
