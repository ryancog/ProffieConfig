#include "bladeidpage.h"

#include "defines.h"
#include "mainwindow.h"
#include "misc.h"

#include <wx/button.h>

BladeIDPage* BladeIDPage::instance;
BladeIDPage::BladeIDPage(wxWindow* window) : wxStaticBoxSizer(wxVERTICAL, window, "") {
  wxBoxSizer* enableSizer = new wxBoxSizer(wxHORIZONTAL);
  enableDetect = new wxCheckBox(GetStaticBox(), ID_BladeDetectEnable, "Enable Blade Detect");
  enableID = new wxCheckBox(GetStaticBox(), ID_BladeIDEnable, "Enable Blade ID");
  enableSizer->Add(enableDetect, FIRSTITEMFLAGS);
  enableSizer->Add(enableID, FIRSTITEMFLAGS);

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

  bindEvents();
  update();
}

void BladeIDPage::bindEvents() {
  GetStaticBox()->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent&) {
        /*
        if (enableDetect->GetValue()) {
          bladeArrays.insert(bladeArrays.begin(), BladeArray{"blade_in", 0});
          bladeArrays.insert(bladeArrays.begin() + 1, BladeArray{"NO_BLADE", -1});
        } else {
          bladeArrays.erase(bladeArrays.begin(), bladeArrays.begin() + 1);
        }
        */
        update();
      }, ID_BladeDetectEnable);
  GetStaticBox()->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent&) {
        update();
      }, ID_BladeIDEnable);
  GetStaticBox()->Bind(wxEVT_COMBOBOX, [&](wxCommandEvent&) {
        update();
      }, ID_BladeIDMode);
  GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) {
        update();
      }, ID_BladeArray);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        update();
      }, ID_AddArray);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        update();
      }, ID_RemoveArray);
  GetStaticBox()->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent&) {
        update();
      }, ID_BladeIDPower);
  GetStaticBox()->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent&) {
        update();
      }, ID_ContinuousScan);
}

wxStaticBoxSizer* BladeIDPage::createBladeArrays(wxWindow* parent) {
  wxStaticBoxSizer* bladeIDSizer = new wxStaticBoxSizer(wxHORIZONTAL, parent, "Blade Arrays");

  bladeIDSizer->Add(createBladeArraysLeft(bladeIDSizer->GetStaticBox()), wxSizerFlags(0).Expand());
  bladeIDSizer->Add(createBladeArraysRight(bladeIDSizer), wxSizerFlags(0).Expand());

  return bladeIDSizer;
}
wxBoxSizer* BladeIDPage::createBladeArraysLeft(wxWindow* parent) {
  wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

  arrayList = new wxListBox(parent, ID_BladeArray, wxDefaultPosition, wxSize(100, 0), {}, 0);

  wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  addID = new wxButton(parent, ID_AddArray, "+", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  removeID = new wxButton(parent, ID_RemoveArray, "-", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
  buttonSizer->Add(addID, MENUITEMFLAGS);
  buttonSizer->Add(removeID, MENUITEMFLAGS);

  leftSizer->Add(arrayList, wxSizerFlags(1).Border(wxLEFT | wxTOP | wxRIGHT, 10).Expand());
  leftSizer->Add(buttonSizer, wxSizerFlags(0).Border(wxALL, 5).Center());

  return leftSizer;
}
wxBoxSizer* BladeIDPage::createBladeArraysRight(wxStaticBoxSizer* parent) {
  wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

  arrayName = Misc::createTextEntry(parent->GetStaticBox(), "Blade Array Name", wxID_ANY, "", 0);
  resistanceID = Misc::createNumEntry(parent->GetStaticBox(), "ID Value", wxID_ANY, 2000, 100000, 40000);
  resistanceID->num->SetIncrement(100);

  rightSizer->Add(arrayName->box, MENUITEMFLAGS.Expand());
  rightSizer->Add(resistanceID->box, MENUITEMFLAGS.Expand());

  return rightSizer;
}

wxStaticBoxSizer* BladeIDPage::createIDSetup(wxWindow* parent) {
  wxStaticBoxSizer* setupSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Blade ID Setup");
  mode = new wxComboBox(setupSizer->GetStaticBox(), ID_BladeIDMode, BLADE_ID_MODE_SNAPSHOT, wxDefaultPosition, wxDefaultSize, Misc::createEntries({ BLADE_ID_MODE_SNAPSHOT, BLADE_ID_MODE_EXTERNAL, BLADE_ID_MODE_BRIDGED }), wxCB_READONLY);
  IDPin = Misc::createTextEntry(setupSizer->GetStaticBox(), "Blade ID Pin", wxID_ANY, "blade4Pin", 0);

  pullupResistance = Misc::createNumEntry(setupSizer->GetStaticBox(), "Pullup Resistance", wxID_ANY, 20000, 50000, 30000);
  pullupResistance->num->SetIncrement(100);
  pullupPin = Misc::createTextEntry(setupSizer->GetStaticBox(), "Pullup Pin", wxID_ANY, "", 0);

  setupSizer->Add(mode, BOXITEMFLAGS);
  setupSizer->Add(IDPin->box, BOXITEMFLAGS);
  setupSizer->Add(pullupResistance->box, BOXITEMFLAGS);
  setupSizer->Add(pullupPin->box, BOXITEMFLAGS);

  return setupSizer;
}
wxStaticBoxSizer* BladeIDPage::createIDPowerSettings(wxWindow* parent) {
  wxStaticBoxSizer* powerForIDSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Power for Blade ID");
  enablePowerForID = new wxCheckBox(powerForIDSizer->GetStaticBox(), ID_BladeIDPower, "Enable Power on ID", wxDefaultPosition, wxDefaultSize, 0);

  wxBoxSizer* powerPinSizer = new wxBoxSizer(wxHORIZONTAL);
  wxBoxSizer* powerPinLeftSizer = new wxBoxSizer(wxVERTICAL);
  powerPin1 = new wxCheckBox(powerForIDSizer->GetStaticBox(), wxID_ANY, "Enable Power Pin 1", wxDefaultPosition, wxDefaultSize, 0);
  powerPin2 = new wxCheckBox(powerForIDSizer->GetStaticBox(), wxID_ANY, "Enable Power Pin 2", wxDefaultPosition, wxDefaultSize, 0);
  powerPin3 = new wxCheckBox(powerForIDSizer->GetStaticBox(), wxID_ANY, "Enable Power Pin 3", wxDefaultPosition, wxDefaultSize, 0);
  powerPinLeftSizer->Add(powerPin1, MENUITEMFLAGS);
  powerPinLeftSizer->Add(powerPin2, MENUITEMFLAGS);
  powerPinLeftSizer->Add(powerPin3, MENUITEMFLAGS);
  wxBoxSizer* powerPinRightSizer = new wxBoxSizer(wxVERTICAL);
  powerPin4 = new wxCheckBox(powerForIDSizer->GetStaticBox(), wxID_ANY, "Enable Power Pin 4", wxDefaultPosition, wxDefaultSize, 0);
  powerPin5 = new wxCheckBox(powerForIDSizer->GetStaticBox(), wxID_ANY, "Enable Power Pin 5", wxDefaultPosition, wxDefaultSize, 0);
  powerPin6 = new wxCheckBox(powerForIDSizer->GetStaticBox(), wxID_ANY, "Enable Power Pin 6", wxDefaultPosition, wxDefaultSize, 0);
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
  continuousScans = new wxCheckBox(continuousScansSizer->GetStaticBox(), ID_ContinuousScan, "Enable Continuous Scanning", wxDefaultPosition, wxDefaultSize, 0);
  numIDTimes = Misc::createNumEntry(continuousScansSizer->GetStaticBox(), "Number of Reads to Average", wxID_ANY, 1, 50, 10);
  scanIDMillis = Misc::createNumEntry(continuousScansSizer->GetStaticBox(), "Scan Interval (ms)", wxID_ANY, 10, 50000, 1000);
  continuousScansSizer->Add(continuousScans, MENUITEMFLAGS);
  continuousScansSizer->Add(numIDTimes->box, MENUITEMFLAGS);
  continuousScansSizer->Add(scanIDMillis->box, MENUITEMFLAGS);

  return continuousScansSizer;
}


wxStaticBoxSizer* BladeIDPage::createBladeDetect(wxWindow* parent) {
  wxStaticBoxSizer* bladeDetectSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Blade Detect");

  detectPin = Misc::createTextEntry(bladeDetectSizer->GetStaticBox(), "Blade Detect Pin", wxID_ANY, "blade4Pin", 0);
  bladeDetectSizer->Add(detectPin->box, wxSizerFlags(0).Border(wxALL, 5).Expand());

  return bladeDetectSizer;
}

void BladeIDPage::update() {
  detectPin->entry->Enable(enableDetect->GetValue());
  mode->Enable(enableID->GetValue());
  IDPin->entry->Enable(enableID->GetValue());
  pullupPin->entry->Enable(enableID->GetValue());
  pullupResistance->num->Enable(enableID->GetValue());
  enablePowerForID->Enable(enableID->GetValue());
  powerPin1->Enable(enableID->GetValue());
  powerPin2->Enable(enableID->GetValue());
  powerPin3->Enable(enableID->GetValue());
  powerPin4->Enable(enableID->GetValue());
  powerPin5->Enable(enableID->GetValue());
  powerPin6->Enable(enableID->GetValue());
  continuousScans->Enable(enableID->GetValue());
  numIDTimes->num->Enable(enableID->GetValue());
  scanIDMillis->num->Enable(enableID->GetValue());
  addID->Enable(enableID->GetValue());
  removeID->Enable(enableID->GetValue());

  if (arrayList->GetSelection() != -1 && bladeArrays[arrayList->GetSelection()].name != "NO_BLADE") {
    arrayName->entry->Enable();
    resistanceID->num->Enable();
  } else {
    arrayName->entry->Disable();
    resistanceID->num->Disable();
  }

  if (mode->GetValue() == BLADE_ID_MODE_SNAPSHOT) {
    pullupResistance->box->Show(false);
    pullupPin->box->Show(false);
  } else if (mode->GetValue() == BLADE_ID_MODE_BRIDGED) {
    pullupResistance->box->Show(false);
    pullupPin->box->Show(true);
  } else if (mode->GetValue() == BLADE_ID_MODE_EXTERNAL) {
    pullupPin->box->Show(false);
    pullupResistance->box->Show(true);
  }

  UPDATEWINDOW;
}
