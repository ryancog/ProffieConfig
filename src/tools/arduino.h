// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once
#include <vector>
#include <wx/combobox.h>

#include "editor/editorwindow.h"
#include "mainmenu/mainmenu.h"
#include "core/utilities/progress.h"

class Arduino {
public:
  static void refreshBoards(MainMenu*, std::function<void(bool)> = [](bool){});
  static void applyToBoard(MainMenu*, EditorWindow*, std::function<void(bool)> = [](bool){});
  static void verifyConfig(wxWindow*, EditorWindow*, std::function<void(bool)> = [](bool){});

  static void init(wxWindow*, std::function<void(bool)> = [](bool){});
  static std::vector<wxString> getBoards();

  enum {
    PROFFIEBOARDV1 = 0,
    PROFFIEBOARDV2 = 1,
    PROFFIEBOARDV3 = 2
  };
private:
  Arduino();
  Arduino(const Arduino&) = delete;

  static FILE* CLI(const wxString& command);

  static bool updateIno(wxString&, EditorWindow*);
  static bool compile(wxString&, EditorWindow*, Progress* = nullptr);
  static bool upload(wxString&, MainMenu*, EditorWindow*, Progress* = nullptr);
  static wxString parseError(const wxString&);
};
