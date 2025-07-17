#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "../editorwindow.h"
#include "ui/controls/controldata.h"

#include <wx/dialog.h>
#include <wx/panel.h>

class CustomOptionsDlg : public wxDialog {
public:
  CustomOptionsDlg(EditorWindow*);
  void addDefine(const wxString&, const wxString& = "");
  std::vector<std::pair<wxString, wxString>> getCustomDefines();

  enum {
    ID_AddDefine,
  };

private:
  wxScrolledWindow *mOptionArea{nullptr};
  wxButton *mAddDefineButton{nullptr};
  wxStaticText *mCricketsText{nullptr};

  class CDefine;
  vector<CDefine*> mCustomDefines;

  void bindEvents();
  void createUI();
  void createOptionArea();
  void updateOptions(bool purge = false);

  wxBoxSizer *header();
  static wxStaticBoxSizer *info(wxWindow*);
};

class CustomOptionsDlg::CDefine : public wxPanel {
  public:
    CDefine(wxScrolledWindow*);

    wxStaticText *defText{nullptr};
    PCUI::Text *name{nullptr};
    PCUI::Text *value{nullptr};
    wxButton *remove{nullptr};

    enum {
      ID_Name,
      ID_Value,
      ID_Remove,
    };
  };
