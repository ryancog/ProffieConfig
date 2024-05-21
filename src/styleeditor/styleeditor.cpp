#include "styleeditor.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * styleeditor/styleeditor.cpp
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

#include <iostream>

#include <wx/sizer.h>
#include <wx/wrapsizer.h>
#include <wx/splitter.h>
#include <wx/gdicmn.h>
#include <wx/srchctrl.h>
#include <wx/event.h>
#include <wx/tglbtn.h>
#include <wx/scrolwin.h>
#include <wx/time.h>
#include <wx/timer.h>
#include <wx/clipbrd.h>
#include <wx/window.h>

#include "styleeditor/blocks/styleblock.h"
#include "styles/bladestyle.h"
#include "styles/elements/colorstyles.h"
#include "styles/elements/functions.h"
#include "styles/elements/transitions.h"
#include "styles/elements/effects.h"
#include "styles/parse.h"
#include "ui/frame.h"
#include "stylepreview/webview.h"
#include "appcore/interfaces.h"
#include "ui/movable.h"
#include "wx/dataobj.h"

static PCUI::Frame* frame{nullptr};
static StyleEditor::StyleWebView* preview{nullptr};

static void createUI();
static void createToolbox(wxPanel*, PCUI::MoveArea*);
static void bindEvents();
static void updateVisibleBlocks();
static void updateToolboxSize();
static uint32_t filterType{BladeStyles::COLOR};

static wxToggleButton* styleFilter;
static wxToggleButton* layerFilter;
static wxToggleButton* funcFilter;
static wxToggleButton* transitionFilter;
static wxToggleButton* effectFilter;
static std::vector<PCUI::Block*>* toolboxBlocks;
static wxScrolledCanvas* toolbox{nullptr};
static PCUI::MoveArea* blockMoveArea{nullptr};
static PCUI::ScrolledMovePanel* blockWorkspace{nullptr};

static constexpr auto jogDistance{20};
static constexpr auto scrollUnits{jogDistance / 2};

void StyleEditor::launch(wxWindow* parent) {
    if (frame) {
        frame->Raise();
        return;
    }

    frame = new PCUI::Frame(parent, AppCore::Interface::STYLEMAN, "ProffieConfig Style Editor");
    frame->setReference(reinterpret_cast<PCUI::Frame**>(&frame));
    createUI();
    updateVisibleBlocks();
    bindEvents();

    frame->Show();
    frame->SetSize(600, 300);
}

void createUI() {
    auto frameSizer{new wxBoxSizer(wxVERTICAL)};
    blockMoveArea = new PCUI::MoveArea(frame, wxID_ANY);
    frameSizer->Add(blockMoveArea, wxSizerFlags(1).Expand());
    auto moveAreaSizer{new wxBoxSizer(wxVERTICAL)};
    auto splitter{new wxSplitterWindow(blockMoveArea, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_PERMIT_UNSPLIT)};
    moveAreaSizer->Add(splitter, wxSizerFlags(1).Expand());
    splitter->SetMinimumPaneSize(220);

    auto toolboxPanel{new wxPanel(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER, "Toolbox")};
    createToolbox(toolboxPanel, blockMoveArea);

    auto workspaceSplitter{new wxSplitterWindow(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE)};
    workspaceSplitter->SetMinimumPaneSize(100);
    preview = new StyleEditor::StyleWebView(workspaceSplitter, wxID_ANY);

    blockWorkspace = new PCUI::ScrolledMovePanel(workspaceSplitter, blockMoveArea, wxID_ANY);
    workspaceSplitter->SplitHorizontally(preview, blockWorkspace->getCanvas());
    splitter->SplitVertically(toolboxPanel, workspaceSplitter);

    blockMoveArea->SetSizerAndFit(moveAreaSizer);
    frame->SetSizerAndFit(frameSizer);
}

void createToolbox(wxPanel* toolboxPanel, PCUI::MoveArea* moveArea) {
    auto padSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto sizer{new wxBoxSizer(wxVERTICAL)};
    padSizer->AddSpacer(5);
    padSizer->Add(sizer, wxSizerFlags(1).Expand());
    padSizer->AddSpacer(5);

    auto blockSearch{new wxSearchCtrl(toolboxPanel, wxID_ANY)};
    sizer->Add(blockSearch, wxSizerFlags(0).Expand().Border(wxTOP | wxBOTTOM, 5));

    auto categorySizer{new wxWrapSizer(wxHORIZONTAL)};
    styleFilter = new wxToggleButton(toolboxPanel, BladeStyles::COLOR, "Styles");
    styleFilter->SetValue(true);
    layerFilter = new wxToggleButton(toolboxPanel, BladeStyles::LAYER, "Layers");
    funcFilter = new wxToggleButton(toolboxPanel, BladeStyles::FUNCTION, "Functions");
    transitionFilter = new wxToggleButton(toolboxPanel, BladeStyles::TRANSITION, "Transitions");
    effectFilter = new wxToggleButton(toolboxPanel, BladeStyles::EFFECT, "Effects");
    categorySizer->Add(styleFilter, wxSizerFlags(0).Border(wxALL, 2));
    categorySizer->Add(layerFilter, wxSizerFlags(0).Border(wxALL, 2));
    categorySizer->Add(funcFilter, wxSizerFlags(0).Border(wxALL, 2));
    categorySizer->Add(transitionFilter, wxSizerFlags(0).Border(wxALL, 2));
    categorySizer->Add(effectFilter, wxSizerFlags(0).Border(wxALL, 2));

    sizer->Add(categorySizer);
    sizer->AddSpacer(10);

    toolbox = new wxScrolledCanvas(toolboxPanel, wxID_ANY);
    auto scrollSizer{new wxBoxSizer(wxVERTICAL)};
    toolboxBlocks = new std::vector<PCUI::Block*>;

    auto addStyleBlock{[&moveArea, &scrollSizer](const BladeStyles::StyleGenerator& generator) {
        auto style{generator(nullptr, {})};
        auto block{new PCUI::StyleBlock(nullptr, toolbox, style)};
        delete style;

        toolboxBlocks->push_back(block);
        block->collapse();
        block->Bind(PCUI::SB_COLLAPSED, [](wxCommandEvent&){ updateToolboxSize(); });
        block->Bind(wxEVT_LEFT_DOWN, [block, moveArea](wxMouseEvent& evt){
            evt.Skip(false);
            if (!block->hitTest(evt.GetPosition())) return;

            auto style{block->getStyle()};
            auto newBlock{new PCUI::StyleBlock(moveArea, toolbox, style)};
            delete style;

            newBlock->collapse(block->isCollapsed());
            newBlock->SetPosition(block->GetPosition());
            newBlock->doGrab(evt);
        });
        for (const auto& child : block->GetChildren()) {
            // Prevent moving children blocks
            child->Bind(wxEVT_LEFT_DOWN, [](wxMouseEvent& evt) { evt.Skip(false); });
            child->Disable();
        }
        scrollSizer->Add(block, wxSizerFlags(0).Border(wxALL, 5));
    }};

    for (const auto& [ _, generator ] : BladeStyles::ColorStyle::getMap()) addStyleBlock(generator);
    for (const auto& [ _, generator ] : BladeStyles::FunctionStyle::getMap()) addStyleBlock(generator);
    for (const auto& [ _, generator ] : BladeStyles::TransitionStyle::getMap()) addStyleBlock(generator);

    toolbox->SetSizerAndFit(scrollSizer);
    toolbox->SetMinSize({200, 200});
    sizer->Add(toolbox, wxSizerFlags(1).Expand());

    toolboxPanel->SetSizerAndFit(padSizer);
}

void updateToolboxSize() {
    toolbox->GetSizer()->Layout();
    toolbox->SetVirtualSize(toolbox->GetBestVirtualSize());
    auto virtSize{toolbox->GetVirtualSize()};
    static constexpr auto scrollUnits{15};
    toolbox->SetScrollbars(scrollUnits, scrollUnits, virtSize.x / scrollUnits, virtSize.y / scrollUnits);
}

void updateVisibleBlocks() {
    for (const auto block : *toolboxBlocks) {
        auto shown{(block->type & BladeStyles::FLAGMASK) & filterType};
        block->Show(shown);
    }
    updateToolboxSize();
}

void bindEvents() {
    frame->Bind(wxEVT_CLOSE_WINDOW, [](wxCloseEvent& evt) {
        evt.Skip();
        if (toolboxBlocks) {
            delete toolboxBlocks;
            toolboxBlocks = nullptr;
        }
    });
    toolbox->Bind(wxEVT_TOGGLEBUTTON, [](wxCommandEvent& evt) {
        auto button{static_cast<wxToggleButton*>(evt.GetEventObject())};
        if (!button->GetValue()) {
            button->SetValue(true);
            return;
        }

#       define FILTER(filter, type) if (button != filter) filter->SetValue(false); else filterType = type
        FILTER(styleFilter, BladeStyles::COLOR);
        FILTER(layerFilter, BladeStyles::LAYER);
        FILTER(funcFilter, BladeStyles::FUNCTION);
        FILTER(transitionFilter, BladeStyles::TRANSITION);
        FILTER(effectFilter, BladeStyles::EFFECT);
#       undef FITLER

        updateVisibleBlocks();
    });
    frame->Bind(wxEVT_TEXT_PASTE, [&](wxClipboardTextEvent&) {
        wxTextDataObject textData;
        if (!wxClipboard::Get()->Open()) return;
        wxClipboard::Get()->GetData(textData);
        wxClipboard::Get()->Close();

        auto styleStr{textData.GetText().ToStdString()};
        auto style{BladeStyles::parseString(styleStr)};
        if (!style) return;

        auto block{new PCUI::StyleBlock(blockMoveArea, blockWorkspace, style)};
        block->SetPosition(blockWorkspace->ScreenToClient(wxGetMousePosition()));
    });
}

// StyleEditor::StyleWebView* StyleEditor::getPreview() { return preview; }

