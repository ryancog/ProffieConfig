#pragma once
#include "mainwindow.h"

#include <vector>
#include <string>
#include <wx/combobox.h>

class Arduino {
public:
  static void refreshBoards(MainWindow*);
  static void updateList(wxComboBox*);

  static void applyToBoard(MainWindow*);
  static void updateIno();
  static std::string compile();
  static std::string parseError(const std::string&);
  static std::string upload();

  static void init();
  static std::vector<std::string> getBoards();

private:
  Arduino();
};
