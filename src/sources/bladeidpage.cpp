#include "bladeidpage.h"

#include "defines.h"
#include "misc.h"

#include <wx/button.h>

BladeIDPage* BladeIDPage::instance;
BladeIDPage::BladeIDPage(wxWindow* window) : wxStaticBoxSizer(wxVERTICAL, window, "") {
  wxBoxSizer* enableSizer = new wxBoxSizer(wxHORIZONTAL);
  enableID = new wxCheckBox(GetStaticBox(), Misc::ID_BladeIDEnable, "Enable Blade ID");
  enableDetect = new wxCheckBox(GetStaticBox(), Misc::ID_BladeDetectEnable, "Enable Blade Detect");
  enableSizer->Add(enableID, FIRSTITEMFLAGS);
  enableSizer->Add(enableDetect, FIRSTITEMFLAGS);

  wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
  topSizer->Add(createBladeDetect(GetStaticBox()), wxSizerFlags(2).Border(wxALL, 10).Expand());
  topSizer->Add(createIDSetup(GetStaticBox()), wxSizerFlags(3).Border(wxALL, 10).Expand());
  topSizer->Add(createBladeArrays(GetStaticBox()), wxSizerFlags(4).Border(wxALL, 10).Expand());

  wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
  bottomSizer->Add(createIDPowerSettings(GetStaticBox()), wxSizerFlags(1).Border(wxALL, 10).Expand());
  bottomSizer->Add(createContinuousScanSettings(GetStaticBox()), wxSizerFlags(1).Border(wxALL, 10).Expand());

  Add(enableSizer);
  Add(topSizer, wxSizerFlags(0).Expand());
  Add(bottomSizer, wxSizerFlags(0).Expand());
  update();
}

wxStaticBoxSizer* BladeIDPage::createBladeArrays(wxWindow* parent) {
  wxStaticBoxSizer* bladeIDSizer = new wxStaticBoxSizer(wxHORIZONTAL, parent, "Blade Arrays");

  bladeIDSizer->Add(createBladeArraysLeft(bladeIDSizer->GetStaticBox()), wxSizerFlags(0).Expand());
  bladeIDSizer->Add(createBladeArraysRight(bladeIDSizer), wxSizerFlags(0).Expand());

  return bladeIDSizer;
}
wxBoxSizer* BladeIDPage::createBladeArraysLeft(wxWindow* parent) {
  wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

  arrayList = new wxListBox(parent, wxID_ANY, wxDefaultPosition, wxSize(100, 0), {}, 0);

  wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  addID = new wxButton(parent, wxID_ANY, "+", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  removeID = new wxButton(parent, wxID_ANY, "-", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  buttonSizer->Add(addID, MENUITEMFLAGS);
  buttonSizer->Add(removeID, MENUITEMFLAGS);

  leftSizer->Add(arrayList, wxSizerFlags(1).Border(wxLEFT | wxTOP | wxRIGHT, 10).Expand());
  leftSizer->Add(buttonSizer, wxSizerFlags(0).Border(wxALL, 5).Center());

  return leftSizer;
}
wxBoxSizer* BladeIDPage::createBladeArraysRight(wxStaticBoxSizer* parent) {
  wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticText* arrayNameText = new wxStaticText(parent->GetStaticBox(), wxID_ANY, "Blade Array Name");
  arrayName = new wxTextCtrl(parent->GetStaticBox(), wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0);
  resistanceID = Misc::createNumEntry(parent, "ID Value", wxID_ANY, 2000, 100000, 40000);
  resistanceID->num->SetIncrement(100);

  rightSizer->Add(arrayNameText, TEXTITEMFLAGS);
  rightSizer->Add(arrayName, MENUITEMFLAGS);
  rightSizer->Add(resistanceID->box, MENUITEMFLAGS);

  return rightSizer;
}

wxStaticBoxSizer* BladeIDPage::createIDSetup(wxWindow* parent) {
  wxStaticBoxSizer* setupSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Blade ID Setup");
  mode = new wxComboBox(setupSizer->GetStaticBox(), Misc::ID_BladeIDMode, BLADE_ID_MODE_SNAPSHOT, wxDefaultPosition, wxDefaultSize, Misc::createEntries({ BLADE_ID_MODE_SNAPSHOT, BLADE_ID_MODE_EXTERNAL, BLADE_ID_MODE_BRIDGED }), wxCB_READONLY);
  pullupResistance = Misc::createNumEntry(setupSizer, "Pullup Resistance", wxID_ANY, 20000, 50000, 30000);
  pullupResistance->num->SetIncrement(100);

  wxBoxSizer* IDPinSizer = new wxBoxSizer(wxHORIZONTAL);
  wxStaticText* IDPinText = new wxStaticText(setupSizer->GetStaticBox(), wxID_ANY, "Blade ID Pin");
  IDPin = new wxTextCtrl(setupSizer->GetStaticBox(), wxID_ANY, "blade4Pin", wxDefaultPosition, wxDefaultSize, 0);
  IDPinSizer->Add(IDPinText, TEXTITEMFLAGS.Center());
  IDPinSizer->Add(IDPin, FIRSTITEMFLAGS);

  wxBoxSizer* pullupPinSizer = new wxBoxSizer(wxHORIZONTAL);
  wxStaticText* pullupPinText = new wxStaticText(setupSizer->GetStaticBox(), wxID_ANY, "Pullup Pin");
  pullupPin = new wxTextCtrl(setupSizer->GetStaticBox(), wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0);
  pullupPinSizer->Add(pullupPinText, TEXTITEMFLAGS.Center());
  pullupPinSizer->Add(pullupPin, FIRSTITEMFLAGS);

  setupSizer->Add(mode, FIRSTITEMFLAGS);
  setupSizer->Add(IDPinSizer, MENUITEMFLAGS);
  setupSizer->Add(pullupResistance->box, MENUITEMFLAGS);
  setupSizer->Add(pullupPinSizer, MENUITEMFLAGS);

  return setupSizer;
}
wxStaticBoxSizer* BladeIDPage::createIDPowerSettings(wxWindow* parent) {
  wxStaticBoxSizer* powerForIDSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Power for Blade ID");
  enablePowerForID = new wxCheckBox(powerForIDSizer->GetStaticBox(), Misc::ID_BladeIDEnablePower, "Enable Power on ID", wxDefaultPosition, wxDefaultSize, 0);

  wxBoxSizer* powerPinSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer* powerPinLeftSizer = new wxBoxSizer(wxVERTICAL);
  powerPin1 = new wxCheckBox(powerForIDSizer->GetStaticBox(), Misc::ID_BladeIDEnablePower, "Enable Power Pin 1", wxDefaultPosition, wxDefaultSize, 0);
  powerPin2 = new wxCheckBox(powerForIDSizer->GetStaticBox(), Misc::ID_BladeIDEnablePower, "Enable Power Pin 2", wxDefaultPosition, wxDefaultSize, 0);
  powerPin3 = new wxCheckBox(powerForIDSizer->GetStaticBox(), Misc::ID_BladeIDEnablePower, "Enable Power Pin 3", wxDefaultPosition, wxDefaultSize, 0);
  powerPinLeftSizer->Add(powerPin1, MENUITEMFLAGS);
  powerPinLeftSizer->Add(powerPin2, MENUITEMFLAGS);
  powerPinLeftSizer->Add(powerPin3, MENUITEMFLAGS);
  wxBoxSizer* powerPinRightSizer = new wxBoxSizer(wxVERTICAL);
  powerPin4 = new wxCheckBox(powerForIDSizer->GetStaticBox(), Misc::ID_BladeIDEnablePower, "Enable Power Pin 4", wxDefaultPosition, wxDefaultSize, 0);
  powerPin5 = new wxCheckBox(powerForIDSizer->GetStaticBox(), Misc::ID_BladeIDEnablePower, "Enable Power Pin 5", wxDefaultPosition, wxDefaultSize, 0);
  powerPin6 = new wxCheckBox(powerForIDSizer->GetStaticBox(), Misc::ID_BladeIDEnablePower, "Enable Power Pin 6", wxDefaultPosition, wxDefaultSize, 0);
  powerPinRightSizer->Add(powerPin4, MENUITEMFLAGS);
  powerPinRightSizer->Add(powerPin5, MENUITEMFLAGS);
  powerPinRightSizer->Add(powerPin6, MENUITEMFLAGS);

  powerPinSizer->Add(powerPinLeftSizer);
  powerPinSizer->Add(powerPinRightSizer);

  powerForIDSizer->Add(enablePowerForID, MENUITEMFLAGS);
  powerForIDSizer->Add(powerPinSizer);

  return powerForIDSizer;
}
wxStaticBoxSizer* BladeIDPage::createContinuousScanSettings(wxWindow* parent) {
  wxStaticBoxSizer* continuousScansSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Continuous Scanning");
  continuousScans = new wxCheckBox(continuousScansSizer->GetStaticBox(), wxID_ANY, "Enable Continuous Scanning", wxDefaultPosition, wxDefaultSize, 0);
  numIDTimes = Misc::createNumEntry(continuousScansSizer, "Number of Reads to Average", wxID_ANY, 1, 50, 10);
  scanIDMillis = Misc::createNumEntry(continuousScansSizer, "Scan Interval (ms)", wxID_ANY, 10, 50000, 1000);
  continuousScansSizer->Add(continuousScans, MENUITEMFLAGS);
  continuousScansSizer->Add(numIDTimes->box, MENUITEMFLAGS);
  continuousScansSizer->Add(scanIDMillis->box, MENUITEMFLAGS);

  return continuousScansSizer;
}


wxStaticBoxSizer* BladeIDPage::createBladeDetect(wxWindow* parent) {
  wxStaticBoxSizer* bladeDetectSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Blade Detect");

  wxStaticText* detectPinText = new wxStaticText(bladeDetectSizer->GetStaticBox(), wxID_ANY, "Detect Pin");
  detectPin = new wxTextCtrl(bladeDetectSizer->GetStaticBox(), wxID_ANY, "blade4Pin");
  bladeDetectSizer->Add(detectPinText, wxSizerFlags(0).Border(wxLEFT | wxTOP | wxRIGHT, 10));
  bladeDetectSizer->Add(detectPin, wxSizerFlags(0).Border(wxLEFT | wxBOTTOM | wxRIGHT, 10));

  return bladeDetectSizer;
}

void BladeIDPage::update() {

}
