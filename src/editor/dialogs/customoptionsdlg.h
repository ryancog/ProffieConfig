#pragma once

#include "editor/editorwindow.h"
#include "ui/pctextctrl.h"

#include <wx/dialog.h>

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
  wxScrolledCanvas* optionArea{nullptr};
  wxButton* addDefineButton{nullptr};
  wxStaticText* cricketsText{nullptr};

  class CDefine;
  std::vector<CDefine*> customDefines{};

  void bindEvents();
  void createUI();
  void createOptionArea();
  void updateOptions();

  wxBoxSizer* header();
  wxBoxSizer* info();
};

class CustomOptionsDlg::CDefine : public wxWindow {
  public:
    CDefine(wxScrolledCanvas*);

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
