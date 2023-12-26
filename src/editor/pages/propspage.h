// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once

#include "editor/editorwindow.h"

#include <wx/window.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/combobox.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>

// Forward declaration to get around circular dependency
class PropFile;

class PropsPage : public wxScrolledWindow {
public:
  PropsPage(wxWindow*);
  ~PropsPage();

  void update();
  void updateProps();
  void updateSelectedProp(const wxString& = "");
  PropFile* getSelectedProp();
  const std::vector<PropFile*>& getLoadedProps();
  wxStaticBoxSizer* sizer{nullptr};

  wxComboBox* propSelection{nullptr};
  wxButton* buttonInfo{nullptr};

private:
  EditorWindow* parent{nullptr};

  std::vector<PropFile*> props;

  enum {
    ID_Select,
    ID_Option,
    ID_Buttons
  };

  void loadProps();
  void bindEvents();
};
