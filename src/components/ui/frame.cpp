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

#ifdef __WXOSX__
#include <wx/osx/menu.h>
#include <objc/objc-runtime.h>
#endif

#include "ui/build.hpp"
#include "ui/priv/tlw.hpp"

using namespace pcui;

namespace {

#ifdef __WXOSX__
/*
 * A lot of the Objective-C runtime is very much written in assembly, it's not
 * convenient to use in C, much less C++, but this works well enough.
 */
template <typename Ret = void, typename ...Args>
Ret objcMessage(id self, cstring op, Args&&... args) {
    using Signature = Ret (*)(id, SEL, Args...);
    auto *func{reinterpret_cast<Signature>(objc_msgSend)};
    return func(self, sel_registerName(op), std::forward<Args>(args)...);
}
#endif

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
    // This causes wx to remove all the menu items, insert some new ones, and
    // then call -[NSApplication setWindowsMenu:]. I want to add items to the
    // Window menu though. I don't fully understand the problem wx was trying
    // to solve removing all the menu items (it claims they might be duplicated
    // in some cases otherwise), but anyways...
    wxMenuBar::SetAutoWindowMenu(false);

    // Still use the wxMenu stuff, can grab the NSMenu from there.
    auto window{new wxMenu};

    // Can't use the NSApp global, dynamically fetch it with
    // +[NSApplication sharedApplication]
    Class NSApplication{objc_getClass("NSApplication")};
    id NSApp{objcMessage<id>((id)NSApplication, "sharedApplication")};
    // [NSApp setWindowsMenu:] does special handling to add the default menu
    // items. wxWidgets manually adds these, but ignore that for now.
    // "Minimize" (CMD+M) : -[NSWindow performMiniaturize:]
    // "Zoom" (none) : -[NSWindow performZoom:]
    // "Bring All to Front" (none) : -[NSApplication arrangInFront:]
    objcMessage(NSApp, "setWindowsMenu:", window->GetHMenu());

    // And now can treat the menu like normal.
    window->Append(wxID_CLOSE, "Close\tCtrl+W");
    Bind(wxEVT_MENU, &Frame::onWindowMenuClose, this, wxID_CLOSE);

    menuBar->Append(window, _("&Window"));
    menuBar->Append(new wxMenu, _("&Help"));
#   endif
}

void Frame::onWindowMenuClose(wxCommandEvent&) {
    Close();
}

