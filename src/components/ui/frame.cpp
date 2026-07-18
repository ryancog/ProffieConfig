#include "frame.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2026 Ryan Ogurek
 *
 * components/ui/frame.cpp
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

#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/window.h>

#include "ui/build.hpp"
#include "ui/priv/tlw.hpp"

using namespace pcui;

namespace {

} // namespace

Frame::Frame(
    wxWindow *parent,
    wxWindowID id,
    const wxString& title,
    long style
) {
    priv::tlw::bindOnCreate(this);
    priv::tlw::preCreate(this);

    Create(
        parent,
        id,
        title,
        wxDefaultPosition,
        wxDefaultSize,
        style,
        "pcui::Frame"
    );

    priv::tlw::postCreate(this);
}

Frame::~Frame() {
    cripple(this);
    if (mReference and *mReference) (*mReference) = nullptr;
}

void Frame::setReference(Frame** ref) {
    mReference = ref;
}

void Frame::Fit() {
    priv::tlw::fit<wxFrame>(this);
}

void Frame::appendDefaultMenuItems(wxMenuBar *menuBar) {
#   ifdef __WXOSX__
    // TODO: So, for now, I think it's best to not bother with CMD+W to close
    // a window. It doesn't actually work like one might expect when, e.g.,
    // a dialog is opened, and it should go in the "File" type menu, not the
    // window menu, in any case.
    //
    // Even if I want it in the window menu, it requires fiddling with
    // wxWidgets' handling to disable its auto window menu management (which
    // clears all menu items) and call -[NSApplication setWindowsMenu:] at some
    // point, and that can only be called after -[NSApplication setMainMenu:],
    // apparently, otherwise an exception occurs in some cases, at least on
    // older macOS versions (e.g. Ventura), and maybe newer ones, since calling
    // it otherwise seems clearly unintended.
    //
    // Really on macOS there should just be one "master" menu bar, and thus
    // quite a different approach than the very Windows-centric one wxWidgets
    // is biased towards. :/
    //
    // window->Append(wxID_CLOSE, "Close\tCtrl+W");
    // Bind(wxEVT_MENU, &Frame::onWindowMenuClose, this, wxID_CLOSE);

    menuBar->Append(new wxMenu, _("&Window"));
    menuBar->Append(new wxMenu, _("&Help"));
#   endif
}

#ifdef __WXOSX__
void Frame::onWindowMenuClose(wxCommandEvent&) {
    Close();
}
#endif

