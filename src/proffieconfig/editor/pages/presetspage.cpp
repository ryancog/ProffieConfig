#include "presetspage.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2023-2026 Ryan Ogurek
 *
 * proffieconfig/editor/pages/presetspage.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <wx/gdicmn.h>
#include <wx/msgdlg.h>
#include <wx/splitter.h>
#include <wx/statbox.h>
#include <wx/tooltip.h>

#include "config/preset/array.h"
#include "ui/controls/button.h"
#include "ui/message.h"
#include "utils/paths.h"

#include "../editorwindow.h"

class RenameArrayDlg : public wxDialog, pcui::NotifyReceiver {
public:
    RenameArrayDlg(
        wxWindow *parent,
        Config::Config& config,
        Config::PresetArray& array,
        const wxString& title,
        bool buttons = false
    ) : wxDialog(parent, wxID_ANY, title),
        NotifyReceiver(this, array.notifyData),
        mConfig{config}, mArray{array} {
        auto *sizer{new wxBoxSizer(wxVERTICAL)};
        auto *entry{new pcui::Text(
            this,
            array.name,
            0,
            false,
            _("Name")
        )};
        auto *emptyText{new wxStaticText(
            this,
            eID_Empty_Text,
            _("Empty Array Name")
        )};
        auto *dupText{new wxStaticText(
            this,
            eID_Dup_Text,
            _("Duplicate Array Name")
        )};
        sizer->Add(
            entry,
            wxSizerFlags().Expand().Border(wxTOP | wxLEFT | wxRIGHT, 10)
        );
        sizer->Add(
            emptyText,
            wxSizerFlags().Right().Border(wxALL, 5)
                .DoubleBorder(wxRIGHT)
        );
        sizer->Add(
            dupText,
            wxSizerFlags().Right().Border(wxALL, 5)
                .DoubleBorder(wxRIGHT)
        );
        if (buttons) {
            sizer->Add(
                CreateStdDialogButtonSizer(wxOK | wxCANCEL),
                wxSizerFlags().Expand().Border(wxTOP | wxBOTTOM, 10)
            );
        } else {
            sizer->AddSpacer(10);
        }

        entry->SetFocus();

        Bind(wxEVT_CHAR_HOOK, [this](wxKeyEvent& evt) {
            if (evt.GetKeyCode() == WXK_ESCAPE) EndModal(wxID_CANCEL);

            auto *okButton{FindWindow(wxID_OK)};
            if (
                    (not okButton or okButton->IsEnabled()) and 
                    evt.GetKeyCode() == WXK_RETURN and
                    evt.GetKeyCode() == WXK_NUMPAD_ENTER
               ) {
                EndModal(wxID_OK);
            }

            evt.Skip();
        });

        SetSizer(sizer);
        initializeNotifier();
    }

private:
    Config::Config& mConfig;
    Config::PresetArray& mArray;

    enum {
        eID_Empty_Text,
        eID_Dup_Text,
    };

    void handleNotification(uint32) final {
        bool duplicate{false};
        for (const auto& otherArray : mConfig.presetArrays.arrays()) {
            if (&*otherArray == &mArray) continue;

            const auto otherArrayName{static_cast<string>(otherArray->name)};
            const auto arrayName{static_cast<string>(mArray.name)};
            if (arrayName == otherArrayName) {
                duplicate = true;
                break;
            }
        }
        bool empty{static_cast<string>(mArray.name).empty()};

        FindWindow(eID_Empty_Text)->Show(empty);
        FindWindow(eID_Dup_Text)->Show(duplicate);
        auto *okButton{FindWindow(wxID_OK)};
        if (okButton) okButton->Enable(not duplicate and not empty);

        SetMinSize({300, -1});
        Layout();
        Fit();
    }
};

PresetsPage::PresetsPage(EditorWindow *window) : 
    wxPanel(window),
    pcui::NotifyReceiver(
        this,
        window->getOpenConfig().presetArrays.notifyData
    ),
    mParent(window) {
    createUI();
    bindEvents();
    
    initializeNotifier();
}

void PresetsPage::createUI() {
    auto *sizer{new wxBoxSizer(wxHORIZONTAL)};
    auto& config{mParent->getOpenConfig()};

    auto *presetSelectionSizer{new wxBoxSizer(wxVERTICAL)};

    auto *arraySectSizer{new pcui::StaticBox(
        wxVERTICAL,
        this,
        _("Presets Array")
    )};
    auto *arraySizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *arraySelection{new pcui::Choice(
        arraySectSizer->childParent(),
        config.presetArrays.selection
    )};
    auto *issueButton{new wxButton(
        arraySectSizer->childParent(),
        ID_IssueButton,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    auto *arrayRename{new pcui::Button(
        arraySectSizer->childParent(),
        ID_RenameArray,
        wxEmptyString,
        wxDefaultSize,
        wxBU_EXACTFIT,
        "edit",
        {-1, 16},
        wxSYS_COLOUR_WINDOWTEXT
    )};
    arraySizer->Add(arraySelection, wxSizerFlags(1));
    arraySizer->Add(issueButton, wxSizerFlags().Border(wxLEFT, 5).Bottom());
    arraySizer->AddSpacer(5);
    arraySizer->Add(arrayRename, wxSizerFlags().Bottom());

    auto *arrayButtonsSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *addArray{new wxButton(
        arraySectSizer->childParent(),
        ID_AddArray,
        _("Add"),
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    auto *removeArray{new wxButton(
        arraySectSizer->childParent(),
        ID_RemoveArray,
        _("Remove"),
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    arrayButtonsSizer->Add(addArray, wxSizerFlags(2));
    arrayButtonsSizer->AddSpacer(5);
    arrayButtonsSizer->Add(removeArray, wxSizerFlags(3));

    auto *presetSectSizer{new pcui::StaticBox(
        wxVERTICAL,
        this,
        _("Presets")
    )};
    auto *presetListSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *presetList{new pcui::List(
        presetSectSizer->childParent(),
        config.presetArrays.presetProxy
    )};
    presetList->SetMinSize(wxSize{-1, 300});
    presetList->SetToolTip(_("The currently-selected preset array to be edited.\nEach preset array has unique presets."));

    auto *arrangeButtonsSizer{new wxBoxSizer(wxVERTICAL)};
#   ifdef __WXOSX__
    wxSize arrangeButtonSize{20, 25};
#   else
    wxSize arrangeButtonSize{15, 25};
#   endif
    auto *movePresetUp = new wxButton(
        presetSectSizer->childParent(),
        ID_MovePresetUp,
        L"\u2191" /*up arrow*/,
        wxDefaultPosition,
        arrangeButtonSize,
        wxBU_EXACTFIT
    );
    auto *movePresetDown{new wxButton(
        presetSectSizer->childParent(),
        ID_MovePresetDown,
        L"\u2193" /*down arrow*/,
        wxDefaultPosition,
        arrangeButtonSize,
        wxBU_EXACTFIT
    )};
    arrangeButtonsSizer->AddSpacer(20);
    arrangeButtonsSizer->Add(movePresetUp);
    arrangeButtonsSizer->Add(movePresetDown);
    
    presetListSizer->Add(presetList, wxSizerFlags(1).Expand());
    presetListSizer->AddSpacer(5);
    presetListSizer->Add(arrangeButtonsSizer);

    auto *presetButtonSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *addPreset {new wxButton(
        presetSectSizer->childParent(),
        ID_AddPreset,
        "+",
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    auto *removePreset{new wxButton(
        presetSectSizer->childParent(),
        ID_RemovePreset,
        "-",
        wxDefaultPosition,
        wxDefaultSize,
        wxBU_EXACTFIT
    )};
    presetButtonSizer->Add(addPreset, wxSizerFlags(1));
    presetButtonSizer->AddSpacer(5);
    presetButtonSizer->Add(removePreset, wxSizerFlags(1));
    presetButtonSizer->AddSpacer(arrangeButtonSize.x + 5);

    arraySectSizer->Add(arraySizer, wxSizerFlags().Expand());
    arraySectSizer->AddSpacer(10);
    arraySectSizer->Add(arrayButtonsSizer, wxSizerFlags().Expand());
    presetSectSizer->Add(presetListSizer, wxSizerFlags(1).Expand());
    presetSectSizer->AddSpacer(5);
    presetSectSizer->Add(presetButtonSizer, wxSizerFlags().Expand());

    presetSelectionSizer->Add(arraySectSizer, wxSizerFlags().Expand());
    presetSelectionSizer->AddSpacer(10);
    presetSelectionSizer->Add(presetSectSizer, wxSizerFlags(1).Expand());

    auto *presetConfigSizer{new wxBoxSizer(wxVERTICAL)};
    presetConfigSizer->SetMinSize(wxSize(200, -1));

    auto *nameInput{new pcui::Text(
        this,
        config.presetArrays.nameProxy,
        wxTE_PROCESS_ENTER,
        true,
        _("Preset Name")
    )};
    nameInput->SetToolTip(_(
        "The name for the preset.\n"
        "This appears on the OLED screen if no bitmap is supplied, otherwise it's just for reference.\n"
        "\"\\n\" means \"enter.\" Hitting \"enter\" will insert \"\\n\" which means a new line in the text displayed on the OLED.\n"
        "For example, \"my\\npreset\" will be displayed on the OLED as two lines, the first being \"my\" and the second being \"preset.\""
    ));

    auto *dirInput{new pcui::Text(
        this,
        config.presetArrays.dirProxy,
        0,
        false,
        _("Font Directory")
    )};
    dirInput->SetToolTip(_(
        "The path of the folder on the SD card where the font is stored.\n"
        "If the font folder is inside another folder, it must be indicated by something like \"folderName/fontFolderName\".\n"
        "In order to specify multiple directories (for example, to include a \"common\" directory), use a semicolon (;) to separate the folders (e.g. \"fontFolderName;common\")."
    ));

    auto *trackSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto *trackInput{new pcui::Text(
        this,
        config.presetArrays.trackProxy,
        0,
        false,
        _("Track File")
    )};
    trackInput->SetToolTip(_(
        "The path of the track file on the SD card. May be empty.\n"
        "If the track is directly inside one of the folders specified in \"Font Directory\" then only the name of the track file is required."
    ));

    auto *wavText{new wxStaticText(
        this,
        ID_WavText,
        ".wav"
    )};
    wavText->Hide();
    trackSizer->Add(trackInput, wxSizerFlags(1));
    trackSizer->Add(wavText, wxSizerFlags().Bottom());

    mInjectionsSizer = new pcui::StaticBox(wxVERTICAL, this, _("Injections"));

    presetConfigSizer->Add(nameInput, wxSizerFlags().Expand());
    presetConfigSizer->AddSpacer(5);
    presetConfigSizer->Add(dirInput, wxSizerFlags().Expand());
    presetConfigSizer->AddSpacer(5);
    presetConfigSizer->Add(trackSizer, wxSizerFlags().Expand());
    presetConfigSizer->AddSpacer(20);
    presetConfigSizer->Add(mInjectionsSizer, wxSizerFlags(1).Expand());

    auto *stylesSizer{new wxBoxSizer(wxVERTICAL)};
    auto *styleDisplay{new pcui::Choice(
        this,
        config.presetArrays.styleDisplay,
        _("Display")
    )};
    styleDisplay->SetToolTip(_("Show blade listing corresponding to the selected blade array."));

    auto *styleList{new pcui::List(
        this,
        config.presetArrays.styleSelectProxy,
        _("Blades")
    )};
    stylesSizer->Add(styleDisplay, wxSizerFlags().Expand());
    stylesSizer->AddSpacer(5);
    stylesSizer->Add(styleList, wxSizerFlags(1).Expand());

    auto *styleCommentSplit{new wxSplitterWindow(
        this,
        wxID_ANY,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_3DSASH | wxSP_LIVE_UPDATE
    )};

    auto *commentInput{new pcui::Text(
        styleCommentSplit,
        config.presetArrays.commentProxy,
        wxTE_MULTILINE | wxNO_BORDER,
        false,
        _("Comments")
    )};
    commentInput->SetToolTip(_(
        "Any comments about the blade style goes here.\n"
        "This doesn't affect the blade style at all, but can be a place for helpful notes!"
    ));

    auto *styleInput{new pcui::Text(
        styleCommentSplit,
        config.presetArrays.styleProxy,
        wxTE_DONTWRAP | wxTE_MULTILINE | wxNO_BORDER,
        false,
        _("Blade Style")
    )};
    styleInput->styleMonospace();
    styleInput->SetToolTip(_(
        "Your blade style goes here.\n"
        "This is the code which sets up what animations and effects your blade (or other LED) will do.\n"
        "For getting/creating blade styles, see the Documentation (in \"Help->Documentation...\")."
    ));

    styleCommentSplit->SetMinSize(wxSize{500, -1});
    styleCommentSplit->SetMinimumPaneSize(60);
    styleCommentSplit->SplitHorizontally(commentInput, styleInput);

    sizer->Add(presetSelectionSizer, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(presetConfigSizer, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(stylesSizer, wxSizerFlags().Expand());
    sizer->AddSpacer(10);
    sizer->Add(styleCommentSplit, wxSizerFlags(1).Expand());

    SetSizerAndFit(sizer);
}

void PresetsPage::bindEvents() {
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& config{mParent->getOpenConfig()};
        auto& presetArray{
            config.presetArrays.array(config.presetArrays.selection)
        };
        presetArray.addPreset();
        const auto lastIdx{
            static_cast<int32>(presetArray.presets().size() - 1)
        };
        presetArray.selection = lastIdx;
    }, ID_AddPreset);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& presetArrays{mParent->getOpenConfig().presetArrays};
        if (presetArrays.selection == -1) return;
        auto& presetArray{presetArrays.array(presetArrays.selection)};
        if (presetArray.selection == -1) return;

        presetArray.removePreset(presetArray.selection);
    }, ID_RemovePreset);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& presetArrays{mParent->getOpenConfig().presetArrays};
        if (presetArrays.selection == -1) return;
        auto& presetArray{presetArrays.array(presetArrays.selection)};
        if (presetArray.selection == -1) return;

        presetArray.movePresetUp(presetArray.selection);
    }, ID_MovePresetUp);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& presetArrays{mParent->getOpenConfig().presetArrays};
        if (presetArrays.selection == -1) return;
        auto& presetArray{presetArrays.array(presetArrays.selection)};
        if (presetArray.selection == -1) return;

        presetArray.movePresetDown(presetArray.selection);
    }, ID_MovePresetDown);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& config{mParent->getOpenConfig()};
        auto& presetArray{config.presetArrays.array(
            config.presetArrays.selection
        )};

        RenameArrayDlg renameDlg(
            mParent,
            config,
            presetArray,
            _("Rename Preset Array")
        );
        renameDlg.ShowModal();
    }, ID_RenameArray);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& config{mParent->getOpenConfig()};

        Config::PresetArray array(config);
        RenameArrayDlg dlg(
            mParent,
            config,
            array,
            _("Add Preset Array"),
            true
        );
        if (wxID_OK == dlg.ShowModal()) {
            config.presetArrays.addArray(array.name);
            const auto lastIdx{static_cast<int32>(
                config.presetArrays.arrays().size() - 1
            )};
            config.presetArrays.selection = lastIdx;
        }

    }, ID_AddArray);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        auto& presetArrays{mParent->getOpenConfig().presetArrays};
        presetArrays.removeArray(presetArrays.selection);
    }, ID_RemoveArray);
}

void PresetsPage::handleNotification(uint32 id) {
    bool rebound{id == pcui::Notifier::eID_Rebound};

    auto& presetArrays{mParent->getOpenConfig().presetArrays};
    if (rebound or id == Config::PresetArrays::NOTIFY_INJECTIONS) {
        rebuildInjections();
    }
    if (rebound or id == Config::PresetArrays::NOTIFY_SELECTION) {
        bool hasSelection{presetArrays.selection != -1};
        FindWindow(ID_RemoveArray)->Enable(hasSelection);
        FindWindow(ID_RenameArray)->Enable(hasSelection);
        FindWindow(ID_AddPreset)->Enable(hasSelection);
    }
    if (
            rebound or
            id == Config::PresetArrays::NOTIFY_ARRAY_NAME or
            id == Config::PresetArrays::NOTIFY_SELECTION
       ) {
        auto *issueButton{FindWindow(ID_IssueButton)};
        // This is late for sizing reasons
        issueButton->SetLabel(L"\u26D4" /* ⛔️ */);

        if (presetArrays.selection == -1) {
            issueButton->Hide();
        } else {
            auto& selectedArray{presetArrays.array(presetArrays.selection)};
            auto selectedArrayName{static_cast<string>(selectedArray.name)};
            bool duplicate{false};
            for (const auto& array : presetArrays.arrays()) {
                if (&*array == &selectedArray) continue;

                if (static_cast<string>(array->name) == selectedArrayName) {
                    duplicate = true;
                    break;
                }
            }
            issueButton->Show(
                static_cast<string>(selectedArray.name).empty() or duplicate
            );
        }
    }
    if (
            rebound or
            id == Config::PresetArrays::NOTIFY_PRESETS or
            id == Config::PresetArrays::NOTIFY_SELECTION
       ) {
        if (presetArrays.selection == -1) {
            FindWindow(ID_MovePresetUp)->Disable();
            FindWindow(ID_MovePresetDown)->Disable();
            FindWindow(ID_RemovePreset)->Disable();
        } else {
            auto& presetArray{presetArrays.array(presetArrays.selection)};
            bool hasPresetSelection{presetArray.selection != -1};

            bool notFirst{presetArray.selection > 0};
            const auto lastIdx{presetArray.selection.choices().size() - 1};
            bool notLast{presetArray.selection < lastIdx};

            auto *movePresetUp{FindWindow(ID_MovePresetUp)};
            movePresetUp->Enable(hasPresetSelection and notFirst);
            auto *movePresetDown{FindWindow(ID_MovePresetDown)};
            movePresetDown->Enable(hasPresetSelection and notLast);
            auto *removePreset{FindWindow(ID_RemovePreset)};
            removePreset->Enable(hasPresetSelection);
        }
    }
    if (
            rebound or
            id == Config::PresetArrays::NOTIFY_TRACK_INPUT or
            id == Config::PresetArrays::NOTIFY_PRESETS or
            id == Config::PresetArrays::NOTIFY_SELECTION
       ) {
        auto hasInput{
            presetArrays.trackProxy.data() and
            not static_cast<string>(*presetArrays.trackProxy.data()).empty()
        };
        FindWindow(ID_WavText)->Show(hasInput);
    }

    Layout();
}

void PresetsPage::rebuildInjections() {
    mInjectionsSizer->Clear(true);

    auto& config{mParent->getOpenConfig()};

    for (const auto& injection : config.presetArrays.injections()) {
        auto *injectionSizer{new wxBoxSizer(wxHORIZONTAL)};
        auto *injectionText{new wxStaticText(
            mInjectionsSizer->childParent(),
            wxID_ANY,
            injection->filename
        )};
        auto *editButton{new wxButton(
            mInjectionsSizer->childParent(),
            wxID_ANY,
            _("Edit"),
            wxDefaultPosition,
            wxDefaultSize,
            wxBU_EXACTFIT
        )};
        auto *deleteButton{new wxButton(
            mInjectionsSizer->childParent(),
            wxID_ANY,
            _("Delete"),
            wxDefaultPosition,
            wxDefaultSize,
            wxBU_EXACTFIT
        )};

        editButton->Bind(wxEVT_BUTTON, [&injection](wxCommandEvent&) {
            wxLaunchDefaultApplication(
                (Paths::injectionDir() / injection->filename).native()
            );
        });

        deleteButton->Bind(wxEVT_BUTTON, [this, &injection](wxCommandEvent&) {
            auto res{pcui::showMessage(
                _("This action cannot be undone!"),
                _("Delete Injection"),
                wxYES_NO | wxNO_DEFAULT
            )};
            if (wxNO == res) return;

            mParent->getOpenConfig().presetArrays.removeInjection(*injection);
        });

        injectionSizer->Add(injectionText, wxSizerFlags(1).Center());
        injectionSizer->AddSpacer(20);
        injectionSizer->Add(editButton);
        injectionSizer->AddSpacer(10);
        injectionSizer->Add(deleteButton);

        mInjectionsSizer->Add(injectionSizer, wxSizerFlags().Expand());
    }

    mInjectionsSizer->Show(not mInjectionsSizer->IsEmpty());
    mInjectionsSizer->Layout();
    Layout();
}

