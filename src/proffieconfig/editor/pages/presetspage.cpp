#include "presetspage.h"
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "ui/message.h"
#include "paths/paths.h"
#include "../../core/defines.h"
#include "../../core/utilities/misc.h"
#include "../editorwindow.h"
#include "../dialogs/bladearraydlg.h"
#include "wx/gdicmn.h"

#include <wx/msgdlg.h>
#include <wx/splitter.h>

#include <wx/tooltip.h>
#ifdef __WXGTK__
#include <wx/clipbrd.h>
#endif

PresetsPage::PresetsPage(wxWindow *window) : wxStaticBoxSizer(wxHORIZONTAL, window), mParent(static_cast<EditorWindow *>(window)) {
    Add(createPresetConfig(), wxSizerFlags(/*proportion*/ 0).Border(wxALL, 10));
    Add(createPresetSelect(), wxSizerFlags(/*proportion*/ 0).Border(wxTOP | wxRIGHT | wxBOTTOM, 10).Expand());

    auto *styleCommentSplit{new wxSplitterWindow(GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH | wxSP_LIVE_UPDATE)};
    commentInput = new PCUI::Text(styleCommentSplit, ID_PresetChange, {},  wxTE_MULTILINE | wxNO_BORDER, _("Comments"));

    styleInput = new PCUI::Text(styleCommentSplit, ID_PresetChange, {}, wxTE_DONTWRAP | wxTE_MULTILINE | wxNO_BORDER, _("Blade Style"));
    styleInput->entry()->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    styleCommentSplit->SetMinSize(wxSize{500, -1});
    styleCommentSplit->SetMinimumPaneSize(60);
    styleCommentSplit->SplitHorizontally(commentInput, styleInput);
    Add(styleCommentSplit, wxSizerFlags(1).Border(wxALL, 10).Expand());

    bindEvents();
    createToolTips();
}

void PresetsPage::bindEvents() {
    GetStaticBox()->Bind(wxEVT_CHOICE, [&](wxCommandEvent&) { 
        mParent->bladesPage->bladeArray->entry()->SetSelection(bladeArrayChoice->entry()->GetSelection());
        update(); 
    }, ID_BladeArray);

    GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) { mParent->bladesPage->update(); update(); }, ID_BladeList);
    GetStaticBox()->Bind(wxEVT_LISTBOX, [&](wxCommandEvent&) { mParent->bladesPage->update(); update(); }, ID_PresetList);

    GetStaticBox()->Bind(wxEVT_TEXT, [&](wxCommandEvent&) { update(); }, ID_PresetChange);
#   ifdef __WXGTK__ // GTK processes double events, leading to crash it seems... this is a workaround.
    GetStaticBox()->Bind(wxEVT_TEXT_PASTE, [&](wxClipboardTextEvent&) {
        if (!wxClipboard::Get()->Open()) return;

        wxTextDataObject data;
        wxClipboard::Get()->GetData(data);
        wxClipboard::Get()->Close();

        auto styleText = styleInput->entry()->GetValue();
        long selectionStart, selectionEnd;
        styleInput->entry()->GetSelection(&selectionStart, &selectionEnd);

        if (selectionStart != selectionEnd) styleText.erase(selectionStart, selectionEnd);
        styleText.insert(selectionStart, data.GetText());

        styleInput->entry()->SetModified(true);
        styleInput->entry()->SetValue(styleText);
        styleInput->entry()->SetInsertionPoint(selectionStart + data.GetText().size());
    }, ID_PresetChange);
#   endif
    GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()].presets.emplace_back();
        mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()].presets[mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()].presets.size() - 1].name = "newpreset";

        mParent->bladesPage->update();
        update();
    }, ID_AddPreset);
    GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        if (presetList->GetSelection() >= 0) {
            mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()].presets.erase(std::next(mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()].presets.begin(), mParent->presetsPage->presetList->GetSelection()));

            mParent->bladesPage->update();
            update();
        }
    }, ID_RemovePreset);
    GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        auto tempStore = mParent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArrayChoice->entry()->GetSelection()).presets.at(presetList->GetSelection());
        mParent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArrayChoice->entry()->GetSelection()).presets.at(presetList->GetSelection()) = mParent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArrayChoice->entry()->GetSelection()).presets.at(presetList->GetSelection() - 1);
        mParent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArrayChoice->entry()->GetSelection()).presets.at(presetList->GetSelection() - 1) = tempStore;
        presetList->SetSelection(presetList->GetSelection() - 1);
        update();
    }, ID_MovePresetUp);
    GetStaticBox()->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        auto tempStore = mParent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArrayChoice->entry()->GetSelection()).presets.at(presetList->GetSelection());
        mParent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArrayChoice->entry()->GetSelection()).presets.at(presetList->GetSelection()) = mParent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArrayChoice->entry()->GetSelection()).presets.at(presetList->GetSelection() + 1);
        mParent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArrayChoice->entry()->GetSelection()).presets.at(presetList->GetSelection() + 1) = tempStore;
        presetList->SetSelection(presetList->GetSelection() + 1);
        update();
    }, ID_MovePresetDown);
}

void PresetsPage::createToolTips() const {
    TIP(nameInput, _(
        "The name for the preset.\n"
        "This appears on the OLED screen if no bitmap is supplied, otherwise it's just for reference.\n"
        "Using \"\\n\" is like hitting \"enter\" when the text is displayed on the OLED.\n"
        "For example, \"my\\npreset\" will be displayed on the OLED as two lines, the first being \"my\" and the second being \"preset\"."
    ));
    TIP(dirInput, _(
        "The path of the folder on the SD card where the font is stored.\n"
        "If the font folder is inside another folder, it must be indicated by something like \"folderName/fontFolderName\".\n"
        "In order to specify multiple directories (for example, to inlclude a \"common\" directory), use a semicolon (;) to seperate the folders (e.g. \"fontFolderName;common\")."
    ));
    TIP(trackInput, _(
        "The path of the track file on the SD card.\n"
        "If the track is directly inside one of the folders specified in \"Font Directory\" then only the name of the track file is required."
    ));
    TIP(bladeArrayChoice, _("The currently-selected blade array to be edited.\nEach blade array has unique presets."));
    TIP(presetList, _("All presets in this blade array.\nSelect a preset and blade to edit associated blade styles."));
    TIP(bladeList, _("All blades in this blade array.\nSelect a preset and blade to edit associated blade styles."));

    TIP(addPreset, _("Add a preset to the currently-selected blade array."));
    TIP(removePreset, _("Delete the currently-selected preset."));

    TIP(commentInput, _(
        "Any comments about the blade style goes here.\n"
        "This doesn't affect the blade style at all, but can be a place for helpful notes!"
    ));
    TIP(styleInput, _(
        "Your blade style goes here.\n"
        "This is the code which sets up what animations and effects your blade (or other LED) will do.\n"
        "For getting/creating blade styles, see the Documentation (in \"Help->Documentation...\")."
    ));
}

wxBoxSizer* PresetsPage::createPresetSelect() {
    auto *presetSelect{new wxBoxSizer(wxVERTICAL)};

    auto * arraySizer{new wxBoxSizer(wxVERTICAL)};
    bladeArrayChoice = new PCUI::Choice(GetStaticBox(), ID_BladeArray, Misc::createEntries({ "blade_in" }), _("Blade Array"));
    arraySizer->Add(bladeArrayChoice, wxSizerFlags(0).Border(wxBOTTOM, 5).Expand());

    auto *listSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *arrangeButtonSizer{new wxBoxSizer(wxVERTICAL)};
    movePresetUp = new wxButton(GetStaticBox(), ID_MovePresetUp, L"\u2191" /*up arrow*/, wxDefaultPosition, wxSize(15, 25), wxBU_EXACTFIT);
    movePresetDown = new wxButton(GetStaticBox(), ID_MovePresetDown, L"\u2193" /*down arrow*/, wxDefaultPosition, wxSize(15, 25), wxBU_EXACTFIT);
    arrangeButtonSizer->Add(movePresetUp, FIRSTITEMFLAGS);
    arrangeButtonSizer->Add(movePresetDown, MENUITEMFLAGS);
    listSizer->Add(arrangeButtonSizer, wxSizerFlags(0));
    auto *presetSizer{new wxBoxSizer(wxVERTICAL)};
    presetList = new wxListBox(GetStaticBox(), ID_PresetList, wxDefaultPosition, wxDefaultSize, wxArrayString{}, wxBORDER_NONE);
    presetSizer->Add(new wxStaticText(GetStaticBox(), wxID_ANY, _("Presets")));
    presetSizer->Add(presetList, wxSizerFlags(1).Expand());
    auto *bladeSizer{new wxBoxSizer(wxVERTICAL)};
    bladeList = new wxListBox(GetStaticBox(), ID_BladeList, wxDefaultPosition, wxDefaultSize, wxArrayString{}, wxBORDER_NONE);
    bladeSizer->Add(new wxStaticText(GetStaticBox(), wxID_ANY, _("Blades")));
    bladeSizer->Add(bladeList, wxSizerFlags(1).Expand());
    listSizer->Add(presetSizer, wxSizerFlags(1).Expand().Border(wxALL, 4));
    listSizer->Add(bladeSizer, wxSizerFlags(1).Expand().Border(wxALL, 4));

    auto* buttonSizer{new wxBoxSizer(wxHORIZONTAL)};
    addPreset = new wxButton(GetStaticBox(), ID_AddPreset, "+", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
    removePreset = new wxButton(GetStaticBox(), ID_RemovePreset, "-", wxDefaultPosition, SMALLBUTTONSIZE, wxBU_EXACTFIT);
    buttonSizer->Add(addPreset, wxSizerFlags(0).Border(wxRIGHT | wxTOP, 5));
    buttonSizer->Add(removePreset, wxSizerFlags(0).Border(wxTOP, 5));

    presetSelect->Add(arraySizer, wxSizerFlags(0).Expand().Border(wxLEFT, 25));
    presetSelect->Add(listSizer, wxSizerFlags(1).Expand());
    presetSelect->Add(buttonSizer, wxSizerFlags(0).Border(wxLEFT, 30));

    return presetSelect;
}

wxBoxSizer* PresetsPage::createPresetConfig() {
    auto *presetConfig{new wxBoxSizer(wxVERTICAL)};

    nameInput = new PCUI::Text(GetStaticBox(), ID_PresetChange, {}, 0, _("Preset Name"));
    nameInput->SetMinSize(wxSize(200, -1));
    dirInput = new PCUI::Text(GetStaticBox(), ID_PresetChange,   {}, 0, _("Font Directory"));
    dirInput->SetMinSize(wxSize(200, -1));
    trackInput = new PCUI::Text(GetStaticBox(), ID_PresetChange, {}, 0, _("Track File"));
    trackInput->SetMinSize(wxSize(200, -1));
    injectionsSizer = new wxBoxSizer(wxVERTICAL);

    presetConfig->Add(nameInput, wxSizerFlags(0).Border(wxLEFT | wxTOP | wxBOTTOM, 10).Expand());
    presetConfig->Add(dirInput, wxSizerFlags(0).Border(wxLEFT | wxTOP | wxBOTTOM, 10).Expand());
    presetConfig->Add(trackInput, wxSizerFlags(0).Border(wxLEFT | wxTOP | wxBOTTOM, 10).Expand());
    presetConfig->Add(injectionsSizer, wxSizerFlags(1).Border(wxLEFT | wxTOP | wxBOTTOM, 10).Expand());

    rebuildInjections();

    return presetConfig;
}

void PresetsPage::rebuildInjections() {
    injectionsSizer->Clear(true);

    if (injections.empty()) return;

    auto *injectionsText{new wxStaticText(GetStaticBox(), wxID_ANY, _("Injections"))};
    injectionsSizer->Add(injectionsText, wxSizerFlags().Border(wxLEFT | wxTOP, 10));

    for (const auto& injection : injections) {
        auto *injectionSizer{new wxBoxSizer(wxHORIZONTAL)};
        auto *injectionText{new wxStaticText(GetStaticBox(), wxID_ANY, injection)};
        auto *editButton{new wxButton(GetStaticBox(), wxID_ANY, _("Edit"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)};
        auto *deleteButton{new wxButton(GetStaticBox(), wxID_ANY, _("Delete"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)};

        editButton->Bind(wxEVT_BUTTON, [=](wxCommandEvent&) {
            wxLaunchDefaultApplication((Paths::injections() / injection).native());
        });

        deleteButton->Bind(wxEVT_BUTTON, [this, injection](wxCommandEvent&) {
            if (wxNO == PCUI::showMessage(_("This action cannot be undone!"), _("Delete Injection"), wxYES_NO | wxNO_DEFAULT)) return;

            injections.erase(std::find(injections.begin(), injections.end(), injection));
            update();
        });

        injectionSizer->Add(injectionText, wxSizerFlags(1).Center());
        injectionSizer->AddSpacer(20);
        injectionSizer->Add(editButton);
        injectionSizer->AddSpacer(10);
        injectionSizer->Add(deleteButton);

        injectionsSizer->Add(injectionSizer, wxSizerFlags().Border(wxTOP | wxLEFT, 10));
    }

    injectionsSizer->Layout();
    Layout();
}

void PresetsPage::update() {
    pushIfNewPreset();
    resizeAndFillPresets();

    if (nameInput->entry()->IsModified()) stripAndSaveName();
    if (dirInput->entry()->IsModified()) stripAndSaveDir();
    if (trackInput->entry()->IsModified()) stripAndSaveTrack();
    if (styleInput->entry()->IsModified()) stripAndSaveEditor();
    if (commentInput->entry()->IsModified()) stripAndSaveComments();
    rebuildInjections();

    rebuildBladeArrayList();
    rebuildPresetList();
    rebuildBladeList();

    updateFields();
}

void PresetsPage::pushIfNewPreset() {
    if (presetList->GetSelection() == -1 && mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()].blades.size() > 0 && (!nameInput->entry()->IsEmpty() || !dirInput->entry()->IsEmpty() || !trackInput->entry()->IsEmpty())) {
        mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()].presets.emplace_back();
        rebuildPresetList();
        presetList->SetSelection(static_cast<int32>(mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()].presets.size()) - 1);
        bladeList->SetSelection(0);
    }
}

void PresetsPage::rebuildBladeArrayList() {
    int32_t arraySelection = bladeArrayChoice->entry()->GetSelection();
    bladeArrayChoice->entry()->Clear();
    for (const BladeArrayDlg::BladeArray& array : mParent->bladesPage->bladeArrayDlg->bladeArrays) {
        bladeArrayChoice->entry()->Append(array.name);
    }
    if (arraySelection >= 0 && arraySelection < static_cast<int32_t>(bladeArrayChoice->entry()->GetCount())) bladeArrayChoice->entry()->SetSelection(arraySelection);
    else bladeArrayChoice->entry()->SetSelection(0);
}

void PresetsPage::rebuildPresetList() {
    int32_t listSelection = presetList->GetSelection();
    presetList->Clear();
    for (const PresetConfig& preset : mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()].presets) {
        presetList->Append(preset.name);
    }
    if (static_cast<int32_t>(presetList->GetCount()) - 1 < listSelection) listSelection -= 1;
    if (listSelection >= 0 && listSelection < static_cast<int32_t>(presetList->GetCount())) presetList->SetSelection(listSelection);
    else if (presetList->GetCount()) presetList->SetSelection(0);
}

void PresetsPage::rebuildBladeList() {
    int32_t listSelection = bladeList->GetSelection();
    bladeList->Clear();

    auto& bladeArray{mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()]};
    for (auto bladeIdx{0}; bladeIdx < bladeArray.blades.size(); ++bladeIdx) {
        auto& blade{bladeArray.blades[bladeIdx]};

        if (blade.subBlades.size() > 0) {
            for (auto subBladeIdx{0}; subBladeIdx < blade.subBlades.size(); ++subBladeIdx) {
                bladeList->Append(wxString::Format(_("Blade %d:%d"), bladeIdx, subBladeIdx));
            }
        } else {
            bladeList->Append(wxString::Format(_("Blade %d"), bladeIdx));
        }
    }
    if (static_cast<int32_t>(bladeList->GetCount()) - 1 < listSelection) listSelection -= 1;
    if (listSelection >= 0) bladeList->SetSelection(listSelection);
}

void PresetsPage::resizeAndFillPresets() {
    auto getNumBlades = [&]() {
        int32 numBlades{0};
        for (const BladesPage::BladeConfig& blade : mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()].blades) {
            numBlades += blade.subBlades.size() > 0 ? static_cast<int32>(blade.subBlades.size()) : 1;
        }
        return numBlades;
    };

    for (PresetConfig& preset : mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()].presets) {
        while (static_cast<int32_t>(preset.styles.size()) < getNumBlades()) {
            preset.styles.push_back({ {}, "StyleNormalPtr<AudioFlicker<Blue,DodgerBlue>,BLUE,300,800>()" });
        }
        while (static_cast<int32_t>(preset.styles.size()) > getNumBlades()) {
            preset.styles.pop_back();
        }
    }
}

void PresetsPage::updateFields() {
    if (presetList->GetSelection() >= 0) {
        const auto& currentPreset = mParent->bladesPage->bladeArrayDlg->bladeArrays.at(bladeArrayChoice->entry()->GetSelection()).presets.at(presetList->GetSelection());
        int32 insertionPoint{};

        if (bladeList->GetSelection() >= 0) {
            const auto& style{currentPreset.styles.at(bladeList->GetSelection())};

            insertionPoint = styleInput->entry()->GetInsertionPoint();
            styleInput->entry()->ChangeValue(style.style);
            styleInput->entry()->SetInsertionPoint(std::min<int32>(insertionPoint, styleInput->entry()->GetLastPosition()));

            insertionPoint = commentInput->entry()->GetInsertionPoint();
            commentInput->entry()->ChangeValue(style.comment);
            commentInput->entry()->SetInsertionPoint(std::min<int32>(insertionPoint, commentInput->entry()->GetLastPosition()));
        } else {
            commentInput->entry()->ChangeValue(_("Select blade to edit style comments..."));
            styleInput->entry()->ChangeValue(_("Select blade to edit style..."));
        }

        insertionPoint = nameInput->entry()->GetInsertionPoint();
        nameInput->entry()->ChangeValue(currentPreset.name);
        nameInput->entry()->SetInsertionPoint(std::min<int32>(insertionPoint, static_cast<int32>(nameInput->entry()->GetValue().size())));

        insertionPoint = dirInput->entry()->GetInsertionPoint();
        dirInput->entry()->ChangeValue(currentPreset.dirs);
        dirInput->entry()->SetInsertionPoint(std::min<int32>(insertionPoint, static_cast<int32>(dirInput->entry()->GetValue().size())));

        insertionPoint = trackInput->entry()->GetInsertionPoint();
        trackInput->entry()->ChangeValue(currentPreset.track);
        auto trackInputLength{static_cast<int32>(trackInput->entry()->GetValue().size() - 4 /* .wav */)};
        trackInput->entry()->SetInsertionPoint(std::min(insertionPoint, trackInputLength));
    }
    else {
        commentInput->entry()->ChangeValue(_("Select or create preset and blade to edit style comments..."));
        styleInput->entry()->ChangeValue(_("Select or create preset and blade to edit style..."));
        nameInput->entry()->ChangeValue({});
        dirInput->entry()->ChangeValue({});
        trackInput->entry()->ChangeValue({});
    }

    commentInput->entry()->Enable(presetList->GetSelection() != -1 and bladeList->GetSelection() != -1);
    styleInput->entry()->Enable(presetList->GetSelection() != -1 and bladeList->GetSelection() != -1);
    removePreset->Enable(presetList->GetSelection() != -1);
    movePresetDown->Enable(presetList->GetSelection() != -1 && presetList->GetSelection() < static_cast<int32_t>(presetList->GetCount()) - 1);
    movePresetUp->Enable(presetList->GetSelection() > 0);

    // Value is flagged as dirty from last change unless we manually reset it, causing overwrites where there shouldn't be.
    styleInput->entry()->SetModified(false);
    commentInput->entry()->SetModified(false);
    nameInput->entry()->SetModified(false);
    dirInput->entry()->SetModified(false);
    trackInput->entry()->SetModified(false);
}

void PresetsPage::stripAndSaveComments() {
    if (presetList->GetSelection() >= 0 && bladeList->GetSelection() >= 0) {
        auto comments{commentInput->entry()->GetValue().ToStdString()};

        size_t illegalStrPos{0};
        while ((illegalStrPos = comments.find("/*")) != string::npos) comments.erase(illegalStrPos, 2);
        while ((illegalStrPos = comments.find("*/")) != string::npos) comments.erase(illegalStrPos, 2);
        while ((illegalStrPos = comments.find("//")) != string::npos) comments.erase(illegalStrPos, 2);

        auto& selectedBladeArray{mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()]};
        auto& selectedPreset{selectedBladeArray.presets[presetList->GetSelection()]};

        selectedPreset.styles[bladeList->GetSelection()].comment = comments;
    }
}

void PresetsPage::stripAndSaveEditor() {
    if (presetList->GetSelection() >= 0 && bladeList->GetSelection() >= 0) {
        auto style{styleInput->entry()->GetValue().ToStdString()};
        if (style.find('{') != string::npos) style.erase(std::remove(style.begin(), style.end(), '{'), style.end());
        if (style.rfind('}') != string::npos) style.erase(std::remove(style.begin(), style.end(), '}'), style.end());
        if (style.rfind(')') != string::npos) style.erase(style.rfind(')') + 1);

        auto& selectedBladeArray{mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()]};
        auto& selectedPreset{selectedBladeArray.presets[presetList->GetSelection()]};

        selectedPreset.styles[bladeList->GetSelection()].style = style;
    }
}

void PresetsPage::stripAndSaveName() {
    if (presetList->GetSelection() >= 0 && mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()].blades.size() > 0) {
        auto name{nameInput->entry()->GetValue().ToStdString()};
        name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char chr){ return std::tolower(chr); }); // to lowercase
        mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()].presets.at(presetList->GetSelection()).name.assign(name);
    }
}

void PresetsPage::stripAndSaveDir() {
    if (presetList->GetSelection() >= 0 && mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()].blades.size() > 0) {
        auto dir{dirInput->entry()->GetValue().ToStdString()};
        // dir.erase(std::remove(dir.begin(), dir.end(), ' '), dir.end());
        mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()].presets.at(presetList->GetSelection()).dirs.assign(dir);
    }
}

void PresetsPage::stripAndSaveTrack() {
    auto track{trackInput->entry()->GetValue().ToStdString()};
    track.erase(std::remove(track.begin(), track.end(), ' '), track.end());
    if (track.find('.') != string::npos) track.erase(track.find('.'));
    if (track.length() > 0) track += ".wav";

    if (presetList->GetSelection() >= 0 && mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()].blades.size() > 0) {
        mParent->bladesPage->bladeArrayDlg->bladeArrays[bladeArrayChoice->entry()->GetSelection()].presets.at(presetList->GetSelection()).track.assign(track);
    } else {
        trackInput->entry()->ChangeValue(track);
        trackInput->entry()->SetInsertionPoint(1);
    }
}
