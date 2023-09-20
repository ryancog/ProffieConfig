#include "presetspage.h"

PresetsPage::PresetsPage(wxWindow* window) : wxStaticBoxSizer(wxHORIZONTAL, window, "General")
{
    wxBoxSizer *presetSelect = new wxBoxSizer(wxVERTICAL);
    settings.presetsEditor = new wxTextCtrl(GetStaticBox(), Misc::ID_PresetEditor, "", wxDefaultPosition, wxSize(400, 20), wxTE_MULTILINE);
    settings.presetsEditor->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    wxBoxSizer *presetsConfig = new wxBoxSizer(wxVERTICAL);

    // Preset Select
    {
        wxBoxSizer *presetLists = new wxBoxSizer(wxHORIZONTAL);

        settings.presetList = new wxListBox(GetStaticBox(), Misc::ID_PresetList);
        settings.bladeList = new wxListBox(GetStaticBox(), Misc::ID_BladeList);
        presetLists->Add(settings.presetList, wxSizerFlags(1).Expand());
        presetLists->Add(settings.bladeList, wxSizerFlags(1).Expand());

        wxBoxSizer *presetButtons = new wxBoxSizer(wxHORIZONTAL);
        settings.addPreset = new wxButton(GetStaticBox(), Misc::ID_AddPreset, "+", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
        settings.removePreset = new wxButton(GetStaticBox(), Misc::ID_RemovePreset, "-", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
        presetButtons->Add(settings.addPreset, wxSizerFlags(0).Border(wxRIGHT, 10));
        presetButtons->Add(settings.removePreset);

        presetSelect->Add(presetLists, wxSizerFlags(1));
        presetSelect->Add(presetButtons, wxSizerFlags(0).Border(wxLEFT | wxTOP | wxBOTTOM, 5));
    }

    // Preset Config
    {
        wxBoxSizer *name = new wxBoxSizer(wxVERTICAL);
        wxStaticText *nameLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Preset Name", wxDefaultPosition, wxSize(150, 20));
        settings.nameInput = new wxTextCtrl(GetStaticBox(), Misc::ID_PresetName);
        name->Add(nameLabel, wxSizerFlags(0).Border(wxLEFT | wxTOP, 10));
        name->Add(settings.nameInput, wxSizerFlags(0).Border(wxLEFT, 10).Expand());

        wxBoxSizer *dir = new wxBoxSizer(wxVERTICAL);
        wxStaticText *dirLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Font Directory", wxDefaultPosition, wxSize(150, 20));
        settings.dirInput = new wxTextCtrl(GetStaticBox(), Misc::ID_PresetDir);
        dir->Add(dirLabel, wxSizerFlags(0).Border(wxLEFT | wxTOP, 10));
        dir->Add(settings.dirInput, wxSizerFlags(0).Border(wxLEFT, 10).Expand());

        wxBoxSizer *track = new wxBoxSizer(wxVERTICAL);
        wxStaticText *trackLabel = new wxStaticText(GetStaticBox(), wxID_ANY, "Track File", wxDefaultPosition, wxSize(150, 20));
        settings.trackInput = new wxTextCtrl(GetStaticBox(), Misc::ID_PresetTrack);
        track->Add(trackLabel, wxSizerFlags(0).Border(wxLEFT | wxTOP, 10));
        track->Add(settings.trackInput, wxSizerFlags(0).Border(wxLEFT | wxBOTTOM, 10).Expand());

        presetsConfig->Add(name);
        presetsConfig->Add(dir);
        presetsConfig->Add(track);
    }

    Add(presetsConfig, wxSizerFlags(/*proportion*/ 0).Border(wxALL, 10));
    Add(presetSelect, wxSizerFlags(/*proportion*/ 0).Border(wxALL, 10).Expand());
    Add(settings.presetsEditor, wxSizerFlags(/*proportion*/ 1).Border(wxALL, 10).Expand());
}

void PresetsPage::update() {
    int32_t presetIndex = settings.presetList->GetSelection();
    int32_t bladeIndex = settings.bladeList->GetSelection();
    int32_t listSelection = presetIndex;
    settings.presetList->Clear();
    for (Configuration::presetConfig preset : Configuration::presets) {
        settings.presetList->Append(preset.name);
    }
    if ((int32_t)settings.presetList->GetCount() - 1 < listSelection) listSelection -= 1;
    if (listSelection >= 0) settings.presetList->SetSelection(listSelection);

    listSelection = bladeIndex;
    settings.bladeList->Clear();
    for (uint32_t blade = 0; blade < Configuration::blades.size(); blade++) {
        if (Configuration::blades[blade].subBlades.size() > 0) {
            for (uint32_t subBlade = 0; subBlade < Configuration::blades[blade].subBlades.size(); subBlade++) {
                settings.bladeList->Append("Blade " + std::to_string(blade) + ":" + std::to_string(subBlade));
            }
        } else {
            settings.bladeList->Append("Blade " + std::to_string(blade));
        }
    }
    if ((int32_t)settings.bladeList->GetCount() - 1 < listSelection) listSelection -= 1;
    if (listSelection >= 0) settings.bladeList->SetSelection(listSelection);

    for (Configuration::presetConfig &preset : Configuration::presets) {
        // Calculate # of presets there should be prior.
        int32_t numBlades = 0;
        for (Configuration::bladeConfig blade : Configuration::blades) {
            numBlades += blade.subBlades.size() > 0 ? blade.subBlades.size() : 1;
        }

        if (numBlades > (int32_t)preset.styles.size()) {
            while (numBlades != (int32_t)preset.styles.size()) {
                preset.styles.push_back("StylePtr<Black>()");
            }
        } else if (numBlades < (int32_t)preset.styles.size()) {
            while (numBlades != (int32_t)preset.styles.size()) {
                preset.styles.pop_back();
            }
        }
    }

    presetIndex = settings.presetList->GetSelection();
    bladeIndex = settings.bladeList->GetSelection();
    if (presetIndex >= 0) {
        if (bladeIndex >= 0) settings.presetsEditor->ChangeValue(wxString::FromUTF8(Configuration::presets[presetIndex].styles[bladeIndex]));
        else settings.presetsEditor->ChangeValue(wxString::FromUTF8("Select Blade to Edit Style..."));
        settings.nameInput->ChangeValue(wxString::FromUTF8(Configuration::presets[presetIndex].name));
        settings.dirInput->ChangeValue(wxString::FromUTF8(Configuration::presets[presetIndex].dirs));
        settings.trackInput->ChangeValue(wxString::FromUTF8(Configuration::presets[presetIndex].track));
    }
    else {
        settings.presetsEditor->ChangeValue(wxString::FromUTF8(""));
        settings.nameInput->ChangeValue(wxString::FromUTF8(""));
        settings.dirInput->ChangeValue(wxString::FromUTF8(""));
        settings.trackInput->ChangeValue(wxString::FromUTF8(""));
    }

}

decltype(PresetsPage::settings) PresetsPage::settings;
