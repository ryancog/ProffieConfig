// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once

#include "editor/editorwindow.h"

#include <wx/frame.h>
#include <wx/button.h>
#include <wx/combobox.h>

class MainMenu : public wxFrame {
public:
  static MainMenu* instance;
  MainMenu();

  wxButton* refreshButton{nullptr};
  wxButton* applyButton{nullptr};
  wxComboBox* devSelect{nullptr};


private:
  EditorWindow* activeEditor{nullptr};

  enum {
    ID_DUMMY1, // on macOS menu items cannot have ID 0
    ID_DUMMY2, // on Win32, for some reason ID #1 is triggerred by hitting enter in wxTextCtrl? This is a workaround.
    ID_Copyright,
    ID_ReRunSetup,
    ID_RefreshDev,
    ID_ApplyChanges,
    ID_DeviceSelect,
    ID_Docs,
    ID_Issue,

    ID_ExportFile,
    ID_ImportFile,

    ID_OpenSerial,
    ID_StyleEditor,
  };

  void createUI();
  void createMenuBar();
  void createTooltips();
  void bindEvents();
};
