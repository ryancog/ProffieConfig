// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2024 Ryan Ogurek

#include "editor/pages/propspage.h"

#include "core/defines.h"
#include "core/appstate.h"
#include "core/utilities/misc.h"
#include "core/config/propfile.h"
#include "editor/editorwindow.h"
#include "editor/pages/generalpage.h"
#include "ui/pcchoice.h"

#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/tooltip.h>

PropsPage::PropsPage(wxWindow* window) : wxStaticBoxSizer(wxVERTICAL, window, ""), parent{static_cast<EditorWindow*>(window)} {
  auto top = new wxBoxSizer(wxHORIZONTAL);
  propSelection = new pcChoice(GetStaticBox(), ID_PropSelect, "Prop File", wxDefaultPosition, wxDefaultSize, Misc::createEntries({"Default"}), wxCB_READONLY);
  // Two ampersands bc wxWidgets formatting
  propInfo = new wxButton(GetStaticBox(), ID_PropInfo, "Prop Description && Usage Info...");
  buttonInfo = new wxButton(GetStaticBox(), ID_Buttons, "Button Controls...");
  TIP(propInfo, "View prop creator-provided information about this prop and its intended usage.");
  TIP(buttonInfo, "View button controls based on specific option settings and number of buttons.");
  top->Add(propSelection, wxSizerFlags(0).Border(wxALL, 10));
  top->Add(propInfo, wxSizerFlags(0).Border(wxALL, 10).Bottom());
  top->Add(buttonInfo, wxSizerFlags(0).Border(wxALL, 10).Bottom());

  propsWindow = new wxScrolledWindow(GetStaticBox(), wxID_ANY);
  auto propsSizer = new wxBoxSizer(wxVERTICAL);
  propsWindow->SetSizerAndFit(propsSizer);
  propsWindow->SetScrollbars(10, 10, -1, 1);

  Add(top);
  Add(propsWindow, wxSizerFlags(1).Expand());
  loadProps();
  bindEvents();

}

void PropsPage::bindEvents() {
  auto propSelectUpdate = [&](wxCommandEvent&) {
    updateSelectedProp();
    update();
    updateSizeAndLayout();
  };
  auto optionSelectUpdate = [&](wxCommandEvent&) {
    int32_t x, y;
    propsWindow->GetViewStart(&x, &y);
    update();
    propsWindow->Scroll(x, y);
    parent->Layout();
  };

  GetStaticBox()->Bind(wxEVT_CHOICE, propSelectUpdate, ID_PropSelect);
  propsWindow->Bind(wxEVT_CHECKBOX, optionSelectUpdate, wxID_ANY);
  propsWindow->Bind(wxEVT_RADIOBUTTON, optionSelectUpdate, wxID_ANY);
  propsWindow->Bind(wxEVT_SPINCTRL, optionSelectUpdate, wxID_ANY);
  propsWindow->Bind(wxEVT_SPINCTRLDOUBLE, optionSelectUpdate, wxID_ANY);

  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
      PropFile* activeProp{nullptr};
      for (auto& prop : props) {
        if (propSelection->entry()->GetStringSelection() == prop->getName()) activeProp = prop;
      }
      auto textSizer = new wxBoxSizer(wxVERTICAL);
      auto buttonDialog = wxDialog(
        parent,
        wxID_ANY,
        (activeProp ? activeProp->getName() : "Default") + " Buttons",
        wxDefaultPosition,
        wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP | wxRESIZE_BORDER
        );

      if (activeProp == nullptr) {
        auto buttons =
          (parent->generalPage->buttons->entry()->GetValue() == 0 ? wxString (
             "On/Off - Twist\n"
             "Next preset - Point up and shake\n"
             "Clash - Hit the blade while saber is on."
             ) : parent->generalPage->buttons->entry()->GetValue() == 1 ? wxString(
                  "On/Off - Click to turn the saber on or off.\n"
                  "Turn On muted - Double-click\n"
                  "Next preset - Hold button and hit the blade while saber is off.\n"
                  "Clash - Hit the blade while saber is on.\n"
                  "Lockup - Hold button, then trigger a clash. Release button to end.\n"
                  "Drag - Hold button, then trigger a clash while pointing down. Release button to end.\n"
                  "Melt - Hold button and stab something.\n"
                  "Force - Long-click button.\n"
                  "Start Soundtrack - Long-click the button while blade is off.\n"
                  "Enter/Exit Color Change - Hold button and Twist."
                  ) : parent->generalPage->buttons->entry()->GetValue() == 2 || parent->generalPage->buttons->entry()->GetValue() == 3 ? wxString (
                  "On/Off - Click POW\n"
                  "Turn On muted - Double-click POW button\n"
                  "Next preset - Hold POW button and hit the blade while saber is off.\n"
                  "Previous Preset - Hold AUX button and click the POW button while saber is off.\n"
                  "Clash - Hit the blade while saber is on.\n"
                  "Lockup -  Hold either POW or AUX, then trigger a clash. Release button to end.\n"
                  "Drag - Hold either POW or AUX, then trigger a clash while pointing down. Release button to end.\n"
                  "Melt - Hold either POW or AUX and stab something.\n"
                  "Force Lightning Block - Click AUX while holding POW.\n"
                  "Force - Long-click POW button.\n"
                  "Start Soundtrack - Long-click the POW button while blade is off.\n"
                  "Blaster block - Short-click AUX button.\n"
                  "Enter/Exit Color Change - Hold Aux and click POW while on."
                  ) : wxString("Button Configuration Not Supported"));
        textSizer->Add(new wxStaticText(&buttonDialog, wxID_ANY, buttons));
      } else {
        auto propButtons = activeProp->getButtons()->at(parent->generalPage->buttons->entry()->GetValue());

        if (propButtons.empty()) {
          textSizer->Add(new wxStaticText(&buttonDialog, wxID_ANY, "Selected number of buttons not supported by prop file."));
        } else for (auto& [ stateName, stateButtons ] : propButtons) {
            auto stateSizer = new wxStaticBoxSizer(wxVERTICAL, &buttonDialog, "Button controls while saber is " + stateName + ":");
            auto controlSizer = new wxBoxSizer(wxHORIZONTAL);
            auto buttonSizer = new wxBoxSizer(wxVERTICAL);
            auto actionSizer = new wxBoxSizer(wxVERTICAL);
            // Must use Spacer, not \t, which caused rendering issues for Windows
            controlSizer->AddSpacer(50);
            controlSizer->Add(buttonSizer);
            controlSizer->Add(actionSizer);
            stateSizer->Add(controlSizer);

            for (auto& button : stateButtons) {
              std::vector<std::string> activePredicates{};
              for (const auto& predicate : button.relevantSettings) {
                auto setting = activeProp->getSettings()->find(predicate);
                if (setting == activeProp->getSettings()->end()) continue;

                if (!setting->second.getOutput().empty()) activePredicates.push_back(setting->first);
              }

              auto key = button.descriptions.find(activePredicates);
              if (key != button.descriptions.end() && key->second != "DISABLED") {
                buttonSizer->Add(new wxStaticText(stateSizer->GetStaticBox(), wxID_ANY, button.name));
                actionSizer->Add(new wxStaticText(stateSizer->GetStaticBox(), wxID_ANY, " - " + key->second));
              }
            }

            if (actionSizer->IsEmpty()) {
              stateSizer->GetStaticBox()->Destroy();
              delete stateSizer;
              continue;
            }
            textSizer->Add(stateSizer, wxSizerFlags(0).Border(wxTOP | wxLEFT | wxRIGHT, 10).Expand());
          }
        textSizer->AddSpacer(10);
      }

      buttonDialog.SetSizerAndFit(textSizer);
      buttonDialog.DoLayoutAdaptation();
      buttonDialog.ShowModal();
    }, ID_Buttons);
  GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
      std::string info;

      PropFile* activeProp{nullptr};
      for (auto& prop : props) {
        if (propSelection->entry()->GetStringSelection() == prop->getName()) activeProp = prop;
      }

      if (activeProp == nullptr) {
        info = "The default ProffieOS prop file.";
      } else {
        info = activeProp->getInfo();
      }

      auto infoDialog = wxDialog(
        parent,
        wxID_ANY,
        propSelection->entry()->GetStringSelection() + " Prop Info",
        wxDefaultPosition,
        wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP
        );
      auto textSizer = new wxBoxSizer(wxVERTICAL);
      textSizer->Add(infoDialog.CreateTextSizer(info), wxSizerFlags(0).Border(wxALL, 10));
      infoDialog.SetSizer(textSizer);
      infoDialog.DoLayoutAdaptation();
      infoDialog.ShowModal();
    }, ID_PropInfo);
}

void PropsPage::updateProps() {
  auto lastSelect = propSelection->entry()->GetStringSelection();
  propSelection->entry()->Clear();
  propSelection->entry()->Append("Default");
  for (const auto& prop : props) {
    propSelection->entry()->Append(prop->getName());
  }
  if ([=]() { for (const auto& prop : propSelection->entry()->GetStrings()) if (prop == lastSelect) return true; return false; }()) {
    propSelection->entry()->SetStringSelection(lastSelect);
  } else propSelection->entry()->SetStringSelection("Default");
}

void PropsPage::update() {
  for (auto& prop : props) {
    prop->SetSize(0, 0);
    if (propSelection->entry()->GetStringSelection() != prop->getName()) continue;

    updateDisables(prop);

    for (auto& [ name, setting ] : *prop->getSettings()) {
      setting.enable(!setting.disabled && setting.checkRequiredSatisfied(*prop->getSettings()));
    }
  }
}
void PropsPage::updateSizeAndLayout() {
  propsWindow->SetSizerAndFit(propsWindow->GetSizer());
  parent->SetSizerAndFit(parent->sizer);
  parent->SetMinSize(wxSize(parent->GetSize().x, 350));
  parent->Refresh();
}

void PropsPage::updateDisables(PropFile* prop) {
  for (auto& [ name, setting ] : *prop->getSettings()) {
    setting.disabled = false;
  }
  for (auto& [ name, setting ] : *prop->getSettings()) {
    for (const auto& disable : setting.disables) {
      auto key = prop->getSettings()->find(disable);
      if (key == prop->getSettings()->end()) continue;

      key->second.disabled |= !setting.getOutput().empty();
    }
  }
}

const std::vector<PropFile*>& PropsPage::getLoadedProps() { return props; }
PropFile* PropsPage::getSelectedProp() {
  for (const auto& prop : props) {
    if (prop->getName() == propSelection->entry()->GetStringSelection()) return prop;
  }
  return nullptr;
}

void PropsPage::updateSelectedProp(const wxString& newProp) {
  if (!newProp.empty()) propSelection->entry()->SetStringSelection(newProp);
  for (auto& prop : props) {
    prop->Show(propSelection->entry()->GetStringSelection() == prop->getName());
  }
}
void PropsPage::loadProps() {
  props.clear();
  for (const auto& prop : AppState::instance->getPropFileNames()) {
    auto propConfig = PropFile::createPropConfig(prop, propsWindow);
    if (propConfig != nullptr) {
      propsWindow->GetSizer()->Add(propConfig);
      props.push_back(propConfig);
    }
  }
  updateProps();
}
