#include "bladeidpage.h"

#include "defines.h"

BladeIDPage* BladeIDPage::instance;
BladeIDPage::BladeIDPage(wxWindow* window) : wxStaticBoxSizer(wxHORIZONTAL, window, ""){
  Add(createBladeID(GetStaticBox()), BOXITEMFLAGS);
  Add(createBladeDetect(GetStaticBox()), BOXITEMFLAGS);

  update();
}

wxStaticBoxSizer* BladeIDPage::createBladeID(wxWindow* parent) {
  wxStaticBoxSizer* bladeIDSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Blade ID");

  enableID = new wxCheckBox(bladeIDSizer->GetStaticBox(), Misc::ID_BladeIDEnable, "Enable Blade ID");
  mode = new wxComboBox(bladeIDSizer->GetStaticBox(), Misc::ID_BladeIDMode, BLADE_ID_MODE_SNAPSHOT, wxDefaultPosition, wxDefaultSize, { BLADE_ID_MODE_SNAPSHOT, BLADE_ID_MODE_EXTERNAL, BLADE_ID_MODE_BRIDGED }, wxCB_READONLY);
  IDPin = new wxTextCtrl(bladeIDSizer->GetStaticBox(), Misc::ID_BladeIDPin, "blade4Pin", wxDefaultPosition, wxDefaultSize, 0);
  //pullupResistance

  bladeIDSizer->Add(enableID, FIRSTITEMFLAGS);
  bladeIDSizer->Add(mode, MENUITEMFLAGS);
  bladeIDSizer->Add(IDPin, MENUITEMFLAGS);

  return bladeIDSizer;
}


wxStaticBoxSizer* BladeIDPage::createBladeDetect(wxWindow* parent) {
  wxStaticBoxSizer* bladeDetectSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Blade Detect");

  return bladeDetectSizer;
}

void BladeIDPage::update() {

}
