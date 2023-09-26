#pragma once
#include <vector>
#include <string>
#include <wx/combobox.h>

class Arduino {
public:
  static void updateList(wxComboBox*);
  static std::string compile();
  static std::string upload();
  static void init();
  static std::vector<std::string> getBoards();

private:
  Arduino();
};
