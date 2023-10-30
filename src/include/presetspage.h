#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/button.h>

#pragma once

class PresetsPage : public wxStaticBoxSizer
{
public:
  PresetsPage(wxWindow*);
  static PresetsPage* instance;

  void update();

  struct {
    wxTextCtrl* presetsEditor{nullptr};
    wxListBox* presetList{nullptr};
    wxListBox* bladeList{nullptr};
    wxButton* addPreset{nullptr};
    wxButton* removePreset{nullptr};

    wxTextCtrl* nameInput{nullptr};
    wxTextCtrl* dirInput{nullptr};
    wxTextCtrl* trackInput{nullptr};

  } settings;

private:
  PresetsPage();
};
