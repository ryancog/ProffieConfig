#include "webview.h"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024 Ryan Ogurek
 *
 * stylepreview/webview.cpp
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

#include <wx/gdicmn.h>
#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/webview.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/tglbtn.h>
#include <wx/button.h>

#ifdef __WX_MSW__
#include <wx/msw/webview_ie.h>
#include <wx/msw/webview_edge.h>
#endif

void StyleEditor::StyleWebView::reload(const std::string& newStyle) {
    static std::string style;
    if (!newStyle.empty()) style = newStyle;
    if (view->IsBusy()) return;

    loadedSize = view->GetSize();
    auto height{std::to_string(loadedSize.GetHeight())};
    auto width{std::to_string(loadedSize.GetWidth())};
    auto scaledHeight{std::to_string(loadedSize.GetHeight() * GetDPIScaleFactor())};
    auto scaledWidth{std::to_string(loadedSize.GetWidth() * GetDPIScaleFactor())};
    view->RemoveAllUserScripts();
    view->AddUserScript(R"(document.querySelectorAll('*').forEach(function(element){ element.style.margin = "0"; if (element !== document.getElementById("canvas_id") && !element.contains(document.getElementById("canvas_id"))) element.style.display = "none"; }))", wxWEBVIEW_INJECT_AT_DOCUMENT_END);
    view->AddUserScript(R"(document.body.style.backgroundColor = "#000000")", wxWEBVIEW_INJECT_AT_DOCUMENT_END);
    view->AddUserScript(R"(let canvas = document.getElementById("canvas_id");)", wxWEBVIEW_INJECT_AT_DOCUMENT_END);
    view->AddUserScript("canvas.style.width = (width = " + width + ") + 'px';", wxWEBVIEW_INJECT_AT_DOCUMENT_END);
    view->AddUserScript("canvas.width = " + scaledWidth + ';', wxWEBVIEW_INJECT_AT_DOCUMENT_END);
    view->AddUserScript("canvas.style.height = (height = " + height + ") + 'px';", wxWEBVIEW_INJECT_AT_DOCUMENT_END);
    view->AddUserScript("canvas.height = " + scaledHeight + ';', wxWEBVIEW_INJECT_AT_DOCUMENT_END);
    view->AddUserScript(R"(document.body.style.overflow = "hidden";)", wxWEBVIEW_INJECT_AT_DOCUMENT_END);
    view->AddUserScript(R"(FIND("style").value = ")" + style + R"("; Run();)", wxWEBVIEW_INJECT_AT_DOCUMENT_END);
    static_cast<wxToggleButton*>(FindWindowById(POWER))->SetValue(false);

    view->Reload();
}

StyleEditor::StyleWebView::StyleWebView(wxWindow* parent, int32_t id) :
    wxPanel(parent, id) {
    auto sizer{new wxBoxSizer(wxVERTICAL)};

    auto styleEditPath{"file://" + wxGetCwd() + STYLE_EDITORPATH "?S=Black"};
#   ifdef __WXMSW__
    view = wxWebView::New(this, wxID_ANY, styleEditPath, wxDefaultPosition, wxDefaultSize);
#   else
    view = wxWebView::New(this, wxID_ANY, styleEditPath);
#	endif

    view->EnableHistory(false);
    view->EnableContextMenu(false);
    sizer->Add(view, wxSizerFlags(1).Expand());

    auto buttonSizer{new wxBoxSizer(wxHORIZONTAL)};
    auto powButton = new wxToggleButton(this, POWER, "Power");
    buttonSizer->Add(powButton);

    auto clashButton = new wxButton(this, CLASH, "Clash");
    buttonSizer->Add(clashButton);

    sizer->Add(buttonSizer);

    SetSizerAndFit(sizer);
    reload();

    Bind(wxEVT_SIZE, [this](wxSizeEvent& event){ event.Skip(); reload(); });
    view->Bind(wxEVT_WEBVIEW_LOADED, [this](wxWebViewEvent&) { if (view->GetSize() != loadedSize) reload(); });
    Bind(wxEVT_TOGGLEBUTTON, [this](wxCommandEvent&) { view->RunScriptAsync("ClickPower();"); }, POWER);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { view->RunScriptAsync("AddClash();"); }, CLASH);
}
