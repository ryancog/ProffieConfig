// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#pragma once

#include "editor/editorwindow.h"
#include "ui/pctextctrl.h"

#include <wx/dialog.h>
#include <wx/panel.h>

class CustomOptionsDlg : public wxDialog {
public:
  CustomOptionsDlg(EditorWindow*);
  void addDefine(const std::string&, const std::string& = "");
  std::vector<std::pair<std::string, std::string>> getCustomDefines();

  enum {
    ID_AddDefine,
  };

private:
  EditorWindow* parent{nullptr};
  wxScrolledWindow* optionArea{nullptr};
  wxButton* addDefineButton{nullptr};
  wxStaticText* cricketsText{nullptr};

  class CDefine;
  std::vector<CDefine*> customDefines{};

  void bindEvents();
  void createUI();
  void createOptionArea();
  void updateOptions(bool purge = false);

  wxBoxSizer* header();
  wxStaticBoxSizer* info(wxWindow*);
};

class CustomOptionsDlg::CDefine : public wxPanel {
  public:
    CDefine(wxScrolledWindow*);

    wxStaticText* defText{nullptr};
    pcTextCtrl* name{nullptr};
    pcTextCtrl* value{nullptr};
    wxButton* remove{nullptr};

    enum {
      ID_Name,
      ID_Value,
      ID_Remove,
    };
  };
