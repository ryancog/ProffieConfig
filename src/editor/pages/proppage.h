// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once
class PropPage; // Forward declaration to get around circular dependencies

#include "core/config/propfile.h"
#include "editor/editorwindow.h"

#include <wx/window.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/combobox.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>

class PropPage : public wxScrolledWindow {
public:
  PropPage(wxWindow*);

  void update();
  void updateProps();
  void updatePropSelection();
  const PropFile& getSelectedProp();
  wxStaticBoxSizer* sizer{nullptr};

  wxComboBox* propSelection{nullptr};
  wxButton* buttonInfo{nullptr};

private:
  enum {
    ID_Select,
    ID_Option,
    ID_Buttons
  };

  EditorWindow* parent;
  void bindEvents();
};
