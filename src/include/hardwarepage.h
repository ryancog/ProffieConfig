#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>

#pragma once

class HardwarePage : public wxStaticBoxSizer
{
public:
  HardwarePage(wxWindow*);
  static HardwarePage* instance;

  void update();

  struct {
    wxCheckBox* bladeDetect{nullptr};
    wxStaticText* bladeDetectPinLabel{nullptr};
    wxComboBox* bladeDetectPin{nullptr};

    wxCheckBox* OLED{nullptr};
  } settings;

private:
  wxStaticBoxSizer* bladeDetect(wxStaticBoxSizer*);
  wxStaticBoxSizer* OLED(wxStaticBoxSizer*);

  HardwarePage();
};
